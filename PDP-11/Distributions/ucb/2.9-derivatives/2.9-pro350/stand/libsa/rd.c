/*
 * Standalone rd50/51 driver for pro3xx
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "../saio.h"
#include <sys/rdreg.h>

#define RDADDR ((struct rddevice *) 0174000)

struct rdst rdst[] = {
	16,	4,	16*4,	305,
	16,	4,	16*4,	152,
	0,	0,	0,	0,
};

rdstrategy(io, func)
register struct iob *io;
{
	register int i, *ptr;
	register struct rdst *st;
	int cc, xfer;
	daddr_t bn;
	int sn;

	st = &rdst[(io->i_unit>>3)&01];
	ptr = io->i_ma;
	cc = io->i_cc;
	xfer = 0;
	while (cc > 0) {
		bn = io->i_bn + xfer;
		sn = bn % (st->nspc);
		while (RDADDR->st & RD_BUSY)
			;
		RDADDR->trk = sn / (st->nsect);
		RDADDR->sec = sn % (st->nsect);
		RDADDR->cyl = bn/(st->nspc);
		if (func == READ)
			RDADDR->csr = RD_READCOM;
		else
			RDADDR->csr = RD_WRITECOM;
		for (i = 0; i < 256; i++) {
			while ((RDADDR->st & (RD_DRQ|RD_OPENDED)) == 0)
				;
			if (RDADDR->csr & RD_ERROR)
				return(-1);
			if (func == READ)
				*ptr++ = RDADDR->db;
			else
				RDADDR->db = *ptr++;
		}
		while ((RDADDR->st & RD_OPENDED) == 0)
			;
		if (RDADDR->csr & RD_ERROR)
			return(-1);
		cc -= 512;
		xfer++;
	}
	return(io->i_cc);
}
rdinit()
{
	RDADDR->st = RD_INIT;
	while (RDADDR->st & RD_BUSY)
		;
	RDADDR->csr = RD_RESTORE;
	while ((RDADDR->st & RD_BUSY) || (!(RDADDR->st & RD_OPENDED)))
		;
}
