/*
 *	SCCS id	%W% (Berkeley)	%G%
 */

#include	"param.h"
#include	<sys/systm.h>
#include	<sys/buf.h>
#include	<sys/tty.h>
#include	<sys/conf.h>
#include	<sys/proc.h>
#include	<sys/text.h>
#include	<sys/dir.h>
#include	<sys/user.h>
#include	<sys/file.h>
#include	<sys/inode.h>
#include	<sys/acct.h>
#include	<sys/map.h>
#include	<sys/filsys.h>
#include	<sys/mount.h>


int	nulldev();
int	nodev();
int	nullioctl();

#include	"bk.h"
#if	NBK > 0
int	bkopen(), bkclose(), bkread(), bkioctl(), bkinput();
#else
#define	bkopen		nodev
#define	bkclose		nodev
#define	bkread		nodev
#define	bkioctl		nodev
#define	bkinput		nodev
#endif	NBK

#include	"dh.h"
#if	NDH > 0
int	dhopen(), dhclose(), dhread(), dhwrite(), dhioctl(), dhstop();
extern	struct	tty	dh11[];
#else
#define	dhopen		nodev
#define	dhclose		nodev
#define	dhread		nodev
#define	dhwrite		nodev
#define	dhioctl		nodev
#define	dhstop		nodev
#define	dh11		((struct tty *) NULL)
#endif	NDH

#define	dnopen		nodev
#define	dnclose		nodev
#define	dnwrite		nodev

#include	"dz.h"
#if	NDZ > 0
int	dzopen(), dzclose(), dzread(), dzwrite(), dzioctl();
#ifdef	DZ_PDMA
int	dzstop();
#else
#define	dzstop		nulldev
#endif
extern	struct	tty	dz11[];
#else
#define	dzopen		nodev
#define	dzclose		nodev
#define	dzread		nodev
#define	dzwrite		nodev
#define	dzioctl		nodev
#define dzstop		nodev
#define	dz11		((struct tty *) NULL)
#endif	NDZ

#include	"cn.h"
#if	NCN > 0
int	cnopen(), cnclose(), cnread(), cnwrite(), cnioctl();
extern	struct	tty	cn11[];
#define cnstop		nodev
#else
#define	cnopen		nodev
#define	cnclose		nodev
#define	cnread		nodev
#define	cnwrite		nodev
#define	cnioctl		nodev
#define	cnstop		nodev
#define	cn11		((struct tty *) NULL)
#endif	NCN

#include	"pc.h"
#if	NPC > 0
int	pcopen(), pcclose(), pcread(), pcwrite(), pcioctl();
extern	struct	tty	pc11[];
#define pcstop		nodev
#else
#define	pcopen		nodev
#define	pcclose		nodev
#define	pcread		nodev
#define	pcwrite		nodev
#define	pcioctl		nodev
#define	pcstop		nodev
#define	pc11		((struct tty *) NULL)
#endif	NPC

#include	"hk.h"
#if	NHK > 0
int	hkstrategy(), hkread(), hkwrite(), hkroot();
extern	struct	buf	hktab;
#define	hkopen		nulldev
#define	hkclose		nulldev
#define	_hktab		&hktab
#else
#define	hkopen		nodev
#define	hkclose		nodev
#define hkroot		nulldev
#define	hkstrategy	nodev
#define	hkread		nodev
#define	hkwrite		nodev
#define	_hktab		((struct buf *) NULL)
#endif	NHK

#include	"ra.h"
#if	NRA > 0
int	rastrategy(), raread(), rawrite(), raopen(), raroot();
extern	struct	buf	ratab;
#define	raclose		nulldev
#define	_ratab		&ratab
#else
#define	raopen		nodev
#define	raclose		nodev
#define	rastrategy	nodev
#define	raread		nodev
#define	rawrite		nodev
#define raroot		nulldev
#define	_ratab		((struct buf *) NULL)
#endif	NRA

#include	"rd.h"
#if	NRD > 0
int	rdstrategy(), rdread(), rdwrite(), rdopen(), rdroot();
extern	struct	buf	rdtab;
#define	rdclose		nulldev
#define	_rdtab		&rdtab
#else
#define	rdopen		nodev
#define	rdclose		nodev
#define	rdstrategy	nodev
#define	rdread		nodev
#define	rdwrite		nodev
#define rdroot		nulldev
#define	_rdtab		((struct buf *) NULL)
#endif	NRD

