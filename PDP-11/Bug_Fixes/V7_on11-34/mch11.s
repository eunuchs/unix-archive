/	@(#)mch11.s	1.6	(Chemeng) 83/10/11
/
/	Machine language assist for small pdp11's
/	11/23, 11/34, 11/35, 11/40, 11/60

/ non-UNIX instructions
mfpi	= 006500^tst
mtpi	= 006600^tst
mfps	= 106700^tst
mtps	= 106400^tst
stst	= 170300^tst
halt	= 0
wait	= 1
reset	= 5
rtt	= 6

.if	RKDUMP
/
/ Parameters for RK dump routine.
/
drive	= 2
block	= 0
.endif	/RKDUMP

.if	BIGUNIX - 1

	.globl	start, _end, _edata, _main
start:
	mov	$trap,34

/ Set location 0 and 2 to catch traps and jumps to 0

	mov	$42,0		/ illegal instruction if jump
	mov	$777,2		/ trace trap at high priority if trap

/ initialize systems segments

	mov	$KISA0,r0
	mov	$KISD0,r1
	mov	$200,r4
	clr	r2
	mov	$6,r3
1:
	mov	r2,(r0)+
	mov	$77406,(r1)+		/ 4k rw
	add	r4,r2
	sob	r3,1b

/ initialize user segment

	mov	$_end+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	mov	r2,(r0)+		/ ksr6 = sysu
	mov	$usize-1\<8|6,(r1)+

/ initialize io segment
/ set up counts on supervisor segments

	mov	$IO,(r0)+
	mov	$77406,(r1)+		/ rw 4k

/ get a sp and start segmentation

	mov	$_u+[usize*64.],sp
	inc	SSR0

/ clear bss

	mov	$_edata,r0
	clr	r1
1:
	mov	r1,(r0)+
	cmp	r0,$_end
	blo	1b

/ clear user block

	mov	$_u,r0
1:
	mov	r1,(r0)+
	cmp	r0,$_u+[usize*64.]
	blo	1b

.if	BUFMAP
/
/ set up buffer pointer
/
	mov	$-1,KISA5		/ illegal address initially
.endif

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$PUMODE,PS
	jsr	pc,_main
	mov	$UMODE,-(sp)
	clr	-(sp)
	rtt
.endif


.if	BIGUNIX

	.globl	start, _end, _edata, _main
	.globl	_etfo, _etfa, _eu, _eto, _etoa, _ers, _eos
start:
	mov	$trap,34

/ Set location 0 and 2 to catch traps and jumps to 0

	mov	$42,0		/ illegal instruction if jump
	mov	$777,2		/ trace trap at high priority if trap

/ initialize systems segments
/ set up all kernel regs to be 4Kw R/W
/ and mapping virual direct to physical

	mov	$KISA0,r0
	mov	$KISD0,r1
	clr	r2
	mov	$8.,r3
1:
	mov	r2,(r0)+
	mov	$77406,(r1)+		/ 4k rw
	add	$200,r2
	sob	r3,1b


/
/ map IO page
/ and set page 6 length to usize
/
	mov	$IO,-(r0)
	mov	$[usize-1\<8]+6,-4(r1)
/
/ now set up to copy overlays
/ using ka5 as source, ka6 as destination
/
	mov	$_eto,-(r0)		/ eto is seg address of overlay code destination
	mov	r0,r1
	mov	$_etfo,-(r0)		/ etfo is seg address of overlay code source
	inc	SSR0			/ enable mapping
	mov	$1,r5			/ region count, 1=onceonly, 0=ovly
	mov	$_etoa,r4		/ init destination virtual addr
	mov	$_etfa,r3		/ init source virtual addr
	mov	$_eos,r2		/ get onceonly seg size (words)
	beq	2f
/
/ copy segments up to their final resting place
/
0:
	tstb	r3
	bne	1f
	add	$100,r3
	dec	(r0)
1:
	tstb	r4
	bne	1f
	add	$100,r4
	dec	(r1)
1:
	mov	-(r3),-(r4)
	sob	r2,0b
2:
	dec	r5
	bmi	2f	/ all done!
/
/ done- now leave gap for proc 0,1 u areas (sysld knows the magic size)
/
	sub	$[3*usize]+3,(r1)
/
/ now copy other overlay segs to their places
/
	mov	$_ers,r2		/ get size in words
	bne	0b
/
/ all done
/
2:
/
/ now set up for call to main
/
	mov	$IO,(r0)		/ done with ka5
	mov	$_eu,(r1)		/ proc0 ka6

.if	BUFMAP
/
/ set up buffer pointer
/
	mov	$-1,-(r0)		/ illegal address initially
.endif
/ clear bss

	clr	r2
	mov	$_end,r1
	mov	$_edata,r0
	sub	r0,r1
	asr	r1			/ size of bss in words
	beq	2f
1:
	mov	r2,(r0)+
	sob	r1,1b
2:

/ clear user block
/ and get initial ksp

	mov	$usize*40,r1
	mov	$_u,r5
1:
	mov	r2,(r5)+
	sob	r1,1b
	mov	r5,sp

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$PUMODE,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt
.endif



	.globl	dump
dump:
/
/ Save regs in low memory and dump
/ core to tape.
/
	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	KISA6,(r0)+
.if	[BIGUNIX | BUFMAP]
	mov	KISA5,(r0)+
.endif
.if	[BIGUNIX * BUFMAP]
	mov	KISA4,(r0)+
.endif

.if	RKDUMP
	mov	$RKCS,r0
	mov	$2,(r0)+
	clr	2(r0)
	mov	$rkaddr,4(r0)
1:
	mov	$-256.,(r0)
	inc	-(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)+
	bge	1b
	halt

RKCS	= 177404
sec	= block%12
cyl	= block\/12
rkaddr	= [drive*20000]+[cyl*20]+sec

.endif

.if	TUDUMP
	mov	$MTC,r0
	mov	$60004,(r0)+
	clr	2(r0)
1:
	mov	$-512.,(r0)
	inc	-(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)+
	bge	1b
	reset

/ end of file and loop

	mov	$60007,-(r0)
	br 	.

MTC	=	172522

.endif

.if	HTDUMP
	reset
	mov	$1300,*$HTTC
	mov	$HTCS1,r0
	mov	$60,(r0)
1:
	mov	$177000,*$HTFC
	mov	$177400,*$HTWC
	inc	(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)
	bge	1b
	mov	$27,(r0)
	br	.

HTCS1	=	172440
HTWC	=	172442
HTBA	=	172444
HTFC	=	172446
HTCS2	=	172450
HTTC	=	172472

.endif

.if	HPBOOT
HPADDR	=	176700
.globl	hpboot
hpboot:
	mov	$HPADDR,r0
	mov	$21,(r0)
1:
	bit	$200,(r0)
	beq	1b
	mov	$177400,2(r0)
	mov	$71,(r0)
1:
	bit	$200,(r0)
	beq	1b
	clr	pc
	halt
.endif
	halt


	.globl	trap, call
	.globl	_trap, _calltime

/ all traps and interrupts are
/ vectored thru this routine.

trap:
	mov	PS,saveps
	tst	nofault
	bne	1f
	mov	SSR0,ssr
	mov	SSR2,ssr+4
	mov	$1,SSR0
	jsr	r0,call1; jmp _trap
	/ no return
1:
	mov	$1,SSR0
	mov	nofault,(sp)
	rtt
.text

	.globl	_runrun
call1:
	mov	saveps,-(sp)
.if	MOVPS - 1
	clrb	PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	$BR0
.endif	/MOVPS
	br	1f

call:
	mov	PS,-(sp)
1:
	mov	r1,-(sp)
	mfpi	sp
	mov	4(sp),-(sp)
	bic	$!37,(sp)
	bit	$PUMODE,PS
	beq	1f
	jsr	pc,(r0)+
	tstb	_runrun
	beq	2f
	mov	$12.,(sp)		/ trap 12 is give up cpu
	jsr	pc,_trap
2:
	tst	(sp)+
	mtpi	sp
	br	2f
1:
	bis	$PUMODE,PS
	jsr	pc,(r0)+
	cmp	(sp)+,(sp)+
2:
	tst	_dotime
	beq	2f
	bit	$BR7,10(sp)		/ test return priority bits
	bne	2f
	jsr	pc,_calltime
2:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	rtt

.if	FPU
	.globl	_savfp
_savfp:
	mov	2(sp),r1
	stfps	(r1)+
	setd
	movf	fr0,(r1)+
	movf	fr1,(r1)+
	movf	fr2,(r1)+
	movf	fr3,(r1)+
	movf	fr4,fr0
	movf	fr0,(r1)+
	movf	fr5,fr0
	movf	fr0,(r1)+
	rts	pc

	.globl	_restfp
_restfp:
	mov	2(sp),r1
	mov	r1,r0
	setd
	add	$8.+2.,r1
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	movf	(r1)+,fr0
	movf	fr0,fr4
	movf	(r1)+,fr0
	movf	fr0,fr5
	movf	2(r0),fr0
	ldfps	(r0)
	rts	pc

	.globl	_stst
_stst:
	stst	r0
	mov	r0,*2(sp)
	rts	pc

.endif	/FPU


	.globl	_addupc
_addupc:
	mov	r2,-(sp)
	mov	6(sp),r2		/ base of prof with base,leng,off,scale
	mov	4(sp),r0		/ pc
	sub	4(r2),r0		/ offset
	clc
	ror	r0
	mov	6(r2),r1
	clc
	ror	r1
	mul	r1,r0		/ scale
	ashc	$-14.,r0
	inc	r1
	bic	$1,r1
	cmp	r1,2(r2)		/ length
	bhis	1f
	add	(r2),r1		/ base
	mov	nofault,-(sp)
	mov	$2f,nofault
	mfpi	(r1)
	add	12.(sp),(sp)
	mtpi	(r1)
	br	3f
2:
	clr	6(r2)
3:
	mov	(sp)+,nofault
1:
	mov	(sp)+,r2
	rts	pc


/
/ back up from a segmentation violation (called from trap.c)
/   backup(u.u_ar0) == 0 if backup assumed to be successful
/
/ Almost all processors of the PDP-11 series behave differently
/ with respect to segmentation violation backup.
/ This backup routine can handle all these
/ processors if the correct options are set.

	.globl	_backup
	.globl	_regloc

_backup:
	mov	2(sp),ssr+2
	mov	r2,-(sp)
	jsr	pc,backup
	mov	r2,ssr+2
	mov	(sp)+,r2
	movb	jflg,r0
	bne	2f
	mov	2(sp),r0
	movb	ssr+2,r1
	jsr	pc,1f
	movb	ssr+3,r1
	jsr	pc,1f
	movb	_regloc+7,r1
	asl	r1
	add	r0,r1
	mov	ssr+4,(r1)
	clr	r0
2:
	rts	pc
1:
	mov	r1,-(sp)
	asr	(sp)
	asr	(sp)
	asr	(sp)
	bic	$!7,r1
	movb	_regloc(r1),r1
	asl	r1
	add	r0,r1
	sub	(sp)+,(r1)
	rts	pc

/	Hard part - simulate the SSR1 register which DEC fail to
/	provide on the /23, /34, /40 and /60. This version of
/	"backup" was concocted from the V6 m40.s, the V7 m40.s,
/	the Stanford m34.s, and the Edinburgh m60.s.
/		D.S.H. Rosenthal	Dec '79
/		EdCAAD Studies, Dept. of Architecture,
/		22 Chambers St., Edinburgh.
/	Some changes for the VU system by
/		Johan Stevenson, Vrije Universiteit, Amsterdam.
/
/	The basic cases are as follows:-
/
/ 1.	op dd
/	The fault happened on the dd.
/
/ 2.	op ss,dd
/	The fault may have happened on either the ss or dd. "backup"
/	evaluates and fetches the ss. If this process faults, the
/	adjustment of the dd register [delta(dd)] is zeroed. Things
/	would be easier if the hardware recorded what it was fetching.
/	If both the ss and dd refer to the same register, and if delta(dd)
/	is non-zero, it is impossible to distinguish an ss from a dd fault.
/	Define REGREG = 1 to assume:-
/		If delta(ss) == 0 then dd fault else failure.
/	This permits, for example, a push fault in mov (sp),-(sp) to be
/	backed-up correctly. However, a source fault would be backed-up
/	incorrectly, and without warning. Remove REGREG unless you are
/	absolutely certain.
/
/ 3.	mfp[id]	ss
/	Handled like mov ss,-(sp).
/
/ 4.	mtp[id] dd
/	Handled like mov (sp)+,dd.
/
/ 5.	jsr r,dd
/	The hardware evaluates, but does not fetch, dd and then pushes
/	the initial value of r onto the stack. "backup" assumes that the
/	fault is in the stack push (you shouldn't be jsr-ing via non-
/	existent locations!) and adjusts both the dd register and the sp.
/
/ 6.	FP11 op
/	The operands may be 2, 4 or 8 bytes long, depending upon the
/	particular operation and the "floating double" and "long integer"
/	bits.
/
/ 7.	illegal stuff like "mark"

/	Enter here with address of register save area in ssr+2.
/	First sort out op-code.

backup:
	clr	r2		/r2 stands for SSR1
	clr	bflg		/jflg = 0; bflg = 0 for byte op.
	mov	ssr+4,r0	/r0 = virtual pc.
	jsr	pc,fetch	/get instruction word.
	mov	r0,r1
	ash	$-11.,r0
	bic	$!36,r0
	mov	0f(r0),pc	/switch on xx----

/	Note that DEC's frequency-driven micro-code design means that
/	mov is usually faster than jmp, but it zaps the condition codes!

0:	t00;	t01;	t02;	t03;	t04;	t05;	t06;	t07
	t10;	t11;	t12;	t13;	t14;	t15;	t16;	t17

t00:
	incb	bflg		/bflg = 1 for 2 byte op.
t10:
	mov	r1,r0
	swab	r0
	bic	$!16,r0
	mov	0f(r0),pc	/switch on 00x--- or 10x---

0:	u0;	u1;	u2;	u3;	u4;	u5;	u6;	u7

u6:
	mov	r1,r0
	ash	$-5,r0
	bic	$!16,r0
	mov	0f(r0),pc	/switch on 006x-- or 106x--

u60 = u5	/ ror, rorb
u61 = u5	/ rol, rolb
u62 = u5	/ asr, asrb
u63 = u5	/ asl, aslb
u64 = u7	/ mark, mtps
u67 = u5	/ sxt, mfps

0:	u60;	u61;	u62;	u63;	u64;	u65;	u66;	u67

u66:	/ simulate mtp[id] with mov (sp)+,dd
	bic	$4000,r1	/ make mode (sp)+
	br	1f

u65:	/ simulate mfp[id] with mov ss,-(sp)
	ash	$6,r1		/ move dd to ss
	bis	$46,r1		/ make dd -(sp)
1:	clrb	bflg		/ 2 byte ops - bflg ends up 1.
	br	t01		/ treat as mov

u4:	/ jsr = x04---
	mov	r1,r0
	jsr	pc,setreg	/set up dd adjust
	bis	$173000,r2	/delta(sp) = 2
	rts	pc

t07:	/ EIS = 07----
	incb	bflg		/bflg = 1 for 2 byte op

u0:	/ swab, jmp = x00---
u5:	/ single operand = x05-dd
.if	MODE2D
	mov	r1,r0		/ (r)+ faults to
	ash	$-3,r0		/ INITIAL state
	bic	$!7,r0
	cmp	r0,$2		/ mode 2?
	bne	1f
	mov	$fixstk,pc	/ yes - fix stack
1:
.endif	/MODE2D
	mov	r1,r0

/	setreg - put reg + delta into low byte of r2

setreg:
	mov	r0,-(sp)	/ mode+reg in r0
	bic	$!7,r0
	bis	r0,r2		/put reg in r2
	mov	(sp)+,r0
	ash	$-3,r0
	bic	$!7,r0		/mode in r0
	movb	adj(r0),r0	/adjust in r0
	movb	bflg,-(sp)	/push shift count
	bne	1f		/not byte operand - skip
	bit	$2,r2		/byte operand with
	beq	3f		/sp or pc
	bit	$4,r2		/is really word op
	beq	3f		/so drop thru
1:
	bit	$10,r0		/length dependent?
	beq	3f		/no - dont shift
2:
	asl	r0		/delta = 2**bflg
	decb	(sp)
	bgt	2b
3:
	tst	(sp)+		/clean stack
	bisb	r0,r2		/put delta in r2
	rts	pc

adj:	.byte	0, 0, 10, 20, -10, -20, 0, 0

t01:	/ mov
t02:	/ cmp
t03:	/ bit
t04:	/ bic
t05:	/ bis
t06:	/ add
t16:	/ sub
	incb	bflg		/bflg = 1 for 2 byte op
t11:	/ movb
t12:	/ cmpb
t13:	/ bitb
t14:	/ bicb
t15:	/ bisb
	mov	r1,r0		/ op ss,dd
	ash	$-6,r0
	jsr	pc,setreg	/ for ss
	swab	r2
	mov	r1,r0
	jsr	pc,setreg	/ for dd

/	r2 now has delta(ss) in hi byte and delta(dd) in low byte.
/	Evaluate and fetch ss, zeroing delta(dd) if a fault occurs.
/	See fetch for details of how this happens.
/ 1.	If delta(dd) == 0 dont bother.

	bit	$370,r2
	beq	1f

/ 2.	If mode(ss) is R, it can't have faulted.

	bit	$7000,r1
	beq	1f

/ 3.	register(ss) == register(dd)?

	mov	r2,-(sp)
	bic	$174370,(sp)
	cmpb	1(sp),(sp)+
	bne	3f

/ 4.	Yes, but if delta(ss) == 0 we may assume a dd fault

	mov	$u7,pc
1:
	rts	pc
3:

.if	MODE2S
/	Try to deal with problems caused by /34 etc. faulting op (r)+,dd
/	source to INITIAL state.

	mov	r1,r0
	bic	$!7000,r0	/r0 = mode(ss)
	cmp	$2000,r0	/ (r)+?
	beq	4f		/yes - problems
	jsr	pc,1f		/no - get r
	jsr	pc,2f		/adjust for increment
	jsr	pc,3f		/fetch it
	rts	pc		/and quit
4:

/	Its an op (r)+,dd. If a fetch on r fails to fault, then it was
/	a destination fault.

	mov	r2,-(sp)	/save SSR1
	jsr	pc,1f		/get r
				/NB - no adjust
	jsr	pc,3f		/fetch it
	tstb	r2		/did it fault?
	beq	4f		/yes - try r-2
	tst	(sp)+		/no - clean up
	rts	pc		/its a dd fault
4:

/	Fetch on r faults. If fetch on r-2 also faults, then its an ss fault.

	mov	(sp)+,r2	/restore SSR1
	jsr	pc,1f		/get r
	jsr	pc,2f		/adjust for increment
	jsr	pc,3f		/fetch it
	tstb	r2		/did it fault?
	bne	u7		/no - failure
	ash	$-6,r1		/yes - place ss
	br	fixstk		/and fix stack
1:
.endif	/MODE2S

/	Zero delta(dd) if the ss faults (see fetch).
/	Start ss cycle by picking up the register value.

	mov	r1,r0
	ash	$-6,r0
	bic	$!7,r0		/r0 = reg no.
	movb	_regloc(r0),r0
	asl	r0
	add	ssr+2,r0
	mov	(r0),r0		/r0 = contents reg(ss)
.if	MODE2S
	rts	pc
2:
.endif	/MODE2S

/	If register incremented, must decrement before fetch. If register
/	decremented, happened before fetch.

	bit	$174000,r2
	ble	4f
	dec	r0
	bit	$10000,r2
	beq	4f		/delta = 1
	dec	r0		/delta = 2
4:
.if	MODE2S
	rts	pc
3:
.endif	/MODE2S

/	If mode 6 or 7 fetch and add X(r)

	bit	$4000,r1
	beq	4f
	bit	$2000,r1
	beq	4f
	mov	r0,-(sp)	/mode 6 or 7
	mov	ssr+4,r0	/virtual pc
	add	$2,r0
	jsr	pc,fetch	/get X
	add	(sp)+,r0	/add it in
4:

/	Fetch operand

	jsr	pc,fetch

/	If mode 3, 5 or 7 fetch *.

	bit	$1000,r1
	beq	4f
	bit	$6000,r1
	bne	fetch
4:
	rts	pc


t17:	/ FP11 op
.if	FPU
	incb	bflg		/assume 2 bytes
	mov	r1,r0
	ash	$-7,r0
	bic	$!36,r0
	mov	0f(r0),pc	/switch on 17x(x..)--

fp50 = u5	/ stexp is 2 bytes
fp64 = u5	/ ldexp is 2 bytes (Check this sometime!)

0:	fp00;	fp04;	fp10;	fp14;	fp20;	fp24;	fp30;	fp34;
	fp40;	fp44;	fp50;	fp54;	fp60;	fp64;	fp70;	fp74;

fp00:	/ ldfps, stfps, stst + others which can't fault.
	cmp	r1,$170300	/stst?
	bhis	1f		/stst is 4 bytes (for this purpose)
	mov	$u5,pc		/ldfps, stfps are 2 bytes.

fp54:	/ stcfi
fp70:	/ ldcif
	stfps	r0		/if long integer mode
	bit	$100,r0
	bne	1f		/its 4 bytes
	mov	$u5,pc		/else its really 2 bytes

fp60:	/ stcfd
fp74:	/ ldcfd
	incb	bflg		/assume 4 bytes
	stfps	r0
	tstb	r0		/if floating double
	bmi	0f		/its really is
	br	1f		/else its 8 bytes

fp04:	/ clrf tstf, absf, negf
fp10:	/ mulf
fp14:	/ modf
fp20:	/ addf
fp24:	/ ldf
fp30:	/ subf
fp34:	/ cmpf
fp40:	/ stf
fp44:	/ divf
	incb	bflg		/assume 4 bytes
	stfps	r0
	tstb	r0		/if not floating double
	bpl	0f		/it really is
1:	incb	bflg		/else its 8 bytes
0:
.if	FPPNOT
	br	fixstk
.endif	/FPPNOT
.if	FPPNOT - 1
	mov	r1,r0
	mov	$setreg,pc
.endif	/FPPNOT - 1
.endif	/FPU
u1:	/br
u2:
u3:
u7:	/illegal
	incb	jflg
	rts	pc

.if	FIXSP
/	Operation was aborted with registers in INITIAL state, so
/	no backup is required (why don't they all work that way?).
/	However, "grow()" examines the sp, and grows the stack iff it
/	points outside the valid stack. If the sp is in its INITIAL
/	state, it will point to a valid stack address, and SIGSEG will
/	occur. To avoid this, the sp seen by "grow()" must be adjusted
/	to its FINAL state. The sp seen by "grow()" is kept in the
/	external variable _backsp during the call to _backup().

	.globl _backsp

fixstk:
	mov	r1,r0
	bic	$!7,r0
	cmp	$6,r0		/affecting sp?
	bne	3f		/no - quit
	mov	r1,r0
	ash	$-3,r0
	bic	$!7,r0		/r0 = mode
	movb	adj(r0),r0	/get adjust
	tstb	bflg		/byte op?
	bne	0f		/no - skip
	incb	bflg		/yes - on sp is word op
0:	ash	$-3,r0		/r0 = adjust proper
	bit	$1,r0		/1 or -1
	beq	2f		/no - deferred or zero
1:	asl	r0		/exponentiate
	decb	bflg		/bflg times
	bgt	1b
2:	add	r0,_backsp	/adjust the saved sp
3:	clr	r2		/no register adjust
	rts	pc
.endif	/FIXSP

fetch:
	bic	$1,r0
	mov	nofault,-(sp)
	mov	$1f,nofault
	mfpi	(r0)
	mov	(sp)+,r0
	mov	(sp)+,nofault
	rts	pc

1:
	mov	(sp)+,nofault
	clrb	r2			/ clear out dest on fault
	mov	$-1,r0
	rts	pc

.bss
bflg:	.=.+1
jflg:	.=.+1
.text

	.globl	_fubyte, _subyte
	.globl	_fuibyte, _suibyte
	.globl	_fuword, _suword
	.globl	_fuiword, _suiword
_fuibyte:
_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	cmp	r1,2(sp)
	beq	1f
	swab	r0
1:
	bic	$!377,r0
	rts	pc

_suibyte:
_subyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,pword
	clr	r0
	rts	pc
_fuiword:
_fuword:
	mov	2(sp),r1
fuword:
	jsr	pc,gword
	rts	pc

gword:
.if	MOVPS - 1
	mov	PS,-(sp)
	movb	$BR7,PS
.endif	/MOVPS - 1
.if	MOVPS
	mfps	-(sp)
	mtps	$BR7
.endif	/MOVPS
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpi	(r1)
	mov	(sp)+,r0
	br	1f

_suiword:
_suword:
	mov	2(sp),r1
	mov	4(sp),r0
suword:
	jsr	pc,pword
	rts	pc

pword:
.if	MOVPS - 1
	mov	PS,-(sp)
	movb	$BR7,PS
.endif	/MOVPS - 1
.if	MOVPS
	mfps	-(sp)
	mtps	$BR7
.endif	/MOVPS
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)
1:
	mov	(sp)+,nofault
.if	MOVPS - 1
	mov	(sp)+,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	(sp)+
.endif	/MOVPS
	rts	pc

err:
	mov	(sp)+,nofault
.if	MOVPS - 1
	mov	(sp)+,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	(sp)+
.endif	/MOVPS
	tst	(sp)+
	mov	$-1,r0
	rts	pc
	rts	pc

	.globl	_copyin, _copyout
	.globl	_copyiin, _copyiout
_copyiin:
_copyin:
	jsr	pc,copsu
1:
	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyiout:
_copyout:
	jsr	pc,copsu
1:
	mov	(r0)+,-(sp)
	mtpi	(r1)+
	sob	r2,1b
2:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	clr	r0
	rts	pc

copsu:
	mov	(sp)+,r0
	mov	r2,-(sp)
	mov	nofault,-(sp)
	mov	r0,-(sp)
	mov	10(sp),r0
	mov	12(sp),r1
	mov	14(sp),r2
	asr	r2
	mov	$1f,nofault
	rts	pc

1:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	mov	$-1,r0
	rts	pc

	.globl	_idle, _waitloc
_idle:
.if	MOVPS - 1
	mov	PS,-(sp)
	clrb	PS
.endif	/MOVPS - 1
.if	MOVPS
	mfps	-(sp)
	mtps	$BR0
.endif	/MOVPS
	wait
waitloc:
.if	MOVPS - 1
	mov	(sp)+,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	(sp)+
.endif	/MOVPS
	rts	pc
	.data
_waitloc:
	waitloc
	.text

	.globl	_save
_save:
	mov	(sp)+,r1
	mov	(sp),r0
.if	[BIGUNIX | BUFMAP]
	mov	KISA5,(r0)+
.endif
.if	[BIGUNIX * BUFMAP]
	mov	KISA4,(r0)+
.endif
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	r1,(r0)+
	clr	r0
	jmp	(r1)

	.globl	_resume
_resume:
	mov	2(sp),r0		/ new process
	mov	4(sp),r1		/ new stack
.if	MOVPS - 1
	movb	$BR7,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	$BR7
.endif	/MOVPS
	mov	r0,KISA6		/ In new process
.if	[BIGUNIX | BUFMAP]
	mov	(r1)+,KISA5
.endif
.if	[BIGUNIX * BUFMAP]
	mov	(r1)+,KISA4
.endif
	mov	(r1)+,r2
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5
	mov	(r1)+,sp
	mov	$1,r0
.if	MOVPS - 1
	clrb	PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	$BR0
.endif	/MOVPS
	jmp	*(r1)+

.if	SPLFIX - 1
	.globl	_spl0, _spl1, _spl3, _spl4, _spl5, _spl6, _spl7, _splx
.if	MOVPS - 1
_spl0:
	mov	PS,r0
	clrb	PS
	rts	pc

_spl1:
	mov	PS,r0
	movb	$BR1,PS
	rts	pc

_spl3:
	mov	PS,r0
	movb	$BR3,PS
	rts	pc

_spl4:
	mov	PS,r0
	movb	$BR4,PS
	rts	pc

_spl5:
	mov	PS,r0
	movb	$BR5,PS
	rts	pc

_spl6:
	mov	PS,r0
	movb	$BR6,PS
	rts	pc

_spl7:
	mov	PS,r0
	movb	$BR7,PS
	rts	pc

_splx:
	mov	PS,r0
	mov	2(sp),PS
	rts	pc
.endif	/MOVPS - 1
.if	MOVPS
_spl0:
	mfps	r0
	mtps	$BR0
	rts	pc

_spl1:
	mfps	r0
	mtps	$BR1
	rts	pc

_spl3:
	mov	PS,r0
	movb	$BR3,PS
	rts	pc

_spl4:
	mfps	r0
	mtps	$BR4
	rts	pc

_spl5:
	mfps	r0
	mtps	$BR5
	rts	pc

_spl6:
	mfps	r0
	mtps	$BR6
	rts	pc

_spl7:
	mfps	r0
	mtps	$BR7
	rts	pc

_splx:
	mfps	r0
	mtps	2(sp)
	rts	pc
.endif	/MOVPS
.endif	/SPLFIX - 1

	.globl	_copyseg
_copyseg:
.if	[BIGUNIX * BUFMAP] - 1
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	UISA1,-(sp)
	mov	$PUMODE+BR7,PS
	mov	10(sp),UISA0
	mov	12(sp),UISA1
	mov	UISD0,-(sp)
	mov	UISD1,-(sp)
	mov	$6,UISD0
	mov	$6,UISD1
	mov	r2,-(sp)
	clr	r0
	mov	$8192.,r1
	mov	$32.,r2
1:
	mfpi	(r0)+
	mtpi	(r1)+
	sob	r2,1b
	mov	(sp)+,r2
	mov	(sp)+,UISD1
	mov	(sp)+,UISD0
	mov	(sp)+,UISA1
	mov	(sp)+,UISA0
	mov	(sp)+,PS
	rts	pc
.endif
.if	[BIGUNIX * BUFMAP]
.if	MOVPS - 1
	mov	PS,-(sp)
.endif	/MOVPS - 1
.if	MOVPS
	mfps	-(sp)
.endif	/MOVPS
	mov	KISA5,-(sp)
	mov	KISA4,-(sp)
	mov	r2,-(sp)
.if	MOVPS - 1
	movb	$BR7,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	$BR7
.endif	/MOVPS
	mov	14(sp),KISA5	/ where to
	mov	$120000,r1
	mov	12(sp),KISA4	/ where from
	mov	$100000,r0
	mov	$10,r2		/ how much
0:
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	sob	r2,0b
	mov	(sp)+,r2
	mov	(sp)+,KISA4
	mov	(sp)+,KISA5
.if	MOVPS - 1
	mov	(sp)+,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	(sp)+
.endif	/MOVPS
	rts	pc
.endif

	.globl	_clearseg
_clearseg:
.if	[BIGUNIX | BUFMAP] - 1
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	$PUMODE+BR7,PS
	mov	6(sp),UISA0
	mov	UISD0,-(sp)
	mov	$6,UISD0
	clr	r0
	mov	$32.,r1
1:
	clr	-(sp)
	mtpi	(r0)+
	sob	r1,1b
	mov	(sp)+,UISD0
	mov	(sp)+,UISA0
	mov	(sp)+,PS
	rts	pc
.endif
.if	[BIGUNIX | BUFMAP]
.if	MOVPS - 1
	mov	PS,-(sp)
.endif	/MOVPS - 1
.if	MOVPS
	mfps	-(sp)
.endif	/MOVPS
	mov	KISA5,-(sp)
	mov	r2,-(sp)
	mov	$10,r2
	mov	$120000,r1
	clr	r0
.if	MOVPS - 1
	movb	$BR7,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	$BR7
.endif	/MOVPS
	mov	10(sp),KISA5
0:
	mov	r0,(r1)+
	mov	r0,(r1)+
	mov	r0,(r1)+
	mov	r0,(r1)+
	sob	r2,0b
	mov	(sp)+,r2
	mov	(sp)+,KISA5
.if	MOVPS - 1
	mov	(sp)+,PS
.endif	/MOVPS - 1
.if	MOVPS
	mtps	(sp)+
.endif	/MOVPS
.endif	/MOVPS - 1
	rts	pc
.endif

.if	PIGET
	.globl	_piget, _piput
_piget:
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	UISD0,-(sp)
	jsr	pc,2f
	mfpi	(r0)
	mov	(sp)+,r0
1:
	mov	(sp)+,UISD0
	mov	(sp)+,UISA0
	mov	(sp)+,PS
	rts	pc

_piput:
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	UISD0,-(sp)
	jsr	pc,2f
	mov	14(sp),r1
	mov	r1,-(sp)
	mtpi	(r0)
	br	1b
2:
	mov	12(sp),r0
	mov	14(sp),r1
	ashc	$-6,r0
	mov	$PUMODE+BR7,PS
	mov	r1,UISA0
	mov	$6,UISD0	/ 1 click r/w
	mov	14(sp),r0
	bic	$!77,r0
	rts	pc
.endif

	.globl	_printn, _putchar
	/ printn
	/
	/ printn(num, base)
	/ unsigned long	num;
	/ int		base;
	/
	/ print out the long 'num' in unsigned form in base 'base'.
	/

_printn:
	mov	r5,-(sp)
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	mov	10(r5),r4		/ base
	mov	6(r5),r1		/ get low part of 'num'
	mov	4(r5),r0		/ get high part of 'num'
0:
	mov	r1,r3
	mov	r0,r1			/ do high part of division
	clr	r0
	div	r4,r0
	mov	r1,r2			/ followed by low part
	div	r4,r2
	mov	r2,r1			/ r0,r1 contain dividend, r3 remainder
	add	$'0,r3			/ convert to ascii
	cmp	$'9,r3
	bge	1f
	add	$'A-'0-10.,r3
1:
	mov	r3,-(sp)
	ashc	$0,r0			/ loop until num == 0
	bne	0b
	mov	$_putchar,r4
	mov	r5,r3
	sub	$6,r3			/ end of number
0:
	jsr	pc,*r4
	tst	(sp)+
	cmp	sp,r3
	bne	0b
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	rts	pc

/ Long multiply

	.globl	lmul
lmul:
	jsr	r5,csv
	mov	6(r5),r2
	sxt	r1
	sub	4(r5),r1
	mov	10.(r5),r0
	sxt	r3
	sub	8.(r5),r3
	mul	r0,r1
	mul	r2,r3
	add	r1,r3
	mul	r2,r0
	sub	r3,r0
	jmp	cret

/ Long quotient

	.globl	ldiv
ldiv:
	jsr	r5,csv
	mov	10.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,8.(r5)
	bne	hardldiv
	mov	6.(r5),r2
	mov	4.(r5),r1
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
	com	r4
1:
	mov	r4,-(sp)
	clr	r0
	div	r3,r0
	mov	r0,r4		/high quotient
	mov	r1,r0
	mov	r2,r1
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ 11/23 clobbers this
	sub	r3,r0		/ this is the clever part
	div	r3,r0
	tst	r1
	sxt	r1
	add	r1,r0		/ cannot overflow!
1:
	mov	r0,r1
	mov	r4,r0
	tst	(sp)+
	bpl	9f
	neg	r0
	neg	r1
	sbc	r0
9:
	jmp	cret

hardldiv:
	clr	-(sp)
	mov	6.(r5),r2
	mov	4.(r5),r1
	bpl	1f
	com	(sp)
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	mov	8.(r5),r3
	bge	1f
	neg	r3
	neg	10.(r5)
	sbc	r3
	com	(sp)
1:
	mov	$16.,r4
1:
	clc
	rol	r2
	rol	r1
	rol	r0
	cmp	r3,r0
	bgt	3f
	blt	2f
	cmp	10.(r5),r1
	blos	2f
3:
	sob	r4,1b
	br	1f
2:
	sub	10.(r5),r1
	sbc	r0
	sub	r3,r0
	inc	r2
	sob	r4,1b
1:
	mov	r2,r1
	clr	r0
	tst	(sp)+
	beq	1f
	neg	r0
	neg	r1
	sbc	r0
1:
	jmp	cret

/ Long remainder

	.globl	lrem
lrem:
	jsr	r5,csv
	mov	10.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,8.(r5)
	bne	hardlrem
	mov	6.(r5),r2
	mov	4.(r5),r1
	mov	r1,r4
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	div	r3,r0
	mov	r1,r0
	mov	r2,r1
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ 11/23 clobbers this
	sub	r3,r0
	div	r3,r0
	tst	r1
	beq	9f
	add	r3,r1
1:
	tst	r4
	bpl	9f
	neg	r1
9:
	sxt	r0
	jmp	cret

/ The divisor is known to be >= 2^15.	Only 16 cycles are
/ needed to get a remainder.
hardlrem:
	mov	6.(r5),r2
	mov	4.(r5),r1
	bpl	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	mov	8.(r5),r3
	bge	1f
	neg	r3
	neg	10.(r5)
	sbc	r3
1:
	mov	$16.,r4
1:
	clc
	rol	r2
	rol	r1
	rol	r0
	cmp	r3,r0
	blt	2f
	bgt	3f
	cmp	10.(r5),r1
	blos	2f
3:
	sob	r4,1b
	br	1f
2:
	sub	10.(r5),r1
	sbc	r0
	sub	r3,r0
	sob	r4,1b
1:
	tst	4(r5)
	bge	1f
	neg	r0
	neg	r1
	sbc	r0
1:
	jmp	cret

	.globl	csv
csv:
	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

	.globl cret
cret:
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

	.globl _display, _mpid
_display:
	mov	_mpid, CSWR
	rts	pc

	.globl	_u
_u	= 140000
usize	= 16.

.if	BUFMAP
	.globl	_maparea
.if	BIGUNIX
_maparea	= 100000
.endif
.if	BIGUNIX - 1
_maparea	= 120000
.endif
.endif

PS	= 177776
BR0	= 000000
BR1	= 000040
BR3	= 000140
BR4	= 000200
BR5	= 000240
BR6	= 000300
BR7	= 000340
PUMODE	= 030000
UMODE	= 170000

SSR0	= 177572
SSR2	= 177576
CSWR	= 177570
KISA0	= 172340
KISA4	= 172350
KISA5	= 172352
KISA6	= 172354
KISA7	= 172356
KISD0	= 172300
KISD4	= 172310
KISD5	= 172312
UISA0	= 177640
UISA1	= 177642
UISD0	= 177600
UISD1	= 177602
IO	= 7600

.data
	.globl	_ka6
_ka6:	KISA6

	.globl	_cputype
_cputype:34.

stk:	0

.bss
	.globl	nofault
nofault:.=.+2
ssr:	.=.+6
saveps: .=.+2
power:	.=.+2
	.globl	_dotime
_dotime: .=.+2
