CFLAGS = -I./include

install: restor dumpdir
	cp restor /usr/public/v7restor
	cp dumpdir /usr/public/dumpdir
restor: restor.o lib.o
	cc restor.o lib.o -o restor
restor.o:  restor.c pdptovax.c gethead.c checksum.c spclconv.c readtape.c
	cc -c $(CFLAGS) restor.c

dumpdir: dumpdir.o lib.o
	cc dumpdir.o lib.o -o dumpdir
dumpdir.o:  dumpdir.c pdptovax.c gethead.c checksum.c spclconv.c readtape.c
	cc -c $(CFLAGS) dumpdir.c

clean:
	rm -f *.o temp blat junk foo bar restor dumpdir