#include	"r5.h"
#if	NR5 > 0
int	r5strategy(), r5read(), r5write(), r5open(), r5close(), r5root();
int	r5ioctl();
extern	struct	buf	r5tab;
#define	_r5tab		&r5tab
#else
#define	r5open		nodev
#define	r5close		nodev
#define	r5strategy	nodev
#define	r5read		nodev
#define	r5write		nodev
#define r5ioctl		nodev
#define r5root		nulldev
#define	_r5tab		((struct buf *) NULL)
#endif	NR5

#include	"hp.h"
#if	NHP > 0
int	hpstrategy(), hpread(), hpwrite(), hproot();
extern	struct	buf	hptab;
#define	hpopen		nulldev
#define	hpclose		nulldev
#define	_hptab		&hptab
#else
#define	hpopen		nodev
#define	hpclose		nodev
#define	hproot		nulldev
#define	hpstrategy	nodev
#define	hpread		nodev
#define	hpwrite		nodev
#define	_hptab		((struct buf *) NULL)
#endif	NHP

#define	hsopen		nodev
#define	hsclose		nodev
#define	hsstrategy	nodev
#define	hsread		nodev
#define	hswrite		nodev
#define	hsroot		nulldev
#define	_hstab		((struct buf *) NULL)

#define	htioctl		nodev
#define	htopen		nodev
#define	htclose		nodev
#define	htread		nodev
#define	htwrite		nodev
#define	htioctl		nodev
#define	htstrategy	nodev
#define	_httab		((struct buf *) NULL)

#define	lpopen		nodev
#define	lpclose		nodev
#define	lpwrite		nodev

#include	"pty.h"
#if	NPTY > 0
int	ptsopen(), ptsclose(), ptsread(), ptswrite(), ptsstop();
int	ptcopen(), ptcclose(), ptcread(), ptcwrite();
int	ptyioctl();
extern	struct tty pt_tty[];
#ifdef	UCB_NET
int	ptcselect();
#endif
#else
#define	ptsopen		nodev
#define	ptsclose	nodev
#define	ptsread		nodev
#define	ptswrite	nodev
#define	ptsstop		nodev
#define	ptcopen		nodev
#define	ptcclose	nodev
#define	ptcread		nodev
#define	ptcwrite	nodev
#define	ptyioctl	nodev
#define	ptcselect	nodev
#define	pt_tty		((struct tty *)NULL)
#endif	NPTY

#define	rkopen		nodev
#define	rkclose		nodev
#define	rkstrategy	nodev
#define	rkread		nodev
#define	rkwrite		nodev
#define	_rktab		((struct buf *) NULL)

#define	rlopen		nodev
#define	rlclose		nodev
#define	rlstrategy	nodev
#define	rlread		nodev
#define	rlwrite		nodev
#define	_rltab		((struct buf *) NULL)

#define	rmopen		nodev
#define	rmclose		nodev
#define	rmroot		nulldev
#define	rmstrategy	nodev
#define	rmread		nodev
#define	rmwrite		nodev
#define	_rmtab		((struct buf *) NULL)

#define	rpopen		nodev
#define	rpclose		nodev
#define	rpstrategy	nodev
#define	rpread		nodev
#define	rpwrite		nodev
#define	_rptab		((struct buf *) NULL)

#include	"tm.h"
#if	NTM > 0
int	tmopen(), tmclose(), tmread(), tmwrite(), tmstrategy();
#ifdef	TM_IOCTL
int	tmioctl();
#else
#define	tmioctl		nodev
#endif
extern	struct	buf	tmtab;
#define	_tmtab		&tmtab
#else
#define	tmopen		nodev
#define	tmclose		nodev
#define	tmread		nodev
#define	tmwrite		nodev
#define	tmioctl		nodev
#define	tmstrategy	nodev
#define	_tmtab		((struct buf *) NULL)
#endif	NTM

#define	tsioctl		nodev
#define	tsopen		nodev
#define	tsclose		nodev
#define	tsread		nodev
#define	tswrite		nodev
#define	tsioctl		nodev
#define	tsstrategy	nodev
#define	_tstab		((struct buf *) NULL)

