#include "ra.h"
#if NRA > 0
/*
 * Ms disk driver for UDA50/RDQX1 controllers.
 * Notes: The macro SWAPW(a) interchanges the words of a long. This is
 * necessary since the controllers expect the low order word of a long
 * to be first. Ok for a vax, but not for the pdp11. The type "shorts"
 * is also used to simplify reference to the 32 bit fields for the
 * controller that require the low order word first.
 * This driver has not been tested on a UDA50, but should work since
 * it was written using UDA50 doc. It works fine with the RQDX1 on the
 * Qbus. The only area that may be a problem is with the UNIBUS_MAP, but
 * I think the code is Ok.
 *
 * Restrictions:
 *	Unit numbers must be less than 8.
 *
 * TO DO:
 *	write dump code
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#ifndef INTRLVE
#include <sys/inline.h>
#endif
#include <sys/map.h>
#include <sys/vm.h>

/* This value is adequate for the RQDX1 but should be increased to 4 or
 * 5 for the UDA50. This is because the credit limit is 4 for the RQDX1
 * and about 22 for the current UDA50 rev.
 * Note: changing the number of packets is only for performance enhancement.
 */
#define	NRSPL2	2		/* log2 number of response packets */
#define	NCMDL2	2		/* log2 number of command packets */
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)
#include <sys/rareg.h>

/* This should probably be in seg.h */
#ifdef	KERN_NONSEP
#ifndef	   ENABLE34
#define	      KDSA0	((u_short *) 0172340)
#else
#define       KDSA0	((u_short *) 0163700)
#endif
#else
#ifndef    ENABLE34
#define	      KDSA0	((u_short *) 0172360)
#else
#define       KDSA0	((u_short *) 0163760)
#endif
#endif

struct ra_softc {
	short	sc_state;	/* state of controller */
	short	sc_mapped;	/* Unibus map allocated for ra struct? */
	short	sc_ivec;	/* interrupt vector address */
	short	sc_credits;	/* transfer credits */
	short	sc_lastcmd;	/* pointer into command ring */
	short	sc_lastrsp;	/* pointer into response ring */
	short	sc_onlin;	/* flags for drives online */
} ra_softc;

/*
 * Controller states
 */
#define	S_IDLE	0		/* hasn't been initialized */
#define	S_STEP1	1		/* doing step 1 init */
#define	S_STEP2	2		/* doing step 2 init */
#define	S_STEP3	3		/* doing step 3 init */
#define	S_SCHAR	4		/* doing "set controller characteristics" */
#define	S_RUN	5		/* running */

struct ra {
	struct raca	ra_ca;		/* communications area */
	struct ms	ra_rsp[NRSP];	/* response packets */
	struct ms	ra_cmd[NCMD];	/* command packets */
} ra;

int	raerr = 0;			/* causes hex dump of packets */

daddr_t	radsize[NRA];			/* disk size, from ONLINE end packet */
struct	ms *ragetcp();
extern struct size ra_sizes[];
extern struct radevice *RAADDR;

#ifdef UCB_DBUFS
struct	buf rrabuf[NRA];
#else
struct	buf rrabuf;
#endif
struct	buf ratab;
struct	buf rautab[NRA];
struct	buf rawtab;			/* I/O wait queue, per controller */
struct	buf ramap;

#define	b_qsize		b_resid		/* queue size per drive, in rautab */

void raroot()
{
	raattach(RAADDR, 0);
}

raattach(addr, unit)
struct radevice *addr;
{
	if (unit == 0) {
		RAADDR = addr;
		return(1);
	}
	return(0);
}

/*
 * Open an RA.  Initialize the device and
 * set the unit online.
 */
