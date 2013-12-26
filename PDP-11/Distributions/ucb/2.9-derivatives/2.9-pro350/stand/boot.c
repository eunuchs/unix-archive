/*
 *	SCCS id	@(#)boot.c	2.1 (Berkeley)	8/5/83
 */

#include	<sys/param.h>
#include	<sys/seg.h>
#include	<sys/ino.h>
#include	<sys/inode.h>
#include	<sys/filsys.h>
#include	<sys/dir.h>
#include	<sys/reboot.h>
#include	<sys/koverlay.h>
#include	<a.out.h>
#include	"../saio.h"

#undef	btoc
#define	KB	* 1024L

#define	KISD0	((u_short *) 0172300)
#define	KISD3	((u_short *) 0172306)
#define	KDSD0	((u_short *) 0172320)
#undef	KISA0
#define	KISA0	((u_short *) 0172340)
#define	KISA3	((u_short *) 0172346)
#define	KDSA0	((u_short *) 0172360)

#define	SEG_DATA	01
#define	SEG_TEXT	02
#define	SEG_OVLY	04

extern	int	cputype;
extern	bool_t	ksep;			/* Is kernel mode currently separated */
extern	bool_t	sep_id;			/* Does the cpu support separate I/D? */
extern	int	bootopts, bootdev, checkword;
#ifdef	BIGKOV
int	bigkov = 0;
#endif
#ifdef GOO
extern	char quietflg;
#endif GOO
char	module[]	= "Boot";	/* This program's name (used by trap) */
char	line[100]	= RB_DEFNAME;
bool_t	overlaid	= 0;
u_short	pdrproto[16 + NOVL]	= {0};
struct	exec	exec;
struct	ovlhdr	ovlhdr;
unsigned	btoc();

struct	loadmap	{
	int	seg_type;
	long	seg_len;
};
struct	loadtable	{
	short	lt_magic;
	struct	loadmap	*lt_map;
};

struct	loadmap	load407[]	=	{
	SEG_DATA,	56 KB,
	0,		0  KB
	};
struct	loadmap	load410[]	=	{
	SEG_TEXT,	48 KB,
	SEG_DATA,	56 KB,
	0,		0  KB
	};
struct	loadmap	load411[]	=	{
	SEG_DATA,	56 KB,
	SEG_TEXT,	64 KB,
	0,		0  KB
	};
struct	loadmap	load430[]	=	{
	SEG_TEXT,	16 KB,			/* minumum, 8 KB + 1 */
	SEG_OVLY,	8  KB,
	SEG_DATA,	24 KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	0,		0  KB
	};
#ifdef	BIGKOV
struct	loadmap big430[]	=	{
	SEG_TEXT,	8 KB,			/* minumum, 8 KB + 1 */
	SEG_OVLY,	16  KB,
	SEG_DATA,	24 KB,
	SEG_OVLY,	16  KB,
	SEG_OVLY,	16  KB,
	SEG_OVLY,	16  KB,
	SEG_OVLY,	16  KB,
	SEG_OVLY,	16  KB,
	SEG_OVLY,	16  KB,
	0,		0   KB
};
#endif
struct	loadmap	load431[]	=	{
	SEG_DATA,	56 KB,			/* minumum, 48 KB + 1 */
	SEG_TEXT,	56 KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	SEG_OVLY,	8  KB,
	0,		0  KB
	};

struct	loadtable	loadtable[]	=	{
	A_MAGIC1,	load407,
	A_MAGIC2,	load410,
	A_MAGIC3,	load411,
	A_MAGIC5,	load430,
	A_MAGIC6,	load431
	};

