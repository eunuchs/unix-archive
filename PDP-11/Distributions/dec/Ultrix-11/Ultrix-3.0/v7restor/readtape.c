#include "stdio.h"
#include "signal.h"
#include "sys/param.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/fblk.h"
#include "sys/filsys.h"
#include "sys/dir.h"
#include "dumprestor.h"
/*
 * Do the tape i\/o, dealling with volume changes
 * etc..
 */
readtape(b)
char *b;
{
	register i;
	struct spcl tmpbuf;
	extern int bct, mt, volno, eflag;
	extern char tbf[], *magtape;
	extern ino_t curino;

	if (bct >= NTREC) {
		for (i = 0; i < NTREC; i++)
		/* fixed 9june85 pdbain
			((struct spcl *)&tbf[i*BSIZE])->c_magic = 0; */
		/* spcl struct is 600 bytes long on pdp, and c_magic
		 * is at an offset of 18 bytes */
			tbf[(i*BSIZE) + 18] = 0;
		bct = 0;
		if ((i = read(mt, tbf, NTREC*BSIZE)) < 0) {
 			printf("Tape read error: inode %u\n", curino);
 			eflag++;
 			for (i = 0; i < NTREC; i++)
 				clearbuf(&tbf[i*BSIZE]);
		}
		if (i == 0) {
			bct = NTREC + 1;
			volno++;
loop:
			flsht();
			close(mt);
			printf("Mount volume %d\n", volno);
			while (getchar() != '\n')
				;
			if ((mt = open(magtape, 0)) == -1) {
				perror(magtape);
				goto loop;
			}
			if (readhdr(&tmpbuf) == 0) {
				printf("Not a dump tape.Try again\n");
				goto loop;
			}
			if (checkvol(&tmpbuf, volno) == 0) {
				printf("Wrong tape. Try again\n");
				goto loop;
			}
			readtape(b);
			return;
		}
	}
	copy(&tbf[(bct++*BSIZE)], b, BSIZE);
}
