#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/tty.h"
#include "../h/conf.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/acct.h"

int	nulldev();
int	nodev();
int	tmopen(), tmclose(), tmstrategy();
struct	buf	tmtab;
int	rlstrategy();
struct	buf	rltab;
int	rx2open(), rx2strategy();
struct	buf	rx2tab;
struct	bdevsw	bdevsw[] =
{
	nodev, nodev, nodev, 0, /* rk = 0 */
	nodev, nodev, nodev, 0, /* rp = 1 */
	nodev, nodev, nodev, 0, /* rf = 2 */
	tmopen, tmclose, tmstrategy, &tmtab, 	/* tm = 3 */
	nodev, nodev, nodev, 0, /* tc = 4 */
	nodev, nodev, nodev, 0, /* hs|ml = 5 */
	nodev, nodev, nodev, 0, /* hp = 6 */
	nodev, nodev, nodev, 0, /* ht = 7 */
	nulldev, nulldev, rlstrategy, &rltab,	/* rl = 8 */
	nodev, nodev, nodev, 0, /* hk = 9 */
	nodev, nodev, nodev, 0, /* ts = 10 */
	rx2open, nulldev, rx2strategy, &rx2tab,	/* rx2 = 11 */
	nodev, nodev, nodev, 0, /* hm = 12 */
	0
};

int	klopen(), klclose(), klread(), klwrite(), klioctl();
int	mmread(), mmwrite();
int	tmread(), tmwrite();
int	syopen(), syread(), sywrite(), sysioctl();
int	rlread(), rlwrite();
int	rx2read(), rx2write();

struct	cdevsw	cdevsw[] =
{
	klopen, klclose, klread, klwrite, klioctl, nulldev, 0,	/* console = 0 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* pc = 1 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* lp = 2 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* dc = 3 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* dh = 4 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* dp = 5 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* dj = 6 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* dn = 7 */
	nulldev, nulldev, mmread, mmwrite, nodev, nulldev, 0, 	/* mem = 8 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* rk = 9 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* rf = 10 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* rp = 11 */
	tmopen, tmclose, tmread, tmwrite, nodev, nulldev, 0,	/* tm = 12 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* hs|ml = 13 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* hp = 14 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* ht = 15 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* du = 16 */
	syopen, nulldev, syread, sywrite, sysioctl, nulldev, 0,	/* tty = 17 */
	nulldev, nulldev, rlread, rlwrite, nodev, nulldev, 0,	/* rl = 18 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* hk = 19 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* ts = 20 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* dz = 21 */
	rx2open, nulldev, rx2read, rx2write, nodev, nulldev, 0,	/* rx2 = 22 */
	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* hm = 23 */
	0
};

int	ttyopen(), ttyclose(), ttread(), ttwrite(), ttyinput(), ttstart();
struct	linesw	linesw[] =
{
	ttyopen, nulldev, ttread, ttwrite, nodev, ttyinput, ttstart, /* 0 */
	0
};
int	rootdev	= makedev(8, 0);
int	swapdev	= makedev(8, 0);
int	pipedev = makedev(8, 0);
int	nldisp = 1;
daddr_t	swplo	= 18000;
int	nswap	= 2480;
	
struct	buf	buf[NBUF];
struct	file	file[NFILE];
struct	inode	inode[NINODE];
#ifdef	MX
int	mpxchan();
int	(*ldmpx)() = mpxchan;
#endif	MX
struct	proc	proc[NPROC];
struct	text	text[NTEXT];
struct	buf	bfreelist;
struct	acct	acctbuf;
struct	inode	*acctp;

/*
 * The following locations are used by commands
 * like ps & pstat to free them from param.h
 */

int	nproc	NPROC;
int	ninode	NINODE;
int	ntext	NTEXT;
int	nofile	NOFILE;
int	nsig	NSIG;
int	nfile	NFILE;
