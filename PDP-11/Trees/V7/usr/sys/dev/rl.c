/*
 * RL disk driver
 * Reworked to handle RL01/RL02
 *	R.A.Mason	Oct.1980
 * Intended to eventually handle overlapped seeks
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/systm.h"

#define DK_N	1

struct device {
	int rlcs;
	int rlba;
	int rlda;
	int rlmp;
};

#define	RLADDR	((struct device *)0174400)
#define	NPDISK	4
#define	NLDISK	1	/* no subdivision into logical disks */
#define RLCYLSZ		10240
#define RLSECSZ		256
#define RL01SIZE	10240
#define RL02SIZE	20480

/* rlcs bits */
#define DRDY		0000001
#define GETSTAT		0000004
#define SEEK		0000006
#define RDHDR		0000010
#define WCOM		0000012
#define RCOM		0000014
#define IENABLE		0000100
#define CRDY		0000200
#define DS		0001400
#define OPI		0002000
#define CRC		0004000
#define DLT		0010000
#define NXM		0020000
#define DE		0040000
#define ERROR		0100000
#define ACCESS_ERROR	(NXM|DLT|CRC|OPI)

/* rlda bits	- during seek */
#define SEEKLO		0000001
#define SEEKHI		0000005
/*		- during get status */
#define GS		0000002
#define RST		0000010
#define RESET		(RST|GS|01)
#define STAT		(GS|01)

/* status bits */
#define HO		0000020
#define DT		0000200
#define VC		0001000
#define WL		0020000

struct	buf	rltab;
struct	buf	rrlbuf;

struct rl
{
	int	status;		/* drive status */
	int	headp;		/* location of heads */
	struct buf *iop;	/* current transfer on drive */
	int	errcnt;		/* error count on drive	*/
	int	com;		/* read or write command word */
	int	chn;		/* cylinder and head number */
	unsigned int	bleft;	/* bytes left to be transferred */
	unsigned int	bpart;	/* number of bytes transferred */
	int	sn;		/* sector number */
	union {
		int	w[2];
		long	l;
	} addr;			/* address of memory for transfer */
} rl[NPDISK];
/* Bit sets for drive status */
#define RL01		01
#define RL02		02
#define HEADKNOWN	010

rlopen(dev,rw)
dev_t dev;
{
	register struct rl *rlp;
	register struct device *rp;
	int drive,status;

	drive = minor(dev);
	if(drive >= NPDISK) {
		u.u_error = ENXIO;
		return;
	}
	rlp = &rl[drive];
	rp = RLADDR;
	spl5();
	while((rp->rlcs & CRDY) == 0);
	rp->rlda = STAT;
	rp->rlcs = (drive << 8) | GETSTAT;
	while((rp->rlcs & CRDY) == 0);
	status = rp->rlmp;
	rp->rlda = RESET;
	rp->rlcs = (drive << 8) | GETSTAT;
	while((rp->rlcs & CRDY) == 0);
	spl0();
	if((status&HO) == 0) {
		u.u_error = ENXIO;
		return;
	}
	if(rw && (status&WL)) {
		u.u_error = EROFS;
		return;
	}
	if(status&DT)
		rlp->status |= RL02;
	else
		rlp->status |= RL01;
}

rlclose(dev)
dev_t dev;
{
	register struct rl *rlp;

	rlp = &rl[minor(dev)];
	/* will do something on next rework */
}

