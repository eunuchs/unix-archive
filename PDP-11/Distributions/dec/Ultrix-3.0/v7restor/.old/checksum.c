
checksum(b)
short *b;
{
	register short i, j;

	j = BSIZE/sizeof(short);
	i = 0;
	do
		i += *b++;
	while (--j);
	if (i != CHECKSUM) {
		printf("Checksum error %o\n", i);
		return(0);
	}
	return(1);
}