raopen(dev)
	dev_t dev;
{
	register int unit;
	register struct ra_softc *sc;

	unit = minor(dev) >> 3;
	if (unit >= NRA || (RAADDR == (struct radevice *)NULL)) {
		u.u_error = ENXIO;
		return;
	}
	sc = &ra_softc;
	(void) _spl4();
	if (sc->sc_state != S_RUN) {
		if (sc->sc_state == S_IDLE)
			rainit();
		/* wait for initialization to complete */
		sleep((caddr_t)sc, PZERO+1);
		if (sc->sc_state != S_RUN) {
			u.u_error = EIO;
			return;
		}
	}
	/* If the unit is not ONLINE, do it */
	if ((sc->sc_onlin & (1<<unit)) == 0) {
		int i;
		struct buf *dp;
		struct ms *mp;
		dp = &rautab[unit];
		if ((sc->sc_credits < 2) || ((mp = ragetcp()) == NULL)) {
			u.u_error = EIO;
			return;
		}
		sc->sc_credits--;
		mp->ms_opcode = M_OP_ONLIN;
		mp->ms_unit = unit;
		dp->b_active = 2;
		ratab.b_active++;
		mp->ms_dscptr->high |= RA_OWN|RA_INT;
		i = RAADDR->raip;
		sleep((caddr_t)sc, PZERO+1);
		if ((sc->sc_onlin & (1<<unit)) == 0||sc->sc_state != S_RUN) {
			u.u_error = EIO;
			return;
		}
	}
	(void) _spl0();
}

/*
 * Initialize an RA.  Set up UB mapping registers,
 * initialize data structures, and start hardware
 * initialization sequence.
 */
rainit()
{
	register struct ra_softc *sc;
	register struct ra *rap;

	sc = &ra_softc;
	sc->sc_ivec = RA_IVEC;
	ratab.b_active++;
	rap = &ra;
	/* Get physical addr. of comm. area and map to Unibus Virtual as
	 * required. This is only done once after boot.
	 */
	if (sc->sc_mapped == 0) {
		long addr;
		/* Convert Kernel virtual to physical. Not too sure if this
		 * code is OK for an ENABLE34.
		 * Currently essentially a noop for 2.9 since kernel virtual
		 * equals physical data addresses, but just in case this
		 * changes?
		 */
		addr = (long) KDSA0[(((unsigned)rap)>>13)&07];
		addr <<= 6;
		addr += (long) (((unsigned) rap) & 017777);
		ramap.b_un.b_addr = loint(addr);
		ramap.b_xmem = hiint(addr);
#ifdef UNIBUS_MAP
		/* Map physical to Unibus virtual via. mapalloc() as req. */
		ramap.b_flags = B_PHYS;
		ramap.b_bcount = (sizeof)ra;
		mapalloc(&ramap);
#endif
		sc->sc_mapped = 1;
	}

	/*
	 * Start the hardware initialization sequence.
	 */
	RAADDR->raip = 0;		/* start initialization */
	while ((RAADDR->rasa & RA_STEP1) == 0)
		;
	RAADDR->rasa = RA_ERR|(NCMDL2<<11)|(NRSPL2<<8)|RA_IE|(sc->sc_ivec/4);
	/*
	 * Initialization continues in interrupt routine.
	 */
	sc->sc_state = S_STEP1;
	sc->sc_credits = 0;
}

rastrategy(bp)
	register struct buf *bp;
{
	register struct buf *dp;
	register int unit;
	int xunit = minor(bp->b_dev) & 07;
	daddr_t sz, maxsz;

	sz = (bp->b_bcount+511) >> 9;
	unit = dkunit(bp);
	if (unit >= NRA)
		goto bad;
	if ((maxsz = ra_sizes[xunit].nblocks) < 0)
		maxsz = radsize[unit] - ra_sizes[xunit].cyloff*RA_CYL;
	if (bp->b_blkno < 0 || dkblock(bp)+sz > maxsz)
		goto bad;
	dp = &rautab[unit];
	(void) _spl4();
	/*
	 * Link the buffer onto the drive queue
	 */
	if (dp->b_actf == 0)
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	bp->av_forw = 0;
	/*
	 * Link the drive onto the controller queue
	 */
	if (dp->b_active == 0) {
		dp->b_forw = NULL;
		if (ratab.b_actf == NULL)
			ratab.b_actf = dp;
		else
			ratab.b_actl->b_forw = dp;
		ratab.b_actl = dp;
		dp->b_active = 1;
	}
	if (ratab.b_active == 0) {
		(void) rastart();
	}
	(void) _spl0();
	return;

bad:
	bp->b_error = EINVAL;
	bp->b_flags |= B_ERROR;
	iodone(bp);
	return;
}

