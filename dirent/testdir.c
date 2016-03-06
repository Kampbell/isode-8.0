/*
	testdir -- basic test for C library directory access routines

	last edit:	25-Apr-1987	D A Gwyn
*/

#include	<sys/types.h>
#include	<stdio.h>
#include	"usr.dirent.h"

extern void	exit();
extern int	strcmp();

int 
main (int argc, char **argv)
{
	DIR		*dirp;
	struct dirent	*dp;
	int			nerrs = 0;	/* total not found */

	if ( (dirp = opendir( "." )) == NULL ) {
		fprintf( stderr, "Cannot open \".\" directory\n" );
		exit( 1 );
	}

	if (argc == 1) {
		while (dp = readdir (dirp))
			printf ("ino=%d len=%d name=\"%s\"\n",
						  dp -> d_ino, strlen (dp -> d_name), dp -> d_name);
		 closedir (dirp);
		exit (0);
	}

	while ( --argc > 0 ) {
		++argv;

		while ( (dp = readdir( dirp )) != NULL )
			if ( strcmp( dp->d_name, *argv ) == 0 ) {
				printf( "\"%s\" found.\n", *argv );
				break;
			}

		if ( dp == NULL ) {
			printf( "\"%s\" not found.\n", *argv );
			++nerrs;
		}

		rewinddir( dirp );
	}

	closedir( dirp );
	exit( nerrs );

	return 0;
}

