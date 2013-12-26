/*
 *	SCCS id	@(#)boot.s	1.2 (Berkeley)	9/6/82
 */
#include "whoami.h"

#ifdef	UCB_AUTOBOOT
/  The boot options and device are placed in the last SZFLAGS bytes
/  at the end of core for the bootstrap.
ENDCORE=	160000		/ end of core, mem. management off
SZFLAGS=	6		/ size of boot flags
BOOTOPTS=	2		/ location of options, bytes below ENDCORE
BOOTDEV=	4
CHECKWORD=	6

reset= 	5

.globl	_doboot, hardboot
.text
_doboot:
	mov	4(sp),r4	/ boot options
	mov	2(sp),r3	/ boot device

#ifndef	KERN_NONSEP
/  If running separate I/D, need to turn off memory management.
/  Call the routine unmap in low text, after setting up a jump
/  in low data where the PC will be pointing.
.globl	unmap
	mov	$137,*$unmap+2		/ jmp *$hardboot
	mov	$hardboot,*$unmap+4
	jmp	unmap
	/ "return" from unmap will be to hardboot in data
.data
#else
/  Reset to turn off memory management
	reset
#endif

/  On power fail, hardboot is the entry point (map is already off)
/  and the args are in r4, r3.

hardboot:
	mov	r4, ENDCORE-BOOTOPTS
	mov	r3, ENDCORE-BOOTDEV
	com	r4		/ if CHECKWORD == ~bootopts, flags are believed
	mov	r4, ENDCORE-CHECKWORD
1:
	reset

/  The remainder of the code is dependent on the boot device.
/  If you have a bootstrap ROM, just jump to the correct entry.
/  Otherwise, use a BOOT opcode, if available;
/  if necessary, read in block 0 to location 0 "by hand".

/ RA bootstrap

/ initialize ra controller
unit = 0		/ drive unit #
raip = 172150
rasa = 172152
ra_step1 = 4000
ra_i1 = 100000
ra_step2 = 10000
ra_step3 = 20000
ra_step4 = 40000
ra_go = 1
op_stcon = 4
op_onlin = 11
op_read = 41
raset = 100000
dscsiz = 60.
	mov	$rasa,r2	/ use r2 as reg. reference (saves text space)
	clr	*$raip		/ start hard init
1:
	bit	$ra_step1,(r2)	/ test for start of step1
	beq	1b
	mov	$ra_i1,(r2)	/ step 1
1:
	bit	$ra_step2,(r2)	/ test for start of step 2
	beq	1b
	mov	$rspdsc,(r2)	/ step 2 - low order addr. of ring
1:
	bit	$ra_step3,(r2)	/ test for start of step 3
	beq	1b
	clr	(r2)		/ step 3 - high order addr. of ring
1:
	bit	$ra_step4,(r2)	/ test for start of step 4
	beq	1b
	mov	$ra_go,(r2)	/ step 4 go
	mov	$rspref,rspdsc	/ set up descriptor addr.
	mov	$cmdref,cmddsc
	clr	cmdflg
	mov	$op_stcon,r0
	jsr	pc,racmd	/ do start connection cmd
	mov	$unit,cmdunit	/ set up drive unit
	mov	$op_onlin,r0	/ do online cmd
	jsr	pc,racmd

	clr	cmdlbn+2	/ get high order disk addr.
	clr	cmdlbn		/ get low order disk addr.
	clr	cmdcnt+2
	mov	$512.,cmdcnt	/ set up byte cnt.
	clr	cmdbuf+2		/ and buffer addr.
	clr	cmdbuf
	mov	$op_read,r0	/ read op
	jsr	pc,racmd	/ do the read op.

	jmp	*$4
/ This function performs the op passed in r0 on the ra
racmd:
	movb	r0,cmdop	/ set up op
	mov	$dscsiz,rsplen	/ and descriptor sizes
	mov	$dscsiz,cmdlen
	bis	$raset,rspdsc+2	/ set port ownership
	bis	$raset,cmddsc+2
	mov	*$raip,r0	/ start polling
1:
	tst	rspdsc+2	/ test for completion
	blt	1b
	rts	pc

raca:
rspdsc = raca+8.
cmddsc = rspdsc+4
rsplen = cmddsc+4
rspref = rsplen+4
rspunit = rspref+4
rspop = rspunit+4
rspcnt = rspop+4
rspbuf = rspcnt+4
rsplbn = rspbuf+12.
cmdlen = rsplbn+24.
cmdref = cmdlen+4
cmdunit = cmdref+4
cmdop = cmdunit+4
cmdcnt = cmdop+4
cmdflg = cmdcnt+2
cmdbuf = cmdflg+2
cmdlbn = cmdbuf+12.
. = raca+136.
#endif	UCB_AUTOBOOT
