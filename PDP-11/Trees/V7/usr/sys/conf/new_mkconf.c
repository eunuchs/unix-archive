/*
 * UNIX/v7m make configuration program (mkconf.c).
 * Creates the files l.s, locore vectors and c.c, 
 * configuration table.
 * Vector addreses are assigned according to the fixed
 * and floating vector rules, if non standard
 * vectors exist the l.s file must be edited after the
 * mkconf program has been run.
 *
 * Added rk06 & rk07 support.
 * Added rm02/3 & ts11 support.
 * Added A. P. Stettner's rl11 modificatons.
 * Added dz11 support.
 * Added improved core dump code,
 * uses unibus map (if requried) to
 * insure that all of memory is dumpped.
 * Added "ov" declaration for overlay kernel.
 * Added "nsid" declaration for no separate I/D space CPU.
 * Added "nfp" declaration for excluding floating point support code.
 * Added "|" for rx|rx2 and hs|ml.
 * Added automatic configuration of the text
 * overlay segments for the overlay kernel.
 * Added "dump" declaration to select core dump tape
 * and optionally specify dump tape CSR address.
 *
 * Fred Canter 8/20/81
 *
 ********************************************************
 *							*
 * The TS11 vector is assigned to location 050, in      *
 * order that the TS11 can function on a system which   *
 * also has a TM11, see comments at the head of the     *
 * TS11 driver "/sys/dev/ts.c" for an explaination.     *
 *							*
 * The startup code copies the TS11 vector from 050 to  *
 * 0224 or 0260 depending on the configuration.         *
 *							*
 ********************************************************
 */

#include <stdio.h>

#define	HT	01
#define	TM	02
#define	TS	03
#define CHAR	01
#define BLOCK	02
#define INTR	04
#define EVEN	010
#define KL	020
#define ROOT	040
#define	SWAP	0100
#define	PIPE	0200

