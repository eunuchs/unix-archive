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
