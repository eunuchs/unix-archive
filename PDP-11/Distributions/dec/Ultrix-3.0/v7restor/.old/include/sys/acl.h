/*
 * Structure of an entry in an
 * access control file.
 */
struct acl {
	short	acl_uid;	/* user ID */
	char	acl_fmode;	/* file permissions */
	char	acl_dmode;	/* directory permissions */
};

#define	ACLSHIFT 6	/* amount to shift to get mode into owner mode bits */
#define	ACLMASK	07	/* mask for permission bits */
