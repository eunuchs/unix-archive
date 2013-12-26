#if	defined(DOSCCS) && !defined(lint)
static char sccsid[] = "@(#)bsdtcp.c	4.3.2 (2.11BSD GTE) 1996/3/22";
#endif

#include "../condevs.h"
#ifdef BSDTCP
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 *	bsdtcpopn -- make a tcp connection
 *
 *	return codes:
 *		>0 - file number - ok
 *		FAIL - failed
 */

bsdtcpopn(flds)
register char *flds[];
{
	struct servent *sp;
	struct hostent *hp;
	struct	sockaddr_in hisctladdr;
	int s = -1, port;
	extern int errno;

	sp = getservbyname(flds[F_CLASS], "tcp");
	if (sp == NULL) {
		port = htons(atoi(flds[F_CLASS]));
		if (port == 0) {
			logent(_FAILED, "UNKNOWN PORT NUMBER");
			return CF_SYSTEM;
		}
	} 	DEBUG(4, "bsdtcpopn host %s, ", flds[F_PHONE]);
	DEBUG(4, "port %d\n", ntohs(port));
	if (setjmp(Sjbuf)) {
		bsdtcpcls(s);
		logent("tcpopen", "TIMEOUT");
		return CF_DIAL;
	}

	bzero((char *)&hisctladdr, sizeof (hisctladdr));
	hp = gethostbyname(flds[F_PHONE]);
	if (hp == NULL) {
		logent("tcpopen","UNKNOWN HOST");
		return CF_DIAL;
	}
	signal(SIGALRM, alarmtr);
	alarm(30);
	hisctladdr.sin_family = hp->h_addrtype;
#ifdef BSD2_9
	s = socket(SOCK_STREAM, 0, &hisctladdr, 0);
#else BSD4_2
	s = socket(hp->h_addrtype, SOCK_STREAM, 0);
#endif BSD4_2
	if (s < 0)
		goto bad;
#ifndef BSD2_9
	if (bind(s, (char *)&hisctladdr, sizeof (hisctladdr), 0) < 0)
		goto bad;
#endif BSD2_9
	bcopy(hp->h_addr, (c&hisctbcopyAM, 0);
#eneof (hisctladdr), 0) pe;
#
#eneoM, 0);length= hp->h_addrtype;
#4, "bsd host SOCK_STREAM, 0,laddconnectizeof (hisctladdr), 0) BSD2_9
p->h_addrtype,laddconnectizeof (hisctladdr), 0) < 0)
		goto bad;
#endif BSD2_9
		goto bad;
#ifbcopy(hp->hn_famil = hpCU_	gobsdpopen", " hp(SIGALR" hhp-:hn_famil = hppopen", "TIMEOUs(port5, "tcpetur failed: errno(setjmp(errnoMEOUHOST");strerror(errnoM, _FAILED) hp(SIGALRM, alarmt}

/*
 *dpopen", " -- cloh_atcp connection
 */
popen", "Tfd)
register int fd;
{
	s(port));
TCP CLOSE calledtjmp(0gent("tcfd > 0) {
		cloh_Tfd)gnals(port));
cloh_d fd(setjmp(fd)gna}t}
		goto badTCP
