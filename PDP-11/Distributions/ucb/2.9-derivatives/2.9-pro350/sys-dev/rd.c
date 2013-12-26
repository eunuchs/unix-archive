/*
 * This driver runs the RD50/51 controller on the pro3xx. It
 * expects bad sector forwarding
 * tables to be in a BAD144 style. Bad sectors are marked by writing a nonzero
 * backup revision field in the sector header. The standalone program, "rdfmt"
 * should be run to format and bad scan the disk before this driver is used
 * if BADSECT is enabled.
 */

#include "rd.h"
#if NRD > 0
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
#ifdef BADSECT
#include <sys/dkbad.h>
#endif BADSECT
#include <sys/ivecpos.h>
#include <sys/rdreg.h>


#define	OPEN	1
#define GOTBAD 2

extern struct size rd_sizes[];
extern struct rddevice *RDADDR;

struct	buf	rdtab;
struct	buf	rrdbuf;
#ifdef BADSECT
struct	dkbad	rdbad;
#endif BADSECT
union	wordval	rdval;
struct rdst rdst[] = {
	16,	4,	16*4,	305,
	16,	4,	16*4,	152,
	0,	0,	0,	0,
};

struct rd_softc {
	char	sc_stat;
	char	sc_type;
} rd_softc;

void rdroot()
{
	rdattach(RDADDR, 0);
}

rdattach(addr, unit)
struct rddevice *addr;
{
	if (unit == 0) {
		RDADDR = addr;
		return(1);
	} 
	return(0);
}

rdopen(dev) {
	register int cnt, *ptr, tmp;

	if (rd_softc.sc_stat & OPEN)
		return;
	rd_softc.sc_stat |= OPEN;
	rdinit();
	if (!(rd_softc.sc_stat & GOTBAD)) {
		while (1) {
			RDADDR->sec = 0;
			RDADDR->trk = rdst[rd_softc.sc_type].ntrak-1;
			RDADDR->cyl = rdst[rd_softc.sc_type].ncyl;
			RDADDR->csr = RD_READCOM;
			while (RDADDR->st & RD_BUSY) ;
			if (RDADDR->csr & RD_ERROR) {
				if (RDADDR->err & RD_IDNF)
					rd_softc.sc_type++;
				rdinit();
			} else {
#ifdef BADSECT
				ptr = &rdbad;
#endif BADSECT
				for (cnt = 0; cnt < 256; cnt++) {
					while ((RDADDR->st & RD_DRQ) == 0) ;
#ifdef BADSECT
					if (cnt < ((sizeof rdbad)/2))
						*ptr++ = RDADDR->db;
					else
#endif BADSECT
						tmp = RDADDR->db;
				}
				break;
			}
		}
		rd_softc.sc_stat |= GOTBAD;
	}
}

rdstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register struct rdst *st;
	register int unit;
	int s;
	long bn;
	long sz;

	unit = minor(bp->b_dev)&07;
	sz = bp->b_bcount;
	sz = (sz+511)>>9;
	st = &rdst[rd_softc.sc_type];
	if (unit >= (NRD<<3) || (RDADDR == (struct rddevice *) NULL)) {
		bp->b_error = ENXIO;
		goto errexit;
	}
	if (bp->b_blkno < 0 || ((bn = dkblock(bp))+sz > rd_sizes[unit].nblocks
	   && rd_sizes[unit].nblocks >= 0)) {
		bp->b_error = EINVAL;
errexit:
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	bp->b_cylin = bn/st->nspc+rd_sizes[unit].cyloff;
	dp = &rdtab;
	(void) _spl4();
	disksort(dp, bp);
	if (dp->b_active == NULL)
		rdstart();
	(void) _spl0();
}

rdstart()
{
	register struct buf *bp;
	register int unit;

	if ((bp = rdtab.b_actf) == NULL)
		return;
	bp->b_resid = bp->b_bcount;
	rdstrt(dkblock(bp), bp);
#ifdef RD_DKN
	dk_busy |= 1<<RD_DKN;
	dk_numb[RD_DKN] += 1;
#endif RD_DKN
}

