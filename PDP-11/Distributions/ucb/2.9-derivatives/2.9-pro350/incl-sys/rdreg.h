/* These defs are for the rd disk controller on the pro 300 */
#define RD_RESTORE 020
#define RD_INIT 010
#define RD_READCOM 040
#define RD_WRITECOM 060
#define	RD_FORMATCOM 0120
#define RD_DRQ 0200
#define RD_BUSY 0100000
#define RD_OPENDED 01
#define RD_WFAULT 020000
#define RD_ERROR 0400
#define RD_DMNF 0400
#define RD_IDNF 010000
#define RD_CRC 060000
#define	RD_ILLCOM 02000

#define RDCS_BITS "\10\11ERR\14DATRQ\15SKCM\16WFLT\17DRDY"
#define RDER_BITS "\10\11DMNF\12TR0\13ILCM\15IDNF\16CRCID\17CRC"

#define RD_BAD	0177400

struct rddevice {
	int	id;
	int	dumb;
	int	err;
	int	sec;
	int	db;
	int	cyl;
	int	trk;
	int	csr;
	int	st;
};

struct rdst {
	int nsect;
	int ntrak;
	int nspc;
	int ncyl;
};

union wordval {
	short	word;
	char	byte[2];
};
