This is my PDP-11 emulator.  It should run on anything mostly-UNIXish;
I don't think I'm doing anything system-specific except possibly for
the tty and signal setup in cons_init(), which you can replace with
your preferred mechanism for getting the console SLU talking to
something useful.

Please note: this is my development directory.  I make no promises that
what's here is in compilable shape even on the development system,
never mind your system.  If you have trouble, send me mail, and if I'm
not too busy I'll see if I can help.  And, of course, if it does
compile I don't promise anything about what it'll do, though it should
bear some resemblance to a PDP-11.  The Makefile will likely need work.
The simulator is currently atrociously slow (about a factor of 50
slower than the real thing on the development machine); I hope to add
some caching of the memory management stuff in the hope of speeding
this up.

pdp11 is the emulator itself; as11 is an assembler that knows how to
generate output files of the sort pdp11 wants as input.  The .s11 files
here are some sample input files.  I tried to stick reasonably closely
to the DEC assembler syntax, though I must admit I was tempted to go
with the UNIX syntax instead...and I may yet add UNIX-style
single-digit numeric labels; they are a great convenience.

aux.shar is a shar of some auxiliary routines that you will probably
find you need.

There is basically no documentation on either as11 or pdp11 included.
I intend to write some, but haven't yet.

Of course, by the time you read it this file may be out of date, since
I won't be updating it for every little tweak I make to the programs.

Timestamp (should be updated at each change of this file):
	Mon May 20 12:15:34 GMT 1991

					der Mouse

			old: mcgill-vision!mouse
			new: mouse@larry.mcrcim.mcgill.edu
