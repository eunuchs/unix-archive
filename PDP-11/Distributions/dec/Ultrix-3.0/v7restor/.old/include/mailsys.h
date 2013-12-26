#ifdef COMMENT
	Proprietary Rand Corporation, 1981.
	Further distribution of this software
	subject to the terms of the Rand
	license agreement.
#endif

#define MSGRETTIME      (5*24*60*60)    /* How long to keep trying */
#define MAILQDIR        "/usr/spool/netmail/"
#define MAILDROP        "/usr/spool/mail/"
#define TMAILQDIR       "/usr/spool/mailt/"
#define MAILLOCKDIR     "/usr/spool/locks/"
#define MAILALIASES     "/etc/MailAliases"

/* Exit codes from net mailer */
#define NM_OK        0  /* Delivered */
#define NM_BAD       1  /* Not delivered to some addresses (perm failure) */
#define NM_MORE      2  /* Some addresses not yet processed (temp failure) */
#define NM_BADMORE   3  /* Both of above */
