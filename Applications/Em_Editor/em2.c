#
/*	This is the second and final segment of the QMC Unix Editor - em */
#define EOF -1
#define LBSIZE	512
#define	SIGHUP	1
#define	SIGINTR	2
#define	SIGQUIT	3
#define UNIXBUFL 100
#define error	goto errlab
#define TABSET	7	/* this should be determined dynamically */

#define RAW	040
#define ECHO	010
#define OPEN	'/'
#define BELL	07
#define ESCAPE	033
#define SPACE	040
#define BACKSL	0134
#define RUBOUT	0177
#define CTRLA	01
#define CTRLB	02
#define CTRLC	03
#define CTRLD	04
#define CTRLE	05
#define CTRLF	06
#define CTRLH	010
#define CTRLI	011
#define CTRLQ	021
#define CTRLR	022
#define CTRLS	023
#define	CTRLV	026
#define CTRLW	027
#define CTRLX	030
#define CTRLZ	032

#define ITT	0100000

extern char	peekc, *linebp, *loc2, linebuf[LBSIZE], genbuf[LBSIZE],
			unixbuffer[UNIXBUFL];
extern int	onquit,onhup;
extern int	*zero, *addr1, *addr2;
extern int	*errlab;
int	margin	LBSIZE - 40;
int	oflag;
char	*threshold, *savethresh;
char	*lnp,*gnp,*brp;
int	savetty, tty[3];


op(inglob)
{	register *a1;
	register char *lp, *sp;
	char seof, ch;
	int t, nl, getopen(), getnil();

	threshold = genbuf + margin;
	savethresh = 0;

	switch (ch = peekc = getchar()) {
		case BACKSL:
			t = 1;
			delete();
			addr2 = addr1;
			break;

		case ';':
		case '+':
			t = 0;
			break;

		case '-':
			t =1;
			break;

		default:
			goto normal;

	}

	peekc = 0;
	if(addr1 != addr2) error;
	oflag = 0;
	append(getnil, addr2 - t);
	addr1 = addr2 =- (t-1);
	setdot();
	nonzero();


normal:
	if(addr1 == zero) error;
	if ((seof = getchar()) == '\n') { loc2 = linebuf-1;
						seof = 0;}
		else compile(seof);
	setraw(); /* terminal into raw mode*/

	for ( a1 = addr1; a1 <= addr2; a1++) {
		if (seof) {if (execute(0,a1) == 0) continue;}
			else getline(*a1);
		puts("\\\r");
		lp = linebuf;
		sp = genbuf;
		inglob =| 01;
		while (lp < loc2){ putch(*lp); *sp++ = *lp++; }
		lnp = lp;
		gnp = sp;


		oflag = gopen(); /* open the current line */
		*a1 = putline(); /* write revised line */
			nl = append( getopen,a1);
			a1 =+ nl;
			addr2 =+ nl;
	}
	setcook(); /* terminal in cook mode */
	putchar('\n');
	if (inglob == 0) { putchar('?'); error;}
}

getnil()
{
	if(oflag == EOF) return EOF;
	linebuf[0] = '\0';
	oflag = EOF;
	return 0;
}

setraw()
{
	if(gtty(0,tty) == -1) error;
	savetty = tty[2];
	tty[2] =| RAW;
	stty(0, tty);
}

setcook()
{
	tty[2] = savetty;
	stty(0, tty);
}

inword(c) char c;
{	if(c -'0' < 0 ) return 0;
	if(c - '9' <= 0) return 1;
	c =& 0137;
	if(c -'A' < 0) return 0;
	if(c - 'Z' <= 0) return 1;
	return 0;
}

rescan()
{	register char *lp, *sp;

	if(savethresh) { threshold = savethresh; savethresh = 0; }
	lp = linebuf;
	sp = genbuf;
	while(*lp++ = *sp++) if(lp>linebuf+LBSIZE) {
				*(--lp) = 0;
				return 0;
				}
}


