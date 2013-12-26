#
# Modified makefile for virtual tape drive.
#
# Note that the first 16 bytes must be stripped from vtboot before it
# is toggled in at 70000. As well, the first 16 bytes must be stripped
# from boot so that it can be used with vtserver.
#
# A \000 should be placed at the end of the text file opened by cat
#
# All other files used by vtserver (restor cat icheck mkfs + all dumps)
# do not need any modifications
#
#
CFLAGS = -O

all:	/usr/lib/libsa.a srt0.o restor cat icheck mkfs \
	mtboot maketape boot vtboot

clean:
	rm /usr/lib/libsa.a restor cat icheck mkfs boot

cp cmp:
	@echo "standalone stuff-- examine this makefile before running"

xcp:	all
	mkdir /stand
	cp restor cat mkfs icheck /stand
	rm restor cat mkfs icheck
	cp boot /
	rm boot
	rm *.o

/usr/lib/libsa.a:	SYS.o conf.o hp.o ht.o prf.o rk.o rp.o tm.o vt.o rl.o
	ar rv /usr/lib/libsa.a $?

restor:	srt0.o /usr/lib/libsa.a
	sh cc+ld-stand restor

dd:	srt0.o /usr/lib/libsa.a
	cc -c dd.c
	ld -s -o dd srt0.o dd.o -lsa -lc

cat:	srt0.o /usr/lib/libsa.a
	cc -c cat.c
	ld -s -o cat srt0.o cat.o -lsa -lc
	rm cat.o

zcat:	srt0.o /usr/lib/libsa.a
	cc -c compress.c
	ld -s -o zcat srt0.o compress.o -lsa -lc

mkfs:	srt0.o /usr/lib/libsa.a
	sh cc+ld-stand mkfs

icheck:	srt0.o /usr/lib/libsa.a
	sh cc+ld-stand icheck

mtboot:	mtboot.s
	as -o mtboot mtboot.s
	strip mtboot

vtboot:	vtboot.s
	as -o vtboot vtboot.s
	strip vtboot
	dd if=vtboot of=vtboot.pdp bs=16 skip=1

maketape:	maketape.c
	cc -o maketape maketape.c

boot:	boot.o M.o /usr/lib/libsa.a
	ld -s -o boot M.o boot.o -lsa -lc
	dd if=boot of=ddboot bs=16 skip=1

distr:	restor cat mkfs boot mtboot maketape
	maketape /dev/nrmt0 tapedir
	dump 0f /dev/nrmt0 /dev/rp11
	dump 0f /dev/rmt0 /dev/rp13

rkdistr:	restor cat mkfs boot mtboot maketape
	maketape /dev/nrmt0 rktapedir
	mv /v7/rp*unix /v7/hp*unix /v7/usr
	dump 0f /dev/nrmt0 /dev/rp11
	mv /v7/usr/*unix /v7
	sh cpgroup /v7/usr/src/cmd `cd /v7/usr/src/cmd; echo [a-m]*`
	dump 0f /dev/nrmt0 /dev/rrk2
	sh cpgroup /v7/usr/src/cmd `cd /v7/usr/src/cmd; echo [n-z]*`
	dump 0f /dev/nrmt0 /dev/rrk2
	sh cpgroup /v7/usr/src `cd /v7/usr/src/; echo [d-z]*`
	dump 0f /dev/nrmt0 /dev/rrk2
	sh cpgroup /v7/usr lib dmr games include mdec pub spool tmp
	dump 0f /dev/nrmt0 /dev/rrk2
	sh cpgroup /v7/usr doc
	dump 0f /dev/nrmt0 /dev/rrk2
	sh cpgroup /v7/usr dict sys man
	dump 0f /dev/rmt0 /dev/rrk2
