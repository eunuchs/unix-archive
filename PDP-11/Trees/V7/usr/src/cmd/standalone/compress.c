#if 0
static char sccsid[]= "@(#)compress.c	@(#)compress.c	5.9 (Berkeley) 5/11/86";
#endif

/*
 * Compress - data compression program. This is an EXTREMELY HACKED version
 * of the old compress program, solely to uncompress a -b12 stream. It runs
 * in standalone mode on PDP-11s, and it is typically used to uncompress and
 * fill a disk with an incoming compressed disk image.
 *
 * There's a whole lot of cruft I haven't bothered to take out, but it's
 * mostly cpp stuff which doesn't make it into the binary.
 *
 * Warren Toomey	wkt@cs.adfa.oz.au	24th March 1998
 */

#define	min(a,b)	((a>b) ? b : a)
#ifndef pdp11
#define pdp11
#endif

/*
 * machine variants which require cc -Dmachine:  pdp11, z8000, pcxt
 */

/*
 * Set USERMEM to the maximum amount of physical user memory available
 * in bytes.  USERMEM is used to determine the maximum BITS that can be used
 * for compression.
 *
 * SACREDMEM is the amount of physical memory saved for others; compress
 * will hog the rest.
 */
#ifndef SACREDMEM
#define SACREDMEM	0
#endif

#ifndef USERMEM
#define USERMEM 	450000	/* default user memory */
#endif

#ifdef interdata		/* (Perkin-Elmer) */
#define SIGNED_COMPARE_SLOW	/* signed compare is slower than unsigned */
#endif

#ifdef pdp11
#define BITS 	12		/* max bits/code for 16-bit machine */
#define NO_UCHAR		/* also if "unsigned char" functions as signed
				   char */
#undef USERMEM
#endif	/* pdp11 */		/* don't forget to compile with -i */

#ifdef z8000
#define BITS 	12
#undef vax			/* weird preprocessor */
#undef USERMEM
#endif				/* z8000 */

#ifdef pcxt
#define BITS   12
#undef USERMEM
#endif				/* pcxt */

#ifdef USERMEM
#if USERMEM >= (433484+SACREDMEM)
#define PBITS	16
#else
#if USERMEM >= (229600+SACREDMEM)
#define PBITS	15
#else
#if USERMEM >= (127536+SACREDMEM)
#define PBITS	14
#else
#if USERMEM >= (73464+SACREDMEM)
#define PBITS	13
#else
#define PBITS	12
#endif
#endif
#endif
#endif
#undef USERMEM
#endif				/* USERMEM */

#ifdef PBITS			/* Preferred BITS for this memory size */
#ifndef BITS
#define BITS PBITS
#endif	/* BITS */
#endif				/* PBITS */

#if BITS == 16
#define HSIZE	69001		/* 95% occupancy */
#endif
#if BITS == 15
#define HSIZE	35023		/* 94% occupancy */
#endif
#if BITS == 14
#define HSIZE	18013		/* 91% occupancy */
#endif
#if BITS == 13
#define HSIZE	9001		/* 91% occupancy */
#endif
#if BITS <= 12
#define HSIZE	5003		/* 80% occupancy */
#endif

#ifdef M_XENIX			/* Stupid compiler can't handle arrays with */
#if BITS == 16			/* more than 65535 bytes - so we fake it */
#define XENIX_16
#else
#if BITS > 13			/* Code only handles BITS = 12, 13, or 16 */
#define BITS	13
#endif
#endif
#endif

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
#if BITS > 15
typedef long int code_int;
#else
typedef int code_int;
#endif

#ifdef SIGNED_COMPARE_SLOW
typedef unsigned long int count_int;
typedef unsigned short int count_short;
#else
typedef long int count_int;
#endif

#ifdef NO_UCHAR
typedef char char_type;
#else
typedef unsigned char char_type;
#endif					/* UCHAR */

char_type magic_header[] = {"\037\235"};	/* 1F 9D */

/* Defines for third byte of header */
#define BIT_MASK	0x1f
#define BLOCK_MASK	0x80
/* Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
 * a fourth header byte (for expansion).
 */
#define INIT_BITS 9		/* initial number of bits/code */

/*
 * compress.c - File compression ala IEEE Computer, June 1984.
 *
 * Authors:	Spencer W. Thomas	(decvax!utah-cs!thomas)
 *		Jim McKie		(decvax!mcvax!jim)
 *		Steve Davies		(decvax!vax135!petsd!peora!srd)
 *		Ken Turkowski		(decvax!decwrl!turtlevax!ken)
 *		James A. Woods		(decvax!ihnp4!ames!jaw)
 *		Joe Orost		(decvax!vax135!petsd!joe)
 *
 * Revision 4.0  85/07/30  12:50:00  joe
 * Removed ferror() calls in output routine on every output except first.
 * Prepared for release to the world.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

int n_bits;			/* number of bits/code */
int maxbits = BITS;		/* user settable max # bits/code */
code_int maxcode;		/* maximum code, given n_bits */
code_int maxmaxcode = 1 << BITS;/* should NEVER generate this code */

