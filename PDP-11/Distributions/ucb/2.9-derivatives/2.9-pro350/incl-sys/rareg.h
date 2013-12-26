/*
 * RA registers and structures
 */

struct radevice {
	short	raip;		/* initialization and polling */
	short	rasa;		/* status and address */
};

#define RA_IVEC	0154		/* This is the interrupt vector */
#define	RA_CYL	64		/* Pseudo cyl size for "sizes" table */

#define	RA_ERR		0100000	/* error bit */
#define	RA_STEP4	0040000	/* step 4 has started */
#define	RA_STEP3	0020000	/* step 3 has started */
#define	RA_STEP2	0010000	/* step 2 has started */
#define	RA_STEP1	0004000	/* step 1 has started */
#define	RA_NV		0002000	/* no host settable interrupt vector */
#define	RA_QB		0001000	/* controller supports Q22 bus */
#define	RA_DI		0000400	/* controller implements diagnostics */
#define	RA_IE		0000200	/* interrupt enable */
#define	RA_PI		0000001	/* host requests adapter purge interrupts */
#define	RA_GO		0000001	/* start operation, after init */

typedef struct { long val[2]; }	quad;
typedef struct { u_short low, high; } shorts;

/*
 * RA Communications Area
 */

struct raca {
	short	ca_xxx1;	/* unused */
	char	ca_xxx2;	/* unused */
	char	ca_bdp;		/* BDP to purge */
	short	ca_cmdint;	/* command queue transition interrupt flag */
	short	ca_rspint;	/* respoe queue transition interrupt flag */
	shorts	ca_rspdsc[NRSP];/* response descriptors */
	shorts	ca_cmddsc[NCMD];/* command descriptors */
};

#define	ca_ringbase	ca_rspdsc[0]

#define	RA_OWN	0x8000	/* UDA owns this descriptor */
#define	RA_INT	0x4000	/* allow interrupt on ring transition */

/*
 * MSCP packet info
 */
struct ms_hdr {
	short	ra_msglen;	/* length of MSCP packet */
	char	ra_credits;	/* low 4 bits: credits, high 4 bits: msgtype */
	char	ra_vcid;	/* virtual circuit id */
};

#define	RA_MSGTYPE_SEQ		0x00	/* Sequential message */
#define	RA_MSGTYPE_DATAGRAM	0x10	/* Datagram */
#define	RA_MSGTYPE_CREDITS	0x20	/* Credit notification */
#define	RA_MSGTYPE_MAINTENANCE	0xf0	/* Who knows */
/*
 * Definitions for the Mass Storage Control Protocol
 */


/*
 * Control message opcodes
 */
#define	M_OP_ABORT	0001	/* Abort command */
#define	M_OP_GTCMD	0002	/* Get command status command */
#define	M_OP_GTUNT	0003	/* Get unit status command */
#define	M_OP_STCON	0004	/* Set controller characteristics command */
#define	M_OP_SEREX	0007	/* Serious exception end message */
#define	M_OP_AVAIL	0010	/* Available command */
#define	M_OP_ONLIN	0011	/* Online command */
#define	M_OP_STUNT	0012	/* Set unit characteristics command */
#define	M_OP_DTACP	0013	/* Determine access paths command */
#define	M_OP_ACCES	0020	/* Access command */
#define	M_OP_CMPCD	0021	/* Compare controller data command */
#define	M_OP_ERASE	0022	/* Erase command */
#define	M_OP_FLUSH	0023	/* Flush command */
#define	M_OP_REPLC	0024	/* Replace command */
#define	M_OP_COMP	0040	/* Compare host data command */
#define	M_OP_READ	0041	/* Read command */
#define	M_OP_WRITE	0042	/* Write command */
#define	M_OP_AVATN	0100	/* Available attention message */
#define	M_OP_DUPUN	0101	/* Duplicate unit number attention message */
#define	M_OP_ACPTH	0102	/* Access path attention message */
#define	M_OP_END	0200	/* End message flag */


/*
 * Generic command modifiers
 */
