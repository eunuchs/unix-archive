CFLAGS = -I./include -O -DDEFTAPE=\"blort\"
OBJ = lib.o pdptovax.o gethead.o getfile.o readtape.o checksum.o

all: restor dumpdir
install: all
	install restor /usr/public/v7restor
	install dumpdir /usr/public/v7dumpdir
restor: restor.o $(OBJ)
	cc restor.o $(OBJ) -o restor

dumpdir: dumpdir.o $(OBJ)
	cc dumpdir.o $(OBJ) -o dumpdir

clean:
	rm -f *.o restor dumpdir