gopen()
	/*leaves revised line in linebuf,
	returns 0 if more to follow, EOF if last line */
{	register char *lp, *sp, *rp;
	char ch, *br, *pr;
	int tabs;
	int retcode, savint, pid, rpid;

	lp = lnp;
	sp = gnp;
	tabs = 0;
	for (rp = genbuf; rp < sp; rp++) if (*rp == CTRLI) tabs =+ TABSET;

	for(;;){
		switch (ch = getchar()) {

			case CTRLD:
			case ESCAPE:	/* close the line (see case '\n' also) */
			close:
				putb(lp);
				while(*sp++ = *lp++);
				rescan();
				return(EOF);

			case CTRLA:	/* verify line */
			verify: puts("\\\r");
				*sp = '\0';
				putb(genbuf);
				continue;

			case CTRLB:	/* back a word */
				if(sp == genbuf) goto backquery;

				while((*(--lp) = *(--sp)) == SPACE)
					if(sp < genbuf) goto out;
				if(inword(*sp)) {
					while(inword((*(--lp) = *(--sp))))
						 if(sp < genbuf) goto out;
					if(*sp == SPACE)
						while((*(--lp) = *(--sp)) == SPACE)
							if(sp < genbuf) goto out;
				}	
				else while(sp >= genbuf && !inword(*sp))
					if((*lp-- = *sp--) == CTRLI) tabs =- TABSET;
			out:	sp++;
				lp++;
				goto verify;

			case CTRLC:
			case CTRLQ: /* forward one char */
				if(*lp == 0) goto backquery;
				putch(*lp);
			forward:
				if(*lp == SPACE && sp + tabs > threshold) {
					putch('\r');
					ch = '\n'; putch(ch);
					lp++;
					*sp++ = ch;
					br = sp;
					break;
				}
				if(*lp == CTRLI) tabs =+ TABSET;
				*sp++ = *lp++;	/* one character */
				if(sp + tabs == threshold) putch(BELL);
				continue;


			case CTRLF:
			/* delete forward */
				while(*lp++);
				lp--;
				goto verify;

			case CTRLE:
				putb(lp);
				goto verify;

			case CTRLH:  help(); goto verify;

			case CTRLR:	/* margin release */
				if(threshold-genbuf<LBSIZE-40) {
					savethresh = threshold;
					threshold = genbuf+LBSIZE-40;
				}
				else goto backquery;
				continue;

			case CTRLS:	/* re-set to start of line */
				while(*sp++ = *lp++);
				rescan();
				lp = linebuf; sp = genbuf;
				tabs = 0;
				goto verify;

			case CTRLV:	/* verify spelling */
				rp = sp;
				pr = unixbuffer+UNIXBUFL-2;
				*pr = 0;
				while(*(--rp) == SPACE);
				while(inword(*rp) && rp >= genbuf)
					*(--pr) = *rp--;
				if(*pr == 0) goto backquery;
				puts("!!");
				setcook();
				if((pid = fork()) == 0)
					{signal(SIGHUP, onhup);
					  signal(SIGQUIT, onquit);
					  execl("/bin/spell", "spell", pr, 0);
					  puts("sorry, can't spell today");
					  exit();
				}
				savint = signal(SIGINTR,1);
				while((rpid = wait(&retcode)) != pid
					 && rpid != -1);
				signal(SIGINTR, savint);
				setraw();
				puts("!!");
				goto verify;


			case CTRLW:	/* forward one word */
				if(*lp == '\0') goto backquery;
				while(*lp == SPACE)
					putch(*sp++ = *lp++);
				if(inword(*lp)) {
					while(inword(*lp)) {
						putch(*sp++ = *lp++);
						if(sp+tabs==threshold) putch(BELL);
					}
					if(*lp == SPACE) {
						if(sp+tabs>threshold) {
							ch = '\n';
							lp++;
							*sp++ = ch;
							br = sp;
							putch('\r');
							putch('\n');
						}
						if(*lp == SPACE)
							while(*(lp+1) == SPACE)
							   putch(*sp++ = *lp++);
					}
				}
				else while(*lp && !inword(*lp)) {
					if(*lp == CTRLI) tabs =+ TABSET;
					putch(*sp++ = *lp++);
					if(sp+tabs==threshold) putch(BELL);
				}
				break;


			case CTRLZ:	/* delete a word */
				if(sp == genbuf) goto backquery;

				while(*(--sp) == SPACE) if(sp < genbuf) goto zout;
				if(inword(*sp)) {
					while(inword(*(--sp)))
						 if(sp < genbuf) goto zout;
					if(*sp == SPACE)
						while(*(--sp) == SPACE)
							if(sp < genbuf) goto zout;
				}	
				else while(sp >= genbuf && !inword(*sp))
					if(*sp-- == CTRLI) tabs =- TABSET;
			zout:	sp++;
				goto verify;

			case '@':	/*delete displayed line */
			/* delete backward */
				sp = genbuf;
				tabs = 0;
				goto verify;

			case RUBOUT:
				puts("\\\r");
				setcook();
				error;

			case CTRLX:
				putch('#');
			case '#':
				if( sp == genbuf) goto backquery;
				if(*(--sp) == CTRLI) tabs =- TABSET;
				if( ch == CTRLX) goto verify;
				continue;

			case '\n':
			case '\r': /* split line, actually handled at
					end of switch block */
				ch = '\n';
				*sp++ = ch;
				br = sp;
				break;

			case BACKSL: /* special symbols */
				switch (ch = peekc = getchar()) {
				case '(':  ch = '{'; peekc = 0; break;
				case ')':  ch = '}'; peekc = 0; break;
				case '!':  ch = '|'; peekc = 0; break;
				case '^':  ch = '~'; peekc = 0; break;
				case '\'':  ch = '`'; peekc = 0; break;
				case BACKSL:
				case '#':
				case '@':  peekc = 0; break;

				default:  if(ch >= 'a' && ch <= 'z') {
						peekc = 0; ch =- 040;}
					else {
						*(--lp) = BACKSL;
						goto forward;
						}
				}

			default:	*(--lp) = ch;
					goto forward;
			}

		if (ch == '\n') { /* split line */
			if(*(br-1) != '\n') puts("!!");	/*debugging only */
			lnp = sp;
			while(*sp++ = *lp++); /*move the rest over */
			brp  = linebuf +(br - genbuf);
			lnp = linebuf + (lnp - br);
			rescan();
			*(brp-1) ='\0';
			return(0);
			}
		else continue;
	backquery: putch(CTRLZ);
		} /* end of forloop block */
} /* end of gopen */