char	*btab[] =
{
	"rk",
	"rp",
	"rf",
	"tm",
	"tc",
	"hs|ml",
	"hp",
	"ht",
	"rl",
	"hk",
	"ts",
	"rx|rx2",
	"hm",
	0
};
char	*ctab[] =
{
	"console",
	"pc",
	"lp",
	"dc",
	"dh",
	"dp",
	"dj",
	"dn",
	"mem",
	"rk",
	"rf",
	"rp",
	"tm",
	"hs|ml",
	"hp",
	"ht",
	"du",
	"tty",
	"rl",
	"hk",
	"ts",
	"dz",
	"rx2",
	"hm",
	0
};
struct tab
{
	char	*name;
	int	count;
	int	address;
	int	key;
	char	*codea;
	char	*codeb;
	char	*codec;
	char	*coded;
	char	*codee;
	char	*codef;
	char	*codeg;
} table[] =
{
	"ts",
	0, 50, BLOCK+CHAR+INTR,
	"	tsio; br5\n",
	".globl	_tsintr\n",
	"tsio:	jsr	r0,call; jmp _tsintr\n",
	"	tsopen, tsclose, tsstrategy, &tstab,",
	"	tsopen, tsclose, tsread, tswrite, nodev, nulldev, 0,",
	"int	tsopen(), tsclose(), tsstrategy();\nstruct	buf	tstab;",
	"int	tsread(), tswrite();",

	"console",
	-1, 60, CHAR+INTR+KL,
	"	klin; br4\n	klou; br4\n",
	".globl	_klrint\nklin:	jsr	r0,call; jmp _klrint\n",
	".globl	_klxint\nklou:	jsr	r0,call; jmp _klxint\n",
	"",
	"	klopen, klclose, klread, klwrite, klioctl, nulldev, 0,",
	"",
	"int	klopen(), klclose(), klread(), klwrite(), klioctl();",

	"mem",
	-1, 300, CHAR,
	"",
	"",
	"",
	"",
	"	nulldev, nulldev, mmread, mmwrite, nodev, nulldev, 0, ",
	"",
	"int	mmread(), mmwrite();",

	"pc",
	0, 70, CHAR+INTR,
	"	pcin; br4\n	pcou; br4\n",
	".globl	_pcrint\npcin:	jsr	r0,call; jmp _pcrint\n",
	".globl	_pcpint\npcou:	jsr	r0,call; jmp _pcpint\n",
	"",
	"	pcopen, pcclose, pcread, pcwrite, nodev, nulldev, 0, ",
	"",
	"int	pcopen(), pcclose(), pcread(), pcwrite();",

	"clock",
	-2, 100, INTR,
	"	kwlp; br6\n",
	".globl	_clock\n",
	"kwlp:	jsr	r0,call; jmp _clock\n",
	"",
	"",
	"",
	"",

	"parity",
	-1, 114, INTR,
	"	trap; br7+10.		/ 11/70 parity\n",
	"",
	"",
	"",
	"",
	"",
	"",

/*
 * 110 unused
 * 114 memory parity
 * 120 XY plotter
 * 124 DR11-B
 * 130 AD01
 * 134 AFC11
 * 140 AA11
 * 144 AA11
 */

	"hm",
	0, 150, BLOCK+CHAR+INTR,
	"	hmio; br5\n",
	".globl	_hmintr\n",
	"hmio:	jsr	r0,call; jmp _hmintr\n",
	"	nulldev, nulldev, hmstrategy, &hmtab,",
	"	nulldev, nulldev, hmread, hmwrite, nodev, nulldev, 0,",
	"int	hmstrategy();\nstruct	buf	hmtab;",
	"int	hmread(), hmwrite();",

/*
 * 154 unused
 */

	"rl",
	0, 160, BLOCK+CHAR+INTR,
	"	rlio; br5\n",
	".globl	_rlintr\n",
	"rlio:	jsr	r0,call; jmp _rlintr\n",
	"	nulldev, nulldev, rlstrategy, &rltab,",
	"	nulldev, nulldev, rlread, rlwrite, nodev, nulldev, 0,",
	"int	rlstrategy();\nstruct	buf	rltab;",
	"int	rlread(), rlwrite();",

/*
 * 164-174 unused
 */

	"lp",
	0, 200, CHAR+INTR,
	"	lpou; br4\n",
	"",
	".globl	_lpintr\nlpou:	jsr	r0,call; jmp _lpintr\n",
	"",
	"	lpopen, lpclose, nodev, lpwrite, nodev, nulldev, 0,",
	"",
	"int	lpopen(), lpclose(), lpwrite();",

	"rf",
	0, 204, BLOCK+CHAR+INTR,
	"	rfio; br5\n",
	".globl	_rfintr\n",
	"rfio:	jsr	r0,call; jmp _rfintr\n",
	"	nulldev, nulldev, rfstrategy, &rftab, ",
	"	nulldev, nulldev, rfread, rfwrite, nodev, nulldev, 0,",
	"int	rfstrategy();\nstruct	buf	rftab;",
	"int	rfread(), rfwrite();",

	"hs",
	0, 204, BLOCK+CHAR+INTR,
	"	hsio; br5\n",
	".globl	_hsintr\n",
	"hsio:	jsr	r0,call; jmp _hsintr\n",
	"	nulldev, nulldev, hsstrategy, &hstab, ",
	"	nulldev, nulldev, hsread, hswrite, nodev, nulldev, 0,",
	"int	hsstrategy();\nstruct	buf	hstab;",
	"int	hsread(), hswrite();",

	"ml",
	0, 204, BLOCK+CHAR+INTR,
	"	mlio; br5\n",
	".globl	_mlintr\n",
	"mlio:	jsr	r0,call; jmp _mlintr\n",
	"	nulldev, nulldev, mlstrategy, &mltab, ",
	"	nulldev, nulldev, mlread, mlwrite, nodev, nulldev, 0,",
	"int	mlstrategy();\nstruct	buf	mltab;",
	"int	mlread(), mlwrite();",

/*
 * 210 RC
 */

	"hk",
	0, 210, BLOCK+CHAR+INTR,
	"	hkio; br5\n",
	".globl	_hkintr\n",
	"hkio:	jsr	r0,call; jmp _hkintr\n",
	"	nulldev, nulldev, hkstrategy, &hktab,",
	"	nulldev, nulldev, hkread, hkwrite, nodev, nulldev, 0,",
	"int	hkstrategy();\nstruct	buf	hktab;",
	"int	hkread(), hkwrite();",

	"tc",
	0, 214, BLOCK+INTR,
	"	tcio; br6\n",
	".globl	_tcintr\n",
	"tcio:	jsr	r0,call; jmp _tcintr\n",
	"	nulldev, tcclose, tcstrategy, &tctab,",
	"",
	"int	tcstrategy(), tcclose();\nstruct	buf	tctab;",
	"",

	"rk",
	0, 220, BLOCK+CHAR+INTR,
	"	rkio; br5\n",
	".globl	_rkintr\n",
	"rkio:	jsr	r0,call; jmp _rkintr\n",
	"	nulldev, nulldev, rkstrategy, &rktab,",
	"	nulldev, nulldev, rkread, rkwrite, nodev, nulldev, 0,",
	"int	rkstrategy();\nstruct	buf	rktab;",
	"int	rkread(), rkwrite();",

	"tm",
	0, 224, BLOCK+CHAR+INTR,
	"	tmio; br5\n",
	".globl	_tmintr\n",
	"tmio:	jsr	r0,call; jmp _tmintr\n",
	"	tmopen, tmclose, tmstrategy, &tmtab, ",
	"	tmopen, tmclose, tmread, tmwrite, nodev, nulldev, 0,",
	"int	tmopen(), tmclose(), tmstrategy();\nstruct	buf	tmtab;",
	"int	tmread(), tmwrite();",

	"ht",
	0, 224, BLOCK+CHAR+INTR,
	"	htio; br5\n",
	".globl	_htintr\n",
	"htio:	jsr	r0,call; jmp _htintr\n",
	"	htopen, htclose, htstrategy, &httab,",
	"	htopen, htclose, htread, htwrite, nodev, nulldev, 0,",
	"int	htopen(), htclose(), htstrategy();\nstruct	buf	httab;",
	"int	htread(), htwrite();",

	"cr",
	0, 230, CHAR+INTR,
	"	crin; br6\n",
	"",
	".globl	_crint\ncrin:	jsr	r0,call; jmp _crint\n",
	"",
	"	cropen, crclose, crread, nodev, nodev, nulldev, 0,",
	"",
	"int	cropen(), crclose(), crread();",

/*
 * 234 UDC11
 */

	"rp",
	0, 254, BLOCK+CHAR+INTR,
	"	rpio; br5\n",
	".globl	_rpintr\n",
	"rpio:	jsr	r0,call; jmp _rpintr\n",
	"	nulldev, nulldev, rpstrategy, &rptab,",
	"	nulldev, nulldev, rpread, rpwrite, nodev, nulldev, 0,",
	"int	rpstrategy();\nstruct	buf	rptab;",
	"int	rpread(), rpwrite();",

	"hp",
	0, 254, BLOCK+CHAR+INTR,
	"	hpio; br5\n",
	".globl	_hpintr\n",
	"hpio:	jsr	r0,call; jmp _hpintr\n",
	"	nulldev, nulldev, hpstrategy, &hptab,",
	"	nulldev, nulldev, hpread, hpwrite, nodev, nulldev, 0,",
	"int	hpstrategy();\nstruct	buf	hptab;",
	"int	hpread(), hpwrite();",

/*
 * 260 TA11 (alt TS11)
 */

	"rx",
	0, 264, BLOCK+INTR,
	"	rxio; br5\n",
	".globl	_rxintr\n",
	"rxio:	jsr	r0,call; jmp _rxintr\n",
	"	rxopen, nulldev, rxstrategy, &rxtab,",
	"",
	"int	rxopen(), rxstrategy();\nstruct buf	rxtab;",
	"",

	"rx2",
	0, 264, BLOCK+CHAR+INTR,
	"	rx2io; br5\n",
	".globl	_rx2intr\n",
	"rx2io:	jsr	r0,call; jmp _rx2intr\n",
	"	rx2open, nulldev, rx2strategy, &rx2tab,",
	"	rx2open, nulldev, rx2read, rx2write, nodev, nulldev, 0,",
	"int	rx2open(), rx2strategy();\nstruct	buf	rx2tab;",
	"int	rx2read(), rx2write();",

	"dc",
	0, 308, CHAR+INTR,
	"	dcin; br5+%d.\n	dcou; br5+%d.\n",
	".globl	_dcrint\ndcin:	jsr	r0,call; jmp _dcrint\n",
	".globl	_dcxint\ndcou:	jsr	r0,call; jmp _dcxint\n",
	"",
	"	dcopen, dcclose, dcread, dcwrite, dcioctl, nulldev, dc11,",
	"",
	"int	dcopen(), dcclose(), dcread(), dcwrite(), dcioctl();\nstruct	tty	dc11[];",

	"kl",
	0, 308, INTR+KL,
	"	klin; br4+%d.\n	klou; br4+%d.\n",
	"",
	"",
	"",
	"",
	"",
	"",

	"dp",
	0, 308, CHAR+INTR,
	"	dpin; br6+%d.\n	dpou; br6+%d.\n",
	".globl	_dprint\ndpin:	jsr	r0,call; jmp _dprint\n",
	".globl	_dpxint\ndpou:	jsr	r0,call; jmp _dpxint\n",
	"",
	"	dpopen, dpclose, dpread, dpwrite, nodev, nulldev, 0,",
	"",
	"int	dpopen(), dpclose(), dpread(), dpwrite();",

/*
 * DM11-A
 */

	"dn",
	0, 304, CHAR+INTR,
	"	dnou; br5+%d.\n",
	"",
	".globl	_dnint\ndnou:	jsr	r0,call; jmp _dnint\n",
	"",
	"	dnopen, dnclose, nodev, dnwrite, nodev, nulldev, 0,",
	"",
	"int	dnopen(), dnclose(), dnwrite();",

	"dhdm",
	0, 304, INTR,
	"	dmin; br4+%d.\n",
	"",
	".globl	_dmint\ndmin:	jsr	r0,call; jmp _dmint\n",
	"",
	"",
	"",
	"",

/*
 * DR11-A+
 * DR11-C+
 * PA611+
 * PA611+
 * DT11+
 * DX11+
 */

	"dl",
	0, 308, INTR+KL,
	"	klin; br4+%d.\n	klou; br4+%d.\n",
	"",
	"",
	"",
	"",
	"",
	"",

/*
 * DJ11
 */

	"dh",
	0, 308, CHAR+INTR+EVEN,
	"	dhin; br5+%d.\n	dhou; br5+%d.\n",
	".globl	_dhrint\ndhin:	jsr	r0,call; jmp _dhrint\n",
	".globl	_dhxint\ndhou:	jsr	r0,call; jmp _dhxint\n",
	"",
	"	dhopen, dhclose, dhread, dhwrite, dhioctl, dhstop, dh11,",
	"",
	"int	dhopen(), dhclose(), dhread(), dhwrite(), dhioctl(), dhstop();\nstruct	tty	dh11[];",

/*
 * GT40
 * LPS+
 * DQ11
 * KW11-W
 */

	"du",
	0, 308, CHAR+INTR,
	"	duin; br6+%d.\n	duou; br6+%d.\n",
	".globl	_durint\nduin:	jsr	r0,call; jmp _durint\n",
	".globl	_duxint\nduou:	jsr	r0,call; jmp _duxint\n",
	"",
	"	duopen, duclose, duread, duwrite, nodev, nulldev, 0,",
	"",
	"int	duopen(), duclose(), duread(), duwrite();",

/*
 * DUP11
 * DV11
 * LK11-A
 * DMC11
 */

	"dz",
	0, 308, CHAR+INTR+EVEN,
	"	dzin; br5+%d.\n	dzou; br5+%d.\n",
	".globl _dzrint\ndzin:	jsr	r0,call; jmp _dzrint\n",
	".globl _dzxint\ndzou:	jsr	r0,call; jmp _dzxint\n",
	"",
	"	dzopen, dzclose, dzread, dzwrite, dzioctl, nulldev, dz_tty,",
	"",
	"int	dzopen(), dzclose(), dzread(), dzwrite(), dzioctl();\nstruct	tty	dz_tty[];",

	"tty",
	1, 0, CHAR,
	"",
	"",
	"",
	"",
	"	syopen, nulldev, syread, sywrite, sysioctl, nulldev, 0,",
	"",
	"int	syopen(), syread(), sywrite(), sysioctl();",

	0
};

