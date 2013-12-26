This area contains material related to CB/UNIX. According to Wikipedia,
CB/UNIX was a variant of the UNIX operating system internal to Bell
Labs. It was developed at the Columbus, Ohio branch and was little-known
outside the company. CB/UNIX was developed to address deficiencies
inherent in Research Unix, notably the lack of interprocess communication
and file locking, considered essential for a database management
system. Several Bell System operation support system products were
based on CB/UNIX such as Switching Control Center System. The primary
innovations were power-fail restart, line disciplines, terminal types,
and IPC features similar to System V's messages and shared memory.

So far we have a scanned copy of the CB/UNIX manuals which were donated
to TUHS by Larry Cipriani. Copies of the binaries and source code would
be much appreciated. There were two volumes of manuals. The first volume
held cbunix_intro, cbunix_man1 and cbunix_man1L. The second volume held
the remaining sections. The 'L' in the scans indicates local sections of
the manuals, i.e. those elements created and maintaned at Columbus.

In an e-mail from Larry, he asked a retired CB/UNIX developer about the
major features that were added to UNIX by CB/UNIX. Was it primarily messages,
semaphores, named pipes, shared memory? The developer replied:

    Other things that immediately come to mind that we added first
    in Columbus Unix were power-fail restart (myself and Jim McGuire did the
    initial work) and line-disciplines and terminal types (Bill Snider did
    the initial work).  Hal Person (or Pierson?) also rewrote the original
    check disk command into something that was useful by someone other than
    researchers.  Bill Snider and Hal Pierson were really instrumental in
    taking UNIX from research and applying it to SCCS (Switching Control
    Center System).  I worked with them when I first hired on.  When we
    first used UNIX on an 11/20 with core memory it was written in assembler
    (1974).  It quickly went through "B" and we started using the C version
    in early 1975 as I recall. We also did some enhancements to the scheduling
    algorithms in UNIX to make them more "real-time" capable.
