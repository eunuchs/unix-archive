
/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */
printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register unsigned int *adx;
	char *s;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c);
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	} else if (c == 'c')
		putchar((char *)*adx);
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
long n;
{
	register long a;

	if (n<0) {	/* shouldn't happen */
		putchar('-');
		n = -n;
	}
	if(a = n/b)
		printn(a, b);
	putchar("0123456789ABCDEF"[(int)(n%b)]);
}



struct	device	{
	int	rcsr,rbuf;
	int	tcsr,tbuf;
};

/* Modifications below allow use of KL11 units 0 and 1 - wkt */

struct	device	*KLADDR[2]= {0177560, 0176500};

putchar(c)
register c;
{ putc(c,0); }

putc(c,unit)
register c;
register unit;
{
	register s;
	register unsigned timo;

	if (unit>1) unit=0;
	/*
	 *  If last char was a break or null, don't print
	if ((KLADDR[unit]->rbuf&0177) == 0)
		return;
	*/
	timo = 60000;
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
	while((KLADDR[unit]->tcsr&0200) == 0)
		if(--timo == 0)
			break;
	if ((unit == 0) && (c == 0)) return;	/* Ignore NULLs on console */
	s = KLADDR[unit]->tcsr;
	KLADDR[unit]->tcsr = 0;
	KLADDR[unit]->tbuf = c;
	if (unit==0) {				/* Translate chars on console */
	   if (c == '\n') {
		putc('\r',0);
		putc(0177,0);
		putc(0177,0);
	   }
	   putc(0,0);
        }
	KLADDR[unit]->tcsr = s;
}

getchar()
{ return(newgetc(0)); }

newgetc(unit)
register unit;
{
	register c;

	KLADDR[unit]->rcsr = 1;
	while((KLADDR[unit]->rcsr&0200)==0);
	c = KLADDR[unit]->rbuf&0177;
	if (unit==0) {
	   if (c=='\r')
		c = '\n';
	   putc(c,unit);	/* Only echo on console */
 	}
	return(c);
}

gets(buf)
char	*buf;
{
register char *lp;
register c;

	lp = buf;
	for (;;) {
		c = getchar() & 0177;
		if (c>='A' && c<='Z')
			c -= 'A' - 'a';
		if (lp != buf && *(lp-1) == '\\') {
			lp--;
			if (c>='a' && c<='z') {
				c += 'A' - 'a';
				goto store;
			}
			switch ( c) {
			case '(':
				c = '{';
				break;
			case ')':
				c = '}';
				break;
			case '!':
				c = '|';
				break;
			case '^':
				c = '~';
				break;
			case '\'':
				c = '`';
				break;
			}
		}
	store:
		switch(c) {
		case '\n':
		case '\r':
			c = '\n';
			*lp++ = '\0';
			return;
		case '\b':
		case '#':
			lp--;
			if (lp < buf)
				lp = buf;
			continue;
		case '@':
			lp = buf;
			putchar('\n');
			continue;
		default:
			*lp++ = c;
		}
	}
}