rastart()
{
	register struct buf *bp, *dp;
	register struct ms *mp;
	register struct ra_softc *sc;
	int i;

	sc = &ra_softc;
	
loop:
	if ((dp = ratab.b_actf) == NULL) {
		ratab.b_active = 0;
		return (0);
	}
	if ((bp = dp->b_actf) == NULL) {
		/*
		 * No more requests for this drive, remove
		 * from controller queue and look at next drive.
		 * We know we're at the head of the controller queue.
		 */
		dp->b_active = 0;
		ratab.b_actf = dp->b_forw;
		goto loop;
	}
	ratab.b_active++;
	if ((RAADDR->rasa&RA_ERR) || sc->sc_state != S_RUN) {
		harderr(bp, "ra");
		printf("rasa %o stat %d\n", RAADDR->rasa, sc->sc_state);
		rainit();
		/* SHOULD REQUEUE OUTSTANDING REQUESTS */
		return (0);
	}
	/*
	 * If no credits, can't issue any commands
	 * until some outstanding commands complete.
	 */
	if (sc->sc_credits < 2)
		return (0);
	if ((mp = ragetcp()) == NULL)
		return (0);
	sc->sc_credits--;	/* committed to issuing a command */
	/* If drive not ONLINE, issue an online com. */
	if ((sc->sc_onlin & (1<<dkunit(bp))) == 0) {
		mp->ms_opcode = M_OP_ONLIN;
		mp->ms_unit = dkunit(bp);
		dp->b_active = 2;
		ratab.b_actf = dp->b_forw;	/* remove from controller q */
		mp->ms_dscptr->high |= RA_OWN|RA_INT;
		i = RAADDR->raip;
		goto loop;
	}
#ifdef UNIBUS_MAP
	mapalloc(bp);
#endif
	mp->ms_cmdref.low = (u_short)bp;	/* pointer to get back */
	mp->ms_opcode = bp->b_flags&B_READ ? M_OP_READ : M_OP_WRITE;
	mp->ms_unit = dkunit(bp);
	mp->ms_lbn = SWAPW(dkblock(bp)+ra_sizes[minor(bp->b_dev)&7].cyloff*RA_CYL);
	mp->ms_bytecnt = SWAPW(bp->b_bcount);
	mp->ms_buffer.high = bp->b_xmem;
	mp->ms_buffer.low = bp->b_un.b_addr;
	mp->ms_dscptr->high |= RA_OWN|RA_INT;
	i = RAADDR->raip;		/* initiate polling */
#ifdef RA_DKN
	dk_busy |= 1<<RA_DKN;
	dp->b_qsize++;
	dk_xfer[RA_DKN]++;
#endif

	/*
	 * Move drive to the end of the controller queue
	 */
	if (dp->b_forw != NULL) {
		ratab.b_actf = dp->b_forw;
		ratab.b_actl->b_forw = dp;
		ratab.b_actl = dp;
		dp->b_forw = NULL;
	}
	/*
	 * Move buffer to I/O wait queue
	 */
	dp->b_actf = bp->av_forw;
	dp = &rawtab;
	bp->av_forw = dp;
	bp->av_back = dp->av_back;
	dp->av_back->av_forw = bp;
	dp->av_back = bp;
	goto loop;
}

/*
 * RA interrupt routine.
 */