main ()
{
	int i, j;
	int retry	= 0;
	struct	loadtable	*setup ();

	segflag	= 3;			/* device drivers care about this */
#ifndef GOO
	printf ("\n%d%s\n", cputype, module);
#endif GOO

#ifdef  UCB_AUTOBOOT
	/*
	 *	The machine language will have gotten the bootopts
	 *	if we're an autoboot and will pass them along.
	 *	If r2 (checkword) was the complement of bootopts,
	 *	this is an automatic reboot, otherwise do it the hard way.
	 */
	if (checkword != ~bootopts)
#endif
		bootopts	= RB_SINGLE | RB_ASKNAME;
#ifdef GOO
	if (bootopts & RB_QUIET)
		quietflg = 1;
	printf ("\n%d%s\n", cputype, module);
#endif GOO
	do	{
		if (bootopts & RB_ASKNAME)	{
			printf (": ");
			gets (line);
		}
		else
			printf (": %s\n", line);
		if (line[0] == '\0')	{
			printf (": %s\n", RB_DEFNAME);
			i	= open (RB_DEFNAME, 0);
		}
		else
			i	= open (line, 0);
		j = -1;
		if (i >= 0)	{
			j	= checkunix (i, setup (i));
			(void) close (i);
			}
		if (++retry > 2)
			bootopts	= RB_SINGLE | RB_ASKNAME;
	}	while (j < 0);

}

struct	loadtable	*
setup (io)
register io;
{
	register	i;

	exec.a_magic	= getw (io);
	exec.a_text	= (unsigned) getw (io);
	exec.a_data	= (unsigned) getw (io);
	exec.a_bss	= (unsigned) getw (io);

	/*
	 *	Space over the remainder of the exec header.  We do this
	 *	instead of seeking because the input might be a tape which
	 *	doesn't know how to seek.
	 */
	getw (io); getw (io); getw (io); getw (io);

	/*
	 *	If overlaid, get overlay header.
	 */
	if (exec.a_magic == A_MAGIC5 || exec.a_magic == A_MAGIC6)	{
		overlaid++;
		ovlhdr.max_ovl	= getw (io);
		for (i = 0; i < NOVL; i++)
			ovlhdr.ov_siz[i]	= (unsigned) getw (io);
		}
#ifdef	BIGKOV
	if (exec.a_magic == A_MAGIC5 && exec.a_text <= 8192)
		bigkov = 1;
#endif
	
	for (i = 0; i < sizeof (loadtable) / sizeof (struct loadtable); i++)
		if (loadtable[i].lt_magic == exec.a_magic) {
#ifdef	BIGKOV
			if (exec.a_magic == A_MAGIC5 && bigkov)
				loadtable[i].lt_map = big430;
#endif
			return (&loadtable[i]);
		}

	printf ("Bad magic number 0%o\n", exec.a_magic);
	return ((struct loadtable *) NULL);
}


checkunix (io, lt)
struct	loadtable	*lt;
{
	char	*segname;
	register	ovseg, segtype;
	register unsigned		seglen;
	struct	loadmap	*lm	= lt->lt_map;

	if (lt == (struct loadtable *) NULL)
		return (-1);

	/*
	 *	Check and set I & D space requirements.
	 */
	if (exec.a_magic == A_MAGIC3 || exec.a_magic == A_MAGIC6)
		if (!sep_id)	{
			printf ("Cannot load separate I & D object files\n");
			return (-1);
			}
		else
			setsep ();
	else
		if (sep_id)
			setnosep ();

	/*
	 *	Check the sizes of each segment.
	 */
	ovseg	= 0;
	while (segtype = lm->seg_type)	{
		switch (segtype)	{
			case SEG_TEXT:
				/*
				 *	Round text size to nearest page.
				 */
				if (exec.a_magic == A_MAGIC2)
					seglen	= ctob (stoc (ctos (btoc (exec.a_text))));
				else
					seglen	= exec.a_text;
				segname	= "Text";
				break;

			case SEG_DATA:
				seglen	= exec.a_data + exec.a_bss;
				segname	= "Data";
				if (exec.a_magic == A_MAGIC1)
					seglen	+= exec.a_text;
				else
					/*
					 *	Force a complaint if the file
					 *	won't fit.  It's here instead
					 *	of in the SEG_TEXT case above
					 *	because it's more likely to be
					 *	a data overflow problem.
					 */
					if (exec.a_magic == A_MAGIC2)
						seglen	+= ctob (stoc (ctos (btoc (exec.a_text))));
				break;

			case SEG_OVLY:
				seglen	= ovlhdr.ov_siz[ovseg];
				segname	= "Overlay";
				ovseg++;
				break;

			default:
				/*
				 *	This ``cannot happen.''
				 */
				printf ("Unknown segment type in load table:  %d\n", segtype);
				return (-1);
				/*NOTREACHED*/
			}

		seglen	= ctob (btoc (seglen));
		if (((long) seglen) > lm->seg_len)	{
			if (segtype == SEG_OVLY)
				printf ("%s %d too large by %D bytes", segname, ovseg, lm->seg_len - ((long) seglen));
			else
				printf ("%s too large by %D bytes", segname, lm->seg_len - ((long) seglen));
			return (-1);
			}
		if (segtype == SEG_TEXT)
		    switch (exec.a_magic) {
			case A_MAGIC5:
#ifdef	BIGKOV
			    if (seglen <= 8 KB && bigkov == 0) {
#else
			    if (seglen <= 8 KB) {
#endif
				printf("Base segment too small, 8K minimum\n");
				return(-1);
			    }
			    break;
			case A_MAGIC6:
			    if (seglen <= 48 KB) {
				printf("Base segment too small, 48K minimum\n");
				return(-1);
			    }
			    break;
			default:
			    break;
		}

		lm++;
		}

	copyunix (io, lt);
	setregs (lt);
	return (0);
}

copyunix (io, lt)
register	io;
struct	loadtable	*lt;
{
	int	i;
	bool_t	donedata = 0;
	register	addr;
	register unsigned	seglen;
	off_t	segoff;
	int	segtype;
	int	nseg, phys, ovseg;
	struct	loadmap	*lm	= lt->lt_map;

