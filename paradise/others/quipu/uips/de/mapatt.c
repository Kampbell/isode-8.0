/* template.c - your comments here */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/de/RCS/mapatt.c,v 9.1 1992/08/25 15:50:26 isode Exp $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/mapatt.c,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: mapatt.c,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:44  isode
 * Release 7.0
 *
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


#include <stdio.h>
#include "util.h"
#include "mapatts.h"
#include "quipu/util.h"

extern struct mapnamelist * mapnamelp;
static struct mapnamelist * taillp;

void
addToAttList(str)
char *str;
{
	char * cp;

	cp = index(str, ' ');
	if (cp == NULLCP) {
		(void) fprintf(stderr, "log an error message about parsing of attribute name mappings...\n");
		return;
	}
	*cp = '\0';
	cp++;
	addAttNode(str, cp);
}

void
addAttNode(tablename, nicename)
char * tablename, * nicename;
{
	struct mapnamelist * mnlp;

	mnlp = mapname_alloc();
	if (mapnamelp != NULLMNLIST) {
		taillp->next = mnlp;
		taillp = mnlp;
	} else
		mapnamelp = taillp = mnlp;
	mnlp->tablename = copy_string(tablename);
	mnlp->nicename = copy_string(nicename);
	mnlp->next = NULLMNLIST;
}

char *
mapAttName(tablename)
char * tablename;
{
	struct mapnamelist * mnlp;

	for (mnlp = mapnamelp; mnlp != NULLMNLIST; mnlp = mnlp->next)
		if (strcmp(tablename, mnlp->tablename) == 0)
			return mnlp->nicename;
	return tablename;
}