#ifdef COMPATIBLE		/* But wrong! */
#define MAXCODE(n_bits)	(1 << (n_bits) - 1)
#else
#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)
#endif				/* COMPATIBLE */

#ifdef XENIX_16
count_int htab0[8192];
count_int htab1[8192];
count_int htab2[8192];
count_int htab3[8192];
count_int htab4[8192];
count_int htab5[8192];
count_int htab6[8192];
count_int htab7[8192];
count_int htab8[HSIZE - 65536];
count_int *htab[9] = {
htab0, htab1, htab2, htab3, htab4, htab5, htab6, htab7, htab8};

#define htabof(i)	(htab[(i) >> 13][(i) & 0x1fff])
unsigned short code0tab[16384];
unsigned short code1tab[16384];
unsigned short code2tab[16384];
unsigned short code3tab[16384];
unsigned short code4tab[16384];
unsigned short *codetab[5] = {
code0tab, code1tab, code2tab, code3tab, code4tab};

#define codetabof(i)	(codetab[(i) >> 14][(i) & 0x3fff])

#else				/* Normal machine */

#ifdef sel			/* gould base register braindamage */
/*NOBASE*/
count_int htab[HSIZE];
unsigned short codetab[HSIZE];

/*NOBASE*/
#else
count_int htab[HSIZE];
unsigned short codetab[HSIZE];

#endif	/* sel */

#define htabof(i)	htab[i]
#define codetabof(i)	codetab[i]
#endif				/* XENIX_16 */
code_int hsize = HSIZE;		/* for dynamic table sizing */
count_int fsize;

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i)	codetabof(i)
#ifdef XENIX_16
#define tab_suffixof(i)	((char_type *)htab[(i)>>15])[(i) & 0x7fff]
#define de_stack		((char_type *)(htab2))
#else				/* Normal machine */
#define tab_suffixof(i)	((char_type *)(htab))[i]
#define de_stack		((char_type *)&tab_suffixof(1<<BITS))
#endif				/* XENIX_16 */

code_int free_ent = 0;		/* first unused entry */
int exit_stat = 0;		/* per-file status */
int perm_stat = 0;		/* permanent status */

code_int getcode();
int decompress();
int myread();

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
int block_compress = BLOCK_MASK;
int clear_flg = 0;
long int ratio = 0;

#define CHECK_GAP 10000		/* ratio check interval */
count_int checkpoint = CHECK_GAP;

/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */
#define FIRST	257		/* first free entry */
#define	CLEAR	256		/* table clear output code */

int main()
{
  int i, j;
  char buf[50];
  char_type c, d;

  printf("zcat disk writer, known devices are ");
  for (i=0; devsw[i].dv_name; i++) printf("%s ", devsw[i].dv_name);
  printf("\n");
  do {
    printf("Enter file containing compressed data: ");
    gets(buf);
    i = open(buf, 0, 0);
  } while (i<=0);
  do {
    printf("Enter device to write uncompressed data: ");
    gets(buf);
    j = open(buf, 1, 0777);
  } while (j<=0);

  /* Check the magic number */
  myread(i, &c, 1);
  myread(i, &d, 1);

  if ((c != magic_header[0]) || (d != magic_header[1])) {
    printf("input not in compressed format\n");
    exit(1);
  }
  /* Get maxbits from file */
  myread(i, &c, 1);
  maxbits = c;
  block_compress = maxbits & BLOCK_MASK;
  maxbits &= BIT_MASK;
  maxmaxcode = 1 << maxbits;
  if (maxbits > BITS) {
    printf("input compressed with %d bits, can only handle %d bits\n",
	   maxbits, BITS);
    exit(1);
  }
  hsize = HSIZE;
  decompress(i, j);
  printf("Image written, looping indefintely\n");
  while (1) ;
}


/*
 * Decompress stdin to stdout.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */

#define BSIZE 512
char buf[BSIZE];

