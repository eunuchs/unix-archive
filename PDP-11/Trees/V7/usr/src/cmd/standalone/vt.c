/*
 * Virtual tape driver - copyright 1998 Warren Toomey	wkt@cs.adfa.oz.au
 *
 * $Revision: 1.9 $
 * $Date: 1998/02/01 00:49:15 $
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

/* Command sent to tape server */
struct vtcmd {
    char hdr1;			/* Header, 31 followed by 42 (decimal) */
    char hdr2;
    char cmd;			/* Command, one of VTC_XXX below */
				/* Error comes back in top 4 bits */
    char record;		/* Record we're accessing */
    char blklo;			/* Block number, in lo/hi format */
    char blkhi;
    char sum0;			/* 16-bit checksum */
    char sum1;
};

/* Header bytes */
#define VT_HDR1		31
#define VT_HDR2		42

/* Commands available */
#define VTC_QUICK	0	/* Quick read, no cksum sent */
#define VTC_OPEN	1
#define VTC_CLOSE	2
#define VTC_READ	3	/* This file only uses READ and OPEN */
#define VTC_WRITE	4

/* Errors returned */
#define VTE_NOREC	1	/* No such record available */
#define VTE_OPEN	2	/* Can't open requested block */
#define VTE_CLOSE	3	/* Can't close requested block */
#define VTE_READ	4	/* Can't read requested block */
#define VTE_WRITE	5	/* Can't write requested block */
#define VTE_NOCMD       6       /* No such command */
#define VTE_EOF         7       /* End of file: no blocks left to read */

#define BLKSIZE         512

/* Static things */
struct vtcmd vtcmd;		/* Command buffer */
struct vtcmd vtreply;		/* Reply from server */
char *vtbuf;			/* Pointer to input buffer */
unsigned int hitim, lotim;	/* Variables for delay loop */
char oddeven=0;			/* Toggle status printing character */
char *oddstr= ".*";

char vtsendcmd();		/* Forward references */
char vtgetc();


/*** Standalone-dependent section */

/* vtopen() is used to inform the server which record we'll be using */
vtopen(io)
register struct iob *io;
{
    register i;

    vtcmd.record= io->i_boff; vtcmd.cmd= VTC_OPEN;
    vtcmd.blklo=0; vtcmd.blkhi=0;

    vtsendcmd();		/* Send the command to the server */
}


/* vtstrategy() must be called as a READ. We send a command
 * to the server, get a reply, and return the amount read
 * (even on EOF, which may be zero) or -1 on error.
 */
vtstrategy(io, func)
register struct iob *io;
{
    register error, i;

    vtbuf= io->i_ma;

    /* Assume record, blklo and blkhi are ok */
    /* Assume i->i_cc is in multiples of BLKSIZE */
    for (i=0; i<io->i_cc; i+=BLKSIZE, vtbuf+=BLKSIZE) {

	vtcmd.cmd= VTC_READ;

        error= vtsendcmd();		/* Send the command to the server */

					/* Some programs rely on */
					/* the buffer being cleared to */
					/* indicate EOF, e.g cat */
	if (error == VTE_EOF) { vtbuf[0]=0; return(i); }

	if (error != 0) {
	  printf("tape error %d", error);
	  return(-1);
 	}
        			/* Increment block number for next time */
	vtcmd.blklo++;
	if (vtcmd.blklo==0) vtcmd.blkhi++;
    }
	return(io->i_cc);
}

/*** Protocol-specific stuff ***/

char vtsendcmd()
{
    register i;
    char error, cmd, sum0, sum1;

			/* Build the checksum */
    vtcmd.hdr1= VT_HDR1; vtcmd.hdr2= VT_HDR2;
    vtcmd.sum0= VT_HDR1 ^ vtcmd.cmd ^ vtcmd.blklo;
    vtcmd.sum1= VT_HDR2 ^ vtcmd.record ^ vtcmd.blkhi;

			/* Send the command to the server */
  sendcmd:
    vtputc(vtcmd.hdr1);  vtputc(vtcmd.hdr2);
    vtputc(vtcmd.cmd);   vtputc(vtcmd.record);
    vtputc(vtcmd.blklo); vtputc(vtcmd.blkhi);
    vtputc(vtcmd.sum0);  vtputc(vtcmd.sum1);

  			/* Now get a valid reply from the server */
  getreply:
    vtreply.hdr1= vtgetc(); if (hitim==0) goto sendcmd;
    if (vtreply.hdr1!=VT_HDR1) goto getreply;
    vtreply.hdr2= vtgetc(); if (hitim==0) goto sendcmd;
    if (vtreply.hdr2!=VT_HDR2) goto getreply;
    vtreply.cmd= vtgetc(); if (hitim==0) goto sendcmd;
    vtreply.record= vtgetc(); if (hitim==0) goto sendcmd;
    vtreply.blklo= vtgetc(); if (hitim==0) goto sendcmd;
    vtreply.blkhi= vtgetc(); if (hitim==0) goto sendcmd;


			/* Calc. the cksum to date */
    sum0= VT_HDR1 ^ vtreply.cmd ^ vtreply.blklo;
    sum1= VT_HDR2 ^ vtreply.record ^ vtreply.blkhi;

			/* Retrieve the block if no errs and a READ cmd */
    if (vtreply.cmd==VTC_READ) {
      for (i=0; i<BLKSIZE; i++) {
	vtbuf[i]= vtgetc(); if (hitim==0) goto sendcmd;
	sum0 ^= vtbuf[i]; i++;
	vtbuf[i]= vtgetc(); if (hitim==0) goto sendcmd;
	sum1 ^= vtbuf[i];
      }
    }
			/* Get the checksum */
    vtreply.sum0= vtgetc(); if (hitim==0) goto sendcmd;
    vtreply.sum1= vtgetc(); if (hitim==0) goto sendcmd;

			/* Try again on a bad checksum */
    if ((sum0!=vtreply.sum0) || (sum1!=vtreply.sum1)) {
	putchar('e'); goto sendcmd;
    }

    putchar(oddstr[oddeven]); oddeven= 1-oddeven;

			/* Extract any error */
    error= vtreply.cmd >> 4; return(error);
}

/*** Harware-specific stuff ***/

struct  device  {
        int     rcsr,rbuf;
        int     tcsr,tbuf;
};

static struct  device *KL1= {0176500};		/* We use KL11 unit 1 */

/* vtgetc() and vtputc(): A sort-of repeat of the getchar/putchar
 * code in prf.c, but without any console stuff
 */

/* Get a character, or timeout and return with hitim zero */
char vtgetc()
{   
        register c;
    
        KL1->rcsr = 1; hitim=3; lotim=65535;
  
        while ((KL1->rcsr&0200)==0) {
	   lotim--;
	   if (lotim==0) hitim--;
	   if (hitim==0) { putchar('t'); return(0); }
	}
        c = KL1->rbuf; return(c);
}


vtputc(c)
register c;
{
        register s;

        while((KL1->tcsr&0200) == 0) ;
        s = KL1->tcsr;
        KL1->tcsr = 0; KL1->tbuf = c; KL1->tcsr = s;
}
