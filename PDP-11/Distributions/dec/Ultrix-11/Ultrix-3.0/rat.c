#include <stdio.h>

main (argc,argv)
  int argc ;
  char ** argv ;
{
	register int fd, bytes, eof ;
	FILE * ofd ;
	char * buffer ;
	extern char * malloc() ;

	buffer = malloc (65535) ;

	if ( ( fd = open ("/dev/rmt1600a",0) ) < 0 )
	{
		fprintf (stderr,"%s: can't open tape unit /dev/rmt1600a\n",
			argv[0]) ;
		exit (1) ;
	}

	if ( ( ofd = fopen ("./tapedump","w") ) == NULL )
	{
		perror ("./tapedump") ;
		exit (1) ;
	}

	for ( eof = 0 ; ; )
	{
		bytes = read (fd,buffer,65535) ;

		if ( bytes == 0 )
		{
			if ( eof++ > 0 )
			{
				printf ("EOT\n") ;
				break ;
			}

			printf ("EOF\n") ;
		}
		else
			if ( bytes < 0 )
			{
				printf ("I/O-ERROR\n") ;
			}
			else
			{
				printf ("%d\n",bytes) ;
				eof = 0 ;

				if ( fwrite (buffer,1,bytes,ofd) != bytes )
					fprintf (stderr,"write error\n") ;
			}
	}

	(void) close (fd) ;
	(void) fclose (ofd) ;

	exit (0) ;
}
