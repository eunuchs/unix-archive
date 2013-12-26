/*
 *  RL disk driver
 *
 *	RL driver modified for the standalone shell.
 *	Armando P. Stettner  Digital Equipment Corp.  July, 1980
 */

#include	<sys/param.h>
#include	<sys/inode.h>
#include	"saio.h"

#define	BLKRL1	10240		/* Number of UNIX blocks for an RL01 drive */
#define BLKRL2	20480		/* Number of UNIX blocks for an RL02 drive */
#define RLCYLSZ 10240		/* bytes per cylinder */
#define RLSECSZ 256		/* bytes per sector */

#define RESET 013
#define	RL02TYP	0200	/* drive type bit */
#define STAT 03
#define GETSTAT 04
#define WCOM 012
#define RCOM 014
#define SEEK 06
#define SEEKHI 5
#define SEEKLO 1
#define RDHDR 010
#define IENABLE 0100
#define CRDY 0200
#define OPI 02000
#define CRCERR 04000
#define TIMOUT 010000
#define NXM 020000
#define DE  040000

struct device
{
	int rlcs ,
	rlba ,
	rlda ,
	rlmp ;
} ;

#define RLADDR	((struct device *)0174400)
#define RL_CNT 1

struct 
{
	int	cn[4] ;		/* location of heads for each drive */
	int	type[4] ;	/* parameter dependent upon drive type  (RL01/02) */
	int	com ;		/* read or write command word */
	int	chn ;		/* cylinder and head number */
	unsigned int	bleft ;	/* bytes left to be transferred */
	unsigned int	bpart ;	/* number of bytes transferred */
	int	sn ;		/* sector number */
	union {
		int	w[2] ;
		long	l ;
	} addr ;			/* address of memory for transfer */

}	rl = {-1,-1,-1,-1, -1,-1,-1,-1} ;	/* initialize cn[] and type[] */

rlstrategy(io, func)
register struct iob *io ;
int func ;
{
	int nblocks ;	/* number of UNIX blocks for the drive in question */
	int drive ;
	int dif ;
	int head ;
	int ctr ;


	/*
	 * We must determine what type of drive we are talking to in order 
	 * to determine how many blocks are on the device.  The rl.type[]
	 * array has been initialized with -1's so that we may test first
	 * contact with a particular drive and do this determination only once.
	 * Sorry tjk for this hack.
	 *
	 * RL02 GET STATUS BAND-AID - Fred Canter 10/14/80
	 *
	 * For some unknown reason the RL02 (seems to be
	 * only drive 1) does not return a valid drive status
	 * the first time that a GET STATUS request is issued
	 * for the drive, in fact it can take up to three or more
	 * GET STATUS requests to obtain the correct status.
	 * In order to overcome this "HACK" the driver has been
	 * modified to issue a GET STATUS request, validate the
	 * drive status returned, and then use it to determine the
	 * drive type. If a valid status is not returned after eight
	 * attempts, then an error message is printed.
	 */
	drive = io->i_unit ;
	if (rl.type[drive] < 0)
		{
		ctr = 0;
		do {
		RLADDR->rlda = RESET ;	/* load this register; what a dumb controller */
		RLADDR->rlcs = (drive << 8) | GETSTAT ;	/* set up csr */
		while ((RLADDR->rlcs & CRDY) == 0)	/* wait for it */
			;
		} while (((RLADDR->rlmp & 0177477) != 035) && (++ctr < 8)) ;
		if (ctr >= 8)
			printf("\nCan't get status of RL unit %d\n",drive) ;
		if (RLADDR->rlmp & RL02TYP) 
			rl.type[drive] = BLKRL2 ;	/* drive is RL02 */
		else
			rl.type[drive] = BLKRL1 ;	/* drive RL01 */
		/*
		 * When the device is first touched, find out where the heads are.
		 */
		/* find where the heads are */
		RLADDR->rlcs = (drive << 8) | RDHDR;
		while ((RLADDR->rlcs&CRDY) == 0)
			;
		/*rl.cn[drive] = (RLADDR->rlmp&0177700) >> 6;*/
		rl.cn[drive] = ((RLADDR->rlmp) >> 6) & 01777; /* fix sign bug */
		}
	nblocks = rl.type[drive] ;	/* how many blocks on this drive */
	if(io->i_bn >= nblocks)
		return -1 ;
	rl.chn = io->i_bn/20;
	rl.sn = (io->i_bn%20) << 1;
	rl.bleft = io->i_cc;
	rl.addr.w[0] = segflag & 3;
	rl.addr.w[1] = (int)io->i_ma ;
	rl.com = (drive << 8) ;
	if (func == READ)
		rl.com |= RCOM;
	else
		rl.com |= WCOM;
reading:
	/*
	 * One has to seek an RL head, relativily.
	 */
	dif =(rl.cn[drive] >> 1) - (rl.chn >>1);
	head = (rl.chn & 1) << 4;
	if (dif < 0)
		RLADDR->rlda = (-dif <<7) | SEEKHI | head;
	else
		RLADDR->rlda = (dif << 7) | SEEKLO | head;
	RLADDR->rlcs = (drive << 8) | SEEK;
	rl.cn[drive] = rl.chn; 	/* keep current, our notion of where the heads are */
	if (rl.bleft < (rl.bpart = RLCYLSZ - (rl.sn * RLSECSZ)))
		rl.bpart = rl.bleft;
	while ((RLADDR->rlcs&CRDY) == 0) ;
	RLADDR->rlda = (rl.chn << 6) | rl.sn;
	RLADDR->rlba = rl.addr.w[1];
	RLADDR->rlmp = -(rl.bpart >> 1);
	RLADDR->rlcs = rl.com | rl.addr.w[0] << 4;
	while ((RLADDR->rlcs & CRDY) == 0)	/* wait for completion */
		;
	if (RLADDR->rlcs < 0)	/* check error bit */
		{
		if (RLADDR->rlcs & 040000)	/* Drive error */
			{
			/*
			 * get status from drive
			 */
			RLADDR->rlda = STAT;
			RLADDR->rlcs = (drive << 8) | GETSTAT;
			while ((RLADDR->rlcs & CRDY) == 0)	/* wait for controller */
				;
			}
		printf("Rl disk error: cyl=%d, head=%d, sector=%d, rlcs=%o, rlmp=%o\n",
			rl.chn>>01, rl.chn&01, rl.sn, RLADDR->rlcs, RLADDR->rlmp) ;
		return -1 ;
		}
	/*
	 * Determine if there is more to read to satisfy this request.
	 * This is to compensate for the lacl of spiraling reads.
	 */
	if ((rl.bleft -= rl.bpart) > 0)
		{
		rl.addr.l += rl.bpart ;
		rl.sn = 0 ;
		rl.chn++ ;
		goto reading ;	/* read some more */
		}
	return io->i_cc ;
}
