/*
 * Driver for the pro300 screen and keyboard.
 * There are a variety of capabilities here:
 *	- default is emulation of a DEC VT52 terminal
 *	- also a variety of ioctl's that permit raster
 *	screen graphics
 *	See the file <sys/cnio.h> for more info.
 *	on the screen ioctl's
 * The font is a fixed 96 char. ascii definition with the
 * keyboard mapped into the US. ascii key caps. The only
 * unusual keys are "compose" which generates alternate
 * Xon/Xoff like a VT100 scroll key and the curser arrows,
 * which generate VT52 sequences.
 * Lots more could be done here, user loadable fonts, redefinition
 * of keys on the keyboard, ioctl's to generate keyboard control
 * codes...
 * NOTE: Extended bit map code has NOT been tested.
 * There are a few constants that may be of interest:
 * XMAX - chars/line
 * YMAX - lines/screen
 * FNTLINE - raster lines for a font char.
 * LNPEROW - raster lines for each char. line (LNPEROW >= FNTLINE)
 * FNTWIDTH - width of bit field that char. is displayed in
 * The interrupt service routines are;
 *	kbrint - keyboard receive
 *	kbxint - keyboard transmit
 *	sctint - screen transfer interrupt, used for xfer completions
 *	scfint - screen end of frame interrupts, used to prod output process
 */
#include "cn.h"
#if NCN > 0
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/systm.h>
#include <sys/seg.h>
#include <sys/buf.h>
#include <sys/ivecpos.h>
#include <sys/cnreg.h>
#include <sys/cnio.h>

extern struct scdevice *SCADDR;
extern struct kbdevice *KBADDR;
struct	tty cn11;

/* The font is an 8x10 bit matrix stored by row in top->bottom
 * order. The low order bit is the leftmost bit when displayed.
 */
