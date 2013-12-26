#
/*
 * RX02 floppy disk device driver
 *
 * Bill Shannon   CWRU   05/29/79
 *
 * Modified for Version 7 - 08/13/79 - Bill Shannon
 *
 *
 *	Layout of logical devices:
 *
 *	name	min dev		unit	density
 *	----	-------		----	-------
 *	rx0	   0		  0	single
 *	rx1	   1		  1	single
 *	rx2	   2		  0	double
 *	rx3	   3		  1	double
 *
 *
 *	Stty function call may be used to format a disk.
 *	To enable this feature, define CTRL in this module.
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/tty.h"


/* The following structure is used to access av_back as an integer */

struct {
	int	dummy0;
	struct	buf *dummy1;
	struct	buf *dummy2;
	struct	buf *dummy3;
	int	seccnt;
};

struct device {
	int	rx2cs;
	int	rx2db;
};


/*
 *	the following defines use some fundamental
 *	constants of the RX02.
 */
#define	RXADDR	((struct device *) 0177170)
#define	NSPB	((minor(bp->b_dev)&2) ? 2 : 4)	/* Number of floppy sectors per unix block */
#define	NBPS	((minor(bp->b_dev)&2) ? 256 : 128)	/* Number of bytes per floppy sector	*/
#define	NRXBLKS	((minor(bp->b_dev)&2) ? 1001 : 500)	/* Number of unix blocks on a floppy */
#define	DENSITY	(minor(bp->b_dev)&2)	/* Density: 0 = single, 2 = double */
#define	UNIT	(minor(bp->b_dev)&1)	/* Unit Number: 0 = left, 1 = right */

#define	CTRL	CTRL			/* define for control functions (formatting) */

#ifdef	CTRL
#define	B_CTRL	020000
#endif

#define	TRWAIT		while (ptcs->lobyte >= 0)

struct {
	char 	lobyte;
	char	hibyte;
};

struct	buf	rx2tab;

struct	buf	rrx2buf;

#ifdef CTRL
struct	buf	crx2buf;	/* buffer header for control functions */
#endif

#define	GO	0000001	/* execute command function	*/
#define	UNIT1	0000020	/* unit select (drive 0=0, 1=1)	*/
#define	RXDONE	0000040	/* function complete		*/
#define	INTENB	0000100	/* interrupt enable		*/
#define	TRANREQ	0000200	/* transfer request (data only)	*/
#define	RXINIT	0040000	/* rx211 initialize		*/
#define	RXERROR	0100000	/* general error bit		*/

/*
 *	rx211 control function bits 1-3 of rx2cs
 */
#define	FILL	0000000	/* fill buffer			*/
#define	EMPTY	0000002	/* empty buffer			*/
#define	WRITE	0000004	/* write buffer to disk		*/
#define	READ	0000006	/* read disk sector to buffer	*/
#define	FORMAT	0000010	/* set media density (format)	*/
#define	RSTAT	0000012	/* read disk status		*/
#define	WSDD	0000014	/* write sector deleted data	*/
#define	RDERR	0000016	/* read error register function	*/

/*
 *	states of driver, kept in b_active
 */
#define	SREAD	1	/* read started  */
#define	SEMPTY	2	/* empty started */
#define	SFILL	3	/* fill started  */
#define	SWRITE	4	/* write started */
#define	SINIT	5	/* init started  */
#define	SFORMAT	6	/* format started */


rx2open(dev, flag)
{
	if(minor(dev) >= 4)
		u.u_error = ENXIO;
}

