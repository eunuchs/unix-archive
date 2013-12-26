/*
 *	SCCS id	@(#)l.s	1.5 (Berkeley)	3/14/83
 *
 *	Low Core
 *
 */

#define		LOCORE
#include	"whoami.h"		/* for localopts */
#include	<sys/trap.h>
#include	<sys/iopage.m>
#include	<sys/koverlay.h>	/* for definition of OVLY_TABLE_BASE */

#include "dh.h"
#include "dn.h"
#include "dz.h"
#include "hk.h"
#include "hp.h"
#include "hs.h"
#include "ht.h"
#include "kl.h"
#include "lp.h"
#include "rk.h"
#include "rl.h"
#include "rm.h"
#include "rp.h"
#include "tm.h"
#include "ts.h"
#include "vp.h"
#include "xp.h"
#include "ra.h"
#include "rd.h"
#include "r5.h"
#include "cn.h"
#include "pc.h"
#include "ec.h"
#include "qn.h"

	.globl	call, trap, buserr, start, _panic
#ifdef	NONFP
	.globl	instrap
#else
#	define	instrap		trap
#endif
#ifdef	MENLO_OVLY
	.globl	emt
#else
#	define	emt		trap
#endif
#ifdef	UCB_AUTOBOOT
	.globl	powrdown
#else
#	define	powrdown	trap
#endif

#ifndef	KERN_NONSEP
	.data
#endif

ZERO:

br4 = 200
br5 = 240
br6 = 300
br7 = 340

. = ZERO+0
#ifdef	KERN_NONSEP
	/  If vectors at 110 and 444 are unused,
	/  autoconfig will set these to something more reasonable.
	/  On jump, this will branch to 112, which branches to 50.
	/  On trap, will vector to 444, where a ZEROTRAP will be simulated.
	42				/ illegal instruction if jump
	777				/ trace trap at high priority if trap
#else
	trap; br7+ZEROTRAP.		/ trap-to-zero trap
#endif

/ trap vectors
. = ZERO+4
	buserr; br7+BUSFLT.		/ bus error
	instrap; br7+INSTRAP.		/ illegal instruction
	trap; br7+BPTTRAP.		/ bpt-trace trap
	trap; br7+IOTTRAP.		/ iot trap
	powrdown; br7+POWRFAIL.		/ power fail
	emt; br7+EMTTRAP.		/ emulator trap
	start;br7+SYSCALL.		/ system  (overlaid by 'syscall')

. = ZERO+40
#ifdef	UCB_AUTOBOOT
.globl	do_panic
	jmp	do_panic
#else
	br	.
#endif

.globl	 dump
. = ZERO+44
	jmp	dump
#ifdef	KERN_NONSEP
	/  Handler for jump-to-zero panic.
. = ZERO+50
	mov	$zjmp, -(sp)
	jsr	pc, _panic
#endif

#if	NKL > 0
. = ZERO+60
	klin; br4
	klou; br4
#endif	NKL

. = ZERO+100
	kwlp; br6
	kwlp; br6

. = ZERO+114
	trap; br7+PARITYFLT.		/ 11/70 parity
	trap; br7+SEGFLT.		/ segmentation violation

#if	NRA > 0
. = ZERO+154
	raio; br5
#endif	NRA

#if	NRL > 0
. = ZERO+160
	rlio; br5
#endif

#if	NCN > 0
. = ZERO+200
	kbin; br4
	kbou; br4
#endif	NCN

#if	NPC > 0
. = ZERO+210
	cmio; br4
#endif	NPC

#if	NHK > 0
. = ZERO+210
	hkio; br5
#endif

#if	NRK > 0
. = ZERO+220
	rkio; br5
#endif

#if	NPC > 0
. = ZERO+220
	pcin; br4
	pcou; br4
#endif	NPC

. = ZERO+230
	kwlp; br4

. = ZERO+240
	trap; br7+PIRQ.			/ programmed interrupt
	trap; br7+ARITHTRAP.		/ floating point
	trap; br7+SEGFLT.		/ segmentation violation

. = ZERO+254
#if	NXP > 0
	xpio; br5
#else
#if	NHP > 0
	hpio; br5
#else
#if	NRM > 0
	rmio; br5
#endif
#endif
#endif

#if	NRD > 0
. = ZERO+300
	rdio; br4
	rdio; br4
#endif	NRD

#if	NR5 > 0
. = ZERO+310
	r5ain; br4
	r5bin; br4
#endif	NR5

#if	NCN > 0
. = ZERO+320
	scain; br4
	scbin; br4
#endif	NCN

#if	NQN > 0
. = ZERO+340
	qnio; br5
#endif	NQN

#if	NEC > 0
. = ZERO + 360
	ecin; br6
	ecjm; br5
	ecou; br4
#endif	NEC
/ floating vectors
. = ZERO + 1000
endvec = .				/ endvec should be past vector space
					/ (if NONSEP, should be at least 450)

