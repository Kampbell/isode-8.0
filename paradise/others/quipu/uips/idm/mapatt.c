#include <stdio.h>
#include "util.h"
#include "mapatts.h"
#include "quipu/util.h"
#include "extmessages.h"
extern struct mapnamelist * mapnamelp;
static struct mapnamelist * taillp;

void
addToAttList(str)
char *str;
{
	char * cp;

	cp = index(str, ' ');
	if (cp == NULLCP) {
		(void) fprintf(stderr, "%s", log_err_pAtt);
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
