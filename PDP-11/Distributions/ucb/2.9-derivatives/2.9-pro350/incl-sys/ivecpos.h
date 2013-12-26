/* These constants are used by ienable, idisable calls to
 * indicate which device is calling.
 * The macro IVEC generates the numbers for option slots using
 * the device's base address and APOS or BPOS to define which
 * vector.
 */
#define	KBRPOS	1
#define KBXPOS	2
#define	CMPOS	3
#define CMMPOS	4
#define PLRPOS	5
#define	PLXPOS	6
#define	CLPOS	7

#define APOS 0
#define BPOS 8

#define IVEC(a,b) (((((unsigned)a)>>7)&07)+8+(b))
