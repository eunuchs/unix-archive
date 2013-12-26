/* This little program downloads a file given as arg1 to the
 * serial line given as arg2. It assumes that the prog. getfile
 * is already running on the slave system at the other end of
 * the line.
 */
#include <stdio.h>
#include <sgtty.h>

struct sgttyb ttyb = { B9600, B9600, 0 , 0, RAW };

#define	PREFIX	05
#define	ACK	07
#define	NAK	011
char buff[514];
main(argc, argv)
int argc;
char *argv[];
{
	register int i, chk;
	int ifl, ofl;
	int cnt;
	char ack;

	if (argc != 3 || (ifl = open(argv[1],0))<0 || (ofl = open(argv[2],2))<0
		|| ioctl(ofl, TIOCSETP, &ttyb)<0) {
		printf("Usage: putfile infile outdev\n");
		exit(2);
	}
	buff[0] = PREFIX;
	cnt = 0;
	while (read(ifl, &buff[1], 512) > 0) {
		chk = 0;
		for (i = 1; i <= 512; i++)
			chk += buff[i];
		buff[513] = chk;
		do {
			write(ofl, buff, 514);
			read(ofl, &ack, 1);
			if (ack == NAK) {
				printf("Err on blk=%d\n",cnt);
				fflush(stdout);
			}
		} while (ack != ACK);
		cnt++;
	}
}