#define	vpopen		nodev
#define	vpclose		nodev
#define	vpwrite		nodev
#define vpioctl		nodev

#include	"xp.h"
#if	NXP > 0
int	xpstrategy(), xpread(), xpwrite(), xproot();
extern	struct	buf	xptab;
#define	xpopen		nulldev
#define	xpclose		nulldev
#define	_xptab		&xptab
#else
#define	xpopen		nodev
#define	xpclose		nodev
#define	xproot		nulldev
#define	xpstrategy	nodev
#define	xpread		nodev
#define	xpwrite		nodev
#define	_xptab		((struct buf *) NULL)
#endif	NXP

struct	bdevsw	bdevsw[] =
{
	rkopen,		rkclose,	rkstrategy,
	nulldev,	_rktab,		/* rk = 0 */

	rpopen,		rpclose,	rpstrategy,
	nulldev,	_rptab,		/* rp = 1 */

	raopen,		raclose,	rastrategy,
	raroot,		_ratab,		/* ra = 2 */

	tmopen,		tmclose,	tmstrategy,
	nulldev,	_tmtab,		/* tm = 3 */

	hkopen,		hkclose,	hkstrategy,
	hkroot,		_hktab,		/* hk = 4 */

	hsopen,		hsclose,	hsstrategy,
	hsroot,		_hstab,		/* hs = 5 */

#if	NXP > 0
	xpopen,		xpclose,	xpstrategy,
	xproot,		_xptab,		/* xp = 6 */
#else

#if	NHP > 0
	hpopen,		hpclose,	hpstrategy,
	hproot,		_hptab,		/* hp = 6 */
#else

	rmopen,		rmclose,	rmstrategy,
	rmroot,		_rmtab,		/* rm = 6 */
#endif
#endif

	htopen,		htclose,	htstrategy,
	nulldev,	_httab,		/* ht = 7 */

	rlopen,		rlclose,	rlstrategy,
	nulldev,	_rltab,		/* rl = 8 */

	tsopen,		tsclose,	tsstrategy,
	nulldev,	_tstab,		/* ts = 9 */

	rdopen,		rdclose,	rdstrategy,
	rdroot,		_rdtab,		/* rd = 10 */

	r5open,		r5close,	r5strategy,
	r5root,		_r5tab,		/* r5 = 11 */
};
int	nblkdev = sizeof(bdevsw) / sizeof(bdevsw[0]);

#include "kl.h"
#if	NKL > 0
int	klopen(), klclose(), klread(), klwrite(), klioctl();
extern	struct	tty	kl11[];
#else
#define klopen	nodev
#define klclose	nodev
#define klread	nodev
#define klwrite	nodev
#define	klioctl	nodev
#define	kl11	((struct tty *) NULL)
#endif	NKL
int	mmread(), mmwrite();
int	syopen(), syread(), sywrite(), sysioctl();

#ifdef UCB_NET
int	syselect();
int	ttselect();
int	seltrue();
#define	SELECT(r)	r,
#else
#define	SELECT(r)
#endif UCB_NET

struct	cdevsw	cdevsw[] =
{
	klopen,		klclose,	klread,		klwrite,
	klioctl,	nulldev,	kl11,		SELECT(ttselect)	/* kl = 0 */

	pcopen,		pcclose,	pcread,		pcwrite,
	pcioctl,	nulldev,	pc11,		SELECT(ttselect) /* pc =1 */

	vpopen,		vpclose,	nodev,		vpwrite,
	vpioctl,	nulldev,	0,		SELECT(nodev)		/* vp = 2 */

	lpopen,		lpclose,	nodev,		lpwrite,
	nodev,		nulldev,	0,		SELECT(nodev)		/* lp = 3 */

	dhopen,		dhclose,	dhread,		dhwrite,
	dhioctl,	dhstop,		dh11,		SELECT(ttselect)	/* dh = 4 */

	cnopen,		cnclose,	cnread,		cnwrite,
	cnioctl,	nulldev,	cn11,		SELECT(ttselect)

	raopen,		raclose,	raread,		rawrite,
	nodev,		nulldev,	0,		SELECT(seltrue)