char profont[][10] = {
	0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
	0020,0020,0020,0020,0020,0000,0000,0020,0020,0000,
	0044,0044,0044,0000,0000,0000,0000,0000,0000,0000,
	0050,0050,0050,0376,0050,0376,0050,0050,0050,0000,
	0020,0374,0022,0022,0174,0220,0220,0176,0020,0000,
	0016,0212,0116,0040,0020,0010,0344,0242,0340,0000,
	0034,0042,0042,0024,0010,0224,0142,0142,0234,0000,
	0060,0060,0020,0010,0000,0000,0000,0000,0000,0000,
	0040,0020,0010,0010,0010,0010,0010,0020,0040,0000,
	0010,0020,0040,0040,0040,0040,0040,0020,0010,0000,
	0000,0020,0222,0124,0070,0124,0222,0020,0000,0000,
	0000,0020,0020,0020,0376,0020,0020,0020,0000,0000,
	0000,0000,0000,0000,0000,0000,0030,0030,0010,0004,
	0000,0000,0000,0000,0174,0000,0000,0000,0000,0000,
	0000,0000,0000,0000,0000,0000,0000,0030,0030,0000,
	0000,0200,0100,0040,0020,0010,0004,0002,0000,0000,
	0174,0202,0302,0242,0222,0212,0206,0202,0174,0000,
	0020,0030,0024,0020,0020,0020,0020,0020,0174,0000,
	0174,0202,0200,0100,0070,0004,0002,0002,0376,0000,
	0174,0202,0200,0200,0170,0200,0200,0202,0174,0000,
	0100,0140,0120,0110,0104,0102,0376,0100,0100,0000,
	0376,0002,0002,0076,0100,0200,0200,0102,0074,0000,
	0170,0004,0002,0002,0176,0202,0202,0202,0174,0000,
	0376,0202,0100,0040,0020,0010,0010,0010,0010,0000,
	0174,0202,0202,0202,0174,0202,0202,0202,0174,0000,
	0174,0202,0202,0202,0374,0200,0200,0100,0074,0000,
	0000,0000,0000,0030,0030,0000,0000,0030,0030,0000,
	0000,0000,0030,0030,0000,0000,0030,0030,0010,0004,
	0040,0020,0010,0004,0002,0004,0010,0020,0040,0000,
	0000,0000,0000,0174,0000,0174,0000,0000,0000,0000,
	0010,0020,0040,0100,0200,0100,0040,0020,0010,0000,
	0170,0204,0204,0200,0140,0020,0020,0000,0020,0000,
	0170,0204,0262,0252,0252,0172,0002,0004,0170,0000,
	0070,0104,0202,0202,0202,0376,0202,0202,0202,0000,
	0176,0204,0204,0204,0174,0204,0204,0204,0176,0000,
	0170,0204,0002,0002,0002,0002,0002,0204,0174,0000,
	0076,0104,0204,0204,0204,0204,0204,0104,0076,0000,
	0376,0002,0002,0002,0036,0002,0002,0002,0376,0000,
	0376,0002,0002,0002,0036,0002,0002,0002,0002,0000,
	0170,0204,0002,0002,0002,0362,0202,0204,0174,0000,
	0202,0202,0202,0202,0376,0202,0202,0202,0202,0000,
	0174,0020,0020,0020,0020,0020,0020,0020,0174,0000,
	0370,0040,0040,0040,0040,0040,0040,0042,0034,0000,
	0202,0102,0042,0022,0012,0026,0042,0102,0202,0000,
	0002,0002,0002,0002,0002,0002,0002,0002,0376,0000,
	0202,0306,0252,0222,0222,0202,0202,0202,0202,0000,
	0202,0206,0212,0222,0242,0302,0202,0202,0202,0000,
	0070,0104,0202,0202,0202,0202,0202,0104,0070,0000,
	0176,0202,0202,0202,0176,0002,0002,0002,0002,0000,
	0070,0104,0202,0202,0202,0222,0242,0104,0270,0000,
	0176,0202,0202,0202,0176,0022,0042,0102,0202,0000,
	0174,0202,0002,0002,0174,0200,0200,0202,0174,0000,
	0376,0020,0020,0020,0020,0020,0020,0020,0020,0000,
	0202,0202,0202,0202,0202,0202,0202,0202,0174,0000,
	0202,0202,0202,0104,0104,0050,0050,0020,0020,0000,
	0202,0202,0202,0202,0222,0222,0252,0306,0202,0000,
	0202,0202,0104,0050,0020,0050,0104,0202,0202,0000,
	0202,0202,0104,0050,0020,0020,0020,0020,0020,0000,
	0376,0200,0100,0040,0020,0010,0004,0002,0376,0000,
	0074,0004,0004,0004,0004,0004,0004,0004,0074,0000,
	0000,0002,0004,0010,0020,0040,0100,0200,0000,0000,
	0074,0040,0040,0040,0040,0040,0040,0040,0074,0000,
	0020,0050,0104,0202,0000,0000,0000,0000,0000,0000,
	0000,0000,0000,0000,0000,0000,0000,0000,0376,0000,
	0030,0030,0020,0040,0000,0000,0000,0000,0000,0000,
	0000,0000,0000,0074,0100,0174,0102,0102,0274,0000,
	0002,0002,0002,0072,0106,0102,0102,0106,0072,0000,
	0000,0000,0000,0074,0102,0002,0002,0102,0074,0000,
	0100,0100,0100,0134,0142,0102,0102,0142,0134,0000,
	0000,0000,0000,0074,0102,0176,0002,0002,0074,0000,
	0060,0110,0010,0010,0076,0010,0010,0010,0010,0000,
	0000,0000,0000,0134,0142,0142,0134,0100,0100,0074,
	0002,0002,0002,0072,0106,0102,0102,0102,0102,0000,
	0000,0020,0000,0030,0020,0020,0020,0020,0070,0000,
	0140,0100,0100,0100,0100,0100,0100,0104,0070,0000,
	0002,0002,0002,0042,0022,0012,0026,0042,0102,0000,
	0030,0020,0020,0020,0020,0020,0020,0020,0070,0000,
	0000,0000,0000,0156,0222,0222,0222,0222,0222,0000,
	0000,0000,0000,0072,0106,0102,0102,0102,0102,0000,
	0000,0000,0000,0074,0102,0102,0102,0102,0074,0000,
	0000,0000,0000,0072,0106,0102,0106,0072,0002,0002,
	0000,0000,0000,0134,0142,0102,0142,0134,0100,0100,
	0000,0000,0000,0072,0106,0002,0002,0002,0002,0000,
	0000,0000,0000,0074,0102,0014,0060,0102,0074,0000,
	0000,0010,0010,0076,0010,0010,0010,0110,0060,0000,
	0000,0000,0000,0102,0102,0102,0102,0142,0134,0000,
	0000,0000,0000,0202,0202,0202,0104,0050,0020,0000,
	0000,0000,0000,0202,0222,0222,0222,0222,0154,0000,
	0000,0000,0000,0102,0044,0030,0030,0044,0102,0000,
	0000,0000,0000,0102,0102,0102,0142,0134,0100,0074,
	0000,0000,0000,0176,0040,0020,0010,0004,0176,0000,
	0160,0010,0010,0010,0004,0010,0010,0010,0160,0000,
	0020,0020,0020,0000,0000,0020,0020,0020,0000,0000,
	0030,0040,0040,0040,0100,0040,0040,0040,0030,0000,
	0014,0222,0140,0000,0000,0000,0000,0000,0000,0000,
	0044,0222,0110,0044,0222,0110,0044,0222,0110,0000,
};
/* This vector maps key codes to ascii equivalents. Characters with funny
 * shift values like )9 are referenced indirectly in the second table by
 * 0200 + (pos in secnd table). Most unused keys are given 07 (bell) or 0.
 */
