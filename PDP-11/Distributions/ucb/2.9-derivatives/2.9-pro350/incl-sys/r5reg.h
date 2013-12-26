#define	R5_SIZE	800
/* These are the register definitions for the RX50 on the pro3xx. */
#define	R5_BUSY	010
#define R5_ERROR 0200
#define	R5_READCOM	0110
#define	R5_WRITECOM	0170
#define	R5_STATUS	0
#define	R5_EXTCOM	0120
#define	R5_SPARM	03
#define	R5_ICRC	0140
#define	R5_DCRC	0200
#define	R5_IDNF	070
#define	R5_DMNF	060
#define	R5_DDM	0364
#define	R5_INIT 060

struct r5device {
	int id;
	int maint;
	int cs0;
	int cs1;
	int cs2;
	int cs3;
	int cs4;
	int cs5;
	int dbo;
	int ca;
	int sc;
	int dbi;
};
