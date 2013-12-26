/	@(#)mch60.s	1.4    (Chemeng) 9/26/83
/
HTDUMP	= 0		/ 1 if dump device is TU16, TE16, TU45, TU77
TUDUMP	= 1		/ 1 if dump device is TU10
RKDUMP	= 0		/ 1 if dump device is RK05
HPBOOT	= 1		/ 1 if boot from unit 0 of rm03 required
FPU	= 1		/ 1 if 11/45 floating point present
BUFMAP	= 1		/ 1 for mapped buffers system
BIGUNIX = 1		/ 1 for kernel overlay system
SPLFIX	= 0		/ 1 if calls to spl? are converted to inline code
MOVPS	= 0		/ 1 if processor has mfps and mtps instructions
PIGET	= 0		/ 1 if some drivers do ECC correction

/	Defines to control intruction restarting and backup.

/ MODE2D for op (r)+
MODE2D	= 0		/ see comment in backup code
/
/	MODE2D = 0 on machines which fault an op (r)+ with the
/	register in its FINAL state. Define it on machines which fault an
/	op (r)+ with the register in its INITIAL state.
/
/ MODE2S for op (r)+,dd
MODE2S	= 0		/ see comment in backup code
/
/	MODE2S = 0 on machines which fault an (r)+ source with the
/	registers in their FINAL state. Define it on machines which fault an
/	(r)+ source with the registers in their INITIAL state. On these
/	machines there are 4 cases:-
/
/	Case	Before		After	Faults
/			------
/	1.		//////	<- r
/			------		none
/		  r ->	/////
/			------
/
/			------
/	2.			<- r
/			------		r, not r-2
/		  r ->	/////
/			------
/
/			------
/	3.	  r ->		<- r
/			------		r, not r-2
/			/////
/			------
/
/			------
/	4.	  r ->		<- r
/			------		r and r-2
/
/			------
/
/	Cases 2 and 3 are indistinguishable after the fact, and so a backup
/	failure must be signalled if either is detected.
/
/ FPPNOT for FP11 ops
FPPNOT	= 1		/ see comment in backup code
/
/	FPPNOT = 1 on machines which fault FP11 ops with the registers
/	in their INITIAL state if the operand was 4 or 8 bytes long. Zero it
/	on machines which fault these operations with the registers in their
/	FINAL state.
/
/ FIXSP
FIXSP	= MODE2S|MODE2D|FPPNOT	
/
/	If any operations are aborted with the registers in their INITIAL
/	state, then FIXSP must be defined to include the code for "fix"-ing
/	the stack in these cases.
/
/	To the best of our knowledge, the settings should be:-
/		(1 is defined, 0 is removed)
/
/	Machine		FPPNOT	MODE2S	MODE2D	FIXSP
/	/40		0	0	0	0
/	/34		0	1	1	1
/	/34+FP11A	0	1	1	1
/	/60		1	0	0	1
/	/60+FP11E	1	0	0	1
/	/23+KEF11A	0	0	0	0
