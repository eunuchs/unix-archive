/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rk.c	2.1 (2.11BSD) 1995/06/08
 */

/*
 * RK disk driver
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

/* bits in rkcs */
#define RKCS_ERR        0100000         /* error */
#define RKCS_HE         0040000         /* hard error */
#define RKCS_SCP        0020000         /* search complete */
/* bit 12 is unused */
#define RKCS_INHBA      0004000         /* inhibit bus address increment */
#define RKCS_FMT        0002000         /* format */
/* bit 9 is unused */
#define RKCS_SSE        0000400         /* stop on soft error */
#define RKCS_RDY        0000200         /* control ready */
#define RKCS_IDE        0000100         /* interrupt on done enable */
/* bits 5-4 are the UNIBUS extension bits */
/* bits 3-1 is the command */
#define RKCS_GO         0000001         /* go */
#define RKCS_BITS       \
"\10\20ERR\17HE\16SCP\15INHBA\14FMT\13SSE\12RDY\11IDE\1GO"

/* commands */
#define RKCS_RESET      0000000         /* control reset */
#define RKCS_WCOM       0000002         /* write */
#define RKCS_RCOM       0000004         /* read */
#define RKCS_WCHK       0000006         /* write check */
#define RKCS_SEEK       0000010         /* seek */
#define RKCS_RCHK       0000012         /* read check */
#define RKCS_DRESET     0000014         /* drive reset */
#define RKCS_WLCK       0000016         /* write lock */


struct  rkdevice
{
        short   rkds;
        short   rker;
        short   rkcs;
        short   rkwc;
        caddr_t rkba;
        short   rkda;
};


#define	NRK	2

	struct	rkdevice *RKcsr[NRK + 1] =
		{
		(struct rkdevice *)0177400,
		(struct	rkdevice *)0,
		(struct rkdevice *)-1
		};

rkstrategy(io, func)
	register struct iob *io;
{
	register com;
	register struct rkdevice *rkaddr;
	daddr_t bn;
	int dn, cn, sn, bae, lo16;

	bn = io->i_bn;
	dn = io->i_unit;
	cn = bn/12;
	sn = bn%12;
	rkaddr = RKcsr[io->i_ctlr];
	rkaddr->rkda = (dn<<13) | (cn<<4) | sn;
	rkaddr->rkba = (caddr_t)lo16;
	rkaddr->rkwc = -(io->i_cc>>1);
	com = (bae<<4)|RKCS_GO;
	if (func == READ)
		com |= RKCS_RCOM;
	else
		com |= RKCS_WCOM;
	rkaddr->rkcs = com;
	while ((rkaddr->rkcs & RKCS_RDY) == 0)
		continue;
	if (rkaddr->rkcs<0) {	/* error bit */
		printf("RK%d,%d err cy=%d sc=%d, er=%o, ds=%o\n",
		    io->i_ctlr, io->i_unit, cn, sn, rkaddr->rker, rkaddr->rkds);
		return(-1);
	}
	return(io->i_cc);
}
