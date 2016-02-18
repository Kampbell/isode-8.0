#include <stdio.h>
#include "util.h"
#include "cnamelist.h"
#include "quipu/util.h"
#include "extmessages.h"

extern struct cnamelist * cnamelp;
static struct cnamelist * taillp;

void
addToCoList(str)
char *str;
{
	char * cp;

	cp = index(str, ' ');
	if (cp == NULLCP) {
		(void) fprintf(stderr, "%s", log_err_c_par);
		return;
	}
	*cp = '\0';
	if (strlen(str) != 2) {
		(void) fprintf(stderr, "%s", log_err_c_not_2);
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