	dnopen,		dnclose,	nodev,		dnwrite,
	nodev,		nulldev,	0,		SELECT(nodev)		/* dn = 7 */

	nulldev,	nulldev,	mmread,		mmwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* mem = 8 */

	rkopen,		rkclose,	rkread,		rkwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* rk = 9 */

	rdopen,		rdclose,	rdread,		rdwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)

	rpopen,		rpclose,	rpread,		rpwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* rp = 11 */

	tmopen,		tmclose,	tmread,		tmwrite,
	tmioctl,	nulldev,	0,		SELECT(seltrue)		/* tm = 12 */

	hsopen,		hsclose,	hsread,		hswrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* hs = 13 */

#if	NXP > 0
	xpopen,		xpclose,	xpread,		xpwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* xp = 14 */
#else
	rmopen,		rmclose,	rmread,		rmwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* rm = 14 */
#endif

#if	NHP > 0
	hpopen,		hpclose,	hpread,		hpwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* hp = 15 */
#else
	htopen,		htclose,	htread,		htwrite,
	htioctl,	nulldev,	0,		SELECT(seltrue)		/* ht = 15 */
#endif

	r5open,		r5close,	r5read,		r5write,
	nodev,		nulldev,	0,		SELECT(seltrue)

	syopen,		nulldev,	syread,		sywrite,
	sysioctl,	nulldev,	0,		SELECT(syselect)	/* tty = 17 */

	rlopen,		rlclose,	rlread,		rlwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* rl = 18 */

	hkopen,		hkclose,	hkread,		hkwrite,
	nodev,		nulldev,	0,		SELECT(seltrue)		/* hk = 19 */

	tsopen,		tsclose,	tsread,		tswrite,
	tsioctl,	nulldev,	0,		SELECT(seltrue)		/* ts = 20 */

	dzopen,		dzclose,	dzread,		dzwrite,
	dzioctl,	dzstop,		dz11,		SELECT(ttselect)	/* dz = 21 */

	ptcopen,	ptcclose,	ptcread,	ptcwrite,
	ptyioctl,	nulldev,	pt_tty,		SELECT(ptcselect)	/* ptc= 22 */

	ptsopen,	ptsclose,	ptsread,	ptswrite,
	ptyioctl,	ptsstop,	pt_tty,		SELECT(ttselect)	/* pts= 23 */
};

int	nchrdev = sizeof(cdevsw) / sizeof(cdevsw[0]);

#ifdef	OLDTTY
int	ttread(), ttyinput(), ttyoutput();
caddr_t	ttwrite();
#define	ttopen		nulldev
#define	ttclose		nulldev
#define	ttioctl		nullioctl
#define	ttmodem		nulldev
#else
#define	ttopen		nodev
#define	ttclose		nodev
#define	ttread		nodev
#define	ttwrite		nodev
#define	ttioctl		nodev
#define	ttyinput	nodev
#define	ttyoutput	nodev
#define	ttmodem		nodev
#endif


#ifdef	UCB_NTTY
int	ntyopen(), ntyclose(), ntread(), ntyinput(), ntyoutput();
caddr_t	ntwrite();
#define	ntyioctl	nullioctl
#define	ntymodem	nulldev
#else
#define	ntyopen		nodev
#define	ntyclose	nodev
#define	ntread		nodev
#define	ntwrite		nodev
#define	ntyioctl	nodev
#define	ntyinput	nodev
#define	ntyoutput	nodev
#define	ntymodem	nodev
#endif

struct	linesw linesw[] =
{
	ttopen,		ttclose,	ttread,		ttwrite,
	ttioctl,	ttyinput,	ttyoutput,	ttmodem,	/*0*/

	ntyopen,	ntyclose,	ntread,		ntwrite,
	ntyioctl,	ntyinput,	ntyoutput,	ntymodem,	/*1*/

#if	NBK > 0
	bkopen,		bkclose,	bkread,		ttwrite,
	bkioctl,	bkinput,	nodev,		nulldev		/*2*/
#endif
};

#ifndef	MPX_FILS
int	nldisp	= sizeof(linesw) / sizeof(linesw[0]);
#else
int	nldisp	= sizeof(linesw) / sizeof(linesw[0]) - 1;
int	mpxchan();
int	(*ldmpx)()	= mpxchan;
#endif