char prokbmap[] = {
	07,	07,	07,	026,	022,	07,	07,	07,
	07,	07,	07,	07,	07,	07,	027,	031,
	05,	025,	04,	07,	07,	07,	07,	07,
	07,	07,	07,
	033,	010,	012,	07,	07,	07,	07,	07,
	07,	07,	07,	07,	015,	07,	07,	07,
	07,	07,	07,	07,	07,	07,	07,	07,
	07,	0167,	0151,	0170,	033,	02,	06,	07,
	07,	060,	07,	056,	015,	061,	062,	063,
	064,	065,	066,	054,	067,	070,	071,	055,
	07,	07,	07,	07,	07,	07,	07,	07,
	07,	07,	07,	07,	07,	07,	07,	07,
	07,	07,	07,	07,	07,	07,	0,	0,
	0,	0,	07,	0177,	015,	011,	0200,	0201,
	0161,	0141,	0172,	07,	0202,	0167,	0163,	0170,
	0203,	07,	0204,	0145,	0144,	0143,	07,	0205,
	0162,	0146,	0166,	040,	07,	0206,	0164,	0147,
	0142,	07,	0207,	0171,	0150,	0156,	07,	0210,
	0165,	0152,	0155,	07,	0211,	0151,	0153,	054,
	07,	0212,	0157,	0154,	056,	07,	0213,	0160,
	07,	0214,	0215,	07,	0216,	0217,	0220,	07,
	0221,	0222,	0223,	07,	07,	07,	07,
};
/* Defines char. pairs for funny shift caps like 9)
 */
proupmap[][2] = {
	0176,	0140,
	041,	061,
	0100,	062,
	076,	074,
	043,	063,
	044,	064,
	045,	065,
	0136,	066,
	046,	067,
	052,	070,
	050,	071,
	051,	060,
	072,	073,
	077,	057,
	053,	075,
	0175,	0135,
	0174,	0134,
	0137,	055,
	0173,	0133,
	042,	047,
};

char kblastc, kbmode, backtodo, backlog, scstate;
unsigned int kbbell, scxpos, scypos, scflash, scflags, scoffset, ot_flags;

int	cnstart();
int	sctimer();
int	ttrstrt();

cnattach(addr, unit)
struct scdevice *addr;
{
	if ((unsigned)unit >= NCN)
		return(0);
	SCADDR = addr;
	return(1);
}