raintr()
{
	struct buf *bp;
	register int i;
	register struct ra_softc *sc = &ra_softc;
	register struct ra *rap = &ra;
	struct ms *mp;
	long addr, uvaddr();

	switch (sc->sc_state) {
	case S_IDLE:
		printf("ra0: randm intr\n");
		return;

	case S_STEP1:
#define	STEP1MASK	0174377
#define	STEP1GOOD	(RA_STEP2|RA_IE|(NCMDL2<<3)|NRSPL2)
		if ((RAADDR->rasa&STEP1MASK) != STEP1GOOD) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)sc);
			return;
		}
		addr = uvaddr(&ra.ra_ca.ca_ringbase);
		RAADDR->rasa = loint(addr);
		sc->sc_state = S_STEP2;
		return;

	case S_STEP2:
#define	STEP2MASK	0174377
#define	STEP2GOOD	(RA_STEP3|RA_IE|(sc->sc_ivec/4))
		if ((RAADDR->rasa&STEP2MASK) != STEP2GOOD) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)sc);
			return;
		}
		RAADDR->rasa = hiint(addr);
		sc->sc_state = S_STEP3;
		return;

	case S_STEP3:
#define	STEP3MASK	0174000
#define	STEP3GOOD	RA_STEP4
		if ((RAADDR->rasa&STEP3MASK) != STEP3GOOD) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)sc);
			return;
		}
		RAADDR->rasa = RA_GO;
		sc->sc_state = S_SCHAR;

		/*
		 * Initialize the data structures.
		 */
		for (i = 0; i < NRSP; i++) {
			addr = uvaddr(&rap->ra_rsp[i].ms_cmdref);
			rap->ra_ca.ca_rspdsc[i].low = loint(addr);
			rap->ra_ca.ca_rspdsc[i].high = RA_OWN|RA_INT|
				hiint(addr);
			rap->ra_rsp[i].ms_dscptr = &rap->ra_ca.ca_rspdsc[i];
			rap->ra_rsp[i].ms_header.ra_msglen = sizeof (struct ms);
		}
		for (i = 0; i < NCMD; i++) {
			addr = uvaddr(&rap->ra_cmd[i].ms_cmdref);
			rap->ra_ca.ca_cmddsc[i].low = loint(addr);
			rap->ra_ca.ca_cmddsc[i].high = RA_INT|
				hiint(addr);
			rap->ra_cmd[i].ms_dscptr = &rap->ra_ca.ca_cmddsc[i];
			rap->ra_cmd[i].ms_header.ra_msglen = sizeof (struct ms);
		}
		bp = &rawtab;
		bp->av_forw = bp->av_back = bp;
		sc->sc_lastcmd = 0;
		sc->sc_lastrsp = 0;
		if ((mp = ragetcp()) == NULL) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)sc);
			return;
		}
		mp->ms_opcode = M_OP_STCON;
		mp->ms_cntflgs = M_CF_ATTN|M_CF_MISC|M_CF_THIS;
		mp->ms_dscptr->high |= RA_OWN|RA_INT;
		i = RAADDR->raip;	/* initiate polling */
		return;

	case S_SCHAR:
	case S_RUN:
		break;

	default:
		printf("ra0: intr in stat %d\n",
			sc->sc_state);
		return;
	}

	if (RAADDR->rasa&RA_ERR) {
		printf("ra0: fatal err (%o)\n", RAADDR->rasa);
		RAADDR->raip = 0;
		wakeup((caddr_t)sc);
	}

	/*
	 * Check for response ring transition.
	 */
	if (rap->ra_ca.ca_rspint) {
		rap->ra_ca.ca_rspint = 0;
		for (i = sc->sc_lastrsp;; i++) {
			i %= NRSP;
			if (rap->ra_ca.ca_rspdsc[i].high&RA_OWN)
				break;
			rarsp(rap, sc, i);
			rap->ra_ca.ca_rspdsc[i].high |= RA_OWN;
		}
		sc->sc_lastrsp = i;
	}

	/*
	 * Check for command ring transition.
	 */
	if (rap->ra_ca.ca_cmdint) {
		rap->ra_ca.ca_cmdint = 0;
	}
	(void) rastart();
}