/*
 * The ovtab structure describes each of the modules
 * used to make the overlay text segments of the
 * overlay kernel.
 * The order of appearance of the modules in this
 * structure is critical and must not be changed.
 */

struct	ovtab
{
	char	*mn;		/* object module name */
	int	mc;		/* non-zero if module is configured */
				/* some modules are always configured, */
				/* others are keyed from COUNT */
				/* field of structure "table" above */
	int	ovno;		/* initial overlay number, */
				/* may be changed later */
	int	mts;		/* module text size in bytes */
	char	*mpn;		/* module full path name */
} ovt [] =
{
/*
 *	overlay 1
 * Mostly system calls, will be added to later.
 * Mem driver is about the only thing that
 * will fit.
 * This overlay should be as full as possible,
 * so as not to waste memory space.
 */
	"pipe",
	1, 1, -1,
	"\t../ovsys/pipe.o \\",
	"sys1",
	1, 1, -1,
	"\t../ovsys/sys1.o \\",
	"sys2",
	1, 1, -1,
	"\t../ovsys/sys2.o \\",
	"sys3",
	1, 1, -1,
	"\t../ovsys/sys3.o \\",
	"sys4",
	1, 1, -1,
	"\t../ovsys/sys4.o \\",
	"text",
	1, 1, -1,
	"\t../ovsys/text.o \\",
/*
 *	overlay 2
 * This overlay contains some system stuff
 * plus bio, will be filled in with tape drivers
 * and what ever else will fit.
 */
	"acct",
	1, 2, -1,
	"\t../ovsys/acct.o \\",
	"main",
	1, 2, -1,
	"\t../ovsys/main.o \\",
	"machdep",
	1, 2, -1,
	"\t../ovsys/machdep.o \\",
	"sig",
	1, 2, -1,
	"\t../ovsys/sig.o \\",
	"bio",
	1, 2, -1,
	"\t../ovdev/bio.o \\",
/*
 *	overlay 3
 * This overlay has the big disk drivers and the
 * disk sort routines. These drivers may not be
 * configured, but in any case this overlay will
 * get as much as possible of the overflow from
 * overlays 1 & 2.
 *
 * If all three disk drivers are configured (hp, hk, & hm)
 * this overlay would overflow, in that case "hm" is
 * changed to overlay 8 on the fly and loaded where ever it
 * will fit on the next pass.
 */
	"hp",
	0, 3, -1,
	"\t../ovdev/hp.o \\",
	"hk",
	0, 3, -1,
	"\t../ovdev/hk.o \\",
	"hm",
	0, 3, -1,
	"\t../ovdev/hm.o \\",
	"dsort",
	0, 3, -1,
	"\t../ovdev/dsort.o \\",
	"dkleave",
	0, 3, -1,
	"\t../ovdev/dkleave.o \\",

/*
 *	overlay 4
 * Initially empty, will be filled in with
 * overflow from previous overlays.
 */

/*
 *	overlay 5
 * The overlay holds most of the tty drivers and
 * associated routines, not much room for fill.
 */
	"tty",
	1, 5, -1,
	"\t../ovdev/tty.o \\",
	"sys",
	1, 5, -1,
	"\t../ovdev/sys.o \\",
	"kl",
	1, 5, -1,
	"\t../ovdev/kl.o \\",
	"dz",
	0, 5, -1,
	"\t../ovdev/dz.o \\",
	"dhdm",
	0, 5, -1,
	"\t../ovdev/dhdm.o \\",
	"dh",
	0, 5, -1,
	"\t../ovdev/dh.o \\",
	"dhfdm",
	0, 5, -1,
	"\t../ovdev/dhfdm.o \\",
	"partab",
	1, 5, -1,
	"\t../ovdev/partab.o \\",
	"prim",
	1, 5, -1,
	"\t../ovsys/prim.o \\",
/*
 *	overlay 6
 * Contains the packet driver stuff.
 * It could hold overflow from previous
 * overlays if necessary.
 */
	"pk0",
	0, 6, -1,
	"\t../ovdev/pk0.o \\",
	"pk1",
	0, 6, -1,
	"\t../ovdev/pk1.o \\",
	"pk2",
	0, 6, -1,
	"\t../ovdev/pk2.o \\",
	"pk3",
	0, 6, -1,
	"\t../ovdev/pk3.o \\",
/*
 *	overlay 7
 * Contains the multiplexed files stuff.
 * could also hold overflow.
 */
	"mx1",
	0, 7, -1,
	"\t../ovdev/mx1.o \\",
	"mx2",
	0, 7, -1,
	"\t../ovdev/mx2.o \\",
/*
 *	overlay 8
 * This is not a real overlay, all of the
 * modules here will be used to fill out
 * overlays 1 thru 7.
 * Contains the mem driver, the HM disk driver,
 * and the magtape drivers.
 */
	"mem",
	1, 8, -1,
	"\t../ovdev/mem.o \\",
	"ts",
	0, 8, -1,
	"\t../ovdev/ts.o \\",
	"ht",
	0, 8, -1,
	"\t../ovdev/ht.o \\",
	"tm",
	0, 8, -1,
	"\t../ovdev/tm.o \\",
	"tc",
	0, 8, -1,
	"\t../ovdev/tc.o \\",
/*
 *	overlay 9
 * Again not a real overlay, used to fill others.
 * Contains all smaller disk drivers.
 */
	"ml",
	0, 9, -1,
	"\t../ovdev/ml.o \\",
	"hs",
	0, 9, -1,
	"\t../ovdev/hs.o \\",
	"rp",
	0, 9, -1,
	"\t../ovdev/rp.o \\",
	"rx2",
	0, 9, -1,
	"\t../ovdev/rx2.o \\",
	"rl",
	0, 9, -1,
	"\t../ovdev/rl.o \\",
	"rk",
	0, 9, -1,
	"\t../ovdev/rk.o \\",
	"rf",
	0, 9, -1,
	"\t../ovdev/rf.o \\",
/*
 *	overlay 10
 * Also not a real overlay, used to fill others.
 * Contains LP driver and misc. comm. device drivers.
 */
	"lp",
	0, 10, -1,
	"\t../ovdev/lp.o \\",
	"du",
	0, 10, -1,
	"\t../ovdev/du.o \\",
	"dn",
	0, 10, -1,
	"\t../ovdev/dn.o \\",
	"dc",
	0, 10, -1,
	"\t../ovdev/dc.o \\",
	"cat",
	0, 10, -1,
	"\t../ovdev/cat.o \\",
	"vp",
	0, 10, -1,
	"\t../ovdev/vp.o \\",
	"vs",
	0, 10, -1,
	"\t../ovdev/vs.o \\",
	0
};