cnopen(dev, flag)
dev_t dev;
{
	register struct tty *tp;
	int s;

	if (minor(dev) >= NCN) {
		u.u_error = EINVAL;
		return;
	}
	if (SCADDR == (struct scdevice *)NULL ||
		KBADDR == (struct kbdevice *)NULL) {
		u.u_error = ENXIO;
		return;
	}
	tp = &cn11;
	tp->t_oproc = cnstart;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_flags = ODDP|EVENP|ECHO|CRMOD;
		tp->t_ispeed = tp->t_ospeed = B9600;
		tp->t_line = DFLT_LDISC;
		ttychars(tp);
	} else if (tp->t_state&XCLUDE && u.u_uid != 0) {
		u.u_error = EBUSY;
		return;
	}
	/* Initialize screen and keyboard port */
	s = spl4();
	if ((scflags & SCSET) == 0) {
		SCADDR->mbr = 0170;
		SCADDR->p1c = 041;
		if ((SCADDR->scsr & 020000) == 0)
			SCADDR->opc = 020441;
		SCADDR->scsr = 040100;
		scflags |= SCSET;
	}
	/* Start timer, used to generate blinking curser. */
	if ((scflags & TIMER) == 0) {
		scflags |= TIMER;
		sctimer();
	}
	scflags |= CURSER;
	splx(s);
	ienable(IVEC(SCADDR, APOS));
	ienable(IVEC(SCADDR, BPOS));
	KBADDR->csr = GOCMD;
	ienable(KBRPOS);
	ienable(KBXPOS);
	ttyopen(dev, tp);
}

cnclose(dev, flag)
dev_t dev;
int flag;
{

	ttyclose(&cn11);
}

cnread(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &cn11;
	(*linesw[tp->t_line].l_read)(tp);
}

/* If PLOT flag set, put raster data directly to the screen, else
 * map onto ascii font.
 */
cnwrite(dev)
dev_t dev;
{
	register struct tty *tp;
	unsigned size;
	char *ptr;
	long tmp;
	segm save;

	if (scflags & PLOT) {
		saveseg5(save);
		scoffset = (scypos<<7)+(scxpos>>3);
		while (u.u_count) {
			size = 0100000-scoffset;
			size = MIN(size, 8192);
			size = MIN(size, u.u_count);
			tmp = scoffset;
			ptr = BITMAP|(tmp & 077);
			tmp = (tmp>>6)&0777;
			mapseg5((BITBASE|tmp), 077406);
			iomove(ptr, size, B_WRITE);
			if (u.u_error) {
				restorseg5(save);
				return;
			}
			scoffset = (scoffset+size)&077777;
		}
		restorseg5(save);
		scxpos = (scoffset & 0177) << 3;
		scypos = scoffset>>7;
		return;
	}
	tp = &cn11;
	(*linesw[tp->t_line].l_write)(tp);
}

kbrint(dev)
dev_t dev;
{
	register int c;
	register struct tty *tp;

	if ((KBADDR->stat & RXDONE) == 0)
		return;
	tp = &cn11;
	c = KBADDR->dbuf;
	if (KBADDR->stat & RXERR)
		KBADDR->csr = GOCMD;
	while ((c = promapkb(c&0377)) != 0) {
		(*linesw[tp->t_line].l_input)(c, tp);
		c = 0;
	}
}

kbxint(dev)
dev_t dev;
{
	if ((KBADDR->stat & TXDONE) == 0)
		return;
	if ((--kbbell) > 0)
		KBADDR->dbuf = 0xa7;
	else
		kbbell = 0;
}

