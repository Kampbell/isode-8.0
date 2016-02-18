#include <stdio.h>
#include "util.h"
#include "mapphone.h"
#include "quipu/util.h"
#include "extmessages.h"

extern struct mapphonelist * mapphonelp;
static struct mapphonelist * tailplp;

void
addToPhoneList(str)
char *str;
{
	char * cp;

	cp = index(str, ':');
	if (cp == NULLCP) {
		(void) fprintf(stderr, "%s", log_err_pPhone);
		return;
	}
	*cp = '\0';
	cp++;
	addPhoneNode(str, cp);
}

void
addPhoneNode(from, to)
char * from, * to;
{
	struct mapphonelist * mplp;

	mplp = mapphone_alloc();
	if (mapphonelp != NULLPHLIST) {
		tailplp->next = mplp;
		tailplp = mplp;
	} else
		mapphonelp = tailplp = mplp;
	mplp->mapfrom = copy_string(from);
	mplp->mapto = copy_string(to);
	mplp->next = NULLPHLIST;
}

char *
mapPhone(from)
char * from;
{
	struct mapphonelist * mplp;
	static char tophone[LINESIZE];

	for (mplp = mapphonelp; mplp != NULLPHLIST; mplp = mplp->next)
		if (strncmp(from, mplp->mapfrom, strlen(mplp->mapfrom)) == 0) {
			(void) strcpy(tophone, mplp->mapto);
			(void) strcat(tophone, from + strlen(mplp->mapfrom));
			return tophone;
		}
	return from;
}