rlstrategy(bp)
register struct buf *bp;
{
	register struct rl *rlp;
	int drive,dsize;

#ifdef UNIBMAP
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
#endif UNIBMAP
	drive = minor(bp->b_dev);
	rlp = &rl[drive];
	dsize = 0;
	if(rlp->status&RL01)
		dsize = RL01SIZE;
	else
	if(rlp->status&RL02)
		dsize = RL02SIZE;
	if(bp->b_blkno >= dsize) {
		if((bp->b_blkno == dsize) && (bp->b_flags&B_READ))
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	spl5();
	if(rltab.b_actf == NULL)
		rltab.b_actf = bp;
	else
		rltab.b_actl->av_forw = bp;
	rltab.b_actl = bp;
	if(rltab.b_active == NULL)
		rlstart();
	spl0();
}

rlstart()
{
	register struct buf *bp;
	register struct rl *rlp;
	int drive;

	if((bp = rltab.b_actf) == NULL)
		return;
	rltab.b_active++;
	drive = minor(bp->b_dev);
	rlp = &rl[drive];
	rlp->iop = bp;
	rlp->chn = bp->b_blkno/20;
	rlp->sn = (bp->b_blkno%20) << 1;
	rlp->bleft = bp->b_bcount;
	rlp->addr.w[0] = bp->b_xmem & 3;
	rlp->addr.w[1] = (int)bp->b_un.b_addr;
	rlp->com = (drive << 8) | IENABLE;
	if(bp->b_flags & B_READ)
		rlp->com |= RCOM;
	else
		rlp->com |= WCOM;
	rlio(drive);
}

rlintr()
{
	register struct buf *bp;
	register struct device *rp;
	register struct rl *rlp;
	int drive,status;

	rp = RLADDR;
	drive = (rp->rlcs&DS) >> 8;
	rlp = &rl[drive];
	if(rlp->iop == NULL) {
/*		logstray(rp); */
		return;
	}
	bp = rlp->iop;
#ifdef INSTRM
	dk_busy &= ~(1<<DK_N);
#endif INSTRM
	if(rp->rlcs&ERROR) {
		if(rp->rlcs&ACCESS_ERROR) {
			if(rlp->errcnt > 2)
				deverror(bp, rp->rlcs, rp->rlda);
		}
		if(rp->rlcs&DE) {
			rp->rlda = STAT;
			rp->rlcs = (drive << 8) | GETSTAT;
			while((rp->rlcs & CRDY) == 0);
			status = rp->rlmp;
			if(rlp->errcnt > 2)
				deverror(bp, status, rp->rlda);
			rp->rlda = RESET;
			rp->rlcs = (drive << 8) | GETSTAT;
			while((rp->rlcs & CRDY) == 0);
			if(status&DT)		/* drive type */
				rlp->status |= RL02;
			else
				rlp->status |= RL01;
			if(status&VC) {		/* volume check */
				rlstart();
				return;
			}
		}
		if(++rlp->errcnt <= 10) {
			rlp->status &= ~HEADKNOWN;
			rlstart();
			return;
		}
		else {
			bp->b_flags |= B_ERROR;
			rlp->bpart = rlp->bleft;
		}
	}
	if((rlp->bleft -= rlp->bpart) > 0) {
		rlp->addr.l += rlp->bpart;
		rlp->sn = 0;
		rlp->chn++;
		rlio(drive);
		return;
	}
	rlp->iop = NULL;
	rlp->errcnt = 0;
	rltab.b_active = NULL;
	rltab.b_actf = bp->av_forw;
	bp->b_resid = 0;
	iodone(bp);
	rlstart();
}

rlio(dn)
int dn;
{
	register struct device *rp;
	register struct rl *rlp;
	int dif,head;

	rp = RLADDR;
	rlp = &rl[dn];
#ifdef INSTRM
	dk_busy |= 1<<DK_N;
	dk_numb[DK_N] += 1;
	dk_wds[DK_N] += (rlp->bpart>>6);
#endif INSTRM
	if((rlp->status&HEADKNOWN) == 0) {
		rp->rlcs = (dn << 8) | RDHDR;
		while((rp->rlcs&CRDY) == 0);
		rlp->headp = ((unsigned)(rp->rlmp&0177700)) >> 6;
		rlp->status |= HEADKNOWN;
	}
	dif = (rlp->headp >> 1) - (rlp->chn >>1);
	head = (rlp->chn & 1) << 4;
	if(dif < 0)
		rp->rlda = (-dif<<7) | SEEKHI | head;
	else
		rp->rlda = (dif<< 7) | SEEKLO | head;
	rp->rlcs = (dn << 8) | SEEK;
	rlp->headp = rlp->chn;
	if(rlp->bleft < (rlp->bpart = RLCYLSZ - (rlp->sn * RLSECSZ)))
		rlp->bpart = rlp->bleft;
	while((rp->rlcs&CRDY) == 0);
	rp->rlda = (rlp->chn << 6) | rlp->sn;
	rp->rlba = rlp->addr.w[1];
	rp->rlmp = -(rlp->bpart >> 1);
	rp->rlcs = rlp->com | rlp->addr.w[0] << 4;
}

rlread(dev)
{
	physio(rlstrategy, &rrlbuf, dev, B_READ);
}

rlwrite(dev)
{
	physio(rlstrategy, &rrlbuf, dev, B_WRITE);
}
