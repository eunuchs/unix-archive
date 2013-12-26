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

/ RX50 bootstrap

read = 100
reset = 60
busy = 10

rxc0 = 174204
rxc1 = 174206
rxc2 = 174210
rxca = 174222
rxdb = 174220
rxsc = 174224

int1 = 173202
int2 = 173206
int3 = 173212

	clr	*$int1		/ disable interrupts
	clr	*$int2
	clr	*$int3
	mov	$reset,*$rxc0	/ do a reset, just in case
	clr	*$rxsc
1:
	bit	$busy,*$rxc0	/ test for done
	beq	1b
	mov	$1.,*$rxc2	/ assign sector #
	mov	$1.,*$rxc1	/ and track
	mov	$read,*$rxc0	/ and do read
	clr	*$rxsc
2:
	bit	$busy,*$rxc0	/ test for done
	beq	2b
	mov	$512.,r0	/ set up transfer
	clr	r1
	clr	*$rxca
3:
	movb	*$rxdb,(r1)+
	sob	r0,3b		/ and transfer the sector

	jmp	*$0		/ transfer to zero

#endif	UCB_AUTOBOOT