#define	M_MD_EXPRS	0100000		/* Express request */
#define	M_MD_COMP	0040000		/* Compare */
#define	M_MD_CLSEX	0020000		/* Clear serious exception */
#define	M_MD_ERROR	0010000		/* Force error */
#define	M_MD_SCCHH	0004000		/* Suppress caching (high speed) */
#define	M_MD_SCCHL	0002000		/* Suppress caching (low speed) */
#define	M_MD_SECOR	0001000		/* Suppress error correction */
#define	M_MD_SEREC	0000400		/* Suppress error recovery */
#define	M_MD_SSHDW	0000200		/* Suppress shadowing */
#define	M_MD_WBKNV	0000100		/* Write back (non-volatile) */
#define	M_MD_WBKVL	0000040		/* Write back (volatile) */
#define	M_MD_WRSEQ	0000020		/* Write shadow set one unit at a time */

/*
 * AVAILABLE command modifiers
 */
#define	M_MD_ALLCD	0000002		/* All class drivers */
#define	M_MD_SPNDW	0000001		/* Spin down */

/*
 * FLUSH command modifiers
 */
#define	M_MD_FLENU	0000001		/* Flush entire unit */
#define	M_MD_VOLTL	0000002		/* Volatile only */

/*
 * GET UNIT STATUS command modifiers
 */
#define	M_MD_NXUNT	0000001		/* Next unit */

/*
 * ONLINE command modifiers
 */
#define	M_MD_RIP	0000001		/* Allow self destruction */
#define	M_MD_IGNMF	0000002		/* Ignore media format error */

/*
 * ONLINE and SET UNIT CHARACTERISTICS command modifiers
 */
#define	M_MD_ALTHI	0000040		/* Alter host identifier */
#define	M_MD_SHDSP	0000020		/* Shadow unit specified */
#define	M_MD_CLWBL	0000010		/* Clear write-back data lost */
#define	M_MD_STWRP	0000004		/* Set write protect */

/*
 * REPLACE command modifiers
 */
#define	M_MD_PRIMR	0000001		/* Primary replacement block */


/*
 * End message flags
 */
#define	M_EF_BBLKR	0200	/* Bad block reported */
#define	M_EF_BBLKU	0100	/* Bad block unreported */
#define	M_EF_ERLOG	0040	/* Error log generated */
#define	M_EF_SEREX	0020	/* Serious exception */


/*
 * Controller flags
 */
#define	M_CF_ATTN	0200	/* Enable attention messages */
#define	M_CF_MISC	0100	/* Enable miscellaneous error log messages */
#define	M_CF_OTHER	0040	/* Enable other host's error log messages */
#define	M_CF_THIS	0020	/* Enable this host's error log messages */
#define	M_CF_MLTHS	0004	/* Multi-host */
#define	M_CF_SHADW	0002	/* Shadowing */
#define	M_CF_576	0001	/* 576 byte sectors */


/*
 * Unit flags
 */
#define	M_UF_REPLC	0100000		/* Controller initiated bad block replacement */
#define	M_UF_INACT	0040000		/* Inactive shadow set unit */
#define	M_UF_WRTPH	0020000		/* Write protect (hardware) */
#define	M_UF_WRTPS	0010000		/* Write protect (software or volume) */
#define	M_UF_SCCHH	0004000		/* Suppress caching (high speed) */
#define	M_UF_SCCHL	0002000		/* Suppress caching (low speed) */
#define	M_UF_RMVBL	0000200		/* Removable media */
#define	M_UF_WBKNV	0000100		/* Write back (non-volatile) */
#define	M_UF_576	0000004		/* 576 byte sectors */
#define	M_UF_CMPWR	0000002		/* Compare writes */
#define	M_UF_CMPRD	0000001		/* Compare reads */


/*
 * Status codes
 */
#define	M_ST_MASK	037		/* Status code mask */
#define	M_ST_SUCC	000		/* Success */
#define	M_ST_ICMD	001		/* Invalid command */
#define	M_ST_ABRTD	002		/* Command aborted */
#define	M_ST_OFFLN	003		/* Unit offline */
#define	M_ST_AVLBL	004		/* Unit available */
#define	M_ST_MFMTE	005		/* Media format error */
#define	M_ST_WRTPR	006		/* Write protected */
#define	M_ST_COMP	007		/* Compare error */
#define	M_ST_DATA	010		/* Data error */
#define	M_ST_HSTBF	011		/* Host buffer access error */
#define	M_ST_CNTLR	012		/* Controller error */
#define	M_ST_DRIVE	013		/* Drive error */
#define	M_ST_DIAG	037		/* Message from an internal diagnostic */

