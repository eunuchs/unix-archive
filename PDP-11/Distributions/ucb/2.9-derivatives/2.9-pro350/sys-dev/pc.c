/*
 * This driver handles the two serial ports on the back of the
 * pro3xx system unit. Although not software compatible, they
 * are handled as minor device 0 & 1 respectively, for the printer
 * and communication port. Modem control is included but no sync
 * serial support for the com. port.
 * NOTE: The DSR line in the printer port is used for carrier
 * detect so terminals or modems should be cabled accordingly.
 * Local terminal cables should jumper DTR-CDT so that the carrier
 * will appear to be up or PC_SOFTCAR defined and devs or'd with 0200.
 * NOTE2: The interrupt service routines are as follows:
 *	plrint - printer port receive
 *	plxint - printer port transmit
 *	cmintr - communication port com. interrupt
 * Modem transition interrupts are NOT used.
 */

#include "pc.h"
#if NPC > 0
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/ivecpos.h>
#include <sys/pcreg.h>

#ifdef PC_SOFTCAR
#define PCLINE(d)	(minor(d) & 0177)
#else
#define PCLINE		minor
#endif

extern struct pcdevice *PCADDR;
int	npctty	= NPC;		/* Only for pstat */
struct	tty	pc11[NPC];
char	pc_stat;
int	pcstart();

char	pc_speeds[] = {
	0, 0, 1, 2, 3, 4, 5, 5, 6, 7, 8, 10,
	12, 14, 15, 15,
};

pcattach(addr, unit)
struct pcdevice *addr;
{
	if ((unsigned)unit < NPC) {
		PCADDR = addr;
		return(1);
	}
	return(0);
}

pcopen(dev, flag) {
	register struct tty *tp;
	register int d;
	int s;

	d = PCLINE(dev);
	if (d >= NPC) {
		u.u_error = EINVAL;
		return;
	}
	if (PCADDR == (struct pcdevice *)NULL) {
		u.u_error = ENXIO;
		return;
	}
	tp = &pc11[d];
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		tp->t_oproc = pcstart;
		tp->t_ispeed = B9600;
		tp->t_ospeed = B9600;
		tp->t_flags = ODDP|EVENP|ECHO|CRMOD;
		tp->t_line = DFLT_LDISC;
		ttychars(tp);
		pcparam(d);
	} else if (tp->t_state&XCLUDE && u.u_uid != 0) {
		u.u_error = EBUSY;
		return;
	}
	pcmodem(d, ON);
#ifdef PC_SOFTCAR
	if (dev & 0200)
		tp->t_state |= CARR_ON;
	else
#endif
	{
	(void) _spl4();
	while ((tp->t_state&CARR_ON)==0) {
		tp->t_state |= WOPEN;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	(void) _spl0();
	}
	tp->t_state = (tp->t_state & ~WOPEN)|ISOPEN;
	ttyopen(dev, tp);
}

pcclose(dev)
dev_t dev;
{
	register struct tty *tp;
	register d;

	d = PCLINE(dev);
	tp = &pc11[d];
	if (tp->t_state&HUPCLS)
		pcmodem(d, OFF);
	ttyclose(tp);
}

pcread(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &pc11[PCLINE(dev)];
	(*linesw[tp->t_line].l_read)(tp);
}

pcwrite(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &pc11[PCLINE(dev)];
	(*linesw[tp->t_line].l_write)(tp);
}

pcioctl(dev, cmd, addr, flag)
dev_t	dev;
int	cmd;
caddr_t	addr;
int	flag;
{
	register struct tty *tp;
	register int d;

	d = PCLINE(dev);
	switch(ttioctl(&pc11[d], cmd, addr, flag)) {
	case TIOCSETN:
	case TIOCSETP:
		pcparam(d);
	case 0:
		break;
	default:
		u.u_error = ENOTTY;
	}
}

pcparam(dev)
int dev;
{
	register struct tty *tp;
	register lpr, flag;

	tp = &pc11[dev];
	if (pc_stat==0) {
		pcscan();
		pc_stat++;
	}
	if (tp->t_ispeed==0) {	/* Hang up line */
		pcmodem(dev, OFF);
		return;
	}
	if (dev == 0) {
		lpr = PCADDR->csr;
		if (tp->t_flags&RAW)
			lpr = BITS8;
		else
			lpr = BITS7|PENABLE;
		if (tp->t_flags&EVENP)
			lpr |= EPAR;
		if (tp->t_ispeed == 3)	/* 110 baud */
			lpr |= TWOSB;
		PCADDR->mode = lpr;
		PCADDR->mode = pc_speeds[tp->t_ispeed]|DIVR;
		((physadr *) 0173700)->r[0] = 0200;
		ienable(PLRPOS);
		ienable(PLXPOS);
	} else {
		PCADDR->baud = (pc_speeds[tp->t_ospeed]<<4)|
			pc_speeds[tp->t_ispeed];
		PCADDR->csa = 1;
		PCADDR->csa = 022;
		PCADDR->csb = 1;
		PCADDR->csb = 04;
		PCADDR->csb = 2;
		PCADDR->csb = 0;
		if (tp->t_flags&RAW) {
			flag = 03;
			lpr = 0104;
		} else {
			flag = 01;
			lpr = 0105;
		}
		if (tp->t_flags&EVENP)
			lpr |= 02;
		if (tp->t_ispeed == 3)
			lpr |= 010;
		PCADDR->csa = 4;
		PCADDR->csa = lpr;
		PCADDR->csa = 3;
		PCADDR->csa = (flag<<6)|01;
		PCADDR->csa = 5;
		PCADDR->csa = (flag<<5)|010;
		ienable(CMPOS);
	}
}