rdstrt(bn, bp)
daddr_t bn;
register struct buf *bp;
{
	register u_short cnt;
	register char *ptr;
	register struct rdst *st;
	long taddr;
	int sn;
	u_short xfer;

	rdtab.b_active++;
	st = &rdst[rd_softc.sc_type];
	while (RDADDR->st & RD_BUSY)
		;
	RDADDR->cyl = bn/(st->nspc)+rd_sizes[minor(bp->b_dev)&07].cyloff;
	sn = bn % st->nspc;
	RDADDR->trk = sn / st->nsect;
	RDADDR->sec = sn % st->nsect;
	if (bp->b_flags & B_READ) {
		ienable(IVEC(RDADDR, APOS));
		ienable(IVEC(RDADDR, BPOS));
		RDADDR->csr = RD_READCOM;
	} else {
		/* Kernel seg. 5 is used to map the i/o buffer into kernel 
		 * virtual so that the copy loop can be performed.
		 */
		segm save;
		xfer = (bp->b_resid < 01000)?bp->b_resid:01000;
		taddr = (((((long)bp->b_xmem)<<16)&0x3f0000l)|
			(((long)bp->b_un.b_addr)&0xffff))+
			(bp->b_bcount-bp->b_resid);
		ptr = (char *)((taddr & 077)+SEG5);
		ienable(IVEC(RDADDR, APOS));
		RDADDR->csr = RD_WRITECOM;
		saveseg5(save);
		mapseg5(((caddr_t)(taddr>>6)), 04406);
		for (cnt = 0; cnt < 0400; cnt++) {
			while ((RDADDR->st & RD_DRQ) == 0)
				;
			if (cnt < (xfer/02)) {
				rdval.byte[0] = *ptr++;
				rdval.byte[1] = *ptr++;
			}
			RDADDR->db = rdval.word;
		}
		restorseg5(save);
	}
}

/* This interrupt service routine is accessed via. both the A and B vectors */
rdintr()
{
	register struct buf *bp;
	register u_short cnt;
	register char *ptr;
	long taddr;
	daddr_t bn;
	u_short xfer;

	if (rdtab.b_active == NULL) {
		idisable(IVEC(RDADDR, APOS));
		idisable(IVEC(RDADDR, BPOS));
		return;
	}
#ifdef RD_DKN
	dk_busy &= ~(1<<RD_DKN);
#endif RD_DKN
	bp = rdtab.b_actf;
	rdtab.b_active = NULL;
	while (RDADDR->st & RD_BUSY)
		;
	if ((RDADDR->csr & (RD_WFAULT|RD_ERROR))
#ifdef BADSECT
		|| (RDADDR->sec & 0177400)
#endif
		) {
#ifdef BADSECT
		if ((RDADDR->err & (RD_DMNF|RD_IDNF|RD_CRC)) || (RDADDR->sec & 0177400)) {
			idisable(IVEC(RDADDR, APOS));
			idisable(IVEC(RDADDR, BPOS));
			if (baderr(bp)) {
				return;
			}
		}
#endif
		if (++rdtab.b_errcnt <= 10) {
			rdinit();
			bn = dkblock(bp)+((bp->b_bcount-bp->b_resid)>>9);
			idisable(IVEC(RDADDR, APOS));
			idisable(IVEC(RDADDR, BPOS));
			rdstrt(bn, bp);
			return;
		}
#ifdef UCB_DEVERR
		harderr(bp, "rd");
		printf("cs=%b er=%b\n",RDADDR->csr,RDCS_BITS,RDADDR->err,RDER_BITS);
#else
		deverror(bp, RDADDR->csr, RDADDR->err);
#endif
		rdinit();
		bp->b_flags |= B_ERROR;
		rdtab.b_errcnt = 0;
		rdtab.b_actf = bp->av_forw;
		bp->b_resid = 0;
		iodone(bp);
		idisable(IVEC(RDADDR, APOS));
		idisable(IVEC(RDADDR, BPOS));
		rdstart();
		return;
	}
	xfer = (bp->b_resid < 01000)?bp->b_resid:01000;
	if (bp->b_flags & B_READ) {
		segm save;
		taddr = (((((long)bp->b_xmem)<<16)&0x3f0000l)|
			(((long)bp->b_un.b_addr)&0xffff))+
			(bp->b_bcount-bp->b_resid);
		ptr = (char *)((taddr & 077)+SEG5);
		saveseg5(save);
		mapseg5(((caddr_t)(taddr>>6)), 04406);
		for (cnt = 0; cnt < 0400; cnt++) {
			while ((RDADDR->st & RD_DRQ) == 0)
				;
			rdval.word = RDADDR->db;
			if (cnt < (xfer/02)) {
				*ptr++ = rdval.byte[0];
				*ptr++ = rdval.byte[1];
			}
		}
		restorseg5(save);
	}
	rdtab.b_errcnt = 0;
	bp->b_resid -= xfer;
	if (bp->b_resid == 0) {
		rdtab.b_actf = bp->av_forw;
		iodone(bp);
		idisable(IVEC(RDADDR, APOS));
		idisable(IVEC(RDADDR, BPOS));
		rdstart();
		return;
	}
	bn = dkblock(bp)+((bp->b_bcount-bp->b_resid)>>9);
	idisable(IVEC(RDADDR, APOS));
	idisable(IVEC(RDADDR, BPOS));
	rdstrt(bn, bp);
}