int decompress(in, out)
  int in, out;
{
  register char_type *stackp;
  register int finchar;
  register code_int code, oldcode, incode;
  int cnt = 0;

  /*
   *  As above, initialize the first 256 entries in the table.
   */
  maxcode = MAXCODE(n_bits = INIT_BITS);
  for (code = 255; code >= 0; code--) {
    tab_prefixof(code) = 0;
    tab_suffixof(code) = (char_type) code;
  }
  free_ent = ((block_compress) ? FIRST : 256);

  finchar = oldcode = getcode(in);
  if (oldcode == -1)		/* EOF already? */
    return (0);			/* Get out of here */
  buf[cnt++] = (char) finchar;	/* first code must be 8 bits = char */
  if (cnt == BSIZE) {
    write(out, buf, BSIZE);
    cnt = 0;
  }
  stackp = de_stack;

  while ((code = getcode(in)) > -1) {

    if ((code == CLEAR) && block_compress) {
      for (code = 255; code >= 0; code--)
	tab_prefixof(code) = 0;
      clear_flg = 1;
      free_ent = FIRST - 1;
      if ((code = getcode(in)) == -1)	/* O, untimely death! */
	break;
    }
    incode = code;
    /*
     *  Special case for KwKwK string.
     */
    if (code >= free_ent) {
      *stackp++ = finchar;
      code = oldcode;
    }
    /*
     *  Generate output characters in reverse order
     */
#ifdef SIGNED_COMPARE_SLOW
    while (((unsigned long) code) >= ((unsigned long) 256)) {
#else
    while (code >= 256) {
#endif
      *stackp++ = tab_suffixof(code);
      code = tab_prefixof(code);
    }
    *stackp++ = finchar = tab_suffixof(code);

    /*
     *  And put them out in forward order
     */
    do {
      buf[cnt++] = *--stackp;
      if (cnt == BSIZE) {
	write(out, buf, BSIZE);
	cnt = 0;
      }
    } while (stackp > de_stack);

    /*
     *  Generate the new entry.
     */
    if ((code = free_ent) < maxmaxcode) {
      tab_prefixof(code) = (unsigned short) oldcode;
      tab_suffixof(code) = finchar;
      free_ent = code + 1;
    }
    /*
     *  Remember previous code.
     */
    oldcode = incode;
  }
  if (cnt)
    write(out, buf, BSIZE);
  return (0);
}

char_type rmask[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

/*****************************************************************
 * TAG( getcode )
 *
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 * 	stdin
 * Outputs:
 * 	code or -1 is returned.
 */

code_int
getcode(in)
  int in;
{
  /*
   * On the VAX, it is important to have the register declarations in exactly
   * the order given, or the asm will break.
   */
  register code_int code;
  static int offset = 0, size = 0;
  static char_type buf[BITS];
  register int r_off, bits;
  register char_type *bp = buf;

  if (clear_flg > 0 || offset >= size || free_ent > maxcode) {
    /*
     * If the next entry will be too big for the current code size, then we
     * must increase the size.  This implies reading a new buffer full, too.
     */
    if (free_ent > maxcode) {
      n_bits++;
      if (n_bits == maxbits)
	maxcode = maxmaxcode;	/* won't get any bigger now */
      else
	maxcode = MAXCODE(n_bits);
    }
    if (clear_flg > 0) {
      maxcode = MAXCODE(n_bits = INIT_BITS);
      clear_flg = 0;
    }
    size = myread(in, buf, n_bits);
    if (size <= 0)
      return -1;		/* end of file */
    offset = 0;
    /* Round size down to integral number of codes */
    size = (size << 3) - (n_bits - 1);
  }
  r_off = offset;
  bits = n_bits;
#ifdef vax
  asm("extzv   r10,r9,(r8),r11");
#else				/* not a vax */
  /*
   * Get to the first byte.
   */
  bp += (r_off >> 3);
  r_off &= 7;
  /* Get first part (low order bits) */
#ifdef NO_UCHAR
  code = ((*bp++ >> r_off) & rmask[8 - r_off]) & 0xff;
#else
  code = (*bp++ >> r_off);
#endif				/* NO_UCHAR */
  bits -= (8 - r_off);
  r_off = 8 - r_off;		/* now, offset into code word */
  /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
  if (bits >= 8) {
#ifdef NO_UCHAR
    code |= (*bp++ & 0xff) << r_off;
#else
    code |= *bp++ << r_off;
#endif				/* NO_UCHAR */
    r_off += 8;
    bits -= 8;
  }
  /* high order bits. */
  code |= (*bp & rmask[bits]) << r_off;
#endif				/* vax */
  offset += n_bits;

  return code;
}

static char mybuf[512];
static int mycnt = 512;

/* Fudge a read of < 512 bytes */

myread(fdesc, buf, count)
  int fdesc;
  char *buf;
  int count;
{
  int j, i = 0;

  while (count) {
    if (mycnt == 512) {
      j= read(fdesc, mybuf, 512);
      if (j==0) return(-1);
      mycnt = 0;
    }
    while (count && (mycnt != 512)) {
      *buf++ = mybuf[mycnt++];
      i++;
      count--;
    }
  }
  return (i);
}
