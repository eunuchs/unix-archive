/*
 * Icheck/dcheck error bits.
 */
#define	IIO	1	/* I/O error */
#define	ICIRC	02	/* Circular free list */
#define	IDUPF	04	/* Dups in free */
#define	IMISS	010	/* Missing */
#define	IBADI	020	/* Bad address in, or under, inode */
#define	IDUPI	040	/* Dups in file (in, or under, inode) */
#define	ICOUNT	0100	/* Bad count in freelist block */
#define	IBADF	0200	/* Bad free block address */

#define	DIO	01	/* I/O error */
#define	DURK	02	/* Impossible situation */
#define	DBAD	04	/* Bad block */
#define	DHUGE	010	/* Huge directory */
#define	DNOENT	020	/* Entry count zero */
#define	DCOUNT	040	/* Entry and link counts differ */
#define	DMLINK	0100	/* Entry greater than link count */
#define	DMENT	0200	/* Entry less than link count, link count>0 */
