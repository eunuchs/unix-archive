/*
 * convert a PDP-format spcl struct to VAX format
 */
spclconv(old,new)
char *old;
struct spcl *new;
{
	char *tempindex;
	int count;
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
	new->c_dinode.di_acl = *((short *) tempindex); tempindex +=2;
	new->c_dinode.di_size = convl(tempindex); tempindex += 4;
	for (count=0; count < 39; ++count)
		new->c_dinode.di_addr[count] = *tempindex++;
	new->c_dinode.di_dmask = *tempindex++;
	new->c_dinode.di_atime = convl(tempindex); tempindex += 4;
	new->c_dinode.di_mtime = convl(tempindex); tempindex += 4;
	new->c_dinode.di_ctime = convl(tempindex); tempindex += 4;
	new->c_count = *((short *) tempindex); tempindex += 2;
	for (count=0; count < BSIZE; ++count)
		new->c_addr[count] = *tempindex++;
}