cnioctl(dev, cmd, addr, flag)
caddr_t addr;
dev_t dev;
int cmd, flag;
{
	register struct tty *tp = &cn11;
	register int tmp;
	int s;

	switch(cmd) {
	case SIOCPLOT:
		if ((scflags & PLOT) == 0) {
			s = spl4();
			ot_flags = tp->t_flags;
			tp->t_flags &= ~ECHO;
			scflags |= PLOT;
			scxpos = scypos = 0;
			scflags &= ~CURSER;
			if (scflags & CURSON) {
				scurser();
				scflags |= XFERSLP;
				while (scflags & XFERSLP)
					sleep((caddr_t)&scflags, TTOPRI);
			}
			scleary(0, 256);
			if ((tmp = fuword(addr)) >= 0) {
				SCADDR->scsr = (tmp&02)|((tmp&01)<<10)|040100;
				tmp = (tmp&014)>>2;
				SCADDR->p1c = (tmp<<3)|041;
				if ((SCADDR->scsr & 020000) == 0)
					SCADDR->opc = (tmp<<3)|(tmp<<11)|0401;
			}
			splx(s);
		}
		break;
	case SIOCPRNT:
		if (scflags & PLOT) {
			s = spl4();
			tp->t_flags = ot_flags;
			if ((SCADDR->scsr & XFERDONE) == 0) {
				scflags |= XFERSLP;
				while (scflags & XFERSLP)
					sleep((caddr_t)&scflags, TTOPRI);
			}
			scxpos = scypos = 0;
			SCADDR->p1c = 041;
			if ((SCADDR->scsr & 020000) == 0)
				SCADDR->opc = 020441;
			SCADDR->scsr = 040100;
			scleary(0, 256);
			scflags |= CURSER;
			scflags &= ~PLOT;
			splx(s);
		}
		break;
	case SIOCCSP:
		if (scflags & PLOT) {
			s = spl4();
			if (scflags & CURSON) {
				scurser();
				scflags |= XFERSLP;
				while (scflags & XFERSLP)
					sleep((caddr_t)&scflags, TTOPRI);
			}
			scxpos = fuword(addr)&01777;
			scypos = fuword(++addr)&0377;
			splx(s);
		}
		break;
	case SIOCCSN:
		s = spl4();
		scflags |= CURSER;
		splx(s);
		break;
	case SIOCCSF:
		s = spl4();
		scflags &= ~CURSER;
		if (scflags & CURSON) {
			scurser();
			scflags |= XFERSLP;
			while (scflags & XFERSLP)
				sleep((caddr_t)&scflags, TTOPRI);
		}
		splx(s);
		break;
	case SIOCSCRL:
		s = spl4();
		if ((SCADDR->scsr & XFERDONE) == 0) {
			scflags |= XFERSLP;
			while (scflags & XFERSLP)
				sleep((caddr_t)&scflags, TTOPRI);
		}
		tmp = SCADDR->scroll&0377;
		SCADDR->scroll = tmp+fuword(addr);
		splx(s);
		break;
	case SIOCGRPH:
		if ((scflags & PLOT) == 0) {
			u.u_error = ENOTTY;
			return;
		}
		s = spl4();
		while ((SCADDR->scsr & XFERDONE) == 0) ;
		SCADDR->p1c = (SCADDR->p1c&070)|02;
		if ((SCADDR->scsr & 020000) == 0)
			SCADDR->opc = (SCADDR->opc&034070)|01002;
		SCADDR->pat = fuword(addr);
		SCADDR->x = fuword(++addr);
		SCADDR->y = fuword(++addr);
		SCADDR->cnt = fuword(++addr);
		scflags |= XFERSLP;
		while (scflags & XFERSLP)
			sleep((caddr_t)&scflags, TTOPRI);
		SCADDR->p1c = (SCADDR->p1c&070)|01;
		if ((SCADDR->scsr & 020000) == 0)
			SCADDR->opc = (SCADDR->opc&034070)|0401;
		splx(s);
		break;
	case SIOCPLEN:
		if ((scflags & PLOT) == 0) {
			u.u_error = ENOTTY;
			return;
		}
		s = spl4();
		tmp = fuword(addr) & 07;
		SCADDR->p1c = (SCADDR->p1c&037)|((tmp&01)<<5);
		if ((SCADDR->scsr & 020000) == 0)
			SCADDR->opc = (SCADDR->opc&017437)|((tmp&02)<<4)|
				((tmp&04)<<11);
		splx(s);
		break;
	case SIOCSCM:
		if ((scflags & PLOT) == 0) {
			u.u_error = ENOTTY;
			return;
		}
		s = spl4();
		SCADDR->cmp = fuword(addr) & 03777;
		splx(s);
		break;
	default:
		switch (ttioctl(tp, cmd, addr, flag)) {
		case TIOCSETN:
		case TIOCSETP:
		case 0:
			break;
		default:
			u.u_error = ENOTTY;
		}
	};
}