/*
 * The ovdtab is an array of structures which
 * describe the actual overlays as they will
 * appear in the "ovload" overlay load file.
 * The first structure (overlay 0) is never used.
 */

struct	ovdes
{
	int	nentry;		/* number of modules in this overlay */
	int	size;		/* total size of this overlay in bytes */
	char	*omns[12];	/* pointers to module pathname strings */
} ovdtab [8];

char	*stra40[] =
{
	"/ low core",
	"",
	0
};

char	*stra70[] =
{
	"/ low core",
	"",
	".data",
	0
};

char	*stra[] =
{
	"ZERO:",
	"",
	"br4 = 200",
	"br5 = 240",
	"br6 = 300",
	"br7 = 340",
	"",
	". = ZERO+0",
	"	br	1f",
	"	4",
	"",
	"/ trap vectors",
	"	trap; br7+0.		/ bus error",
	"	trap; br7+1.		/ illegal instruction",
	"	trap; br7+2.		/ bpt-trace trap",
	"	trap; br7+3.		/ iot trap",
	"	trap; br7+4.		/ power fail",
	"	trap; br7+5.		/ emulator trap",
	"	start;br7+6.		/ system  (overlaid by 'trap')",
	"",
	". = ZERO+40",
	".globl	start, dump",
	"1:	jmp	start",
	"",
	0,
};

