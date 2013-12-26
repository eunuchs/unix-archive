/*
 * This driver handles the RX50 controller on the pro3xx. It should be
 * noted that track 0 is never used, just to be consistent with good ole
 * dec software. The reading of foreign diskettes has not been thoroughly
 * tested, but seems to work. Anyone keen enough could implement an ms-dos
 * or cp/m file system handling utility. Its handy to modify dump/restor so
 * that a version knows about RX50's at 790 sectors each.
 */

#include "r5.h"
#if NR5 > 0
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/dir.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/seg.h>
#ifndef INTRLVE
#include <sys/inline.h>
#endif INTRLVE
#include <sys/r5reg.h>
#include <sys/r5io.h>
#include <sys/ivecpos.h>


#define	OPEN	1
#define	VOLCHNGE 2
#define DISMNT	4

/* These are the diskette type parameters handled by the rx50 controller.
 * See the Professional 300 Series Technical Manual for more details.
 */
struct r5parm {
	int	trks;
	int	sectrfm;
	int	dens;
	int	sectrk;
	int	bytesec;
} r5parm[4] = {
	0120,	0152,	0132,	10,	512,
	050,	0152,	0131,	10,	512,
	050,	0151,	0131,	9,	512,
	050,	0120,	0131,	16,	256,
};

extern struct r5device *R5ADDR;

struct	buf	r5tab;
struct	buf	rr5buf;

struct r5_softc {
	char	sc_stat;
	char	sc_type;
	char	sc_dstat[NR5];
} r5_softc;

void r5root()
{
	r5attach(R5ADDR, 0);
}

r5attach(addr, unit)
struct r5device *addr;
{
	if (unit == 0) {
		R5ADDR = addr;
		return(1);
	}
	return(0);
}

r5open(dev)
dev_t dev;
{
	register int i;
	int s;

	if (minor(dev) >= NR5 || (R5ADDR == (struct r5device *) NULL)) {
		u.u_error = ENXIO;
		return;
	}
	if ((r5_softc.sc_stat & OPEN) == 0) {
		(void) _spl4();
		r5_softc.sc_stat |= OPEN;
		r5rstat(0);
		ienable(IVEC(R5ADDR, BPOS));
		for(i = 0; i < NR5; i++) {
			if (i == minor(dev))
				r5_softc.sc_dstat[i] |= OPEN;
			else
				r5_softc.sc_dstat[i] = 0;
		}
		(void) _spl0();
	} else {
		r5_softc.sc_dstat[minor(dev)] |= OPEN;
	}
}

r5close(dev)
dev_t dev;
{
	r5_softc.sc_dstat[minor(dev)] = 0;
}
r5strategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	long sz, bn;
	int s;

	sz = bp->b_bcount;
	sz = (sz+511)>>9;
	if (bp->b_blkno < 0 || (bn = dkblock(bp))+sz > R5_SIZE) {
		bp->b_error = EINVAL;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	bp->b_cylin = bn/10;
	dp = &r5tab;
	(void) _spl4();
	disksort(dp, bp);
	if (dp->b_active == NULL)
		r5start();
	(void) _spl0();
}

r5start()
{
	register struct buf *bp;
	register int unit;

	if ((bp = r5tab.b_actf) == NULL)
		return;
	bp->b_resid = bp->b_bcount;
	r5strt((dkblock(bp)*(512/r5parm[r5_softc.sc_type].bytesec)), bp);
#ifdef R5_DKN
	dk_busy |= 1<<R5_DKN;
	dk_numb[R5_DKN] += 1;
#endif R5_DKN
}

r5strt(bn, bp)
daddr_t bn;
register struct buf *bp;
{
	register char *ptr;
	register u_short cnt;
	u_short tn, sn;
	int unit;
	long taddr;
	u_short xfer;

	if (r5_softc.sc_stat & VOLCHNGE) {
		r5rstat(1);
		r5_softc.sc_stat &= ~VOLCHNGE;
	}
	r5tab.b_active++;
	unit = minor(bp->b_dev);
	tn = bn/r5parm[r5_softc.sc_type].sectrk;
	sn = bn % r5parm[r5_softc.sc_type].sectrk;
	if (r5_softc.sc_type == 0)  {
		sn = ((sn<<1)%10 + (sn<<1)/10 + (tn<<1))%10;
		tn = (tn+1) % 80;
	}
	R5ADDR->cs1 = tn;
	R5ADDR->cs2 = sn+1;
	if (bp->b_flags & B_READ) {
		R5ADDR->cs0 = R5_READCOM|((unit&03)<<1);
		ienable(IVEC(R5ADDR, APOS));
		R5ADDR->sc = 0;
	} else {
		segm save;
		if (r5_softc.sc_type)
			r5param(r5_softc.sc_type = 0);
		xfer = (bp->b_resid < 01000)?bp->b_resid:01000;
		R5ADDR->cs0 = R5_WRITECOM|((unit&03)<<1);
		R5ADDR->ca = 0;
		taddr = (((((long)bp->b_xmem)<<16)&0x3f0000l)|
			(((long)bp->b_un.b_addr)&0xffff))+
			(bp->b_bcount-bp->b_resid);
		ptr = (char *)((taddr & 077)+SEG5);
		saveseg5(save);
		mapseg5(((caddr_t)(taddr>>6)), 04406);
		for (cnt = 0; cnt < xfer; cnt++) {
			R5ADDR->dbi = *ptr++;
		}
		restorseg5(save);
		ienable(IVEC(R5ADDR, APOS));
		R5ADDR->sc = 0;
	}
}

