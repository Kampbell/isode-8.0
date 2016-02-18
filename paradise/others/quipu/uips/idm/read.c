#include "isoaddrs.h"
#include "quipu/util.h"
#include "demanifest.h"
#include "extmessages.h"
#include "namelist.h"
#include "quipu/ds_search.h"
#include "quipu/read.h"
#include "quipu/remove.h"
extern char *oidtable;
extern char *tailfile;
extern char *dsa_address;
extern char *local_dit;
extern int assoc;
extern int boundToDSA;
extern int exactMatch;
extern int highNumber;
extern char default_person[];
extern char person[];
extern char posdit[];
extern char yes_string[];
extern char no_string [];
static struct namelist * plp = NULLLIST;

void searchFail(), de_exit();


int de_Read() {
	char * more;
	char * rdn;
	char name[LINESIZE];

	int noEntries;
	int objectType;
	int status;

	noEntries = 0;

	more = malloc(LINESIZE);
	plp = (struct namelist *) malloc(sizeof(struct namelist));
	rdn = malloc(LINESIZE);

	fillMostRmArg();

	(void) sprintf(more, yes_string);		/* Enter loop */

	highNumber = 0;

	while (!(strcmp(more, yes_string))) {
		(void) sprintf(default_person, "");	/* Avoid default */
		enterString(ENTRY, person, plp);

		if (!(strcmp(person, quit_String))) {
			(void) sprintf(more, no_string);
			if (noEntries > 0) {
				freePRRs(&plp);
			}
			continue;
		}
		if (strlen(person) <= 0) {
			if (strlen(default_person) <= 0) {
				(void) printf("%s %s %s\n", enter_entry_name, quit_String, to_quit);
				continue;
			} else {
				(void) sprintf(person, "%s", default_person);
			}
		}
		(void) printf("\n");

		if (noEntries > 0) {
			freePRRs(&plp);
		}

		if (listPRRs(posdit, person, &plp) != OK) {
			searchFail(person);
			de_exit(-1);
		}

		noEntries = listlen(plp);

		if (noEntries == 0) {
			(void) printf(no_ent_found);
			freePRRs(&plp);
			continue;
		} else if (noEntries == 1) {
			rdn = copy_string(lastComponent(plp->name, PERSON));
			status = get_objectClassPRR(plp, &objectType);
			if (status != OK) {
				(void) printf(no_p_rl_rm);
				continue;
			} else if (objectType == PERSON) {
				if (listPRRs(posdit, rdn, &plp) != OK) {
					searchFail(rdn);
				}
			} else if (objectType == ROLE) {
				if (listPRRRl(posdit, rdn, &plp) != OK) {
					searchFail(rdn);
				}
			} else if (objectType == ROOM) {
				if (listPRRRm(posdit, rdn, &plp) != OK) {
					searchFail(rdn);
				}
			} else {
				(void) printf(no_p_rl_rm);
				continue;
			}
			pagerOn(NUMBER_NOT_ALLOWED);
			printListPRRs(person, plp, COUNTRY, TRUE);
			(void) sprintf(default_person, "%s", rdn);
			highNumber = 0;
		} else {
			/* more than 1, select from list */
			pagerOn(NUMBER_ALLOWED);
			printListPRRs(name, plp, COUNTRY, FALSE);
			continue;
		}
	}

	free(more);
	free(rdn);
	return OK;
}
