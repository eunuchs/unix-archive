/*
 * RA standalone disk driver
 */
#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

/*
 * Parameters for the communications area
 */
#define	NRSPL2	0
#define	NCMDL2	0
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)
#include <sys/rareg.h>

#define	RAADDR	((struct radevice *) 0172150)

struct ra {
	struct raca	ra_ca;
	struct ms	ra_rsp;
	struct ms	ra_cmd;
} ra;


struct ms *racmd();

raopen(io)
	register struct iob *io;
{
	register struct ms *mp;
	int i;

	RAADDR->raip = 0;
	while ((RAADDR->rasa & RA_STEP1) == 0)
		;
	RAADDR->rasa = RA_ERR;
	while ((RAADDR->rasa & RA_STEP2) == 0)
		;
	RAADDR->rasa = (short)&ra.ra_ca.ca_ringbase;
	while ((RAADDR->rasa & RA_STEP3) == 0)
		;
	RAADDR->rasa = segflag;
	while ((RAADDR->rasa & RA_STEP4) == 0)
		;
	RAADDR->rasa = RA_GO;
	ra.ra_ca.ca_rspdsc[0].low = &ra.ra_rsp.ms_cmdref;
	ra.ra_ca.ca_rspdsc[0].high = segflag;
	ra.ra_ca.ca_cmddsc[0].low = &ra.ra_cmd.ms_cmdref;
	ra.ra_ca.ca_cmddsc[0].high = segflag;
	ra.ra_cmd.ms_cntflgs = 0;
	if (racmd(M_OP_STCON) == 0) {
		_stop("ra: open error, STCON");
		return;
	}
	ra.ra_cmd.ms_unit = io->i_unit&7;
	if (racmd(M_OP_ONLIN) == 0) {
		_stop("ra: open error, ONLIN");
	}
}

struct ms *
racmd(op)
	int op;
{
	struct ms *mp;
	int i;

	ra.ra_cmd.ms_opcode = op;
	ra.ra_rsp.ms_header.ra_msglen = sizeof (struct ms);
	ra.ra_cmd.ms_header.ra_msglen = sizeof (struct ms);
	ra.ra_ca.ca_rspdsc[0].high |= RA_OWN|RA_INT;
	ra.ra_ca.ca_cmddsc[0].high |= RA_OWN|RA_INT;
	i = RAADDR->raip;
	for (;;) {
		if (ra.ra_ca.ca_cmdint)
			ra.ra_ca.ca_cmdint = 0;
		if (ra.ra_ca.ca_rspint)
			break;
	}
	ra.ra_ca.ca_rspint = 0;
	mp = &ra.ra_rsp;
	if ((mp->ms_opcode&0377) != (op|M_OP_END) ||
	    (mp->ms_status&M_ST_MASK) != M_ST_SUCC)
		{
		printf("op = %o\n",mp->ms_opcode);
		return(0);
		}
	return(mp);
}

rastrategy(io, func)
	register struct iob *io;
{
	register struct ms *mp;

	mp = &ra.ra_cmd;
	mp->ms_lbn = SWAPW(io->i_bn);
	mp->ms_unit = io->i_unit&7;
	mp->ms_bytecnt = SWAPW(io->i_cc);
	mp->ms_buffer.low = io->i_ma;
	mp->ms_buffer.high = segflag;
	if ((mp = racmd(func == READ ? M_OP_READ : M_OP_WRITE)) == 0) {
		printf("ra: I/O error\n");
		return(-1);
	}
	return(io->i_cc);
}
