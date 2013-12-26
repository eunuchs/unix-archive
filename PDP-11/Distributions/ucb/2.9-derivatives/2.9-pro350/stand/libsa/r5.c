/*
 * Standalone rx50 driver for pro3xx
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "../saio.h"
#include <sys/r5reg.h>

#define R5ADDR ((struct r5device *) 0174200)

r5strategy(io, func)
register struct iob *io;
{
	register char *ptr;
	register int i;
	int cc;
	daddr_t bn, xfer;
	unsigned short sn, tn;

	ptr = io->i_ma;
	cc = io->i_cc;
	xfer = 0;
	while (cc > 0) {
		bn = io->i_bn+xfer;
		if (bn >= R5_SIZE)
			return(-1);
		tn = bn/10;
		sn = bn%10;
		/* This interleave and skew is compatible with the
		 * DEC mscp Qbus controller for RX50's
		 */
		sn = ((sn<<1)%10 + (sn<<1)/10 + (tn<<1))%10;
		/* This is consistent with DEC in starting at trk 1 */
		tn = (tn+1) % 80;
		while ((R5ADDR->cs0 & R5_BUSY) == 0)
			;
		R5ADDR->cs1 = tn;
		R5ADDR->cs2 = sn+1;
		if (func == READ) {
			R5ADDR->cs0 = R5_READCOM|(((io->i_unit&03)<<1)&06);
			R5ADDR->sc = 0;
		}
		else
		{
			R5ADDR->cs0 = R5_WRITECOM|(((io->i_unit&03)<<1)&06);
		}
		while ((R5ADDR->cs0 & R5_BUSY) == 0)
				;
		R5ADDR->ca = 0;
		for (i = 0; i < 512; i++) {
			if (R5ADDR->cs0 & R5_ERROR)
				return(-1);
			if (func == READ)
				*ptr++ = R5ADDR->dbo;
			else
				R5ADDR->dbi = *ptr++;
		}
		if (func == WRITE)
			R5ADDR->sc = 0;
		while ((R5ADDR->cs0 & R5_BUSY) == 0)
			;
		if (R5ADDR->cs0 & R5_ERROR)
			return(-1);
		cc -= 512;
		xfer++;
	}
	return(io->i_cc);
}