cnstart(tp)
register struct tty *tp;
{
	register c;

	 while ((c=getc(&tp->t_outq)) >= 0) {
		proputc(c&0177);
	}
}

char	*msgbufp = msgbuf;	/* Next saved printf character */
/*
 * Print a character on console.
 *
 * Whether or not printing is inhibited,
 * the last MSGBUFS characters
 * are saved in msgbuf for inspection later.
 */
putchar(c)
register c;
{
	int s;

	if (c != '\0' && c != '\r' && c != 0177) {
		*msgbufp++ = c;
		if(msgbufp >= &msgbuf[MSGBUFS])
			msgbufp = msgbuf;
	}
	if(c == 0)
		return;
	/* Do not print if screen in plot mode. */
	s = spl4();
	if ((scflags & PLOT) == 0) {
		if ((scflags & SCSET) == 0) {
			SCADDR->mbr = 0170;
			SCADDR->p1c = 041;
			if ((SCADDR->scsr & 020000) == 0)
				SCADDR->opc = 020441;
			SCADDR->scsr = 040100;
			scleary(0, 256);
			scflags |= SCSET;
		}
		proputc(c&0177);
	}
	splx(s);
	if(c == '\n') {
		putchar('\r');
	}
}

/* This function displays a charcter on the bit mapped screen using
 * a vt52 emulation of control sequences.
 */
proputc(c)
char c;
{
	register int val, i;

	/* Turn off the blinking!! curser */
	scflags &= ~CURSER;
	if (scflags & CURSON)
		scurser();

	/* Now update the screen as required by the next char. */
	if (scstate)
		checkesc(c);
	else if (c >= 040 && c <= 0177)
		scputc(c);
	else
		switch(c) {
		case 011:
			val = 8-(scxpos%8);
			for (i = 0; i < val; i++)
				scputc(' ');
			break;
		case 010:
			if (scxpos > 0)
				scxpos--;
			break;
		case 012:
			if (scypos >= YMAX)
				scroll(LNPEROW);
			else
				scypos++;
			break;
		case 015:
			scxpos = 0;
			break;
		case 033:
			scstate = GETESC;
			break;
		case 07:
			kbring();
			break;
		default:
			;
		};
	/* Done so turn the curser back on. */
	scflags |= CURSER;
}

/* This function validates and performs a vt52 escape sequence. */
checkesc(c)
char c;
{
	register int val;

	switch(scstate) {
	case GETX:
		val = c-' ';
		if (val >= 0 && val <= XMAX)
			scxpos = val;
		scstate = ESCDONE;
		break;
	case GETY:
		val = c-' ';
		if (val >= 0 && val <= YMAX)
			scypos = val;
		scstate = GETX;
		break;
	default:
		scstate = ESCDONE;
		switch(c) {
		case 'A':
			if (scypos > 0)
				scypos--;
			break;
		case 'B':
			if (scypos < YMAX)
				scypos++;
			break;
		case 'C':
			if (scxpos < XMAX)
				scxpos++;
			break;
		case 'D':
			if (scxpos > 0)
				scxpos--;
			break;
		case 'F':
		case 'G':
			break;
		case 'H':
			scypos = scxpos = 0;
			break;
		case 'I':
			if (scypos > 0)
				scypos--;
			else
				scroll(-LNPEROW);
			break;
		case 'J':
			scleary(scypos, (YMAX-scypos)*LNPEROW);
			break;
		case 'K':
			sclearx();
			break;
		case 'Y':
			scstate = GETY;
			break;
		default:
			scputc(c);
		};
	};
}

