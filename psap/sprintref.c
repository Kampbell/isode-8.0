/* sprintref.c - manage encoded session addresses */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap/RCS/sprintref.c,v 9.0 1992/06/16 12:25:44 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap/RCS/sprintref.c,v 9.0 1992/06/16 12:25:44 isode Rel $
 *
 *
 * $Log: sprintref.c,v $
 * Revision 9.0  1992/06/16  12:25:44  isode
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


/* LINTLIBRARY */

#include <stdio.h>
#include "psap.h"
#include "ssap.h"

/*  */

char *
sprintref (struct SSAPref *sr) {
	char  *cp;
	static char buffer[BUFSIZ];

	cp = buffer;
	*cp++ = '<';

	if (sr -> sr_ulen) {
		if (sr -> sr_ulen > 1 && *(sr -> sr_udata + 1) + 2 == sr -> sr_ulen)
			sprintf (cp, "%*.*s", sr -> sr_ulen - 2, sr -> sr_ulen - 2,
					 sr -> sr_udata + 2);
		else
			sprintf (cp, "%*.*s", sr -> sr_ulen, sr -> sr_ulen,
					 sr -> sr_udata);
		cp += strlen (cp);
	}
	*cp++ = ',';

	if (sr -> sr_clen) {
		if (sr -> sr_clen > 1 && *(sr -> sr_cdata + 1) + 2 == sr -> sr_clen)
			sprintf (cp, "%*.*s", sr -> sr_clen - 2, sr -> sr_clen - 2,
					 sr -> sr_cdata + 2);
		else
			sprintf (cp, "%*.*s", sr -> sr_clen, sr -> sr_clen,
					 sr -> sr_cdata);
		cp += strlen (cp);
	}
	*cp++ = ',';

	if (sr -> sr_alen) {
		if (sr -> sr_alen > 1 && *(sr -> sr_adata + 1) + 2 == sr -> sr_alen)
			sprintf (cp, "%*.*s", sr -> sr_alen - 2, sr -> sr_alen - 2,
					 sr -> sr_adata + 2);
		else
			sprintf (cp, "%*.*s", sr -> sr_alen, sr -> sr_alen,
					 sr -> sr_adata);
		cp += strlen (cp);
	}
	*cp++ = ',';

	if (sr -> sr_vlen) {
		if (sr -> sr_vlen > 1 && *(sr -> sr_vdata + 1) + 2 == sr -> sr_vlen)
			sprintf (cp, "%*.*s", sr -> sr_vlen - 2, sr -> sr_vlen - 2,
					 sr -> sr_vdata + 2);
		else
			sprintf (cp, "%*.*s", sr -> sr_vlen, sr -> sr_vlen,
					 sr -> sr_vdata);
		cp += strlen (cp);
	}
	*cp++ = '>';

	*cp = NULL;

	return buffer;
}
