/* This program is a standalone formatter/bad block locator for
 * the rd50/51 on the pro 300. It writes 0377 in the backup revision
 * byte to mark a sector bad and creates a sector log ala bad144.
 */
#include <sys/types.h>
#define BADSECT 1
#include <sys/dkbad.h>
#include <sys/rdreg.h>
#define	SKEW	4
#define	INTRLV	2

#define RDADDR	((struct rddevice *) 0174000)
struct rdst rdst[] = {
	16,	4,	16*4,	305,
	16,	4,	16*4,	152,
	0,	0,	0,	0,
};

char module[] = {"Rdfmt"};
struct dkbad badblk;
int badpos, secpos;
int inblk[256];

main() {
	register int cnt, *ptr;
	register struct rdst *st;
	int cylder, track, sector, out;
	char buff[100];

	do {
		printf("Drive type (RD51 - 0 or RD50 - 1) ?");
		gets(buff);
	} while(buff[0] < '0' || buff[0] > '1');
	st = &rdst[buff[0]-'0'];
	badblk.bt_csn = 1;
	badblk.bt_mbz = 0;
	badblk.bt_flag = 0;
	for (cnt = 0; cnt < 126; cnt++) {
		badblk.bt_bad[cnt].bt_cyl = 0xffff;
		badblk.bt_bad[cnt].bt_trksec = 0xffff;
	}
	badpos = 0;
	secpos = 0;
	for (cnt = 0; cnt < 256; cnt++)
		inblk[cnt] = 0x5555;
	rdinit();
	printf("Format begins...\n");
	for(cylder = 0; cylder <= st->ncyl; cylder++)
	  for(track = 0; track < st->ntrak; track++) {
	for(cnt = 0; cnt < st->nsect; cnt++)
		inblk[((((cnt*INTRLV)%(st->nsect))+(cnt/((st->nsect)/INTRLV))+secpos)%(st->nsect))] = cnt;
	secpos = ((secpos+SKEW)%(st->nsect));
	RDADDR->cyl = cylder;
	RDADDR->trk = track;
	RDADDR->csr = RD_FORMATCOM;
	for (cnt = 0; cnt < 256; cnt++) {
		while ((RDADDR->st & RD_DRQ) == 0) ;
		RDADDR->db = inblk[cnt];
	}
	while ((RDADDR->st & RD_OPENDED) == 0) ;
	if (RDADDR->csr & RD_ERROR) {
		printf("Hard err cyl=%d trk=%d sec=%d stat=%o\n",
			cylder,track,sector,RDADDR->err);
			halt();
	}
    for(sector = 0; sector < st->nsect; sector++) {
		RDADDR->cyl = cylder;
		RDADDR->trk = track;
		RDADDR->sec = sector;
		RDADDR->csr = RD_READCOM;
		while (RDADDR->st & RD_BUSY) ;
		if (RDADDR->csr & RD_ERROR) {
			printf("Bad block cyl=%d trk=%d sec=%d stat=%o\n",
			cylder,track,sector,RDADDR->err);
			badblk.bt_bad[badpos].bt_cyl = cylder;
			badblk.bt_bad[badpos++].bt_trksec = ((track<<8)&0177400)|(sector);
			rdinit();
			RDADDR->cyl = cylder;
			RDADDR->trk = track;
			RDADDR->sec = RD_BAD|sector;
			RDADDR->csr = RD_WRITECOM;
			for(cnt = 0; cnt < 256; cnt++) {
				while ((RDADDR->st & RD_DRQ) == 0) ;
				RDADDR->db = 0;
			}
			while ((RDADDR->st & RD_OPENDED) == 0) ;
			if (RDADDR->csr & RD_ERROR)
				rdinit();
		} else {
			for(cnt = 0; cnt < 256; cnt++) {
				while ((RDADDR->st & RD_DRQ) == 0) ;
				out = RDADDR->db;
			}
			while ((RDADDR->st & RD_OPENDED) == 0) ;
		}
	  }
	}
	for(sector = 0; sector < st->nsect; sector++) {
		RDADDR->cyl = st->ncyl;
		RDADDR->trk = st->ntrak-1;
		RDADDR->sec = sector;
		RDADDR->csr = RD_WRITECOM;
		ptr = &badblk;
		for(cnt = 0; cnt < 256; cnt++) {
			while ((RDADDR->st & RD_DRQ) == 0) ;
			RDADDR->db = *ptr++;
		}
		while ((RDADDR->st & RD_OPENDED) == 0) ;
		if (RDADDR->csr & RD_ERROR) {
			printf("Hard err cyl=%d trk=%d sec=%d stat=%o\n",
			cylder,track,sector,RDADDR->err);
			halt();
		}
	}
	printf("Format and bad scan done\n");
	printf("Total bad blocks = %d\n",badpos);
}
halt() {

	for(;;) ;
}