pcrint(dev)
int dev;
{
	register struct tty *tp;
	register c, tmp;
	int tmp1;

	if (dev == 0) {
		tmp = PCADDR->stat;
		if (tmp & RXDONE) {
			tp = pc11;
			c = PCADDR->dbuf;
			PCADDR->csr = GOCMD;
			if((tp->t_state&ISOPEN)==0) {
				wakeup((caddr_t)&tp->t_rawq);
				return;
			}
			if (tmp&FRERROR) {		/* break */
				if (tp->t_flags&RAW)
					c = 0;		/* null (for getty) */
				else
					c = tun.t_intrc;
			} else if (tmp&PERROR) {
				if ((tp->t_flags&(EVENP|ODDP))==EVENP
				 || (tp->t_flags&(EVENP|ODDP))==ODDP )
					return;
			}
			(*linesw[tp->t_line].l_input)(c, tp);
		}
	} else {
		tmp1 = PCADDR->csa;
		PCADDR->csa = 1;
		tmp = PCADDR->csa;
		PCADDR->csa = ERESET;
		if (tmp1 & RXCA) {
			tp = &pc11[1];
			c = PCADDR->cdb;
			if ((tp->t_state&ISOPEN)==0) {
				wakeup((caddr_t)&tp->t_rawq);
				return;
			}
			if (tmp&CFERROR) {
				if (tp->t_flags&RAW)
					c = 0;
				else
					c = tun.t_intrc;
			} else if (tmp&CPERROR) {
				if ((tp->t_flags&(EVENP|ODDP))==EVENP
				 || (tp->t_flags&(EVENP|ODDP))==ODDP )
					return;
			}
			(*linesw[tp->t_line].l_input) (c, tp);
		}
	}
}

pcxint(dev)
int dev;
{
	register struct tty *tp;

	tp = &pc11[dev];
	ttstart(tp);
	if (tp->t_state&ASLEEP && tp->t_outq.c_cc<=TTLOWAT(tp))
#ifdef MPX_FIL
		if (tp->t_chan)
			mcstart(tp->t_chan, (caddr_t)&tp->t_outq);
		else
#endif
			wakeup((caddr_t)&tp->t_outq);
}

pcstart(tp)
register struct tty *tp;
{
	register unit, c;
	extern ttrstrt();

	unit = tp - pc11;
	if (unit == 0) {
		if ((PCADDR->stat & TXDONE)==0)
			return;
	} else {
		if ((PCADDR->csa & CTXDONE)==0)
			return;
	}
	if ((c = getc(&tp->t_outq)) >= 0) {
		if ((tp->t_flags & RAW) == 0 && c >= 0200) {
			tp->t_state |= TIMEOUT;
			timeout(ttrstrt, (caddr_t)tp, (c&0177)+PCDELAY);
		} else {
			if (unit == 0)
				PCADDR->dbuf = c;
			else
				PCADDR->cdb = c;
		}
	} else {
		if (unit == 1)
			PCADDR->csa = RTINTP;
	}
}

pcmodem(dev, flag)
int dev, flag;
{

	if (dev == 0) {
		if (flag==OFF)
			PCADDR->csr = 0;
		else
			PCADDR->csr = GOCMD;
	} else {
		if (flag==OFF)
			PCADDR->mc0 = 0;
		else
			PCADDR->mc0 = 030;
	}
}

pcscan()
{
	register i, flag;
	register struct tty *tp;

	for (i=0; i<NPC; i++) {
		if (i == 0) {
			flag = PCADDR->stat & DSRON;
		} else {
			flag = PCADDR->mc1 & CARDTC;
		}
		tp = &pc11[i];
		if (flag) {
			if ((tp->t_state&CARR_ON)==0) {
				wakeup((caddr_t)&tp->t_rawq);
				tp->t_state |= CARR_ON;
			}
		} else {
			if ((tp->t_state&CARR_ON)
#ifdef PC_SOFTCAR
			   && ((tp->t_dev & 0200) == 0)
#endif
#ifdef UCB_NTTY
			    && ((tp->t_local&LNOHANG)==0)
#endif
			) {
				if (tp->t_state&ISOPEN) {
					gsignal(tp->t_pgrp, SIGHUP);
					if (i == 0) {
						PCADDR->csr = 0;
					} else {
						PCADDR->mc0 = 0;
					}
					flushtty(tp, FREAD|FWRITE);
				}
				tp->t_state &= ~CARR_ON;
			}
		}
	}
	timeout(pcscan, (caddr_t)0, 120);
}

cmintr(dev)
int dev;
{
	register int tmp;

	do {
		PCADDR->csb = 2;
		tmp = (PCADDR->csb>>2)&03;
		switch (tmp) {
		case TRAN:
			pcxint(1);
			break;
		case XTERN:
			break;
		case RCV:
		case SRCV:
			pcrint(1);
		};
		PCADDR->csa = EOFINTR;
	} while (PCADDR->csa & INTRP);
	PCADDR->csa = REXINTR;
}
#endif
