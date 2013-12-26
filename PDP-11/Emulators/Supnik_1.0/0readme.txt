The following is the original 'announcement' of the availablility of the
pdp8 and pdp11 simulators to a private list.  It has been edited to
remove certain sensitive pieces of information:

- - - - -

I am pleased to announce the (internal field test) for the PDP-8 and
PDP-11 simulators.  To get the sources and the simulated OS8 and RT11
disks, copy xxxxxx to your system and proceed as outlined in these notes.
Sketchy documentation on the simulators is in file 0simdoc.txt
	[ed. - the OS8 and RT11 disks are not available due to
	 licensing issues ... if you have a disk, simply copy
	 the disk to an image file and use that.]

1. The simulators work on both VAX/VMS and on Alpha/OSF1.
	[ed. - and on mips/ultrix]

2. Each simulator includes a CPU, memory, reader, punch, terminal, line
   clock, RK05 disk, and RX01 disk.  (The PDP-8 has an RF08, and the
   PDP-11 has an RL11.)

3. There's a VMS build file for each simulator:

	pdp8_build	builds PDP8.EXE
	pdp11_build	builds PDP11.EXE

	[ed. - A preliminary Makefile is provided which will build
	 the simulators, but may not be as concise or as full-purpose
	 as it could be]

   The build files include the debugger, remove that if you want.

   For OSF/1, the build command lines are:

	cc pdp8*.c scp*.c -lm -o pdp8
	cc pdp11*.c scp*.c -o pdp11

4. The simulators are only partially tested.

   I would APPRECIATE YOUR FEEDBACK.  Feel free to try small programs
   to test the EAE, FP11, and devices.  If you find bugs, feel free to
   look through the sources and send me the fixes...:-)

	[ed. - The author has set up a mail alias to receive such
	 mail.  You can send mail to him at dsmaint@pa.dec.com]

5. To get the simulated operating systems up and running (OSF/1 shown):

	% pdp8

	PDP-8 simulator V1.0
	sim> att rk0 os8.dsk
	sim> boot rk0

	. <-- OS/8 prompt

   This is pretty much instantaneous on either VAX or Alpha.  You
   must first set the date (note that OS/8 REQUIRES CAPITAL LETTERS
   FOR INPUT) and then you can type HELP or DIR to explore further:

	.DA 14-MAR-94

	.DIR

   For the PDP-11 and RT11:

	% pdp11

	PDP-11 simulator V1.0
	
	sim> att rk0 rt11.dsk
	sim> boot rk0

	RT11SJ (S) V5.04

	.
	. <-- RT11 prompt

   Note that on a slow VAX (eg, a 3100) it takes 45 seconds to get to
   the banner, and another 15-20 seconds to get to the prompts.  On
   Alpha/OSF1, it takes a few seconds to reach the prompt.  You can
   then set the date, type HELP or DIR, and so on.  RT11 accepts lower
   case input.

   The simulator is stopped by typing ^E (control-E).  This can be
   changed through the WRU register.