char	*strb[] =
{
	"",
	". = ZERO+240",
	"	trap; br7+7.		/ programmed interrupt",
	"	trap; br7+8.		/ floating point",
	"	trap; br7+9.		/ segmentation violation",
	0
};

char	*strc[] =
{
	"",
	"/ floating vectors",
	". = ZERO+300",
	0,
};

char	*strov[] =
{
	"",
	"/ overlay descriptor tables",
	"",
	".globl ova, ovd, ovend",
	"ova:\t.=.+16.\t/ overlay addresses",
	"ovd:\t.=.+16.\t/ overlay sizes",
	"ovend:\t.=.+2\t/ end of overlays",
	0
};

char	*sizcmd = {"size ../ovsys/*.o ../ovdev/*.o > text.sizes"};
char	*cmcmd = {"chmod 744 ovload"};

char	*strovh[] =
{
	"covld -X -n -o unix_ov l.o mch_ov.o c_ov.o \\",
	0,
};

char	*strovl[] =
{
	"-L \\",
	"\t../ovsys/LIB1_ov",
	0,
};

char	*strovz[] =
{
	"-Z \\",
	0,
};

char	*strd[] =
{
	"",
	"//////////////////////////////////////////////////////",
	"/		interface code to C",
	"//////////////////////////////////////////////////////",
	"",
	".text",
	".globl	call, trap",
	0
};

char	*stre[] =
{
	"#include \"../h/param.h\"",
	"#include \"../h/systm.h\"",
	"#include \"../h/buf.h\"",
	"#include \"../h/tty.h\"",
	"#include \"../h/conf.h\"",
	"#include \"../h/proc.h\"",
	"#include \"../h/text.h\"",
	"#include \"../h/dir.h\"",
	"#include \"../h/user.h\"",
	"#include \"../h/file.h\"",
	"#include \"../h/inode.h\"",
	"#include \"../h/acct.h\"",
	"",
	"int	nulldev();",
	"int	nodev();",
	0
};

char	*stre1[] =
{
	"struct	bdevsw	bdevsw[] =",
	"{",
	0,
};

char	*strf[] =
{
	"	0",
	"};",
	"",
	0,
};

char	*strf1[] =
{
	"",
	"struct	cdevsw	cdevsw[] =",
	"{",
	0,
};

char	strg[] =
{
"	0\n\
};\n\
int	rootdev	= makedev(%d, %d);\n\
int	swapdev	= makedev(%d, %d);\n\
int	pipedev = makedev(%d, %d);\n\
int	nldisp = %d;\n\
daddr_t	swplo	= %ld;\n\
int	nswap	= %l;\n\
"};

char	strg1[] =
{
"	\n\
struct	buf	buf[NBUF];\n\
struct	file	file[NFILE];\n\
struct	inode	inode[NINODE];\n"
};

char	*strg1a[] =
{
	"#ifdef\tMX",
	"int	mpxchan();",
	"int	(*ldmpx)() = mpxchan;",
	"#endif\tMX",
	0
};

char	strg2[] =
{
"struct	proc	proc[NPROC];\n\
struct	text	text[NTEXT];\n\
struct	buf	bfreelist;\n\
struct	acct	acctbuf;\n\
struct	inode	*acctp;\n"
};

char	*strg3[] =
{
	"",
	"/*",
	" * The following locations are used by commands",
	" * like ps & pstat to free them from param.h",
	" */",
	"",
	"int	nproc	NPROC;",
	"int	ninode	NINODE;",
	"int	ntext	NTEXT;",
	"int	nofile	NOFILE;",
	"int	nsig	NSIG;",
	"int	nfile	NFILE;",
	0
};

char	*strh[] =
{
	"	0",
	"};",
	"",
	"int	ttyopen(), ttyclose(), ttread(), ttwrite(), ttyinput(), ttstart();",
	0
};

char	*stri[] =
{
	"int	pkopen(), pkclose(), pkread(), pkwrite(), pkioctl(), pkrint(), pkxint();",
	0
};

char	*strj[] =
{
	"struct	linesw	linesw[] =",
	"{",
	"	ttyopen, nulldev, ttread, ttwrite, nodev, ttyinput, ttstart, /* 0 */",
	0
};

char	*strk[] =
{
	"	pkopen, pkclose, pkread, pkwrite, pkioctl, pkrint, pkxint, /* 1 */",
	0
};

int	mpx;
int	ov;
int	nsid;
int	nfp;
int	dump;
int	cdcsr;
int	rootmaj = -1;
int	rootmin;
int	swapmaj = -1;
int	swapmin;
int	pipemaj = -1;
int	pipemin;
long	swplo	= 4000;
int	nswap = 872;
int	pack;
int	nldisp = 1;

char	trash[100];
char	omn[20];
char	mtsize[20];

