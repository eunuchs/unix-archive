/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)string.h	5.1.3 (2.11BSD) 1996/3/20
 */

#include <sys/types.h>

#ifndef	NULL
#define	NULL	0
#endif

extern	char	*strcat(), *strncat(), *strcpy(), *strncpy(), *index();
extern	char	*rindex(), *strstr(), *syserrlst();
extern	int	strcmp(), strncmp(), strcasecmp(), strncasecmp(), strlen();
extern	int	memcmp();

extern	char	*memccpy(), *memchr(), *memcpy(), *memset(), *strchr();
extern	char	*strdup(), *strpbrk(), *strrchr(), *strsep(), *strtok();
extern	size_t	strcspn(), strspn();

extern	char	*strerror();