/*
 *  Sub-codes of M_ST_SUCC
 */
#define	M_ST_SUCC_SPINDOWN	(1*040)	/* Spin-down Ignored */
#define	M_ST_SUCC_CONNECTED	(2*040)	/* Still Connected */
#define	M_ST_SUCC_DUPLICATE	(4*040)	/* Duplicate Unit Number */
#define	M_ST_SUCC_ONLINE	(8*040)	/* Already Online */
#define	M_ST_SUCC_STILLONLINE	(16*040)/* Still Online */

/*
 *  Sub-codes of M_ST_ICMD
 */
#define	M_ST_ICMD_LENGTH	(0*040)	/* Invalid Message Length */

/*
 *  Sub-codes of M_ST_OFFLN
 */
#define	M_ST_OFFLN_UNKNOWN	(0*040)	/* Unknown on online to another ctlr */
#define	M_ST_OFFLN_UNMOUNTED	(1*040)	/* Unmounted or run/stop at stop */
#define	M_ST_OFFLN_INOPERATIVE	(2*040)	/* Inoperative? */
#define	M_ST_OFFLN_DUPLICATE	(4*040)	/* Duplicate Unit Number */
#define	M_ST_OFFLN_DISABLED	(8*040)	/* Disabled by FS or diagnostic */

/*
 *  Sub-codes of M_ST_WRTPR
 */
#define	M_ST_WRTPR_HARDWARE	(256*040)/* Hardware Write Protect */
#define	M_ST_WRTPR_SOFTWARE	(128*040)/* Software Write Protect */

/*
 * An MSCP packet
 */

struct ms {
	struct	ms_hdr ms_header;/* device specific header */
	shorts	ms_cmdref;		/* cmf ref # - assigned 'bp' */
	short	ms_unit;		/* unit number */
	short	ms_xxx1;		/* unused */
	char	ms_opcode;		/* opcode */
	char	ms_flags;		/* end message flags */
	short	ms_modifier;		/* modifiers */
	union {
	struct {
		long	Ms_bytecnt;	/* byte count */
		shorts	Ms_buffer;	/* buffer descriptor */
		long	Ms_xxx2[2];	/* unused */
		long	Ms_lbn;	/* logical block number */
		long	Ms_xxx4;	/* unused */
		shorts	*Ms_dscptr;	/* pointer to descriptor (software) */
		short	Ms_hsize;	/* High order word of disk size */
		long	Ms_sftwds[4];	/* software words, padding */
	} ms_generic;
	struct {
		short	Ms_version;	/* MSCP version */
		short	Ms_cntflgs;	/* controller flags */
		short	Ms_hsttmo;	/* host timeout */
		short	Ms_usefrac;	/* use fraction */
		long	Ms_time;	/* time and date */
	} ms_setcntchar;
	struct {
		short	Ms_multunt;	/* multi-unit code */
		short	Ms_unitflgs;	/* unit flags */
		long	Ms_hostid;	/* host identifier */
		quad	Ms_unitid;	/* unit identifier */
		long	Ms_mediaid;	/* media type identifier */
		short	Ms_shdwunt;	/* shadow unit */
		short	Ms_shdwsts;	/* shadow status */
		short	Ms_track;	/* track size */
		short	Ms_group;	/* group size */
		short	Ms_cylinder;	/* cylinder size */
		short	Ms_xxx3;	/* reserved */
		short	Ms_rctsize;	/* RCT table size */
		char	Ms_rbns;	/* RBNs / track */
		char	Ms_rctcpys;	/* RCT copies */
	} ms_getunitsts;
	} ms_un;
};

/*
 * generic packet
 */

#define	ms_bytecnt	ms_un.ms_generic.Ms_bytecnt
#define	ms_buffer	ms_un.ms_generic.Ms_buffer
#define	ms_lbn	ms_un.ms_generic.Ms_lbn
#define	ms_dscptr	ms_un.ms_generic.Ms_dscptr
#define	ms_sftwds	ms_un.ms_generic.Ms_sftwds
#define	ms_status	ms_modifier

