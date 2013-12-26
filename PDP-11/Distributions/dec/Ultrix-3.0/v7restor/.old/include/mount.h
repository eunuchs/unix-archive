/*
 * Structure of mount table
 * (/etc/mtab).
 */

#ifndef MMAX
#define	MMAX	32
#endif
struct mtab {
	char	m_mount[MMAX];	/* pathname where mounted */
	char	m_dev[MMAX];	/* device name mounted there */
};