#ifdef	MENLO_KOV
/ overlay descriptor tables
. = ZERO+OVLY_TABLE_BASE
.globl	ova, ovd, ovend
#ifdef	BIGKOV
.globl	ova1, ovd1
ova1:	.=.+20
ovd1:	.=.+20
#endif	BIGKOV
ova:	.=.+20				/ overlay addresses
ovd:	.=.+20				/ overlay sizes
ovend:	.=.+2				/ end of overlays
#endif

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

#ifndef	KERN_NONSEP
.text
	/  This is text location 0 for separate I/D kernels.
	mov	$zjmp, -(sp)
	jsr	pc, _panic
	/*NOTREACHED*/

	/  Unmap is called from _doboot to turn off memory management.
	/  The "return" is arranged by putting a jmp at unmap+2 (data space).

	reset=	5
	.globl unmap
unmap:
	reset
	/  The next instruction executed is from unmap+2 in physical memory,
	/  which is unmap+2 in data space.

#endif	KERN_NONSEP

	.data
zjmp:	<jump to 0\0>
	.text

/  CGOOD and CBAD are used by autoconfig.
/  All unused vectors are set to CBAD
/  before probing the devices.

.globl	CGOOD, CBAD, _conf_int
	rtt = 6
CGOOD:	mov	$1, _conf_int ; rtt
CBAD:	mov	$-1,_conf_int ; rtt

#if	NKL > 0
.globl	_klrint
klin:	jsr	r0,call; jmp _klrint
.globl	_klxint
klou:	jsr	r0,call; jmp _klxint
#endif	NKL

.globl	_clock
kwlp:	jsr	r0,call; jmp _clock

#if	NDH > 0
.globl	_dhrint
dhin:	jsr	r0,call; jmp _dhrint
.globl	_dhxint
dhou:	jsr	r0,call; jmp _dhxint
#endif

#if	NDM > 0
.globl	_dmintr
dmin:	jsr	r0,call; jmp _dmintr
#endif

#if	NDZ > 0
.globl	_dzrint
dzin:	jsr	r0,call; jmp _dzrint
#ifndef	DZ_PDMA
.globl	_dzxint
dzou:	jsr	r0,call; jmp _dzxint
#endif	DZ_PDMA
#endif	NDZ

#if	NHK > 0
.globl	_hkintr
hkio:	jsr	r0,call; jmp _hkintr
#endif

#if	NHP > 0
.globl	_hpintr
hpio:	jsr	r0,call; jmp _hpintr
#endif

#if	NHS > 0
.globl	_hsintr
hsio:	jsr	r0,call; jmp _hsintr
#endif

#if	NHT > 0
.globl	_htintr
htio:	jsr	r0,call; jmp _htintr
#endif

#if	NLP > 0
.globl	_lpintr
lpio:	jsr	r0,call; jmp _lpintr
#endif

#if	NRK > 0
.globl	_rkintr
rkio:	jsr	r0,call; jmp _rkintr
#endif

#if	NRL > 0
.globl	_rlintr
rlio:	jsr	r0,call; jmp _rlintr
#endif

#if	NRM > 0
.globl	_rmintr
rmio:	jsr	r0,call; jmp _rmintr
#endif

#if	NRP > 0
.globl	_rpintr
rpio:	jsr	r0,call; jmp _rpintr
#endif

#if	NTM > 0
.globl	_tmintr
tmio:	jsr	r0,call; jmp _tmintr
#endif

#if	NTS > 0
.globl	_tsintr
tsio:	jsr	r0,call; jmp _tsintr
#endif

#if	NVP > 0
.globl	_vpintr
vpio:	jsr	r0,call; jmp _vpintr
#endif

#if	NXP > 0
.globl	_xpintr
xpio:	jsr	r0,call; jmp _xpintr
#endif

#if	NCN > 0
.globl	_kbrint
kbin:	jsr	r0,call; jmp _kbrint
.globl	_kbxint
kbou:	jsr	r0,call; jmp _kbxint
.globl	_sctint
scbin:	jsr	r0,call; jmp _sctint
.globl	_scfint
scain:	jsr	r0,call; jmp _scfint
#endif	NCN

#if	NPC > 0
.globl	_pcrint
pcin:	jsr	r0,call; jmp _pcrint
.globl	_pcxint
pcou:	jsr	r0,call; jmp _pcxint
.globl	_cmintr
cmio:	jsr	r0,call; jmp _cmintr
#endif	NPC

#if	NRA > 0
.globl	_raintr
raio:	jsr	r0,call; jmp _raintr
#endif	NRA

#if	NRD > 0
.globl	_rdintr
rdio:	jsr	r0,call; jmp _rdintr
#endif	NRD

#if	NR5 > 0
.globl	_r5aint
r5ain:	jsr	r0,call; jmp _r5aint
.globl	_r5bint
r5bin:	jsr	r0,call; jmp _r5bint
#endif	NR5

#if	NEC > 0
.globl	_ecrint, _eccol, _ecxint
ecin:	jsr	r0,call; jmp _ecrint
ecjm:	jsr	r0,call; jmp _eccol
ecou:	jsr	r0,call; jmp _ecxint
#endif	NEC

#if	NQN > 0
.globl	_qnintr
qnio:	jsr	r0,call; jmp _qnintr
#endif	NQN
