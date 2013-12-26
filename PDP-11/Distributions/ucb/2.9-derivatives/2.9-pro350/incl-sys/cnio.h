/* These definitions are used for ioctl's that allow the pro3xx screen
 * to be driven in a bit mapped mode.
 */
#define SIOCPLOT	(('s'<<8)|127)	/* Set screen to plot mode */
#define SIOCPRNT	(('s'<<8)|126)	/* Set screen to print mode */
#define SIOCGRPH	(('s'<<8)|125)	/* Do a graphics update */
#define SIOCSCRL	(('s'<<8)|124)	/* Do a raw scroll, no clearing */
#define SIOCSOFF	(('s'<<8)|123)	/* Set plot offset into bitmap */
#define SIOCCSN		(('s'<<8)|122)	/* Turn curser on */
#define SIOCCSF		(('s'<<8)|121)	/* Turn curser off */
#define SIOCCSP		(('s'<<8)|120)	/* Move curser to... */
#define	SIOCPLEN	(('s'<<8)|119)	/* Set memory planes enable */
#define	SIOCSCM		(('s'<<8)|118)	/* Set colour map entry */
