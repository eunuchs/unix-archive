/*
 * The I node is the focus of all
 * file activity in unix. There is a unique
 * inode allocated for each active file,
 * each current directory, each mounted-on
 * file, text file, and the root. An inode is 'named'
 * by its dev/inumber pair. (iget/iget.c)
 * Data, from mode on, is read in
 * from permanent inode on volume.
 */
#define	ONADDR	13
struct	oinode
{
	char	o_flag;
	char	o_count;	/* reference count */
	dev_t	o_dev;		/* device where inode resides */
	ino_t	o_number;	/* i number, 1-to-1 with device address */
	unsigned short	o_mode;
	short	o_nlink;	/* directory entries */
	short	o_uid;		/* owner */
	short	o_gid;		/* group of owner */
	off_t	o_size;		/* size of file */
	union {
		struct {
			daddr_t o_addr[ONADDR];	/* if normal file/directory */
			daddr_t	o_lastr;	/* last logical block read (for read-ahead) */
		};
	} o_un;
};


extern struct inode inode[];	/* The inode table itself */
struct inode *mpxip;		/* mpx virtual inode */

/* flags */
#define	OLOCK	01		/* inode is locked */
#define	OUPD	02		/* file has been modified */
#define	OACC	04		/* inode access time to be updated */
#define	OMOUNT	010		/* inode is mounted on */
#define	OWANT	020		/* some process waiting on lock */
#define	OTEXT	040		/* inode is pure text prototype */
#define	OCHG	0100		/* inode has been changed */

/* modes */
#define	OFMT	0170000		/* type of file */
#define		OFDIR	0040000	/* directory */
#define		OFCHR	0020000	/* character special */
#define		OFBLK	0060000	/* block special */
#define		OFREG	0100000	/* regular */
#define		OFMPC	0030000	/* multiplexed char special */
#define		OFMPB	0070000	/* multiplexed block special */
#define	OSUID	04000		/* set user id on execution */
#define	OSGID	02000		/* set group id on execution */
#define OSVTX	01000		/* save swapped text even after use */
#define	OREAD	0400		/* read, write, execute permissions */
#define	OWRITE	0200
#define	OEXEC	0100
