/* isoservent.c - look-up ISODE services */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/compat/RCS/isoservent.c,v 9.0 1992/06/16 12:07:00 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/compat/RCS/isoservent.c,v 9.0 1992/06/16 12:07:00 isode Rel $
 *
 *
 * $Log: isoservent.c,v $
 * Revision 9.0  1992/06/16  12:07:00  isode
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

#include <ctype.h>
#include <stdio.h>
#include "general.h"
#include "manifest.h"
#include "isoservent.h"
#include "tailor.h"

/*    DATA */

static char *isoservices = "isoservices";

static FILE *servf = NULL;
static int  stayopen = 0;

static struct isoservent    iss;

/*  */

int
setisoservent (int f) {
	if (servf == NULL)
		servf = fopen (isodefile (isoservices, 0), "r");
	else
		rewind (servf);
	stayopen |= f;

	return (servf != NULL);
}


int
endisoservent (void) {
	if (servf && !stayopen) {
		(void) fclose (servf);
		servf = NULL;
	}

	return 1;
}

/*  */

struct isoservent *
getisoservent (void) {
	int	    mask,
			vecp;
	char  *cp;
	struct isoservent *is = &iss;
	static char buffer[BUFSIZ + 1],
		   file[BUFSIZ];
	static char *vec[NVEC + NSLACK + 1];

	if (servf == NULL
			&& (servf = fopen (isodefile (isoservices, 0), "r")) == NULL)
		return NULL;

	bzero ((char *) is, sizeof *is);

	while (fgets (buffer, sizeof buffer, servf) != NULL) {
		if (*buffer == '#')
			continue;
		if (cp = index (buffer, '\n'))
			*cp = NULL;
		if ((vecp = str2vecX (buffer, vec, 1 + 1, &mask, NULL, 1)) < 3)
			continue;

		if ((cp = index (vec[0], '/')) == NULL)
			continue;
		*cp++ = NULL;

		is -> is_provider = vec[0];
		is -> is_entity = cp;
		is -> is_selectlen = str2sel (vec[1], (mask & (1 << 1)) ? 1 : 0,
									  is -> is_selector, ISSIZE);

		is -> is_vec = vec + 2;
		is -> is_tail = vec + vecp;

		if (strcmp (cp = is -> is_vec[0], "tsapd-bootstrap"))
			(void) strcpy (is -> is_vec[0] = file, isodefile (cp, 1));

		return is;
	}

	return NULL;
}

/*  */

#ifdef	DEBUG
int
_printsrv (struct isoservent *is) {
	int    n = is -> is_tail - is -> is_vec - 1;
	char **ap = is -> is_vec;

	LLOG (addr_log, LLOG_DEBUG,
		  ("\tENT: \"%s\" PRV: \"%s\" SEL: %s",
		   is -> is_entity, is -> is_provider,
		   sel2str (is -> is_selector, is -> is_selectlen, 1)));

	for (; n >= 0; ap++, n--)
		LLOG (addr_log, LLOG_DEBUG,
			  ("\t\t%d: \"%s\"\n", ap - is -> is_vec, *ap));
}
#endif