rdinit()
{
	RDADDR->st = RD_INIT;
	while (RDADDR->st & RD_BUSY)
		;
	RDADDR->csr = RD_RESTORE;
	while ((RDADDR->st & RD_BUSY) || (!(RDADDR->st & RD_OPENDED)))
		;
}

rdread(dev)
{

	physio(rdstrategy, &rrdbuf, dev, B_READ);
}

rdwrite(dev)
{

	physio(rdstrategy, &rrdbuf, dev, B_WRITE);
}
#ifdef BADSECT
baderr(bp)
struct buf *bp;
{
	register int sn,cn;
	register struct rdst *st;
	int tn;
	daddr_t bn, isbad();

	st = &rdst[rd_softc.sc_type];
	bn = dkblock(bp) + ((bp->b_bcount-bp->b_resid)>>9)+
		rd_sizes[(minor(bp->b_dev)&07)].cyloff;
	cn = bn/(st->nspc);
	sn = bn%(st->nspc);
	tn = sn/(st->nsect);
	sn %= (st->nsect);
	if ((bn = isbad(&rdbad, cn, tn, sn)) < 0)
		return(0);
	cn = st->ncyl-rd_sizes[(minor(bp->b_dev)&07)].cyloff;
	bn = cn*st->nspc+st->nsect*(st->ntrak-1)-1-bn;
	rdstrt(bn, bp);
	return(1);
}
#endif
#ifdef RD_DUMP
/* This is the RD50/51 dump routine. It uses mapin/mapout to get
 * at all of memory.
 */

rddump(dev)
	dev_t dev;
{
	register int *ptr, cnt;
	register struct rdst *st = &rdst[rd_softc.sc_type];
	daddr_t bn, dumpsize;
	long paddr;
	int sn, unit;
	segm save;

	unit = minor(dev) >> 3;
	if ((bdevsw[major(dev)].d_strategy != rdstrategy)
		|| unit >= NRD)
		return(EINVAL);
	dumpsize = rd_sizes[minor(dev)&07].nblocks;
	if ((dumplo < 0) || (dumplo >= dumpsize))
		return(EINVAL);
	dumpsize -= dumplo;
	/* Initialize the controller/drive */
	rdinit();
	/* Loop until dump complete */
	for (paddr = 0L; dumpsize > 0; dumpsize--) {
		bn = dumplo + (paddr>>PGSHIFT);
		RDADDR->cyl = (bn/st->nspc) + rd_sizes[minor(dev)&07].cyloff;
		sn = bn%st->nspc;
		RDADDR->trk = sn/st->nsect;
		RDADDR->sec = sn%st->nsect;
		RDADDR->csr = RD_WRITECOM;
		ptr = (int *)((paddr & 077)+SEG5);
		saveseg5(save);
		mapseg5(((caddr_t)(paddr>>6)), 04406);
		for (cnt = 0; cnt < 256; cnt++) {
			while ((RDADDR->st & RD_DRQ) == 0)
				;
			RDADDR->db = *ptr++;
		}
		restorseg5(save);
		while ((RDADDR->st & RD_BUSY) || (!(RDADDR->st & RD_OPENDED)))
			;
		if ((RDADDR->csr & (RD_WFAULT|RD_ERROR))
			return(EIO);
		paddr += (1<<PGSHIFT);
	}
	return(0);
}
#endif RD_DUMP
#endif
