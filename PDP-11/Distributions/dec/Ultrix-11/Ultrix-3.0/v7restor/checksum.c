#include "stdio.h"
#include "signal.h"
#include "sys/param.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/fblk.h"
#include "sys/filsys.h"
#include "sys/dir.h"
#include "dumprestor.h"

checksum(b)
	register short *b;
{
/*
	register short i;
	register j;

	j = BSIZE/sizeof(short);
	i = 0;
	do
		i += *b++;
	while (--j);
	if (i != CHECKSUM) {
		printf("Checksum error %o\n", i);
		return(0);
	}
*/
	return(1);
}
