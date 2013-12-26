/* These two fuctions handle the pro 300's clock
 * This code is defunct at the end of the century.
 * Will Unix still be here then??
 */
#include "whoami.h"
#if PDP11 == 21
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ivecpos.h>

#define	MINSEC	60l
#define	HRSEC	3600l
#define	DAYSEC	86400l

#define	CLK_VRT	0200
#define	CLK_UIP	0200
#define	CLK_RATE	052
#define	CLK_ENABLE	0106
#define	CLK_SET	0206

struct cldevice {
	int	sec;
	int	secalrm;
	int	min;
	int	minalrm;
	int	hr;
	int	hralrm;
	int	dayofwk;
	int	day;
	int	mon;
	int	yr;
	int	csr0;
	int	csr1;
	int	csr2;
	int	csr3;
};

#define	CLKADDR	((struct cldevice *)0173000)

short dayyr[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, };
/* Starts the pro 300 clock. Called from clkstart... */
proclock()
{
	register int tmp1, tmp2;

	CLKADDR->csr1 = CLK_SET;
	while (CLKADDR->csr0 & CLK_UIP)
		;
	/* If the clock is valid, use it. */
	if ((CLKADDR->csr3 & CLK_VRT) && ((CLKADDR->csr1&07)==06)) {
		/* Convert yr,mon,day,hr,min,sec to sec past Jan.1, 1970. */
		tmp2 = 0;
		for (tmp1 = 70; tmp1 < CLKADDR->yr; tmp1++) {
			tmp2 += 365;
			/* I just luv leap years... */
			if ((tmp1 % 4) == 0)
				tmp2++;
		}
		tmp2 += (dayyr[CLKADDR->mon-1]+CLKADDR->day-1);
		if ((CLKADDR->yr % 4) == 0 && CLKADDR->mon > 2)
			tmp2++;
		/* Finally got days past Jan. 1,1970. the rest is easy.. */
		time = tmp2*DAYSEC+CLKADDR->hr*HRSEC+
			CLKADDR->min*MINSEC+CLKADDR->sec;
	}
	CLKADDR->csr0 = CLK_RATE;
	ienable(CLPOS);
	tmp1 = CLKADDR->csr2;
	CLKADDR->csr1 = CLK_ENABLE;
}
/* Set the time of day clock, called via. stime system call.. */
sdtime()
{
	register int tmp1, tmp3;
	long tmp2, tmp4;

	CLKADDR->csr1 = CLK_SET;
	while (CLKADDR->csr0 & CLK_UIP)
		;
	/* The reverse of above, sec. past Jan. 1,1970 to yr, mon... */
	tmp2 = time/3600;
	tmp4 = tmp2 = tmp2/24;
	tmp1 = 69;
	while (tmp2 >= 0) {
		tmp3 = tmp2;
		tmp2 -= 365;
		tmp1++;
		if ((tmp1 % 4) == 0)
			tmp2--;
	}
	/* Got the year... */
	CLKADDR->yr = tmp1;
	tmp1 = -1;
	do {
		tmp2 = tmp3-dayyr[++tmp1];
		if ((CLKADDR->yr % 4) == 0 && tmp1 > 1)
			tmp2--;
	} while (tmp2 >= 0);
	/* Finally, got the rest... */
	CLKADDR->mon = tmp1;
	CLKADDR->day = tmp3-dayyr[tmp1-1]+1;
	if ((CLKADDR->yr%4) == 0 && tmp1 > 2)
		CLKADDR->day--;
	tmp2 = time-(tmp4*DAYSEC);
	CLKADDR->hr = tmp2/3600;
	tmp2 = tmp2%3600;
	CLKADDR->min = tmp2/60;
	tmp2 = tmp2%60;
	CLKADDR->sec = tmp2;
	tmp1 = CLKADDR->csr2;
	tmp1 = CLKADDR->csr3;
	CLKADDR->csr1 = CLK_ENABLE;
}
/* These two little functions interface to the pro's interrupt
 * handler chips. Most of the work in setting these up is done
 * in firmware at powerup, so just enable, disable each device
 * as required.
 * Note: The device numbers are kept in "...sys/h/ivecpos.h
 */

struct indevice {
	int dat;
	int csr;
};

char	imask[3] = { 0, 0, 0 };

#define	IVECADDR	((struct indevice *)0173200)
#define	IVEC_CIM	050
#define	IVEC_SIM	070
#define	IVEC_CIR	0110
#define	IVEC_PACR	0300
#define	IVEC_M5T7	0245
#define	IVEC_M0T4	0201
#define	IVEC_PIMR	0260

/* Enable interrupts for "device"
 * returns 1 - was enabled
 *         0 - was disabled
 */
ienable(device)
int device;
{
	register int dev, intr;
	register struct indevice *iaddr;
	int tmp;

	dev = device>>3;
	intr = device & 07;
	iaddr = IVECADDR+dev;
	iaddr->csr = IVEC_CIM | intr;
	tmp = (imask[dev]>>intr) & 01;
	imask[dev] |= (01<<intr);
	return(tmp);
}

/* Disable interrupts for "device", return as above. */
idisable(device)
int device;
{
	register int dev, intr;
	register struct indevice *iaddr;
	int tmp;

	dev = device>>3;
	intr = device & 07;
	iaddr = IVECADDR+dev;
	iaddr->csr = IVEC_SIM | intr;
	iaddr->csr = IVEC_CIR | intr;
	tmp = (imask[dev]>>intr) & 01;
	imask[dev] &= ~(01<<intr);
	return(tmp);
}

ivinit()
{
	register struct indevice *iaddr = IVECADDR;
	register int i;

	for (i = 0; i < 3; i++) {
		iaddr->csr = IVEC_PACR;
		iaddr->dat = 0377;
		iaddr->csr = IVEC_M5T7;
		if (i > 0)
			iaddr->csr = IVEC_M0T4;
		iaddr->csr = IVEC_PIMR;
		iaddr++;
	}
}
#endif
