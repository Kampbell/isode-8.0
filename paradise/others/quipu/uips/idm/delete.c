#include "isoaddrs.h"
#include "quipu/util.h"
#include "demanifest.h"
#include "extmessages.h"
#include "namelist.h"
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
static struct ds_removeentry_arg rmarg;
static struct namelist * plp = NULLLIST;

void searchFail(), de_exit();

int de_Delete() {
	char * more;
	char * rdn;
	char * sure;
	char name[LINESIZE];

	int fillMostRmArg();
	int noEntries;
	int objectType;
	int status;

	noEntries = 0;

	more = malloc(LINESIZE);
	plp = (struct namelist *) malloc(sizeof(struct namelist));
	sure = malloc(LINESIZE);

	fillMostRmArg();

	(void) sprintf(default_person, ""); /* Clear default value */
	(void) sprintf(more, yes_string);  /* Enter loop */

	highNumber = 0;

	while (!(strcmp(more, yes_string))) {
		enterString(DELETE, person, plp);

		if (!(strcmp(person, quit_String))) {
			(void) sprintf(more, no_string);
			if (noEntries > 0) {
				freePRRs(&plp);
			}
			continue;
		}
		if (strlen(person) <= 0) {
			(void) printf("%s %s %s", enter_entry_name, quit_String, to_quit);
			continue;
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
			}
			pagerOn(NUMBER_NOT_ALLOWED);
			printListPRRs(person, plp, COUNTRY, TRUE);
			highNumber = 0;
			de_prompt_yesno(sure_delete, sure, no_string);
			if (!(strcmp(sure, yes_string))) {
				/* Delete the entry */
				(void) dm_Delete_entry();
				freePRRs(&plp);
			} else {
				freePRRs(&plp);
			}
		} else {
			/* more than 1, select from list */
			pagerOn(NUMBER_ALLOWED);
			printListPRRs(name, plp, COUNTRY, FALSE);
		}
	}

	free(more);
	free(rdn);
	free(sure);
	return OK;
}

int fillMostRmArg() {
	rmarg.rma_common.ca_servicecontrol.svc_options = SVC_OPT_CHAININGPROHIBIT;
	rmarg.rma_common.ca_servicecontrol.svc_prio = SVC_PRIO_MED;
	rmarg.rma_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	rmarg.rma_common.ca_servicecontrol.svc_sizelimit = SVC_NOSIZELIMIT;
	rmarg.rma_common.ca_servicecontrol.svc_scopeofreferral = SVC_REFSCOPE_COUNTRY;
	rmarg.rma_common.ca_servicecontrol.svc_tmp = NULL;
	rmarg.rma_common.ca_servicecontrol.svc_len = 0;
	rmarg.rma_common.ca_requestor = NULL;
	rmarg.rma_common.ca_progress.op_resolution_phase = OP_PHASE_NOTDEFINED;
	rmarg.rma_common.ca_progress.op_nextrdntoberesolved = 0;
	rmarg.rma_common.ca_aliased_rdns = NULL;
	rmarg.rma_common.ca_security = (struct security_parms *)NULL;
	rmarg.rma_common.ca_sig = (struct signature *) NULL;
	rmarg.rma_common.ca_extensions = (struct extension *) NULL;
}

int dm_Delete_entry() {
	extern int rfrl_msg;
	int status;

	struct DSError error;


	fillMostRmArg();
	rmarg.rma_object = str2dn(plp->name);
	rfrl_msg = TRUE;
	status = REFERRAL;
	(void) printf("%s   %s... ", del_entry, please_wait );
	(void) fflush(stdout);
	while (status == REFERRAL) {
		status = ds_removeentry(&rmarg, &error);
		if (status != DS_OK) {
			rfrl_msg = FALSE;
			status = check_error(error);
			if (status != REFERRAL) {
				(void) printf(delete_failure);
				return OK;
			}
		} else {
			(void) printf(done);
		}
	}
	rfrl_msg = TRUE;
	return OK;
}
