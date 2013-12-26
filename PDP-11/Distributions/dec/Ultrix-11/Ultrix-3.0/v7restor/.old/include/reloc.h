/*
 * Relocation bits
 */
#define	R_PCREL	01	/* relocation is pc-relative */
#define	R_TYPE	016
#define	R_ABS	0
#define	R_TEXT	02
#define	R_DATA	04
#define	R_BSS	06
#define	R_EXT	010
#define	R_SHIFT	4	/* shift for symbol number */
#define	R_SYMNO	(07777<<R_SHIFT)	/* mask for same */
