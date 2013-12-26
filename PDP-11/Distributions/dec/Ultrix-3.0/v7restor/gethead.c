#include "stdio.h"
#include "sys/param.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/fblk.h"
#include "sys/filsys.h"
#include "sys/dir.h"
#include "dumprestor.h"
/*
 * read the tape into buf, then return whether or
 * or not it is a header block.
 */
gethead(buf)
struct spcl *buf;
{
	char temp[sizeof(spcl)];

	readtape(temp);
	/* fixed 30may85 pdbain. convert structure read in off tape
	to VAX format */
	spclconv(temp,buf);
	/* fixed 31may85 pdbain
	if (buf->c_magic != MAGIC || checksum((int *) buf) == 0) */
	if (buf->c_magic != MAGIC || checksum((short *) temp) == 0)
		return(0);
	return(1);
}

/*
 * convert a PDP-format spcl struct to VAX format
 */
static
spclconv(old,new)
char *old;
register struct spcl *new;
{
	register char *tempindex;
	register int count;

	tempindex = old;
	new->c_type = *((short *) tempindex); tempindex +=2;
	new->c_date = convl(tempindex); tempindex += 4;
	new->c_ddate = convl(tempindex); tempindex += 4;
	new->c_volume = *((short *) tempindex); tempindex +=2;
	new->c_tapea = convl(tempindex); tempindex += 4;
	new->c_inumber = *((short *) tempindex); tempindex +=2;
	new->c_magic = *((short *) tempindex); tempindex +=2;
	new->c_checksum = *((short *) tempindex); tempindex +=2;
	new->c_dinode.di_mode = *((unsigned short *) tempindex); tempindex +=2;
	new->c_dinode.di_nlink = *((short *) tempindex); tempindex +=2;
	new->c_dinode.di_uid = *((short *) tempindex); tempindex +=2;
	new->c_dinode.di_gid = *((short *) tempindex); tempindex +=2;
	new->c_dinode.di_size = convl(tempindex); tempindex += 4;
	for (count=0; count < 40; ++count)
		new->c_dinode.di_addr[count] = *tempindex++;
	new->c_dinode.di_atime = convl(tempindex); tempindex += 4;
	new->c_dinode.di_mtime = convl(tempindex); tempindex += 4;
	new->c_dinode.di_ctime = convl(tempindex); tempindex += 4;
	new->c_count = *((short *) tempindex); tempindex += 2;
	for (count=0; count < BSIZE; ++count)
		new->c_addr[count] = *tempindex++;
}