rx2strategy(bp)
register struct buf *bp;
{

	if(bp->b_flags & B_PHYS)
		mapalloc(bp);
	if(bp->b_blkno >= NRXBLKS) {
		if(bp->b_flags&B_READ)
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_flags =| B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = (struct buf *) NULL;
	/*
	 * seccnt is actually the number of floppy sectors transferred,
	 * incremented by one after each successful transfer of a sector.
	 */
	bp->seccnt = 0;
	/*
	 * We'll modify b_resid as each piece of the transfer
	 * successfully completes.  It will also tell us when
	 * the transfer is complete.
	 */
	bp->b_resid = bp->b_bcount;
	spl5();
	if(rx2tab.b_actf == NULL)
		rx2tab.b_actf = bp;
	else
		rx2tab.b_actl->av_forw = bp;
	rx2tab.b_actl = bp;
	if(rx2tab.b_active == NULL)
		rx2start();
	spl0();
}

rx2start()
{
	register int *ptcs, *ptdb;
	register struct buf *bp;
	int sector, track;
	char *addr, *xmem;

	if((bp = rx2tab.b_actf) == NULL) {
		rx2tab.b_active = NULL;
		return;
	}

	ptcs = &RXADDR->rx2cs;
	ptdb = &RXADDR->rx2db;


#ifdef CTRL
	if (bp->b_flags&B_CTRL) {	/* is it a control request ? */
		rx2tab.b_active = SFORMAT;
		*ptcs = FORMAT | GO | INTENB | (UNIT << 4) | (DENSITY << 7);
		TRWAIT;
		*ptdb = 'I';
	} else
#endif
	if(bp->b_flags&B_READ) {
		rx2tab.b_active = SREAD;
		rx2factr((int)bp->b_blkno * NSPB + bp->seccnt, &sector, &track);
		*ptcs = READ | GO | INTENB | (UNIT << 4) | (DENSITY << 7);
		TRWAIT;
		*ptdb = sector;
		TRWAIT;
		*ptdb = track;
	} else {
		rx2tab.b_active = SFILL;
		rx2addr(bp, &addr, &xmem);
		*ptcs = FILL | GO | INTENB | (xmem << 12) | (DENSITY << 7);
		TRWAIT;
		*ptdb = (bp->b_resid >= NBPS ? NBPS : bp->b_resid) >> 1;
		TRWAIT;
		*ptdb = addr;
	}
}

rx2intr() {
	register int *ptcs, *ptdb;
	register struct buf *bp;
	int sector, track;
	char *addr, *xmem;

	/* fix for error recovery by duke!phs!dennis */
	if (rx2tab.b_active == SINIT) {
		rx2start();
		return;
	}

	if((bp = rx2tab.b_actf) == NULL)
		return;

	ptcs = &RXADDR->rx2cs;
	ptdb = &RXADDR->rx2db;

	if(*ptcs < 0) {
		if(rx2tab.b_errcnt++ > 10 || rx2tab.b_active == SFORMAT) {
			bp->b_flags =| B_ERROR;
			deverror(bp, *ptcs, *ptdb);
			rx2tab.b_errcnt = 0;
			rx2tab.b_actf = bp->av_forw;
			iodone(bp);
		}
		*ptcs = RXINIT;
		*ptcs = INTENB;
		rx2tab.b_active = SINIT;
		return;
	}
	switch (rx2tab.b_active) {

	case SREAD:			/* read done, start empty */
		rx2tab.b_active = SEMPTY;
		rx2addr(bp, &addr, &xmem);
		*ptcs = EMPTY | GO | INTENB | (xmem << 12) | (DENSITY << 7);
		TRWAIT;
		*ptdb = (bp->b_resid >= NBPS ? NBPS : bp->b_resid) >> 1;
		TRWAIT;
		*ptdb = addr;
		return;

	case SFILL:			/* fill done, start write */
		rx2tab.b_active = SWRITE;
		rx2factr((int)bp->b_blkno * NSPB + bp->seccnt, &sector, &track);
		*ptcs = WRITE | GO | INTENB | (UNIT << 4) | (DENSITY << 7);
		TRWAIT;
		*ptdb = sector;
		TRWAIT;
		*ptdb = track;
		return;

	case SWRITE:			/* write done, start next fill */
	case SEMPTY:			/* empty done, start next read */
		/*
		 * increment amount remaining to be transferred.
		 * if it becomes positive, last transfer was a
		 * partial sector and we're done, so set remaining
		 * to zero.
		 */
		if (bp->b_resid <= NBPS) {
done:
			bp->b_resid = 0;
			rx2tab.b_errcnt = 0;
			rx2tab.b_actf = bp->av_forw;
			iodone(bp);
			break;
		}

		bp->b_resid -= NBPS;
		bp->seccnt++;
		break;

#ifdef CTRL
	case SFORMAT:			/* format done (whew!!!) */
		goto done;		/* driver's getting too big... */
#endif
	}
	/* end up here from states SWRITE, SEMPTY, and SINIT */

	rx2start();
}

/*
 *	rx2factr -- calculates the physical sector and physical
 *	track on the disk for a given logical sector.
 *	call:
 *		rx2factr(logical_sector,&p_sector,&p_track);
 *	the logical sector number (0 - 2001) is converted
 *	to a physical sector number (1 - 26) and a physical
 *	track number (0 - 76).
 *	the logical sectors specify physical sectors that
 *	are interleaved with a factor of 2. thus the sectors
 *	are read in the following order for increasing
 *	logical sector numbers (1,3, ... 23,25,2,4, ... 24,26)
 *	There is also a 6 sector slew between tracks.
 *	Logical sectors start at track 1, sector 1; go to
 *	track 76 and then to track 0.  Thus, for example, unix block number
 *	498 starts at track 0, sector 21 and runs thru track 0, sector 2.
 */
rx2factr(sectr, psectr, ptrck)
register int sectr;
int *psectr, *ptrck;
{
	register int p1, p2;

	p1 = sectr/26;
	p2 = sectr%26;
	/* 2 to 1 interleave */
	p2 = (2*p2 + (p2 >= 13 ? 1 : 0)) % 26;
	/* 6 sector per track slew */
	*psectr = 1 + (p2 + 6*p1) % 26;
	if (++p1 >= 77)
		p1 = 0;
	*ptrck = p1;
}


/*
 *	rx2addr -- compute core address where next sector
 *	goes to / comes from based on bp->b_un.b_addr, bp->b_xmem,
 *	and bp->seccnt.
 */
rx2addr(bp, addr, xmem)
register struct buf *bp;
register char **addr, **xmem;
{
	*addr = bp->b_un.b_addr + bp->seccnt * NBPS;
	*xmem = bp->b_xmem;
	if (*addr < bp->b_un.b_addr)			/* overflow, bump xmem */
		(*xmem)++;
}


rx2read(dev)
{
	physio(rx2strategy, &rrx2buf, dev, B_READ, minor(dev)&2 ? 1001 : 500);
}


rx2write(dev)
{
	physio(rx2strategy, &rrx2buf, dev, B_WRITE, minor(dev)&2 ? 1001 : 500);
}


#ifdef CTRL
/*
 *	rx2sgtty -- format RX02 disk, single or double density.
 *	stty with word 0 == 010 does format.  density determined
 *	by device opened.
 */
rx2ioctl(dev, cmd, addr, flag)
{
	register struct buf *bp;
	struct rx2iocb {
		int	ioc_cmd;	/* command */
		int	ioc_res1;	/* reserved */
		int	ioc_res2;	/* reserved */
	} iocb;

	if (cmd != TIOCSETP) {
err:
		u.u_error = ENXIO;
		return(0);
	}
	if (copyin(addr, (caddr_t)&iocb, sizeof (iocb))) {
		u.u_error = EFAULT;
		return(1);
	}
	if (iocb.ioc_cmd != FORMAT)
		goto err;
	bp = &crx2buf;
	spl6();
	while (bp->b_flags & B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
	bp->b_flags = B_BUSY | B_CTRL;
	bp->b_dev = dev;
	bp->b_error = 0;
	rx2strategy(bp);
	iowait(bp);
	bp->b_flags = 0;
}
#endif
