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

void searchFail(), onint1(), de_exit();

int dm_List() {

	extern int discardInput;		/* defined in pageprint */

	char * buf;
	char * rdn;

	int noEntries;
	int objectType;
	int status;

	struct namelist * plp2 = NULLLIST;

	noEntries  = 0;
	highNumber = 0;


	plp = (struct namelist *) malloc(sizeof(struct namelist));
	plp = NULLLIST;
	plp2 = (struct namelist *) malloc(sizeof(struct namelist));
	plp2 = NULLLIST;

	rdn = malloc(LINESIZE);

	(void) sprintf(default_person, "");	/* Avoid default */

	(void) printf("\n");

	(void) sprintf(person, "%s", "*");

	if (listPRRs(posdit, person, &plp) != OK) {
		searchFail(person);
		de_exit(-1);
	}

	noEntries = listlen(plp);

	if (noEntries == 0) {
		(void) printf(no_ent_found);
		freePRRs(&plp);
		return OK;
	} else {
		pagerOn(NUMBER_NOT_ALLOWED);
next_entry:
		/* get rdn of first element */
		rdn = copy_string(lastComponent(plp->name, PERSON));
		status = get_objectClassPRR(plp, &objectType);
		if (status != OK) {
			(void) printf(no_p_rl_rm);
			goto skip_entry;
		} else if (objectType == PERSON) {
			if (listPRRs(posdit, rdn, &plp2) != OK) {
				searchFail(rdn);
			}
		} else if (objectType == ROLE) {
			if (listPRRRl(posdit, rdn, &plp2) != OK) {
				searchFail(rdn);
			}
		} else if (objectType == ROOM) {
			if (listPRRRm(posdit, rdn, &plp2) != OK) {
				searchFail(rdn);
			}
		} else {
			(void) printf(no_p_rl_rm);
		}
		printListPRRs(person, plp2, COUNTRY, TRUE);
		freePRRs(&plp2);
		highNumber = 0;
		if (discardInput == TRUE) {
			goto exit_list;
		}
skip_entry:
		if (plp->next != NULL) {
			plp = plp->next;
			goto next_entry;
		}
		/* continue in loop */
	}

	(void) printf(press_CR);
	buf = malloc(LINESIZE);
	if (gets(buf) == NULLCP) {
		free(buf);
		clearerr(stdin);
		onint1();
	}
exit_list:
	freePRRs(&plp);
	free(buf);

	return OK;
}