	/*
	 *	Load the segments and set up prototype PDRs.
	 */
	nseg	= 0;
	phys	= 0;
	ovseg	= 0;
	lm	= lt->lt_map;
	while (segtype = lm++->seg_type)	{

		segoff	= (off_t) N_TXTOFF(exec);

		switch (segtype)	{
			case SEG_TEXT:
				seglen	= exec.a_text;
				break;

			case SEG_DATA:
				seglen	= exec.a_data;
				if (exec.a_magic != A_MAGIC1)	{
					segoff	+= (off_t) exec.a_text;
					if (overlaid)
						for (i = 0; i < NOVL; i++)
							segoff	+= (off_t) ovlhdr.ov_siz[i];
					}
				else
					seglen	+= exec.a_text;
				donedata++;
				break;

			case SEG_OVLY:
				seglen	= ovlhdr.ov_siz[ovseg];
				segoff	+= exec.a_text;
				for (i = 0; i < ovseg; i++)
					segoff	+= (off_t) ovlhdr.ov_siz[i];
				ovseg++;
				break;

			}

		if (!seglen)
			continue;
		setseg (phys);
		if (exec.a_magic != A_MAGIC1)
			(void) lseek (io, segoff, 0);
		for (addr = 0; addr < seglen; addr += 2)
			mtpi (getw (io), addr);

		if (segtype == SEG_DATA)	{
			clrseg (addr, exec.a_bss);
			seglen	+= exec.a_bss;
			}

		pdrproto[nseg++]	= btoc (seglen);
		if (!donedata)
			seglen	= ctob (stoc (ctos (btoc (seglen))));
		phys	+= btoc (seglen);
		}
}

/*
 *	Set the real segmentation registers.
 */
setregs (lt)
struct	loadtable	*lt;
{
	register	i;
	register u_short	*par_base, *pdr_base;
	bool_t	donedata = 0;
	int	phys, segtype;
	int	nseg, ntextpgs, novlypgs, npages, pagelen;
	struct	loadmap	*lm	= lt->lt_map;

	nseg	= 0;
	phys	= 0;
	ntextpgs	= 0;
	novlypgs	= 0;

	setseg (0);
	if (exec.a_magic == A_MAGIC1)
		return;

	/*
	 *	First deny access to all except I/O page.
	 */
	par_base	= KISA0;
	pdr_base	= KISD0;
	for (i = 0; i < (ksep ?  8 : 7); i++)	{
		*par_base++	= 0;
		*pdr_base++	= NOACC;
		}
	if (ksep)	{
		par_base	= KDSA0;
		pdr_base	= KDSD0;
		for (i = 0; i < 7; i++)	{
			*par_base++	= 0;
			*pdr_base++	= NOACC;
			}
		}
	
