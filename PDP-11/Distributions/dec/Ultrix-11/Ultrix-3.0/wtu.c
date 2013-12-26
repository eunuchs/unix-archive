#include <stdio.h>

main (argc,argv)
  int argc ;
  char ** argv ;
{
	register int n, fd, bytes ;
	FILE * lfd, * dfd ;
	char * buffer ;
	char line[256] ;
	extern char * malloc() ;

	if ( argc != 3 )
	{
		fprintf (stderr,"Usage: %s log-file dump-file\n",argv[0]) ;
		exit (1) ;
	}

	buffer = malloc (65535) ;

	if ( ( lfd = fopen (argv[1],"r") ) == NULL )
	{
		perror (argv[1]) ;
		exit (1) ;
	}

	if ( ( dfd = fopen (argv[2],"r") ) == NULL )
	{
		perror (argv[2]) ;
		exit (1) ;
	}

	for ( n = 1, fd = -1 ; fgets (line,sizeof(line),lfd) != NULL ; )
	{
		if ( strncmp (line,"EOF",3) == 0 )
		{
			fprintf (stderr," done.\n") ;
			(void) close (fd) ;
			fd = -1 ;
			n++ ;
		}
		else if ( strncmp (line,"EOT",3) == 0 )
		{
			fprintf (stderr,"end of dump.\n") ;
			break ;
		}
		else
		{
			bytes = atoi (line) ;

			if ( bytes <= 0 || (bytes%512) != 0 )
			{
				fprintf (stderr,"bogus write line?\n") ;
				break ;
			}

			if ( fd == -1 )
			{
				sprintf (line,"file%02d",n) ;

				if ( ( fd = creat (line,0644) ) < 0 )
				{
					perror (line) ;
					break ;
				}

				fprintf (stderr,"%s ... ",line) ;
			}

			if ( fread (buffer,1,bytes,dfd) != bytes )
			{
				fprintf (stderr,"read error?\n") ;
				break ;
			}

			if ( write (fd,buffer,bytes) != bytes )
			{
				fprintf (stderr,"write error?\n") ;
				break ;
			}
		}
	}

	(void) fclose (lfd) ;
	(void) fclose (dfd) ;

	exit (0) ;
}
