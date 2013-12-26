/ low core

.data
ZERO:

br4 = 200
br5 = 240
br6 = 300
br7 = 340

. = ZERO+0
	br	1f
	4

/ trap vectors
	trap; br7+0.		/ bus error
	trap; br7+1.		/ illegal instruction
	trap; br7+2.		/ bpt-trace trap
	trap; br7+3.		/ iot trap
	trap; br7+4.		/ power fail
	trap; br7+5.		/ emulator trap
	start;br7+6.		/ system  (overlaid by 'trap')

. = ZERO+40
.globl	start, dump
1:	jmp	start


. = ZERO+60
	klin; br4
	klou; br4

. = ZERO+100
	kwlp; br6
	kwlp; br6

. = ZERO+114
	trap; br7+10.		/ 11/70 parity

. = ZERO+160
	rlio; br5

. = ZERO+224
	tmio; br5

. = ZERO+240
	trap; br7+7.		/ programmed interrupt
	trap; br7+8.		/ floating point
	trap; br7+9.		/ segmentation violation

. = ZERO+264
	rx2io; br5

/ floating vectors
. = ZERO+300
/dl 300
	klin; br4+1.
	klou; br4+1.
	klin; br4+2.
	klou; br4+2.
	klin; br4+3.
	klou; br4+3.
	klin; br4+4.
	klou; br4+4.

. = ZERO+1000

	jmp	dump	/ jump to core dump code

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

.text
.globl	call, trap

.globl	_klrint
klin:	jsr	r0,call; jmp _klrint
.globl	_klxint
klou:	jsr	r0,call; jmp _klxint

.globl	_clock
kwlp:	jsr	r0,call; jmp _clock


.globl	_rlintr
rlio:	jsr	r0,call; jmp _rlintr

.globl	_tmintr
tmio:	jsr	r0,call; jmp _tmintr

.globl	_rx2intr
rx2io:	jsr	r0,call; jmp _rx2intr

