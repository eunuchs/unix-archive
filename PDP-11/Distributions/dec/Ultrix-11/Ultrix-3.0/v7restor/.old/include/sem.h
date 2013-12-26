/*
 * Macroes to access a
 * semaphore file.
 */

int	_semu; 

#define	unlock(sem)	write(_semu, 0, sem)
#define	lock(sem)	write(_semu, 1, sem)
#define	lockif(sem)	write(_semu, 2, sem)
#define	locked(sem)	write(_semu, 3, sem)
#define	semopen(s)	(_semu=open((s), 1))
#define	semclose()	close(_semu)