/*
 * Abort / Get Command Status packet
 */

#define	ms_outref	ms_bytecnt

/*
 * Online / Set Unit Characteristics packet
 */

#define	ms_errlgfl	ms_lbn
#define	ms_copyspd	ms_shdwsts

/*
 * Replace packet
 */

#define	ms_rbn	ms_bytecnt

/*
 * Set Controller Characteristics packet
 */

#define	ms_version	ms_un.ms_setcntchar.Ms_version
#define	ms_cntflgs	ms_un.ms_setcntchar.Ms_cntflgs
#define	ms_hsttmo	ms_un.ms_setcntchar.Ms_hsttmo
#define	ms_usefrac	ms_un.ms_setcntchar.Ms_usefrac
#define	ms_time	ms_un.ms_setcntchar.Ms_time

/*
 * Get Unit Status end packet
 */

#define	ms_multunt	ms_un.ms_getunitsts.Ms_multunt
#define	ms_unitflgs	ms_un.ms_getunitsts.Ms_unitflgs
#define	ms_hostid	ms_un.ms_getunitsts.Ms_hostid
#define	ms_unitid	ms_un.ms_getunitsts.Ms_unitid
#define	ms_mediaid	ms_un.ms_getunitsts.Ms_mediaid
#define	ms_shdwunt	ms_un.ms_getunitsts.Ms_shdwunt
#define	ms_shdwsts	ms_un.ms_getunitsts.Ms_shdwsts
#define	ms_track	ms_un.ms_getunitsts.Ms_track
#define	ms_group	ms_un.ms_getunitsts.Ms_group
#define	ms_cylinder	ms_un.ms_getunitsts.Ms_cylinder
#define	ms_rctsize	ms_un.ms_getunitsts.Ms_rctsize
#define	ms_rbns	ms_un.ms_getunitsts.Ms_rbns
#define	ms_rctcpys	ms_un.ms_getunitsts.Ms_rctcpys

/*
 * Online / Set Unit Characteristics end packet
 */

#define	ms_unt1		ms_dscptr
#define ms_unt2		ms_un.ms_generic.Ms_hsize
#define	ms_volser	ms_sftwds[0]

/*
 * Set Controller Characteristics end packet
 */

#define	ms_cnttmo	ms_hsttmo
#define	ms_cntcmdl	ms_usefrac
#define	ms_cntid	ms_unitid


/*
 * Error Log message format codes
 */
#define	M_FM_CNTERR	0	/* Controller error */
#define	M_FM_BUSADDR	1	/* Host memory access error */
#define	M_FM_DISKTRN	2	/* Disk transfer error */
#define	M_FM_SDI	3	/* SDI error */
#define	M_FM_SMLDSK	4	/* Small disk error */

/*
 * Error Log message flags
 */
#define	M_LF_SUCC	0200	/* Operation successful */
#define	M_LF_CONT	0100	/* Operation continuing */
#define	M_LF_SQNRS	0001	/* Sequence number reset */

/*
 * MSCP Error Log packet
 *
 *	NOTE: MSCP packet must be padded to this size.
 */

struct ml {
	struct	ms_hdr ml_header;/* device specific header */
	shorts	ml_cmdref;		/* command reference number */
	short	ml_unit;		/* unit number */
	short	ml_seqnum;		/* sequence number */
	char	ml_format;		/* format */
	char	ml_flags;		/* error log message flags */
	short	ml_event;		/* event code */
	quad	ml_cntid;		/* controller id */
	char	ml_cntsvr;		/* controller software version */
	char	ml_cnthvr;		/* controller hardware version */
	short	ml_multunt;		/* multi-unit code */
	quad	ml_unitid;		/* unit id */
	char	ml_unitsvr;		/* unit software version */
	char	ml_unithvr;		/* unit hardware version */
	short	ml_group;		/* group; retry + level */
	long	ml_volser;		/* volume serial number */
	long	ml_hdr;		/* header */
	char	ml_sdistat[12];	/* SDI status information */
};

#define	ml_busaddr	ml_unitid.val[0]
#define	ml_sdecyl	ml_group

/* This macro swaps the words in a long to keep the controller happy. */
#define	SWAPW(a)	((((long)(a))<<16)|((((long)(a))>>16)&0xffff))