getopen()  /* calls gopen, deals with multiple lines etc. */
{	register char *lp, *sp;
	if (oflag == EOF) return EOF;

/* otherwise, multiple lines */

	lp = linebuf;
	sp = brp;
	while(*lp++ = *sp++); /*move it down */
	sp = genbuf;
	lp = linebuf;
	while (lp < lnp) *sp++ = *lp++;
	gnp = sp;
	/* should check whether empty line returned */
	oflag = gopen();
	return 0;
}

putch(ch) char ch;
{ write(1, &ch, 1); }

putb(ptr)	char *ptr;	/*display string */

{	register char *p;

	p = ptr;
	if(*p == '\0') return;
	while(*(++p));
	write(1,ptr,p-ptr);
}

help()
{	puts("\n");
	puts("	^A	display Again		^Q	next character");
	puts("	^B	backup word		^R	Release margin");
	puts("	ESCAPE				^S	re-scan from Start");
	puts("	or ^D	close line and exit	^V	verify spelling");
	puts("	^E	display to End		^W	next Word");
	puts("	^F	delete line Forward	^Z	delete word");
	puts("	^H	Help			# or ^X delete character");
	puts("	RUBOUT	exit unchanged		@	delete line backward\n");
	puts("	Other characters (including RETURN) inserted as typed");
}

