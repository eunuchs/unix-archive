	long
convl(pdp)
	short *pdp;
{
	long i;
	short *buff = (short *) &i;
	*buff = *(pdp+1); *(buff+1) = *pdp;
	return(i);
}