main()
{
	register struct tab *p;
	struct	ovtab	*otp;
	struct	ovdes	*ovdp;
	register char *q;
	char	*c;
	int i, n, ev, nkl;
	int flagf, flagb;
	int dumpht, dumptm, dumpts;
	int fi, ovcnt;

	while(input());

/*
 * pass1 -- create interrupt vectors
 */
	fprintf(stderr, "\n");
	if(ov)
		fprintf(stderr, "(overlay kernel specified)\n");
	if(ov && nsid)
		fprintf(stderr, "(ov & nsid specified, nsid ignored)\n");
	else if(nsid)
		fprintf(stderr, "(non-separate I/D space CPU specified)\n");
	if(nfp)
		fprintf(stderr, "(floating point support excluded)\n");
	nkl = 0;
	flagf = flagb = 1;
	freopen("l.s", "w", stdout);
	if(!ov && !nsid)
		puke(stra70);
	else
		puke(stra40);
	puke(stra);
	ev = 0;
	for(p=table; p->name; p++)
	if(p->count != 0 && p->key & INTR) {
		if(p->address>240 && flagb) {
			flagb = 0;
			puke(strb);
		}
		if(p->address >= 300) {
			if(flagf) {
				ev = 0;
				flagf = 0;
				puke(strc);
			}
			if(p->key & EVEN && ev & 07) {
				printf("	.=.+4\n");
				ev += 4;
			}
			printf("/%s %o\n", p->name, 0300+ev);
		} else
			printf("\n. = ZERO+%d\n", p->address);
		n = p->count;
		if(n < 0)
			n = -n;
		for(i=0; i<n; i++) {
			if(p->key & KL) {
				printf(p->codea, nkl, nkl);
				nkl++;
			} else
				printf(p->codea, i, i);
			if (p->address<300) {
				fprintf(stderr, "%s at %d", p->name, p->address+4*i);
				if (equal(p->name, "ts"))
					fprintf(stderr, "   (TS auto-config)");
				fprintf(stderr, "\n");
			} else
				fprintf(stderr, "%s at %o\n", p->name, 0300+ev);
			ev += p->address - 300;
		}
	}
	if(flagb)
		puke(strb);
/*
 * TS11 auto vector select
 *
 * Text must not be allowed to start at
 * an address that is less than 0300.
 * The vector at 0260 must be initialized to
 * zero for the TS11 auto vector
 * select to function.
 * With the advent of v7m 2.0, text does not
 * ever start before 1000.
 */

	printf("\n. = ZERO+1000\n");
	printf("\n\tjmp\tdump\t/ jump to core dump code\n");
	if(ov)
		puke(strov);
	puke(strd);
	for(p=table; p->name; p++)
	if(p->count != 0 && p->key & INTR)
		printf("\n%s%s", p->codeb, p->codec);

/*
 * pass 2 -- create configuration table
 */

	freopen("c.c", "w", stdout);
	/*
	 * declarations
	 */
	puke(stre);
	for (i=0; q=btab[i]; i++) {
		for (p=table; p->name; p++)
		if (match(q, p->name) &&
		   (p->key&BLOCK) && p->count && *p->codef)
			printf("%s\n", p->codef);
	}
	puke(stre1);
	for(i=0; q=btab[i]; i++) {
		for(p=table; p->name; p++)
		if(match(q, p->name) &&
		   (p->key&BLOCK) && p->count) {
			printf("%s	/* %s = %d */\n", p->coded, p->name, i);
			if(p->key & ROOT)
				rootmaj = i;
			if (p->key & SWAP)
				swapmaj = i;
			if (p->key & PIPE)
				pipemaj = i;
			goto newb;
		}
		printf("	nodev, nodev, nodev, 0, /* %s = %d */\n", q, i);
	newb:;
	}
	if (swapmaj == -1) {
		swapmaj = rootmaj;
		swapmin = rootmin;
	}
	if (pipemaj == -1) {
		pipemaj = rootmaj;
		pipemin = rootmin;
	}
	puke(strf);
	for (i=0; q=ctab[i]; i++) {
		for (p=table; p->name; p++)
		if (match(q, p->name) &&
		   (p->key&CHAR) && p->count && *p->codeg)
			printf("%s\n", p->codeg);
	}
	puke(strf1);
	for(i=0; q=ctab[i]; i++) {
		for(p=table; p->name; p++)
		if(match(q, p->name) &&
		   (p->key&CHAR) && p->count) {
			printf("%s	/* %s = %d */\n", p->codee, p->name, i);
			goto newc;
		}
		printf("	nodev, nodev, nodev, nodev, nodev, nulldev, 0, /* %s = %d */\n", q, i);
	newc:;
	}
	puke(strh);
	if (pack) {
		nldisp++;
		puke(stri);
	}
	puke(strj);
	if (pack)
		puke(strk);
	printf(strg, rootmaj, rootmin,
		swapmaj, swapmin,
		pipemaj, pipemin,
		nldisp,
		swplo, nswap);
	printf(strg1);
	if (!mpx)
		puke(strg1a);
	printf(strg2);
	puke(strg3);
	if(rootmaj < 0)
		fprintf(stderr, "No root device given\n");
	freopen("mch0.s", "w", stdout);
	dumpht = 0;
	dumptm = 0;
	dumpts = 0;
	for (i=0; table[i].name; i++) {
		if (equal(table[i].name, "ht") && table[i].count)
			dumpht = 1;
		if (equal(table[i].name, "tm") && table[i].count)
			dumptm = 1;
		if (equal(table[i].name, "ts") && table[i].count)
			dumpts = 1;
	}
	if((dump == HT) && dumpht) {
		dumptm = 0;
		dumpts = 0;
		}
	if((dump == TM) && dumptm) {
		dumpht = 0;
		dumpts = 0;
		}
	if((dump == TS) && dumpts) {
		dumpht = 0;
		dumptm = 0;
		}
	if (dumpht) {
		printf("HTDUMP = 1\n");
		printf("TUDUMP = 0\n");
		printf("TSDUMP = 0\n");
		if(cdcsr)
			printf("\nHTCS1	= %o\n", cdcsr);
		else
			printf("\nHTCS1	= 172440\n");
	} else if (dumptm) {
		printf("HTDUMP = 0\n");
		printf("TUDUMP = 1\n");
		printf("TSDUMP = 0\n");
		if(cdcsr)
			printf("\nMTS	= %o\n", cdcsr);
		else
			printf("\nMTS	= 172520\n");
		printf("MTC	= MTS+2\n");
	} else if (dumpts) {
		printf("HTDUMP = 0\n");
		printf("TUDUMP = 0\n");
		printf("TSDUMP = 1\n");
		if(cdcsr)
			printf("\nTSDB	= %o\n", cdcsr);
		else
			printf("\nTSDB	= 172520\n");
		printf("TSSR	= TSDB+2\n");
	} else {
		fprintf(stderr, "\n(No tape, so no core dump included)\n");
		printf("HTDUMP = 0\n");
		printf("TUDUMP = 0\n");
		printf("TSDUMP = 0\n");
		}
	if(nfp)
		printf("\n.fpp\t= 0\n");
	else
		printf("\n.fpp\t= 1\n");
/*
 * pass3 - create "ovload" file
 *
 * "ovload" is a shell file used by /sys/conf/makefile
 * to invoke the covld command to link the overlay
 * kernel and produce the executable file unix_ov.
 *
 * The method to this madness is as follows:
 *
 * 1.	If the "ov" specification was not present in
 *	the conf file, then skip this pass !
 *
 * 2.	Use the size command to create the file text.sizes
 *	containing the text sizes of each object module.
 *	../ovdev/bio.o 2086+30+0 = 2116b = 04104b, etc.
 *
 * 3.	Transfer the text size of each object module from the
 *	text.sizes file to that module's slot in the ovtab structure.
 *
 * 4.	Check each module in the ovtab structure to insure that it's
 *	size was found and initialize the ovdes (overlay descriptor)
 *	array to all zeroes (all overlays empty).
 *
 * 5.	Scan the ovtab structure and load all of the modules that
 *	are always configured into their designated overlays.
 *
 * 6.	Search the mkconf "table" to find all configured devices
 *	(count > 0) and mark them as optional configured devices in
 *	the ovtab structure, one exception is "tty" which always included.
 *	If the "hp", "hk", or "hm" disks are configured,
 *	then the "dsort" and "dkleave" modules must be included
 *	in overlay 3.
 *	If the "dh" driver is configured and the "dhdm" module
 *	is not included, then the "dhfdm" module must be included
 *	in overlay 5.
 *	If the "rx" specification is included, it's name is changed
 *	to "rx2" because the rx2.o module supports both rx01 & rx02.
 *
 * 7.	Check for "mpx" and "pack", if specified mark their object
 *	modules as optional configured devices in the ovtab structure.
 *
 * 8.	Scan the ovtab and load all optionaly configured modules
 *	that have fixed overlay assignments into their designated overlays.
 *	Except for the following special case any module with a
 *	fixed overlay assignment, which causes that overlay to
 *	overflow will cause a fatal error, i.e., overlay
 *	3 could overflow if all three big disk drivers (hp, hm, & hk)
 *	are configured. If this occurs the "hm" driver will be
 *	loaded into the first overlay with sufficient free space.
 *
 * 9.	Scan the ovtab and transfer any configured modules from
 *	pseudo overlays 8 thru 10 to real overlays 1 thru 7.
 *	The modules are placed into the overlays starting at one
 *	on a first fit basis. Actually this is done in two passes,
 *	first the existing overlays are scanned to locate a
 *	slot for the module, if the module will not fit into
 *	any existing overlay, then a new overlay is created and
 *	the module is loaded into it.
 *
 * 10.	Create the "ovload" shell file by scanning the overlay
 *	descriptor array and writing the pathnames of the
 *	modules to be loaded into the file.
 */

/* 1. */
	if(!ov)
		exit(0);
/* 2. */
	system(sizcmd);
	if((fi = fopen("text.sizes", "r")) == 0) {
		fprintf(stderr, "\nCan't open text.sizes file\n");
		exit();
		}
/* 3. */
scanlp:
    ovcnt = fscanf(fi, "%9s%[^\.]%s%[^+]%[^\n]",trash,omn,trash,mtsize,trash);
	if(ovcnt == EOF)
		goto endovs;
	if(ovcnt != 5) {
		fprintf(stderr, "\ntext.sizes file format error\n");
		exit();
		}
	for(otp = ovt; otp->mn; otp++) {
		q = &omn;
		if(equal(otp->mn, q)) {
			otp->mts = atoi(mtsize);
			break;
			}
		}
	goto scanlp;
endovs:
	fclose(fi);
/* 4. */
	for(otp = ovt; otp->mn; otp++)
		if(otp->mts < 0)
		fprintf(stderr, "\n%s.o object file size not found\n", otp->mn);
	for(i=0; i<8; i++) {
		ovdtab[i].nentry = 0;
		ovdtab[i].size = 0;
		}
/* 5. */
	for(otp = ovt; otp->mn; otp++)
		if((otp->mc == 1) && (otp->ovno < 6)) {
			if(ovload(otp, otp->ovno) < 0)
				goto ovlerr;
			}
/* 6. */
	for(p=table; p->name; p++)
		if(p->count > 0) {
			if(equal(p->name, "rx"))
				c = ctab[22];	/* rx2 */
			else
				c = p->name;
			n = 0;
			for(otp=ovt; otp->mn; otp++)
				if(equal(c, q=otp->mn)) {
					if(otp->mc == 0)
						otp->mc = 2;
					n++;
					break;
					}
				if(!n) {
			fprintf(stderr, "\n%s not in overlay table\n", c);
					exit();
					}
			}

	/* if hp, hm or hk configured, must include dsort and dkleave */

	n = 0;
	for(otp=ovt; otp->mn; otp++) {
		if((equal(otp->mn, "hp") ||
		equal(otp->mn, "hm") ||
		equal(otp->mn, "hk")) &&
		(otp->mc == 2))
			n++;
		if(equal(otp->mn, "dsort") && n)
			otp->mc = 2;
		if(equal(otp->mn, "dkleave") && n) {
			otp->mc = 2;
			break;
			}
		}

	/* if dh configured without dhdm, must include dhfdm */

	n = 0;
	for(otp=ovt; otp->mn; otp++)
		if(equal(otp->mn, "dhdm"))
			break;
	if(otp->mc == 0)
		n++;
	otp++;
	if((otp->mc == 2) && n) {
		otp++;
		otp->mc = 2;
		}
/* 7. */
	for(otp=ovt; otp->mn; otp++) {
		if((otp->ovno == 6) && pack)
			otp->mc = 2;
		if((otp->ovno == 7) && mpx)
			otp->mc = 2;
		}
/* 8. */
	for(otp=ovt; otp->mn; otp++)
		if((otp->mc == 2) && (otp->ovno < 8))
			if(ovload(otp, otp->ovno) < 0) {
				if(otp->ovno == 3)
					otp->ovno = 8;
				else
					goto ovlerr;
				}
/* 9. */
	for(otp=ovt; otp->mn; otp++) {
		if(otp->ovno < 8)
ovldok:
		continue;
		if(otp->mc) {
			for(i=1; i<8; i++) {
				if(ovdtab[i].size)
					if(ovload(otp, i) >= 0)
						goto ovldok;
				}
			for(i=1; i<8; i++) {
				if(ovload(otp, i) >= 0)
					goto ovldok;
				}
		fprintf(stderr,"%s will not fit in overlay %d\n",otp->mn,i);
			exit();
			}
		}
/* 10. */
	freopen("ovload", "w", stdout);
	puke(strovh);
	for(i=1; i<8; i++) {
		ovdp = &ovdtab[i];
		if(ovdp->nentry) {
			puke(strovz);
			for(n=0; n < ovdp->nentry; n++) {
				printf("%s", ovdp->omns[n]);
				printf("\n");
				}
			}
		}
	puke(strovl);
	printf("\n");
	fclose(stdout);
	system(cmcmd);
	exit();
ovlerr:
	fprintf(stderr, "\noverlay %d size overflow\n",otp->ovno);
	exit();
}