	if (overlaid)	{
		/*
		 *	We must write the prototype overlay register table.
		 *	N.B.:  we assume that
		 *		the table lies in the first 8k of kernel virtual
		 *		space, and
		 *
		 *		the appropriate page lies at physical 0.
		 */
		if (ksep)
			*KDSD0	=	((128 -1) << 8) | RW;
		else
			*KISD0	=	((128 -1) << 8) | RW;
		par_base	= &(((u_short *) OVLY_TABLE_BASE)[0]);
		pdr_base	= &(((u_short *) OVLY_TABLE_BASE)[1 + NOVL]);
		for (i = 0; i < NOVL; i++)	{
#ifdef	BIGKOV
		     if (bigkov) {
			mtpd (0, (par_base+16));
			mtpd (NOACC, (pdr_base+16));
		     }
#endif	BIGKOV
			mtpd (0, par_base++);
			mtpd (NOACC, pdr_base++);
			}
		}

	/*
	 *	Now set all registers which should be nonzero.
	 */
	lm	= lt->lt_map;
	while (segtype = lm++->seg_type)	{
		if (!(npages = ctos (pdrproto[nseg])))
			continue;

		switch (segtype)	{
			case SEG_TEXT:
				/*
				 *	Text always starts at KI0;
				 */
				par_base	= KISA0;
				pdr_base	= KISD0;
				ntextpgs	+= npages;
				break;

			case SEG_DATA:
				if (overlaid)
					if (ksep)	{
						par_base	= I_DATA_PAR_BASE;
						pdr_base	= I_DATA_PDR_BASE;
						}
					else
						{
						par_base	= N_DATA_PAR_BASE;
						pdr_base	= N_DATA_PDR_BASE;
						}
				else
					if (ksep)	{
						par_base	= KDSA0;
						pdr_base	= KDSD0;
						}
					else
						{
						par_base	= &KISA0[ntextpgs + novlypgs];
						pdr_base	= &KISD0[ntextpgs + novlypgs];
						}
				donedata++;
				break;

			case SEG_OVLY:
				par_base	= &(((u_short *) OVLY_TABLE_BASE)[1 + novlypgs]);
				pdr_base	= &(((u_short *) OVLY_TABLE_BASE)[1 + NOVL + 1 + novlypgs]);
				novlypgs	+= 1;
				break;

			}

		for (i = 0; i < npages; i++)	{
			pagelen	= MIN (btoc ((int) (8 KB)), pdrproto[nseg]);
			if (segtype == SEG_OVLY)	{
				mtpd (phys, par_base);
				mtpd (((pagelen - 1) << 8) | RO, pdr_base);
				}
			else
				{
				*par_base	= phys;
				if (segtype == SEG_TEXT)
					if (ksep)
						*pdr_base	= ((pagelen - 1) << 8) | RO;
					else
						/*
						 *	Nonseparate kernels will
						 *	write into text page 0
						 *	when first booted.
						 */
						if (i == 0)
							*pdr_base	= ((pagelen - 1) << 8) | RW;
						else
							*pdr_base	= ((pagelen - 1) << 8) | RO;
				else
					*pdr_base	= ((pagelen - 1) << 8) | RW;
				}
#ifdef	BIGKOV
			if (segtype == SEG_OVLY && bigkov) {
				par_base += 16;
				pdr_base += 16;
			} else {
#endif	BIGKOV
			par_base++, pdr_base++;
#ifdef	BIGKOV
			}
#endif	BIGKOV
			if (donedata)
				phys	+= pagelen;
			else
				phys	+= stoc (ctos (pagelen));
			pdrproto[nseg]	-= pagelen;
			}

		nseg++;
		}

	/*
	 *	Phys now contains the address of the start of
	 *	free memory.  We set K[ID]6 now or systrap to
	 *	kernel mode will clobber text at 0140000.
	 */
	if (ksep)	{
		KDSA0[6]	= phys;
		KDSD0[6]	= (stoc(1) - 1) << 8 | RW;
		}
	else
		{
		KISA0[6]	= phys;
		KISD0[6]	= (stoc(1) - 1) << 8 | RW;
		}

	if (overlaid)
#ifdef	BIGKOV
	    if (bigkov)
		mtpd (phys, &(((u_short *) OVLY_TABLE_BASE)[32]));
	    else
#endif
		mtpd (phys, &(((u_short *) OVLY_TABLE_BASE)[1 + NOVL + 1 + NOVL]));
}

unsigned
btoc (nclicks)
unsigned nclicks;
{
	return ((unsigned) (((((long) nclicks) + ((long) 63)) >> 6)));
}
