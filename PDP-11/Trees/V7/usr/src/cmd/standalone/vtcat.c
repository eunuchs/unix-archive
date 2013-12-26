#include <stdio.h>
#include <sgtty.h>

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

#define BLKSIZE		512

int fd;
struct vtcmd vtcmd;		/* Command buffer */
struct vtcmd vtreply;		/* Reply from server */
char vtbuf[BLKSIZE];		/* Pointer to input buffer */
char *vterr[8]= {
	"", "No such record available", "Can't open requested record",
	"Can't close requested record", "Can't read requested block",
	"Can't write requested block", "No such command",
	"End of file: no blocks left to read"
};

char vtgetc();

usage()
 {
  fprintf(stderr, "Usage: vtcat serial_device record_number\n"); exit(1);
 }

main(argc, argv)
 int argc;
 char *argv[];
 {
  extern int errno;
  extern char *sys_errlist[];
  struct sgttyb tty;
  int i;
  int record;
  char error, cmd, sum0, sum1;

  if (argc!=3) usage();
  
  record= atoi(argv[2]);	/* Get the record we want */

  fd=open(argv[1], 2);		/* Open the terminal */
  if (fd==-1) {
    fprintf(stderr, "Error opening %s: %s\n", argv[1], sys_errlist[errno]);
    exit(1);
  }

				/* Set the terminal state to raw */
  i=gtty(fd, &tty);
  if (i==-1) {
    fprintf(stderr, "Error getting %s state: %s\n",
						argv[1], sys_errlist[errno]);
    exit(1);
  }
  tty.sg_flags |= RAW;
  i=stty(fd, &tty);
  if (i==-1) {
    fprintf(stderr, "Error setting %s state: %s\n",
						argv[1], sys_errlist[errno]);
    exit(1);
  }

				/* Set up the basic command */
				/* and the block number of 0 */

  vtcmd.record= record; vtcmd.cmd= VTC_READ;
  vtcmd.blklo=0; vtcmd.blkhi=0;

				/* Loop getting blocks until an error */
  while (1) {

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
    vtreply.hdr1= vtgetc(); if (vtreply.hdr1!=VT_HDR1) goto getreply;
    vtreply.hdr2= vtgetc(); if (vtreply.hdr2!=VT_HDR2) goto getreply;
    vtreply.cmd= vtgetc();  vtreply.record= vtgetc();
    vtreply.blklo= vtgetc(); vtreply.blkhi= vtgetc();


			/* Calc. the cksum to date */
    sum0= VT_HDR1 ^ vtreply.cmd ^ vtreply.blklo;
    sum1= VT_HDR2 ^ vtreply.record ^ vtreply.blkhi;

			/* Retrieve the block if no errs and a READ cmd */
    if (vtreply.cmd==VTC_READ) {
      for (i=0; i< BLKSIZE; i++) {
	vtbuf[i]= vtgetc(); sum0 ^= vtbuf[i]; i++;
	vtbuf[i]= vtgetc(); sum1 ^= vtbuf[i];
      }
    }
			/* Get the checksum */
    vtreply.sum0= vtgetc(); vtreply.sum1= vtgetc();

			/* Try again on a bad checksum */
    if ((sum0!=vtreply.sum0) || (sum1!=vtreply.sum1)) goto sendcmd;
    

			/* Extract any error. Exit if one */
    error= vtreply.cmd >> 4;

    if (error) {
	if (error==VTE_EOF) exit(0);
	fprintf(stderr, "Error from vtserver: %s\n", vterr[error]); exit(1);
    }

			/* Else, send the block to stdout */
    write(1, vtbuf, BLKSIZE);

			/* Move to the next block and loop */
    vtcmd.blklo++; if (vtcmd.blklo==0) vtcmd.blkhi++;
  }
 }

/* I could have inlined these, but I couldn't be bothered */

char vtgetc()
 {   
  char c;

  read(fd, &c, 1); return(c);
 }


vtputc(c)
 char c;
 {
  write(fd, &c, 1);
 }