/*
 * ovload checks to see if the module will fit
 * into the overlay and then loads it if possible
 * or returns -1 if it can't be loaded.
 */

ovload(tp, ovn)
struct ovtab *tp;
{
	register struct ovdes *dp;

	dp = &ovdtab[ovn];
	if((dp->nentry >= 12) ||
	(dp->size >= 8192) ||
	((dp->size + tp->mts) > 8192))
		return(-1);
	dp->omns[dp->nentry] = tp->mpn;
	dp->size += tp->mts;
	dp->nentry++;
	return(1);
}

puke(s)
char **s;
{
	char *c;

	while(c = *s++) {
		printf(c);
		printf("\n");
	}
}

input()
{
	char line[100];
	register struct tab *q;
	int count, n;
	long num;
	unsigned int onum;
	char keyw[32], dev[32];

	if (fgets(line, 100, stdin) == NULL)
		return(0);
	count = -1;
	 sscanf(line, "%d%s%s%o", &count, keyw, dev, &onum);
	n = sscanf(line, "%d%s%s%ld", &count, keyw, dev, &num);
	if (count == -1 && n>0) {
		count = 1;
		n++;
	}
	if (n<2)
		goto badl;
	for(q=table; q->name; q++)
	if(equal(q->name, keyw)) {
		if(q->count < 0) {
			fprintf(stderr, "%s: no more, no less\n", keyw);
			return(1);
		}
		q->count += count;
		if(q->address < 300 && q->count > 1) {
			q->count = 1;
			fprintf(stderr, "%s: only one\n", keyw);
		}
		return(1);
	}
	if (equal(keyw, "nswap")) {
		if (n<3)
			goto badl;
		if (sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		nswap = num;
		return(1);
	}
	if (equal(keyw, "swplo")) {
		if (n<3)
			goto badl;
		if (sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		swplo = num;
		return(1);
	}
	if (equal(keyw, "pack")) {
		pack++;
		return(1);
	}
	if (equal(keyw, "mpx")) {
		mpx++;
		return(1);
	}
	if (equal(keyw, "ov")) {
		ov++;
		return;
	}
	if (equal(keyw, "nsid")) {
		nsid++;
		return(1);
	}
	if (equal(keyw, "nfp")) {
		nfp++;
		return(1);
	}
	dump = 0;
	if (equal(keyw, "dump")) {
		if (n<3)
			goto badl;
		if (equal(dev, "ht"))
			dump = HT;
		else if(equal(dev, "tm"))
			dump = TM;
		else if(equal(dev, "ts"))
			dump = TS;
		else
			goto badl;
		cdcsr = 0;
		if (n == 4) {
			if ((onum > 0160006) && (onum < 0177514))
				cdcsr = onum & 0177776;
			else
				goto badl;
			}
		return(1);
		}
	if (equal(keyw, "done"))
		return(0);
	if (equal(keyw, "root")) {
		if (n<4)
			goto badl;
		for (q=table; q->name; q++) {
			if (equal(q->name, dev)) {
				q->key |= ROOT;
				rootmin = num;
				return(1);
			}
		}
		fprintf(stderr, "Can't find root\n");
		return(1);
	}
	if (equal(keyw, "swap")) {
		if (n<4)
			goto badl;
		for (q=table; q->name; q++) {
			if (equal(q->name, dev)) {
				q->key |= SWAP;
				swapmin = num;
				return(1);
			}
		}
		fprintf(stderr, "Can't find swap\n");
		return(1);
	}
	if (equal(keyw, "pipe")) {
		if (n<4)
			goto badl;
		for (q=table; q->name; q++) {
			if (equal(q->name, dev)) {
				q->key |= PIPE;
				pipemin = num;
				return(1);
			}
		}
		fprintf(stderr, "Can't find pipe\n");
		return(1);
	}
	fprintf(stderr, "%s: cannot find\n", keyw);
	return(1);
badl:
	fprintf(stderr, "Bad line: %s", line);
	return(1);
}

equal(a, b)
char *a, *b;
{
	return(!strcmp(a, b));
}

match(a, b)
register char *a, *b;
{
	register char *t;

	for (t = b; *a && *t; a++) {
		if (*a != *t) {
			while (*a && *a != '|')
				a++;
			if (*a == '\0')
				return(0);
			t = b;
		} else
			t++;
	}
	if (*a == '\0' || *a == '|')
		return(1);
	return(0);
}
