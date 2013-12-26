/* Registers and bits for the pro3xx screen and keyboard units. */
struct scdevice {
	int id;
	int dumb;
	int scsr;
	int p1c;
	int opc;
	int cmp;
	int scroll;
	int x;
	int y;
	int cnt;
	int pat;
	int mbr;
};

struct kbdevice {
	int dbuf;
	int stat;
	int mode;
	int csr;
};

#define BITMAP SEG5
#define BITBASE 0170000
#define XMAX 79
#define YMAX 23
#define ESCDONE 0
#define GETESC 1
#define GETX 2
#define GETY 3
#define LNPEROW 10
#define EOFRAME 0200
#define OPPRES 020000
#define XFERDONE 0100000
#define TIMER 01
#define CURSER 02
#define CURSON 04
#define PLOT 010
#define XFERSLP 020
#define	SCSET	040
#define FNTWIDTH 12
#define FNTLINE 10
#define GOCMD 067
#define RXERR 070
#define	RXDONE 02
#define TXDONE 01

#define CNTL 01
#define SHIFT 02
#define LOCK 04
#define STOPD 010
#define COMPOSE 0xb1
#define MTRNOME 0xb4
#define KSHIFT 0xae
#define KCNTL 0xaf
#define KLOCK 0xb0
#define	KPRNT 0x56
#define ALLUPS 0xb3
#define UP 0xaa
#define DOWN 0xa9
#define RIGHT 0xa8
#define LEFT 0xa7
#define ESC 033