/*
 * Process a response packet
 */
rarsp(ra, sc, i)
	register struct ra *ra;
	register struct ra_softc *sc;
	int i;
{
	register struct ms *mp;
	struct buf *dp, *bp;
	int st;

	mp = &ra->ra_rsp[i];
	mp->ms_header.ra_msglen = sizeof (struct ms);
	sc->sc_credits += mp->ms_header.ra_credits & 0xf;
	if ((mp->ms_header.ra_credits & 0xf0) > 0x10)
		return;
	/*
	 * If it's an error log message (datagram),
	 * pass it on for more extensive processing.
	 */
	if ((mp->ms_header.ra_credits & 0xf0) == 0x10) {
		raerror((struct ml *)mp);
		return;
	}
	if (mp->ms_unit >= 8)
		return;
	st = mp->ms_status&M_ST_MASK;
	switch ((mp->ms_opcode)&0377) {
	case M_OP_STCON|M_OP_END:
		if (st == M_ST_SUCC)
			sc->sc_state = S_RUN;
		else
			sc->sc_state = S_IDLE;
		ratab.b_active = 0;
		wakeup((caddr_t)sc);
		break;

	case M_OP_ONLIN|M_OP_END:
		/*
		 * Link the drive onto the controller queue
		 */
		dp = &rautab[mp->ms_unit];
		dp->b_forw = NULL;
		if (ratab.b_actf == NULL)
			ratab.b_actf = dp;
		else
			ratab.b_actl->b_forw = dp;
		ratab.b_actl = dp;
		if (st == M_ST_SUCC) {
			sc->sc_onlin |= (1<<mp->ms_unit);
			radsize[mp->ms_unit] = (((long)(mp->ms_unt2))<<16)|
				(((long)(mp->ms_unt1))&0xffff);
		} else {
			harderr(dp->b_actf, "ra");
			printf("OFFLINE\n");
			while (bp = dp->b_actf) {
				dp->b_actf = bp->av_forw;
				bp->b_flags |= B_ERROR;
				iodone(bp);
			}
		}
		dp->b_active = 1;
		wakeup((caddr_t)sc);
		break;

	case M_OP_AVATN:
		sc->sc_onlin &= ~(1<<mp->ms_unit);
		break;

	case M_OP_READ|M_OP_END:
	case M_OP_WRITE|M_OP_END:
		bp = (struct buf *)mp->ms_cmdref.low;
		/*
		 * Unlink buffer from I/O wait queue.
		 */
		bp->av_back->av_forw = bp->av_forw;
		bp->av_forw->av_back = bp->av_back;
		dp = &rautab[mp->ms_unit];
#ifdef RA_DKN
		if (--dp->b_qsize == 0) {
			dk_busy &= ~(1<<RA_DKN);
		}
#endif RA_DKN
		if (st == M_ST_OFFLN || st == M_ST_AVLBL) {
			sc->sc_onlin &= ~(1<<mp->ms_unit);
			/*
			 * Link the buffer onto the front of the drive queue
			 */
			if ((bp->av_forw = dp->b_actf) == 0)
				dp->b_actl = bp;
			dp->b_actf = bp;
			/*
			 * Link the drive onto the controller queue
			 */
			if (dp->b_active == 0) {
				dp->b_forw = NULL;
				if (ratab.b_actf == NULL)
					ratab.b_actf = dp;
				else
					ratab.b_actl->b_forw = dp;
				ratab.b_actl = dp;
				dp->b_active = 1;
			}
			return;
		}
		if (st != M_ST_SUCC) {
			harderr(bp, "ra");
			printf("stat %o\n", mp->ms_status);
			bp->b_flags |= B_ERROR;
		}
		bp->b_resid = bp->b_bcount - ((u_short)SWAPW(mp->ms_bytecnt));
		iodone(bp);
		break;

	case M_OP_GTUNT|M_OP_END:
		break;

	default:
		printf("ra: unknown pckt\n");
	}
}


