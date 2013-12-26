/* This program runs standalone to get a disk image off the
 * line printer serial port. The program "putfile" is expected
 * to be run on a unix machine connected to the line to do the
 * downloading. Although some error recovery is implemented, the
 * protocol is NOT fail safe.
 */
#include <stdio.h>
#include <sys/param.h>
#include <sys/inode.h>
#include "../saio.h"
char module[] = {"Getfile"};
struct iob iodev;
#define	PREFIX	05
#define	ACK	07
#define	NAK	011
struct device {
	int rcsr;
	int rbuf;
	int xcsr;
	int xbuf;
};
#define KLADDR ((struct device *)0177560)
#define DONE 0200
main()
{
	register int i, chk;
	register char *ptr;
	char ack, str[100];
	int devtype, ret;

	iodev.i_cc = 512;
	iodev.i_ma = iodev.i_buf;
	do {
		printf("Controller type (0-R5 1-RD 2-RA) ?");
		gets(str);
	} while (str[0] < '0' || str[0] > '2');
	devtype = (int)(str[0]-'0');
	do {
		printf("Drive # ?");
		gets(str);
	} while (str[0] < '0' || str[0] > '3');
	iodev.i_unit = (int)(str[0] - '0');
	do {
		printf("Start offset ?");
		gets(str);
	} while (sscanf(str, "%D", &iodev.i_bn) != 1);
	if (devtype == 2)
		raopen(&iodev);
	printf("Ready for putfile command\n");
	while(1) {
		chk = 0;
		ptr = iodev.i_buf;
		do {
			ack = getch();
		} while (ack != PREFIX);
		for (i = 0; i < 512; i++) {
			chk += (*ptr++ = getch());
		}
		ack = getch();
		if ((ack & 0377) != (chk & 0377)) {
			ack = NAK;
		} else {
			switch(devtype) {
			case 0:
				ret = r5strategy(&iodev, WRITE);
				break;
			case 1:
				ret = rdstrategy(&iodev, WRITE);
				break;
			case 2:
				ret = rastrategy(&iodev, WRITE);
			};
			if (ret >= 0) {
				ack = ACK;
				iodev.i_bn++;
			}
			else
				ack = NAK;
		}
		putch(ack);
	}
}
putch(c)
char c;
{
	while ((KLADDR->xcsr & DONE) == 0) ;
	KLADDR->xbuf = c;
}
getch()
{
	register char c;
	while ((KLADDR->rcsr & DONE) == 0) ;
	c = KLADDR->rbuf & 0377;
	return(c);
}
