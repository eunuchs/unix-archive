#include <stdio.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/dir.h>

main (argc,argv)
  int argc ;
  char ** argv ;
{
	register int fd, bytes ;
	FILE * lfd, * dfd ;
	char * buffer ;
	char line[256] ;
	struct mtop mt_command;
	extern char * malloc() ;

	if ( argc != 3 )
	{
		fprintf (stderr,"Usage: %s log-file dump-file\n",argv[0]) ;
		exit (1) ;
	}

	buffer = malloc (65535) ;

	if ( ( fd = open ("/dev/rmt1600a",1) ) < 0 )
	{
		fprintf (stderr,"%s: can't open tape unit /dev/rmt1600a\n",
			argv[0]) ;
		exit (1) ;
	}

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

	while ( fgets (line,sizeof(line),lfd) != NULL )
	{
		if ( strncmp (line,"EOF",3) == 0 )
		{
			mt_command.mt_op = MTWEOF ;
			mt_command.mt_count = 1 ;

			if ( ioctl (fd,MTIOCTOP,&mt_command) != 0 )
				fprintf (stderr,"tape EOF problem?\n") ;

			fprintf (stderr,"\n end of file.\n") ;
		}
		else if ( strncmp (line,"EOT",3) == 0 )
		{
			fprintf (stderr,"End of Tape.\n") ;
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

			if ( fread (buffer,1,bytes,dfd) != bytes )
			{
				fprintf (stderr,"bad read chunk?\n") ;
				break ;
			}

			if ( write (fd,buffer,bytes) != bytes )
			{
				fprintf (stderr,"tape write error?\n") ;
				break ;
			}

			fprintf (stderr,".") ;
		}
	}

	(void) close (fd) ;
	(void) fclose (lfd) ;
	(void) fclose (dfd) ;

	exit (0) ;
}