/*
 * Process an error log message
 *
 * For now, just log the error on the console.
 * Only minimal decoding is done, only "useful"
 * information is printed.  Eventually should
 * send message to an error logger.
 */
raerror(mp)
	register struct ml *mp;
{
	printf("ra0: %s err, ",
		mp->ml_flags&(M_LF_SUCC|M_LF_CONT) ? "soft" : "hard");
	switch (mp->ml_format&0377) {
	case M_FM_CNTERR:
		printf("cntrl err ev 0%o\n", mp->ml_event);
		break;

	case M_FM_BUSADDR:
		printf("mem err ev 0%o, addr 0%O\n",
			mp->ml_event, SWAPW(mp->ml_busaddr));
		break;

	case M_FM_DISKTRN:
		printf("dsk xfer err unit %d, grp 0x%x, hdr 0x%X\n",
			mp->ml_unit, mp->ml_group, SWAPW(mp->ml_hdr));
		break;

	case M_FM_SDI:
		printf("SDI err, unit %d, ev 0%o, hdr 0x%X\n",
			mp->ml_unit, mp->ml_event, SWAPW(mp->ml_hdr));
		break;

	case M_FM_SMLDSK:
		printf("sml dsk err unit %d, ev 0%o, cyl %d\n",
			mp->ml_unit, mp->ml_event, mp->ml_sdecyl);
		break;

	default:
		printf("unknown err, unit %d, fmt 0%o, ev 0%o\n",
			mp->ml_unit, mp->ml_format, mp->ml_event);
	}

	if (raerr) {
		register short *p = (short *)mp;
		register int i;

		for (i = 0; i < mp->ml_header.ra_msglen; i += sizeof(*p))
			printf("%x ", *p++);
		printf("\n");
	}
}


/*
 * Find an unused command packet
 */
struct ms *
ragetcp()
{
	register struct ms *mp;
	register struct raca *cp;
	register struct ra_softc *sc;
	register int i;

	cp = &ra.ra_ca;
	sc = &ra_softc;
	i = sc->sc_lastcmd;
	if ((cp->ca_cmddsc[i].high & (RA_OWN|RA_INT)) == RA_INT) {
		cp->ca_cmddsc[i].high &= ~RA_INT;
		mp = &ra.ra_cmd[i];
		mp->ms_unit = mp->ms_modifier = 0;
		mp->ms_opcode = mp->ms_flags = 0;
		mp->ms_buffer.high = mp->ms_buffer.low = 0;
		mp->ms_bytecnt = 0;
		mp->ms_errlgfl = mp->ms_copyspd = 0;
		sc->sc_lastcmd = (i + 1) % NCMD;
		return(mp);
	}
	return(NULL);
}

raread(dev)
	dev_t dev;
{
#ifdef UCB_DBUFS
	register int unit = minor(dev) >> 3;

	if (unit >= NRA) {
		u.u_error = ENXIO;
		return;
	}
	physio(rastrategy, &rrabuf[unit], dev, B_READ);
#else
	physio(rastrategy, &rrabuf, dev, B_READ);
#endif
}

rawrite(dev)
	dev_t dev;
{
#ifdef UCB_DBUFS
	register int unit = minor(dev) >> 3;

	if (unit >= NRA) {
		u.u_error = ENXIO;
		return;
	}
	physio(rastrategy, &rrabuf[unit], dev, B_WRITE);
#else
	physio(rastrategy, &rrabuf, dev, B_WRITE);
#endif
}

/* This function returns the Unibus virtual address for a given
 * kernel virtual address in the "ra" structure.
 */
long uvaddr(ptr)
u_short ptr;
{
	long addr;

	addr = ramap.b_xmem;
	addr <<= 16;
	addr |= (long)ramap.b_un.b_addr;
	addr += ptr-((u_short)&ra);
	return(addr);
}
#endif
