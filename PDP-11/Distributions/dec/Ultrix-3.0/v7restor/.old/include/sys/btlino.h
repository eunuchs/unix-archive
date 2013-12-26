/*
 * Inode structure as it appears on
 * a disk block.
 * Version 7 original BTL format.
 */
struct oinode
{
	unsigned short	oi_mode;     	/* mode and type of file */
	short	oi_nlink;    	/* number of links to file */
	short	oi_uid;      	/* owner's user id */
	short	oi_gid;      	/* owner's group id */
	off_t	oi_size;     	/* number of bytes in file */
	char  	oi_addr[40];	/* disk block addresses */
	time_t	oi_atime;   	/* time last accessed */
	time_t	oi_mtime;   	/* time last modified */
	time_t	oi_ctime;   	/* time created */
};
#define	ONOPB	8	/* 8 inodes per block */
/*
 * the 40 address bytes:
 *	39 used; 13 addresses
 *	of 3 bytes each.
 */
