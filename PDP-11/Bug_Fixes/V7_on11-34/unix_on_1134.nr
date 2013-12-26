.de np
.ti +8
..
.de fo
'sp 5
..
.wh -5 fo
.ls 2 
.ce 6
Implementing UNIX on a PDP-11/34
or
What does the `F' in "RK05-F" really stand for ?
.sp
Dave Horsfall
Computing Services Unit (CSU)
University of NSW
.np
This article is relates some of the author's experiences in
implementing UNIX on a PDP-11/34.
My efforts were not quite as straight-forward
as they could have been, since all my previous experience
has been with 11/40's.
This article will point out some of the major differences between
the two models that affect system implementation, and gives some advice
to would-be purchasers of 11/34's intending to run UNIX.
.np
Most difficulties appeared when trying to transport an 11/40 system
to the 11/34.
The first difficulty that cropped up (apart from the lack of a console;
I'll come to that later) is the lack of a stack limit register.
This was actually the result of a modification to UNIX to
utilize the register should it exist, to prevent the kernel
stack from smashing the per-user area.
Also, the code to handle bus traps had a few nasty side effects.
This showed up when using `m40.s' as a basis for the low core program.
This meant that all code referring to the
stack limit register had to be `conditionalised'.
This affects `mch.s' and `once.c'.
If the FPU floating point unit is installed (as it usually is;
at least on the 11/34's I have seen) then `mch.s' must be changed
to support it.
The CSU system generation procedures assume
a generalised `mch.s' program with appropriate features conditionally
assembled in; `mkconf' will generate a file to be prefixed to `mch.s'
containing the definitions such as "FPU\ =\ 1" etc.
`Mkconf' has been extended to recognize keywords such as "stacklimit",
"fpu" etc and generate the appropriate prefix file.
Since we generate most of the UNIX systems on campus
for PDP's of various configurations, 'mkconf' is a great help.
.np
The second difficulty is the lack of a conventional console.
Given that UNIX refers to the switch register quite a lot during
various stages of booting and running, this is indeed quite a problem.
Instead of a conventional switch register and display, there is an
arrangement of little buttons and a seven-segment display, vaguely
reminiscent of a pocket calculator.
There is no switch register as such; you have to button in a number
(which appears in the display; it's quite fun just watching it)
then press a button to actually load the damned thing, whereupon
an LED come on to indicate that the display is indeed displaying
the switch register (as opposed to displaying something else
e.g the last value examined).
This lack of a display such as `ADDR/DATA' also means that you can't
tell what the system is doing - if it is doing anything at all.
.np
The boot procedure is quite funny (funny queer; not funny ha-ha;
although it does have its hilarious aspects).
While holding down the `CNTRL' button, one presses the `HALT'
button, then the `BOOT' button.
The ROM console emulator program then comes alive on the console
terminal.
If you are booting from say `RK0' you then type in `DK0'
(which must be in upper case, although you can say just `DK').
It would be a good idea at this point if the switch register
contained neither `0' nor `173030' (for obvious reasons; none
of this "Load Addr 773110; Start" business).
The value I normally use is plain `1'.
At least there is a button to clear the switch register.
Given that the ROM expects upper case, and that UNIX prefers
you to talk in lower case, it is quite easy for problems
to occur here.
.np
The subtitle of this article refers to the fact that the two
11/34 systems I have installed both have an RK05-J (the normal
one) and an RK05-F (double density; equivalent logically to two
drives but on one cartridge).
DEC must be flogging lots of these.
The implications of this is that there is but one removable volume, and
UNIX (for all its reliability - sorry Ian) requires two for effective
system backups.
The second volume does not have to be a disk; it can be a mag-tape drive.
Given that you can copy half of the `F' on to the `J', followed
by the other half; the question that naturally arises is "How the
hell do you back up the `J' if it is not being used
as pure scratch ?".
How indeed !
The only technique is to bring up a stand-alone utility such as `RKDF'
and when the `F' is nicely backed up, you copy the `J' on to
one half of it, copy this in turn to another `J' then restore
said half of the `F'.
This also introduces another problem; that of recovering from
file system loss.
Should a file be spread over both halves of the `F', it will
naturally appear on two `J's, and the only way to recover it
(for the ordinary mortal user) is to restore the whole damned
lot in a manner analogous to the backup procedure.
It is also possible to treat the RK as a tape
and use `dump/restor' on it.
However, you still need some sort of emergency backup system
just in case you can't even use `restor'.
Ken Higgs of Water Research Laboratory can tell a few stories about this.
In other words, one removable drive (with or without a fixed drive)
is not enough.
You need either a second removable drive or a mag-tape drive as well.
Life may not be meant to be easy, but there is no point in
making it hard either.
.np
There is another consequence of the RK05-F in that UNIX regards
it as two platters, and will therefore try to initiate a seek
on one of them while the other is busy.
Needless to say, this doesn't quite work.
We modified our RK driver to implement the concept of
`concatenated volumes', in which several contiguous drives may
be treated as one enormous drive with one scheduling queue.
Should this then be specified as "optimized" (with the funny rotated
blocks) the driver does the right thing and plants block #1 in the
middle of the whole virtual disk (this calculation is done automatically
by the driver; it knows how big each individual drive is and hence knows
where the middle of the whole lot is).
This does not work too well however on multiple physical drives,
but it is better than nothing.
.np
All in all, transporting UNIX from a PDP-11/40 to an 11/34 is
not quite a trivial exercise, and it is arguable whether the problems
encountered stem from the idiosyncracies of the 11/34 or from
UNIX itself ...

-- Dave
