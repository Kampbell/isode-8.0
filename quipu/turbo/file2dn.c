#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/quipu/turbo/RCS/file2dn.c,v 9.0 1992/06/16 12:36:15 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/quipu/turbo/RCS/file2dn.c,v 9.0 1992/06/16 12:36:15 isode Rel $
 *
 *
 * $Log: file2dn.c,v $
 * Revision 9.0  1992/06/16  12:36:15  isode
 * Release 8.0
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


#include "general.h"
#include <stdio.h>

main(argc, argv)
int	argc;
char	**argv;
{
	char	*dn, *p, *mp;
	int	prefixlen;
	int	mapped;
	char	mapbuf[BUFSIZ];
	char	dnbuf[3 * BUFSIZ];
	char	mapfile[BUFSIZ];
	FILE	*map;

	if (argc < 3)
		prefixlen = 0;
	else
		prefixlen = strlen(argv[1]);

	dn = argv[argc - 1] + prefixlen;

	if (*dn == '/')
		dn++;

	dnbuf[0] = '\0';

	while ((p = index(dn, '/')) != NULL) {
		 strncpy(mapfile, argv[argc - 1], dn - argv[argc - 1]);
		mapfile[dn - argv[argc - 1]] = '\0';
		 strcat(mapfile, "EDB.map");
		mapped = 0;
		if ((map = fopen(mapfile, "r")) != NULL) {
			while (fgets(mapbuf, BUFSIZ, map) != NULL)
				if ((mp = index(mapbuf, '#')) != NULL) {
					*mp++ = '\0';
					if (!strncmp(mp, dn, p - dn)) {
						 strcat(dnbuf, mapbuf);
						mapped++;
						break;
					}
				}
			 fclose(map);
		}
		if (!mapped)
			 strncat(dnbuf, dn, p - dn);
		 strcat(dnbuf, "@");
		dn = p + 1;
	}
	if (strlen(dnbuf) == 0)
		 strcpy(dnbuf, "root@");
	p = rindex(dnbuf, '@');
	*p = '\0';
	puts(dnbuf);

	return(0);
}
