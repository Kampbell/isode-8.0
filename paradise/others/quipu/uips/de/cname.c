/* template.c - your comments here */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/de/RCS/cname.c,v 9.1 1992/08/25 15:50:26 isode Exp $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/cname.c,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: cname.c,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:36  isode
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
#include "cnamelist.h"
#include "quipu/util.h"

extern struct cnamelist * cnamelp;
static struct cnamelist * taillp;

void
addToCoList(str)
char *str;
{
	char * cp;

	cp = index(str, ' ');
	if (cp == NULLCP) {
		(void) fprintf(stderr, "log an error message about parsing of country name mappings...\n");
		return;
	}
	*cp = '\0';
	if (strlen(str) != 2) {
		(void) fprintf(stderr, "log an error message about short country name not 2 chars\n");
		return;
	}
	cp++;
	addCoNode(str, cp);
}

void
addCoNode(shortname, longname)
char * shortname, * longname;
{
	struct cnamelist * colp;

	colp = cname_alloc();
	if (cnamelp != NULLCNLIST) {
		taillp->next = colp;
		taillp = colp;
	} else
		cnamelp = taillp = colp;
	colp->shortname = copy_string(shortname);
	colp->longname = copy_string(longname);
	colp->next = NULLCNLIST;
}

char *
mapCoName(shortname)
char * shortname;
{
	struct cnamelist * cnlp;

	for (cnlp = cnamelp; cnlp != NULLCNLIST; cnlp = cnlp->next)
		if (strcmp(shortname, cnlp->shortname) == 0)
			return cnlp->longname;
	return shortname;
}