/* This function copies the font for one char. onto the bitmap
 * at the current position.
 */
scputc(c)
char c;
{
	register char *ptr, *ptr2;
	register int i;
	unsigned int tmp2;
	union {
		unsigned int word;
		char byte[2];
	} tmp3;
	segm save;
	long tmp;

	ptr = &profont[c-040][0];
	tmp = scypos*1280l+((scxpos*FNTWIDTH)>>3);
	tmp2 = (scxpos*FNTWIDTH)&07;
	ptr2 = BITMAP|(tmp&077);
	tmp = (tmp>>6)&0777;
	saveseg5(save);
	mapseg5((BITBASE|tmp), 077406);
	for (i = 0; i < FNTLINE; i++) {
		if (tmp2) {
			tmp3.byte[0] = *ptr2;
			tmp3.byte[1] = *(ptr2+1);
			tmp3.word &= ~(0377<<(8-tmp2));
			tmp3.word |= ((unsigned)(*ptr++))<<(8-tmp2);
			*ptr2 = tmp3.byte[0];
			*(ptr2+1) = tmp3.byte[1];
		} else {
			*ptr2 = *ptr++;
		}
		ptr2 += 128;
	}
	restorseg5(save);
	if (scxpos < XMAX) {
		scxpos++;
	}
}

/* This function clears the current line from the curser to the end. */
sclearx() {
	register int i, j;
	register char *ptr2;
	char *ptr;
	int tmp2, tmp3;
	long tmp;
	segm save;

	tmp = scypos*1280l+((scxpos*FNTWIDTH)>>3);
	tmp2 = (scxpos*FNTWIDTH)&07;
	ptr = BITMAP|(tmp&077);
	tmp = (tmp>>6)&0777;
	saveseg5(save);
	mapseg5((BITBASE|tmp), 077406);
	for (i = 0; i < FNTLINE; i++) {
		ptr2 = ptr;
		if (tmp2) {
			tmp3 = *ptr2;
			tmp3 &= ~(0377<<(8-tmp2));
			*ptr2++ = tmp3;
		}
		for (j = 0; j < ((1024-scxpos*FNTWIDTH)>>3); j++)
			*ptr2++ = 0;
		ptr += 128;
	}
	restorseg5(save);
}

/* This function clears the screen from the y pos. given for lines
 * raster lines.
 */
scleary(ypos, lines)
int ypos, lines;
{
	register int i, j, *ptr;
	segm save;

	saveseg5(save);
	for (i = 0; i < lines; i++) {
		ptr = (int *)BITMAP;
		mapseg5((BITBASE|(((128l*ypos)>>6)&0777)), 077406);
		for (j = 0; j < 64; j++)
			*ptr++ = 0;
		ypos++;
	}
	restorseg5(save);
}

/* This function scrolls the screen up/down the num. of raster lines
 * given.
 */
scroll(lines)
int lines;
{
	register int i, tmp;

	if (lines > 0) {
		for (i = 0; i < lines; i++) {
			scleary(240, 1);
			tmp = SCADDR->scroll & 0377;
			SCADDR->scroll = ++tmp;
		}
	} else {
		for (i = 0; i > lines; i--) {
			scleary(255, 1);
			tmp = SCADDR->scroll & 0377;
			SCADDR->scroll = --tmp;
		}
	}
}

/* This function toggles the blinking curser on/off each call. */
scurser()
{

	/* Toggle the curser on/off flag and then toggle the curser */
	if (scflags & CURSON)
		scflags &= ~CURSON;
	else
		scflags |= CURSON;
	while ((SCADDR->scsr & XFERDONE) == 0) ;
	SCADDR->scsr &= 0176377;
	SCADDR->p1c = (SCADDR->p1c&070)|01;
	if ((SCADDR->scsr & 020000) == 0)
		SCADDR->opc = (SCADDR->opc&034070)|0401;
	SCADDR->pat = 0177777;
	if (scflags & PLOT) {
		SCADDR->x = scxpos+1;
		SCADDR->y = scypos+1;
		SCADDR->cnt = 3;
	} else {
		SCADDR->x = scxpos*FNTWIDTH;
		SCADDR->y = scypos*LNPEROW+LNPEROW-1;
		SCADDR->cnt = FNTWIDTH;
	}
}

