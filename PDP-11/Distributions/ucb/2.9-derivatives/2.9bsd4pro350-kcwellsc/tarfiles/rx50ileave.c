/* Written by Warren Toomey. Hacked up in an hour, so this isn't
 * very well written.
 *
 * Undo the RX50 interleaving on stdin, writing disk image to stdout.
 */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#define BLKSIZE 512
#define BLKSPERTRACK 10
#define TRACKS 79

/* The interleaving is a pattern of 10 tracks and 10 sectors.
 * The table below shows the interleaving. If the current
 * track (mod 10) is t, and the current sector is s, then
 * the next block will be s + iltable[s][t].
 */

int iltable[10][10]= {
	{ 1, 1, 1, 1, 1, 1, 1, 1, 6, 10 },
	{ 1, 1, 1, 1, 1, 1, 6, 10, 1, 1 },
	{ 1, 1, 1, 1, 6, 10, 1, 1, 1, 1 },
	{ 1, 1, 6, 10, 1, 1, 1, 1, 1, 1 },
	{ 1, 10, -4, -4, -4, -4, -4, -4, -4, -4 },
	{ 1, 1, 1, 1, 1, 1, 1, -3, 10, 1 },
	{ 1, 1, 1, 1, 1, -3, 10, 1, 1, 1 },
	{ 1, 1, 1, -3, 10, 1, 1, 1, 1, 1 },
	{ 1, -8, 10, 1, 1, 1, 1, 1, 1, 1 },
	{ 10, -4, -4, -4, -4, -4, -4, -4, -4, -8 }
};


main()
{
 char tkbuf[BLKSIZE * BLKSPERTRACK * TRACKS];
 int track;
 int sector;
 int s,t,count;

 /* Read the entire disk */
 for (track=0; track<TRACKS; track++) {
   read(0,&tkbuf[track * BLKSIZE * BLKSPERTRACK], BLKSIZE * BLKSPERTRACK);
 }

   /* Now output the track in interleave form */
 for (track=0,sector=0, count=0; count<(BLKSPERTRACK*TRACKS); count++) {
   write(1, &tkbuf[BLKSIZE * sector], BLKSIZE);
   t= track % 10; s= sector % 10;
   sector += iltable[s][t]; track= sector / BLKSPERTRACK;
 }
}
