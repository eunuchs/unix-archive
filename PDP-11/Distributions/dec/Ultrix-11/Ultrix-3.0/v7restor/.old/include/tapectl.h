/*
 * tape ioctl commands
 */

#define	MTIOCOM	(('T'<<8)|1)	/* mag. tape IO commands */
#define		MT_NOREWC 1	/* no rewind on close */
#define		MT_REWC	2	/* rewind on close */
#define		MT_WREOF 3	/* write eof */
#define		MT_REW	4	/* rewind */
#define		MT_FSR	5	/* forward space record */
#define		MT_BSR	6	/* back space record */
#define		MT_OFFLN 7	/* put drive off line */
#define		MT_MSG	8	/* turn on deverrors */
#define		MT_NOMSG 9	/* turn off deverrors */