r5aint()
{
	register struct buf *bp;
	register char *ptr;
	register u_short cnt;
	int err;
	long taddr;
	daddr_t bn;
	u_short xfer;

	if (r5tab.b_active == NULL) {
		idisable(IVEC(R5ADDR, APOS));
		return;
	}
	r5tab.b_active = NULL;
#ifdef R5_DKN
	dk_busy &= ~(1<<R5_DKN);
#endif R5_DKN
	bp = r5tab.b_actf;
	while ((R5ADDR->cs0 & R5_BUSY) == 0)
		;
	if (R5ADDR->cs0 & R5_ERROR) {
		if (++r5tab.b_errcnt <= 10) {
			r5init();
			r5rstat(0);
			bn = dkblock(bp)*(512/r5parm[r5_softc.sc_type].bytesec)+
				((bp->b_bcount-bp->b_resid)/
				r5parm[r5_softc.sc_type].bytesec);
			idisable(IVEC(R5ADDR, APOS));
			r5strt(bn, bp);
			return;
		}
#ifdef UCB_DEVERR
		harderr(bp, "r5");
		printf("cs0=%o cs1=%o\n",R5ADDR->cs0,R5ADDR->cs1);
#else
		deverror(bp, R5ADDR->cs0, R5ADDR->cs1);
#endif
		r5init();
		r5rstat(0);
		bp->b_flags |= B_ERROR;
		r5tab.b_errcnt = 0;
		r5tab.b_actf = bp->av_forw;
		bp->b_resid = 0;
		iodone(bp);
		idisable(IVEC(R5ADDR, APOS));
		r5start();
		return;
	}
	xfer = (bp->b_resid < r5parm[r5_softc.sc_type].bytesec)?bp->b_resid:
		r5parm[r5_softc.sc_type].bytesec;
	if (bp->b_flags & B_READ) {
		segm save;
		R5ADDR->ca = 0;
		taddr = (((((long)bp->b_xmem)<<16)&0x3f0000l)|
			(((long)bp->b_un.b_addr)&0xffff))+
			(bp->b_bcount-bp->b_resid);
		ptr = (char *)((taddr & 077)+SEG5);
		saveseg5(save);
		mapseg5(((caddr_t)(taddr>>6)), 04406);
		for (cnt = 0; cnt < xfer; cnt++) {
			*ptr++ = R5ADDR->dbo;
		}
		restorseg5(save);
	}
	r5tab.b_errcnt = 0;
	bp->b_resid -= xfer;
	if (bp->b_resid == 0) {
		r5tab.b_actf = bp->av_forw;
		iodone(bp);
		idisable(IVEC(R5ADDR, APOS));
		r5start();
		return;
	}
	bn = dkblock(bp)*(512/r5parm[r5_softc.sc_type].bytesec)+
	     ((bp->b_bcount-bp->b_resid)/r5parm[r5_softc.sc_type].bytesec);
	idisable(IVEC(R5ADDR, APOS));
	r5strt(bn, bp);
}

r5init()
{
	R5ADDR->cs0 = R5_INIT;
	R5ADDR->sc = 0;
	while ((R5ADDR->cs0 & R5_BUSY) == 0)
		;
}

r5read(dev)
{

	physio(r5strategy, &rr5buf, dev, B_READ);
}

r5write(dev)
{

	physio(r5strategy, &rr5buf, dev, B_WRITE);
}

r5ioctl(dev, cmd, addr, flag)
dev_t dev;
int cmd;
caddr_t addr;
int flag;
{
	int parmvl;

	switch(cmd) {
	case R5IOCSETP:
		if (copyin(addr, (caddr_t)&parmvl, sizeof(parmvl)) ||
			(parmvl < 0 || parmvl > 3)) {
			u.u_error = EFAULT;
			return;
		}
		r5param(r5_softc.sc_type = parmvl);
		break;
	case R5IOCGETP:
		parmvl = r5_softc.sc_type;
		if (copyout((caddr_t)&parmvl, addr, sizeof(parmvl))) {
			u.u_error = EFAULT;
			return;
		}
		break;
	};
}

r5rstat(flag)
int flag;
{
	register int tmp, i;

	R5ADDR->cs0 = R5_STATUS;
	R5ADDR->sc = 0;
	while ((R5ADDR->cs0 & R5_BUSY) == 0)
		;
	tmp = R5ADDR->cs3 >> 4;
	for (i = 0; i < NR5; i++) {
		if (tmp & 01) {
			if (r5_softc.sc_dstat[i] & OPEN)
			   if (flag && !(r5_softc.sc_dstat[i] & DISMNT)) {
				printf("Umount floppy %d before removing\n",i);
				r5_softc.sc_dstat[i] |= DISMNT;
			   } else {
				r5_softc.sc_dstat[i] &= ~DISMNT;
			   }
			if (r5_softc.sc_type)
				r5param(r5_softc.sc_type = 0);
		}
		tmp >>= 1;
	}
}

r5bint()
{
	if (r5tab.b_active == NULL)
		r5rstat(1);
	else
		r5_softc.sc_stat |= VOLCHNGE;
}

r5param(num)
int num;
{
	R5ADDR->cs1 = r5parm[num].trks;
	R5ADDR->cs2 = r5parm[num].sectrfm;
	R5ADDR->cs3 = r5parm[num].dens;
	R5ADDR->cs5 = R5_SPARM;
	R5ADDR->cs0 = R5_EXTCOM;
	R5ADDR->sc = 0;
	while ((R5ADDR->cs0 & R5_BUSY) == 0)
		;
}
#endif