/* This timer routine blinks the curser on/off as required. */
unsigned sctimcnt;
sctimer()
{

	timeout(sctimer, (caddr_t)0, hz/5);
	sctimcnt++;
	if (scflags & CURSER) {
		if (SCADDR->scsr & XFERDONE) {
			scurser();
		} else {
			scflash++;
		}
	}
}

sctint()
{

	if (scflash && (scflags & CURSER)) {
		scurser();
		scflash = 0;
		return;
	}
	if (scflags & XFERSLP) {
		scflags &= ~XFERSLP;
		wakeup((caddr_t)&scflags);
	}
}

/* End of frame interrupts are used to prod. the output process.
 * Of historical interest, the original driver synchronized updates
 * with end of frames, but this made things too slow.
 */
scfint(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = &cn11;
	ttstart(tp);
	if (tp->t_state&ASLEEP && tp->t_outq.c_cc<=TTLOWAT(tp))
#ifdef MPX_FILS
		if (tp->t_chan)
			mcstart(tp->t_chan, (caddr_t)&tp->t_outq);
		else
#endif
			wakeup((caddr_t)&tp->t_outq);
	if ((scflags & XFERSLP) && (SCADDR->scsr & XFERDONE)) {
		scflags &= ~XFERSLP;
		wakeup((caddr_t)&scflags);
	}
}

/* This function maps the kb codes to appropriate ascii codes.
 * This is specific to the US keyboard layout.
 */
promapkb(c)
int c;
{
	if (c == 0) {
		if (backtodo) {
			backtodo = 0;
			return(backlog);
		} else {
			return(0);
		}
	}
	if (c == MTRNOME) {
		if (kblastc == ESC)
			backtodo = 1;
		return(kblastc);
	}
	switch(c) {
	case KSHIFT:
		if (kbmode & SHIFT)
			kbmode &= ~SHIFT;
		else
			kbmode |= SHIFT;
		return(0);
	case KCNTL:
		if (kbmode & CNTL)
			kbmode &= ~CNTL;
		else
			kbmode |= CNTL;
		return(0);
	case KLOCK:
		if (kbmode & LOCK)
			kbmode &= ~LOCK;
		else
			kbmode |= LOCK;
		return(0);
	case ALLUPS:
		kbmode &= ~(CNTL|SHIFT);
		return(0);
	case UP:
		backlog = 0101;
		backtodo = 1;
		kblastc = ESC;
		return(ESC);
	case DOWN:
		backlog = 0102;
		backtodo = 1;
		kblastc = ESC;
		return(ESC);
	case RIGHT:
		backlog = 0103;
		backtodo = 1;
		kblastc = ESC;
		return(ESC);
	case LEFT:
		backlog = 0104;
		backtodo = 1;
		kblastc = ESC;
		return(ESC);
	case COMPOSE:
		if (kbmode & STOPD) {
			kbmode &= ~STOPD;
			return(021);
		} else {
			kbmode |= STOPD;
			return(023);
		}
	case KPRNT:
		cnioctl(0, SIOCPRNT, 0, 0);
		return(0);
	default:
		if (c < 86)
			return(0);
		c = prokbmap[c-86]&0377;
		if (c & 0200) {
			if (kbmode & SHIFT)
				c = proupmap[c-0200][0]&0177;
			else
				c = proupmap[c-0200][1]&0177;
		} else if (c >= 'a' && c <= 'z') {
			if (kbmode & (SHIFT|LOCK))
				c -= 040;
			else if (kbmode & CNTL)
				c -= 0140;
		}
		kblastc = c;
		return(c);
	};
}

/* This function rings the kb bell. If busy, just let kbxint know... */
kbring()
{
	kbbell++;
	if (KBADDR->stat & TXDONE)
		KBADDR->dbuf = 0xa7;
}
#endif
