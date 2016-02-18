/* template.c - your comments here */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/de/RCS/de.c,v 9.1 1992/08/25 15:50:26 isode Exp $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/de.c,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: de.c,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:38  isode
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

/*****************************************************************************

  main.c

*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "quipu/ufn.h"
#include "quipu/util.h"
#include "demanifest.h"
#include "destrings.h"
#include "bind.h"
#include "namelist.h"
#include "cnamelist.h"
#include "mapatts.h"
#include "mapphone.h"
#include "tailor.h"
#include "query.h"

LLog    _de_log = {
	"de.log", NULLCP, NULLCP, LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE,
	LLOG_NONE, -1, LLOGCLS | LLOGCRT | LLOGZER, NOTOK
};
LLog *de_log = &_de_log;


extern char *TidyString();
extern char *findHelp();
void onint1(), de_exit();
void foundFollowing(), matchFollowing();
char enterYesNo();

struct query qinfo[MAXTYPE + 1];

int boundToDSA = FALSE;
int limitProblem = FALSE;
int browseMess = FALSE;
int notAllReached = FALSE;
int alarmCount = 0;
int abandoned = FALSE;
int accessrightproblem = FALSE;
int searchfail = FALSE;
int byeByeMessage = FALSE;
int controlCtoQuit = TRUE;
int wanAccess = FALSE;
int deptQ = TRUE;
int fuzzyMatching = TRUE;
int quipuMastersCo = FALSE;
int exactMatch;
int highNumber = 0;
int maxOrgs = 1;
int maxDepts = 1;
int maxPersons = 3;
int localAlarmTime = 15;
int remoteAlarmTime = 30;
int bindTimeout = 120;
int quitChars = 1;
int deLogLevel = 1;
int alarmLeft = 0;

int dots;
int dotsPrinted;


struct cnamelist * cnamelp = NULLCNLIST;
struct mapnamelist * mapnamelp = NULLMNLIST;
struct mapphonelist * mapphonelp = NULLPHLIST;

struct namelist * coatts = NULLLIST;
struct namelist * locatts = NULLLIST;
struct namelist * orgatts = NULLLIST;
struct namelist * ouatts = NULLLIST;
struct namelist * prratts = NULLLIST;

/* Defaults for unspecified name parts */
char origDefaultCo[LINESIZE], origDefaultOrg[LINESIZE],
	 exactString[LINESIZE],
	 welcomeMessage[LINESIZE] = "Welcome to the Directory Service",
								matched[LINESIZE],
								callingDteNumber[LINESIZE];


char * username, * backup_dsa_address, * password;
jmp_buf sjbuf;

SFD cleanupok();

int main(argc, argv)
int argc;
char *argv[];
{
	int res;
	void doUfnSearch();

	/*  pdu_dump_init("/tmp"); */
	if (initialisations(argc, argv) != OK)
		de_exit(-1);

	(void) tailorHelp();

	if (de_bind(FALSE) != OK)
		de_exit(-1);

	ll_hdinit(de_log, "de");

	(void)strcpy(origDefaultCo, qinfo[COUNTRY].defvalue);
	(void)strcpy(origDefaultOrg, qinfo[ORG].defvalue);

	qinfo[COUNTRY].lp = NULLLIST;
	qinfo[LOCALITY].lp = NULLLIST;
	qinfo[ORG].lp = NULLLIST;
	qinfo[ORGUNIT].lp = NULLLIST;
	qinfo[PERSON].lp = NULLLIST;

	welcome();

	/* exit from this loop (and thus from the program) is handled by the
	   enterString routine */
	for (;;) {
		(void) setjmp(sjbuf);
		highNumber = 0;
		abandoned = FALSE;
		accessrightproblem = FALSE;
		if (controlCtoQuit == TRUE)
			(void) signal(SIGINT, cleanupok);
		else
			(void) signal(SIGINT, onint1);

		(void) enterString(PERSON);

		(void) signal(SIGINT, onint1);

		if (index(qinfo[PERSON].entered, ',') != NULLCP) {
			doUfnSearch();
			continue;
		}


		if (boundToDSA == FALSE)
			if (de_bind(FALSE) == NOTOK) /* don`t block */
				de_exit(-1);

		if (deptQ) {
			highNumber = 0;
			(void) enterString(ORGUNIT);

			if (boundToDSA == FALSE)
				if (de_bind(FALSE) == NOTOK) /* don`t block */
					de_exit(-1);
		}
enterorg:
		(void) enterString(ORG);
		highNumber = 0;
		if ((exactMatch != ORG) ||
				((strcmp(qinfo[ORG].entered, qinfo[ORG].defvalue) != 0) &&
				 (exactMatch != ORG)) ||
				(searchfail == TRUE)) {
entercountry:
			(void) enterString(COUNTRY);
		}

		/* look at the input strings and decide what sort of search or list
		   to do */

		/* must have entered a country */
		if (strlen(qinfo[COUNTRY].entered) == 0) {
			if (strlen(qinfo[ORG].entered) == 0) {
				(void) printf("Must enter an organisation and country\n\n");
				goto enterorg;
			} else {
				(void) printf("Must enter a country\n\n");
				goto entercountry;
			}
		}

		if (boundToDSA == FALSE) {
			if (de_bind(TRUE) == NOTOK) /* block until bound */
				de_exit(-1);
#ifdef SPEC_MALL
			start_malloc_trace(0);
#endif
		}

		if (deLogLevel)
			(void) ll_log (de_log, LLOG_NOTICE, NULLCP,
						   "Search: co=%s, org=%s, ou=%s, cn=%s",
						   qinfo[COUNTRY].entered, qinfo[ORG].entered,
						   qinfo[ORGUNIT].entered, qinfo[PERSON].entered);

		searchfail = FALSE;

		pagerOn(NUMBER_ALLOWED);

		res = doCountry();
		switch (res) {
		case QUERY_ERROR:
			goto set_up_defaults;
		case NAME_PRINTED:
		case LIST_PRINTED:
			if (((int)strlen(qinfo[ORG].entered) == 0) &&
					(strlen(qinfo[ORGUNIT].entered) == 0) &&
					(strlen(qinfo[PERSON].entered) == 0)) {
				goto set_up_defaults;
			} else
				break;
		}

		res = doOrganisation(matched, COUNTRY);
		switch (res) {
		case QUERY_ERROR:
		case NO_ORG_ENTERED:
		case NO_ORG_FOUND:
		case NO_LOCALITY:
		case NO_LOCALITY_SELECTED:
			goto set_up_defaults;
		case NAME_PRINTED:
		case LIST_PRINTED:
			if (((int)strlen(qinfo[ORGUNIT].entered) == 0) &&
					(strlen(qinfo[PERSON].entered) == 0)) {
				goto set_up_defaults;
			} else
				break;
		}

		if (strlen(qinfo[ORGUNIT].entered) != 0) {
			res = doOU(matched);
			switch (res) {
			case NAME_PRINTED:
			case NO_DEPT_FOUND:
				if (strlen(qinfo[PERSON].entered) == 0)
					goto set_up_defaults;
				else {
					if (res == NO_DEPT_FOUND) {
						(void) doPRR(matched, ORG);
						goto set_up_defaults;
					} else
						break;
				}
			case PARENT_PRINTED:
				goto set_up_defaults;
			}
			(void) doPRR(matched, ORGUNIT);
		} else { /* ou is null */
			freeOUs(&qinfo[ORGUNIT].lp);
			/* if prev ou value was non-null, force search for people */
			if (strlen(qinfo[ORGUNIT].defvalue) != 0)
				freePRRs(&qinfo[PERSON].lp);
			(void) doPRR(matched, ORG);
		}

set_up_defaults:
		/* set up defaults for next time round */
		/* note that the list default is converted to null */
		/* except for country */
		(void) strcpy(qinfo[COUNTRY].defvalue, qinfo[COUNTRY].entered);

		if (strcmp(qinfo[ORG].entered, "*") == 0)
			(void) strcpy(qinfo[ORG].defvalue, "");
		else
			(void) strcpy(qinfo[ORG].defvalue, qinfo[ORG].entered);

		if (strcmp(qinfo[ORGUNIT].entered, "*") == 0)
			(void) strcpy(qinfo[ORGUNIT].defvalue, "");
		else
			(void) strcpy(qinfo[ORGUNIT].defvalue, qinfo[ORGUNIT].entered);

		if (strcmp(qinfo[PERSON].entered, "*") == 0)
			(void) strcpy(qinfo[PERSON].defvalue, "");
		else
			(void) strcpy(qinfo[PERSON].defvalue, qinfo[PERSON].entered);
	}
	/* and that's all for now */
}

int
doCountry() {
	Attr_Sequence at;

	/* if we already have the values set up, use the cached results */
	if ((strcmp(qinfo[COUNTRY].entered, qinfo[COUNTRY].defvalue) == 0) &&
			(qinfo[COUNTRY].lp != NULLLIST) && (qinfo[COUNTRY].listlen == 1)) {
		printLastComponent(INDENTON, qinfo[COUNTRY].lp->name, COUNTRY, 0);
		(void)strcpy(matched, qinfo[COUNTRY].lp->name);
		return NAME_PRINTED;
	} else {
		freeLocs(&qinfo[LOCALITY].lp);
		freeOrgs(&qinfo[ORG].lp);
		freeOUs(&qinfo[ORGUNIT].lp);
		freePRRs(&qinfo[PERSON].lp);
	}

	for (;;) {
		/* inits */
		freeCos(&qinfo[COUNTRY].lp);
		pagerOn(NUMBER_ALLOWED);
		/*
		    if (strcmp(qinfo[COUNTRY].entered, "*") == 0)
		      (void) strcpy(qinfo[COUNTRY].defvalue, "*");
		    else
		      (void) strcpy(qinfo[COUNTRY].defvalue, qinfo[COUNTRY].entered);
		*/
		if (listCos(qinfo[COUNTRY].entered, &qinfo[COUNTRY].lp) != OK) {
			(void) searchFail(COUNTRY);
			return QUERY_ERROR;
		}
		qinfo[COUNTRY].listlen = listlen(qinfo[COUNTRY].lp);
		if (qinfo[COUNTRY].listlen == 0) {
			if (strcmp(qinfo[COUNTRY].entered, "*") == 0) {
				resetprint("\n Probably a temporary problem with the Directory Service.\n\n");
				return QUERY_ERROR;
			} else {
				resetprint("No countries match `%s'\n\n", qinfo[COUNTRY].entered);
				(void) strcpy(qinfo[COUNTRY].defvalue, "");
				(void) enterString(COUNTRY);
				continue;
			}
		} else if (qinfo[COUNTRY].listlen == 1) {
			/* is the next level mastered by a quipu DSA */
			quipuMastersCo = FALSE;
			for (at = qinfo[COUNTRY].lp->ats; at != NULLATTR; at = at->attr_link) {
				if (strcmp(attr2name(at->attr_type, OIDPART), "masterDSA") == 0) {
					quipuMastersCo = TRUE;
					break;
				}
			}
			(void) strcpy(qinfo[COUNTRY].displayed, qinfo[COUNTRY].lp->name);
			printLastComponent(INDENTON, qinfo[COUNTRY].lp->name, COUNTRY, 0);
			(void)strcpy(matched, qinfo[COUNTRY].lp->name);
			return NAME_PRINTED;
		} else { /* qinfo[COUNTRY].listlen > 1 */
			if (strcmp(qinfo[COUNTRY].entered, "*") == 0)
				if (((int)strlen(qinfo[ORG].entered) != 0) ||
						((strlen(qinfo[ORGUNIT].entered) != 0)) ||
						(strlen(qinfo[PERSON].entered) != 0)) {
					foundFollowing();
					printListCos(qinfo[COUNTRY].entered, qinfo[COUNTRY].lp);
					(void) enterString(COUNTRY);
					continue;
				} else {
					printListCos(qinfo[COUNTRY].entered, qinfo[COUNTRY].lp);
					return LIST_PRINTED;
				}
			else {
				matchFollowing();
				printListCos(qinfo[COUNTRY].entered, qinfo[COUNTRY].lp);
				(void) enterString(COUNTRY);
				continue;
			}
		}
	}
}

int
doOrganisation(matchstring, searchUnder)
char matchstring [];
int searchUnder;
{
	int res;

	if ((strcmp(qinfo[ORG].entered, qinfo[ORG].defvalue) == 0) &&
			(qinfo[ORG].lp != NULLLIST) && (qinfo[ORG].listlen == 1)) {
		if (qinfo[LOCALITY].lp != NULLLIST)
			printLastComponent(INDENTON, qinfo[LOCALITY].lp->name,
							   LOCALITY, 0);
		printLastComponent(INDENTON, qinfo[ORG].lp->name, ORG, 0);
		(void)strcpy(matched, qinfo[ORG].lp->name);
		if (((int)strlen(qinfo[ORGUNIT].entered) == 0) &&
				(strlen(qinfo[PERSON].entered) == 0))
			printDetails(ORG, qinfo[ORG].lp);
		return NAME_PRINTED;
	} else {
		freeOUs(&qinfo[ORGUNIT].lp);
		freePRRs(&qinfo[PERSON].lp);
	}

	for (;;) {
		/* inits */
		freeOrgs(&qinfo[ORG].lp);

		if (strlen(qinfo[ORG].entered) == 0) {
			resetprint("\nNothing to search for as no organisation name entered.  Either enter\n");
			resetprint("an organisation name at the prompt, or press <CR> to start the\n");
			resetprint("query again\n\n");
			(void) enterString(ORG);
			if (strlen(qinfo[ORG].entered) == 0)
				return NO_ORG_ENTERED;
		}

		pagerOn(NUMBER_ALLOWED);

		if (listOrgs(matchstring, qinfo[ORG].entered, &qinfo[ORG].lp) != OK) {
			(void) searchFail(ORG);
			return QUERY_ERROR;
		}
		qinfo[ORG].listlen = listlen(qinfo[ORG].lp);
		if ((qinfo[ORG].listlen > 0) && (searchUnder != LOCALITY)) {
			/* get rid of any locality lists */
			freeLocs(&qinfo[LOCALITY].lp);
		}
		if (qinfo[ORG].listlen == 0) {
			if (strcmp(qinfo[ORG].entered, "*") == 0) {
				resetprint("No organisations found. Probably a temporary Directory Service error.\n\n");
				return QUERY_ERROR;
			} else {
				/* lets see if the organisation is under a locality -
				but lets make sure first we haven't already checked!! */
				if (searchUnder == LOCALITY) {
					resetprint("Couldn't find an organisation matching `%s'\n\n",
							   qinfo[ORG].entered);
					/* this isn't really consistent with the standard DE behaviour,
					   which would normally return to the org prompt. However, after
					   an excursion into localities, it is not obvious what to type
					   at this prompt - so back to the person prompt */
					return NO_ORG_FOUND;
				} else {
					/* lets see if the organisation is under a locality */
					res = searchOrgUnderLocality(matchstring);
					/* restore matched string to just the country component
					   unless an org has been found */
					if (res != NAME_PRINTED)
						(void)strcpy(matched, qinfo[COUNTRY].lp->name);
					/* see comments above on where to return after searching failure */
					/*
					if (res == NO_ORG_FOUND)
					{
					  (void) enterString(ORG);
					  continue;
					}
					*/
					return res;
				}
			}
		} else if (qinfo[ORG].listlen == 1) {
			if (testRedisplay())
				printNames(LOCALITY);
			printLastComponent(INDENTON, qinfo[ORG].lp->name, ORG, 0);
			if (((int)strlen(qinfo[ORGUNIT].entered) == 0) &&
					(strlen(qinfo[PERSON].entered) == 0))
				printDetails(ORG, qinfo[ORG].lp);
			(void)strcpy(matched, qinfo[ORG].lp->name);
			return NAME_PRINTED;
		} else { /* qinfo[ORG].listlen > 1 */
			if (strcmp(qinfo[ORG].entered, "*") == 0)
				foundFollowing();
			else
				matchFollowing();
			printNames(LOCALITY);
			printListOrgs(qinfo[ORG].entered, qinfo[ORG].lp);
			(void) enterString(ORG);
			continue;
		}
	}
}

int
searchOrgUnderLocality(costr)
char * costr;
{
	int res, noLocs;

	resetprint("No organisations match `%s'\n\n", qinfo[ORG].entered);
	resetprint("Checking to see if entry might be under a locality\n");
	if (listLocs(costr, "*", &qinfo[LOCALITY].lp) != OK) {
		(void) searchFail(LOCALITY);
		return QUERY_ERROR;
	}
	noLocs = qinfo[LOCALITY].listlen = listlen(qinfo[LOCALITY].lp);
	saveLocs(); /* not a very good way of doing this */
	if (qinfo[LOCALITY].listlen == 0) {
		resetprint("No locality information available\n");
		return NO_LOCALITY;
	}
	for(;;) /* escape by finding an organisation, or the user abandoning
             the search */
	{
		if (qinfo[LOCALITY].listlen == 1) {
			resetprint("Only one locality found - `%s'- having a look there ...\n",
					   lastComponent(qinfo[LOCALITY].lp->name, LOCALITY));
		} else {
			/* allow user to choose a locality to search */
			foundFollowing();
			printNames(COUNTRY);
			printListLocs("*", qinfo[LOCALITY].lp);
			(void) enterString(LOCALITY);
			if (strcmp(qinfo[LOCALITY].entered, "") == 0) {
				return NO_LOCALITY_SELECTED;
			}
			if (strcmp(qinfo[LOCALITY].entered, "*") == 0)
				continue;
			freeLocs(&qinfo[LOCALITY].lp);
			if (listLocs(costr, qinfo[LOCALITY].entered, &qinfo[LOCALITY].lp) != OK) {
				(void) searchFail(LOCALITY);
				return QUERY_ERROR;
			}
		}
		(void)strcpy(matched, qinfo[LOCALITY].lp->name);
		/* now lets do org search */
		setRedisplay();
		res = doOrganisation(matched, LOCALITY);
		if ((res == NO_ORG_FOUND) && (noLocs > 1)) {
			if (enterYesNo("Do you want to search another locality (y/n)? ")
					== 'y') {
				restoreSavedLocs();
				continue;
			}
		}
		freeSavedLocs();
		return res;
	}
}

int
doOU(matchstring)
char matchstring [];
{

	if ((strcmp(qinfo[ORGUNIT].entered, qinfo[ORGUNIT].defvalue) == 0) &&
			(qinfo[ORGUNIT].lp != NULLLIST) && (qinfo[ORGUNIT].listlen == 1)) {
		printLastComponent(INDENTON, qinfo[ORGUNIT].lp->name, ORGUNIT, 0);
		if (strlen(qinfo[PERSON].entered) == 0)
			printDetails(ORGUNIT, qinfo[ORGUNIT].lp);
		(void)strcpy(matched, qinfo[ORGUNIT].lp->name);
		return NAME_PRINTED;
	} else
		freePRRs(&qinfo[PERSON].lp);

	for (;;) {
		/* inits */
		freeOUs(&qinfo[ORGUNIT].lp);
		pagerOn(NUMBER_ALLOWED);
		/*
		    if (strcmp(qinfo[ORGUNIT].entered, "*") == 0)
		      (void) strcpy(qinfo[ORGUNIT].defvalue, "");
		    else
		      (void) strcpy(qinfo[ORGUNIT].defvalue, qinfo[ORGUNIT].entered);
		*/
		if (listOUs(matchstring, qinfo[ORGUNIT].entered, &qinfo[ORGUNIT].lp) != OK) {
			if (searchFail(ORGUNIT) != SF_ABANDONED) {
				/* initialise value to discard default which failed */
				(void) strcpy(qinfo[ORGUNIT].entered, "");
			}
			return PARENT_PRINTED;
		}
		qinfo[ORGUNIT].listlen = listlen(qinfo[ORGUNIT].lp);
		if (qinfo[ORGUNIT].listlen == 0) {
			if (strcmp(qinfo[ORGUNIT].entered, "*") == 0)
				if (strlen(qinfo[PERSON].entered) != 0)
					return NO_DEPT_FOUND;
				else
					resetprint("\n      Can't find any departments.");
			else {
				/*        (void) strcpy(qinfo[ORGUNIT].defvalue, ""); */
				resetprint("\n      No departments match `%s'.\n",
						   qinfo[ORGUNIT].entered);
				(void) strcpy(qinfo[ORGUNIT].entered, "");
			}
			if (strlen(qinfo[PERSON].entered) == 0) {
				resetprint("      Displaying organisation details.\n\n");
				printNames(ORG);
				printDetails(ORG, qinfo[ORG].lp);
				return PARENT_PRINTED;
			} else {
				resetprint ("      Continuing to search for person called `%s'\n",
							qinfo[PERSON].entered);
				resetprint ("      within the whole organisation.\n\n");
				return NO_DEPT_FOUND;
			}
		} else if (qinfo[ORGUNIT].listlen == 1) {
			if (testRedisplay())
				printNames(ORG);
			printLastComponent(INDENTON, qinfo[ORGUNIT].lp->name, ORGUNIT, 0);
			if (strlen(qinfo[PERSON].entered) == 0)
				printDetails(ORGUNIT, qinfo[ORGUNIT].lp);
			(void)strcpy(matched, qinfo[ORGUNIT].lp->name);
			return NAME_PRINTED;
		} else { /* qinfo[ORGUNIT].listlen > 1 */
			if (strcmp(qinfo[ORGUNIT].entered, "*") == 0) {
				foundFollowing();
				printNames(ORG);
				printListOUs(qinfo[ORGUNIT].entered, qinfo[ORGUNIT].lp);
				(void) enterString(ORGUNIT);
				continue;
			} else {
				matchFollowing();
				printNames(ORG);
				printListOUs(qinfo[ORGUNIT].entered, qinfo[ORGUNIT].lp);
				(void) enterString(ORGUNIT);
				if (strlen(qinfo[ORGUNIT].entered) == 0)
					return NO_DEPT_FOUND;
				continue;
			}
		}
	}
}

int
doPRR(matchstring, searchparent)
char matchstring [];
int searchparent;
{

	if ((strcmp(qinfo[PERSON].entered, qinfo[PERSON].defvalue) == 0) &&
			(qinfo[PERSON].lp != NULLLIST) && (qinfo[PERSON].listlen == 1)) {
		printListPRRs(qinfo[PERSON].entered, qinfo[PERSON].lp, searchparent, TRUE);
		return NAME_PRINTED;
	}

	for (;;) {
		/* inits */
		freePRRs(&qinfo[PERSON].lp);
		pagerOn(NUMBER_ALLOWED);

		if (listPRRs(matchstring, qinfo[PERSON].entered, &qinfo[PERSON].lp) != OK) {
			if (searchFail(PERSON) != SF_ABANDONED)
				(void) strcpy(qinfo[PERSON].entered, "");
			return PARENT_PRINTED;
		}
		qinfo[PERSON].listlen = listlen(qinfo[PERSON].lp);
		if (qinfo[PERSON].listlen == 0) {
			if (strcmp(qinfo[PERSON].entered, "*") == 0)
				resetprint("      No entries for people found.  ");
			else
				resetprint("      No persons match `%s'.  ", qinfo[PERSON].entered);
			if (qinfo[ORGUNIT].lp != NULLLIST) {
				resetprint("Printing departmental details.\n\n");
				printNames(ORGUNIT);
				printDetails(ORGUNIT, qinfo[ORGUNIT].lp);
			} else {
				resetprint("Printing organisation details.\n\n");
				printNames(ORGUNIT);
				printDetails(ORG, qinfo[ORG].lp);
			}
			return PARENT_PRINTED;
		} else if (qinfo[PERSON].listlen == 1) {
			if (testRedisplay())
				printNames(ORGUNIT);
			printListPRRs(qinfo[PERSON].entered, qinfo[PERSON].lp, searchparent, TRUE);
			return NAME_PRINTED;
		} else { /* qinfo[PERSON].listlen > 1 */
			if (qinfo[PERSON].listlen > maxPersons) {
				if (strcmp(qinfo[PERSON].entered, "*") == 0)
					foundFollowing();
				else
					matchFollowing();
				printNames(ORGUNIT);
				printListPRRs(qinfo[PERSON].entered, qinfo[PERSON].lp, searchparent, FALSE);
				/* If number entered, continue trying to resolve the query.
				Otherwise start a fresh query */
				if (strcmp(qinfo[PERSON].entered, "*") == 0)
					(void) strcpy(qinfo[PERSON].defvalue, "");
				else
					(void) strcpy(qinfo[PERSON].defvalue, qinfo[PERSON].entered);
				if (enterString(PERSON) == 0)
					break;
				else
					continue;
			} else {
				if (testRedisplay())
					printNames(ORGUNIT);
				printListPRRs(qinfo[PERSON].entered, qinfo[PERSON].lp, searchparent, TRUE);
			}
			return NAME_PRINTED;
		}
	}
	return START_NEW_QUERY;
}


printNames(objectType)
int objectType;
{
	switch (objectType) {
	case COUNTRY:
		printLastComponent(INDENTON, qinfo[COUNTRY].lp->name,
						   COUNTRY, 0);
		break;
	case LOCALITY:
		printLastComponent(INDENTON, qinfo[COUNTRY].lp->name,
						   COUNTRY, 0);
		if (qinfo[LOCALITY].lp != NULLLIST)
			printLastComponent(INDENTON, qinfo[LOCALITY].lp->name,
							   LOCALITY, 0);
		break;
	case ORG:
		printLastComponent(INDENTON, qinfo[COUNTRY].lp->name,
						   COUNTRY, 0);
		if (qinfo[LOCALITY].lp != NULLLIST)
			printLastComponent(INDENTON, qinfo[LOCALITY].lp->name,
							   LOCALITY, 0);
		printLastComponent(INDENTON, qinfo[ORG].lp->name,
						   ORG, 0);
		break;
	case ORGUNIT:
		printLastComponent(INDENTON, qinfo[COUNTRY].lp->name,
						   COUNTRY, 0);
		if (qinfo[LOCALITY].lp != NULLLIST)
			printLastComponent(INDENTON, qinfo[LOCALITY].lp->name,
							   LOCALITY, 0);
		printLastComponent(INDENTON, qinfo[ORG].lp->name,
						   ORG, 0);
		if (qinfo[ORGUNIT].lp != NULLLIST)
			printLastComponent(INDENTON, qinfo[ORGUNIT].lp->name,
							   ORGUNIT, 0);
		break;
	case PERSON:
		break;
	}
}

printCountry() {
	printLastComponent(INDENTON, qinfo[COUNTRY].lp->name, COUNTRY, 0);
}

void
foundFollowing() {
	resetprint("\nFound the following entries.  Please select one from the list\n");
	resetprint("by typing the number corresponding to the entry you want.\n\n");
}

void
matchFollowing() {
	resetprint("\nGot the following approximate matches.  Please select one from the list\n");
	resetprint("by typing the number corresponding to the entry you want.\n\n");
}



/* routine returns the number of an entry selected from a list, or zero
   otherwise */
int
enterString(objectType)
int objectType;
{
	char prompt[LINESIZE];
	static char prstr[] = ":-";
	static char gotValue[LINESIZE] = "";
	int numEnt;

	setRedisplay();
	if (boundToDSA == TRUE)
		startUnbindTimer();
	switch(objectType) {
	case PERSON:
		if (strlen(gotValue) != 0) {
			(void) strcpy(qinfo[objectType].entered, gotValue);
			gotValue[0] = '\0';
			break;
		}
		(void) sprintf(prompt, "\nPerson's name, q to quit, ");
		if (strlen(qinfo[objectType].defvalue) != 0)
			(void) sprintf(prompt, "%s<CR> for `%s', ", prompt,
						   qinfo[objectType].defvalue);
		(void) sprintf(prompt, "%s* to %s, ? for help\n%s", prompt,
					   browseMess ? "browse" : "list people", prstr);
		enterAndValidate(prompt, qinfo[objectType].entered, objectType,
						 qinfo[objectType].defvalue, qinfo[objectType].lp, &numEnt);
		/* if a string was entered, but a number could validly have been
		entered, store value in gotValue for next invocation of this function */
		if ((highNumber > 0) && (numEnt == 0))
			(void) strcpy(gotValue, qinfo[objectType].entered);
		break;
	case ORGUNIT:
		(void) sprintf(prompt, "Department name, * to %s, ",
					   browseMess ? "browse" : "list depts");
		if (strcmp(qinfo[objectType].defvalue, "*") == 0)
			(void) sprintf(prompt, "%s<CR> to %s all depts, ", prompt,
						   browseMess ? "browse" : "list");
		else {
			if (strlen(qinfo[objectType].defvalue) == 0) {
				if (strlen(qinfo[PERSON].entered) != 0)
					(void) sprintf(prompt, "%s<CR> to search all depts, ", prompt);
			} else {
				(void) strcat(prompt, "- to search all departments,\n           ");
				(void) sprintf(prompt, "%s<CR> to search for `%s', ",
							   prompt, qinfo[objectType].defvalue);
			}
		}
		(void) sprintf(prompt, "%s? for help\n%s", prompt, prstr);
		enterAndValidate(prompt, qinfo[objectType].entered, objectType,
						 qinfo[objectType].defvalue, qinfo[objectType].lp, &numEnt);
		break;
	case ORG:
		(void) sprintf(prompt, "Organisation name, ");
		if (strcmp(qinfo[objectType].defvalue, "") != 0) {
			(void) sprintf(prompt, "%s<CR> to search `%s', ", prompt,
						   qinfo[objectType].defvalue);
			if ((int)strlen(qinfo[objectType].defvalue) > 10)
				(void) sprintf(prompt, "%s\n           ", prompt);
		}
		(void) sprintf(prompt, "%s* to %s, ", prompt,
					   browseMess ? "browse" : "list orgs");
		(void) sprintf(prompt, "%s? for help\n%s", prompt, prstr);
		enterAndValidate(prompt, qinfo[objectType].entered, objectType,
						 qinfo[objectType].defvalue, qinfo[objectType].lp, &numEnt);
		break;
	case LOCALITY:
		(void) sprintf(prompt, "Locality name, ");
		if (strcmp(qinfo[objectType].defvalue, "") != 0) {
			(void) sprintf(prompt, "%s<CR> to search `%s', ", prompt,
						   qinfo[objectType].defvalue);
			if ((int)strlen(qinfo[objectType].defvalue) > 10)
				(void) sprintf(prompt, "%s\n           ", prompt);
		}
		(void) sprintf(prompt, "%s* to %s, ", prompt,
					   browseMess ? "browse" : "list localities");
		(void) sprintf(prompt, "%s? for help\n%s", prompt, prstr);
		enterAndValidate(prompt, qinfo[objectType].entered, objectType,
						 qinfo[objectType].defvalue, qinfo[objectType].lp, &numEnt);
		break;
	case COUNTRY:
		(void) sprintf(prompt, "Country name, ");
		if ((strcmp(qinfo[objectType].defvalue, "") != 0) &&
				(strcmp(qinfo[objectType].defvalue, "*") != 0))
			(void) sprintf(prompt, "%s<CR> to search `%s', ", prompt,
						   qinfo[objectType].defvalue);
		(void) sprintf(prompt, "%s* to %s, ", prompt,
					   browseMess ? "browse" : "list countries");
		(void) sprintf(prompt, "%s? for help\n%s", prompt, prstr);
		enterAndValidate(prompt, qinfo[objectType].entered, objectType,
						 qinfo[objectType].defvalue, qinfo[objectType].lp, &numEnt);
		break;
	default:
		(void) fprintf(stderr, "Unknown type in enterString\n");
		break;
	}
	stopUnbindTimer();
	if (lexnequ(qinfo[objectType].entered, "quit",
				strlen(qinfo[objectType].entered)) == 0) {
		if (objectType == PERSON)
			cleanupok();
		if (enterYesNo("Do you want to quit the Directory Service (y/n) ") == 'y')
			cleanupok();
		else
			longjmp(sjbuf, 0);
	}
	return numEnt;
}


enterAndValidate(prompt, buf, objectType, defaultValue, lp, nep)
char * prompt, * buf;
int objectType;
char * defaultValue;
struct namelist * lp;
int * nep;
{
	char * cp, * cp2;
	int i, n, isnum;

	*nep = 0;
	/* this picks up any number that was entered at the pager prompt */
	if ((n = getpnum()) != -1) {
		if ((n > highNumber) || (n < 1)) { /* check number against valid range */
			if (highNumber > 0)
				(void) fprintf(stderr, "Invalid number entered (maximum = %d)\n\n",
							   highNumber);
			else
				(void) fprintf(stderr,
							   "No list of entries current.  Entry of a number invalid\n");
		} else { /* valid number */
			for (i = 1; i < n; i++, lp = lp->next) {};
			cp = copy_string(lastComponent(lp->name, objectType));
			exactMatch = objectType;
			(void) strcpy(exactString, lp->name);
			(void) strcpy(buf, cp);
			free(cp);
			*nep = n;
			return;
		}
	}

	for (;;) {
		exactMatch = -1;
		writeInverse(prompt);
		(void) putchar(' ');
		if (gets(buf) == NULLCP) /* deal with control-D */
			if (objectType == PERSON)
				/* exit the program */
				cleanup(0);
			else {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			}
		cp = copy_string(TidyString(buf));
		if (strlen(cp) == 0) { /* default accepted */
			free(cp);
			cp = copy_string(defaultValue);
			break;
		}
		/* if "-" entered, convert this to a null entry, unless entering a
		   country.  In this case, "-" is treated as search international orgs */
		if (objectType != COUNTRY)
			if (strcmp(cp, "-") == 0) {
				*cp ='\0';
				break;
			}
		if (strcmp(cp, "?") == 0) { /* help on current input requested */
			switch (objectType) {
			case PERSON:
				displayHelp(findHelp("name"));
				break;
			case ORGUNIT:
				displayHelp(findHelp("department"));
				break;
			case ORG:
				displayHelp(findHelp("organisation"));
				break;
			case LOCALITY:
				displayHelp(findHelp("locality"));
				break;
			case COUNTRY:
				displayHelp(findHelp("country"));
				break;
			}
			continue;
		}
		if (*cp == '?') { /* help on some other specific topic requested */
			for (cp++; *cp == ' '; cp++) {};
			if ((lexequ(cp, "?") == 0) ||
					(lexequ(cp, "help") == 0)) /* help about help requested */
				displayHelp(findHelp("help"));
			else
				displayHelp(findHelp(cp));
			continue;
		}

		/* if a number has been entered, check that it is in range, and
		   map the number onto the appropriate name */
		isnum = TRUE;
		for (cp2 = cp; *cp2 != '\0'; cp2++) {
			if (! isdigit(*cp2)) {
				isnum = FALSE;
				break;
			}
		}
		if (isnum) {
			n = atoi(cp);
			if ((n > highNumber) || (n < 1)) { /* check number against valid range */
				if (highNumber > 0)
					(void) fprintf(stderr, "Invalid number entered (maximum = %d)\n\n",
								   highNumber);
				else
					(void) fprintf(stderr,
								   "No list of entries current.  Entry of a number invalid\n");
				continue;
			} else {
				for (i = 1; i < n; i++, lp = lp->next) {};
				free(cp);
				cp = copy_string(lastComponent(lp->name, objectType));
				exactMatch = objectType;
				(void) strcpy(exactString, lp->name);
				*nep = n;
			}
		}

		if (index(cp, '*') == 0) /* no wild cards */
			break;

		if (*cp == '*') {
			if (strlen(cp) == 1)
				break;
			cp2 = index(cp + 1, '*');
			if (cp2 == NULLCP)
				break;
			if (cp2 == cp + 1) { /* i.e. string is ** */
				displayValidWildCards();
				free(cp);
				continue;
			}
			if (*(cp2 + 1) != '\0') {
				displayValidWildCards();
				free(cp);
				continue;
			}
			break;
		}
		/* string has at least one asterisk - make sure it's only one */
		if (index(cp, '*') == rindex(cp, '*'))
			break;
		else {
			displayValidWildCards();
			free(cp);
			continue;
		}
	}
	(void) strcpy(buf, cp);
	free(cp);
}

char enterYesNo(str)
char * str;
{
	char buf[LINESIZE];
	int i;

	for (;;) {
		(void) printf("%s", str);
		if (gets(buf) == NULLCP) /* control-D */
			return 'y';
		for (i = 0; buf[i] != '\0'; i++)
			buf[i] = uptolow(buf[i]);
		if ((buf[0] == 'y') || (buf[0] == 'n'))
			return buf[0];
		else
			continue;
	}
}

displayValidWildCards() {
	(void) printf("The following wild-card formats are acceptable:\n");
	(void) printf("\t*\n\txxx*\n\t*xxx*\n\t*xxx\n\txx*xx\n\n");
}

countryCodeMessage(str) {
	(void) printf("<%s> is not a valid two-letter country code.\n", str);
	(void) printf("Either enter a valid two-letter code, or enter the country name more fully.\n\n", str);
}

void
onint1() {
	(void) putchar('\n');
	/* simulate search failure -
	   this ensures that the "country question" is asked */
	searchfail = TRUE;
	/* destroy default values - a bit of a hack
	   but it is difficult to know what the defaults should be when a
	   query is interrupted */
	(void) strcpy(qinfo[COUNTRY].defvalue, "");
	(void) strcpy(qinfo[LOCALITY].defvalue, "");
	(void) strcpy(qinfo[ORG].defvalue, "");
	(void) strcpy(qinfo[ORGUNIT].defvalue, "");
	(void) strcpy(qinfo[PERSON].defvalue, "");
	longjmp(sjbuf, 0);
}

SFD cleanupok() {
	cleanup(0);
}

int
cleanup(exitCode)
int exitCode;
{
	if (boundToDSA == TRUE)
		(void) de_unbind();
#ifdef SPEC_MALL
	stop_malloc_trace();
#endif
	de_exit(exitCode);
}

/* the flushes need dealing with properly */
void
onalarm() {

	(void) signal(SIGALRM, (VFP) onalarm);
	(void) alarm(2);
	if (dotsPrinted == 0)
		(void)printf("%s", SEARCHMESS);
	if (dotsPrinted == dots)
		resetprint("\nThis operation is taking some time.\nControl-C, if you wish to abandon the operation.\n\n");
	else if (dotsPrinted == (dots + 1)) {
		resetprint("Still trying ...");
		(void) fflush(stdout);
	} else if (dotsPrinted < dots) {
		(void)printf(".");
		(void) fflush(stdout);
	} else {
		resetprint(".");
		(void) fflush(stdout);
	}
	dotsPrinted++;
}

searchFail(objectType)
int objectType;
{
	int problem;
	searchfail = TRUE;
	if (abandoned) {
		resetprint("\nSearch abandoned\n");
		return SF_ABANDONED;
	}
	if (accessrightproblem) {
		problem = SF_ACCRIGHT;
		resetprint("\nSearching this part of the Directory is restricted by the managers\n");
		resetprint("of the data.\n\n");
	} else {
		problem = SF_OPFAIL;
		resetprint("\nThe search ");
		if (strcmp(qinfo[objectType].entered, "*") != 0)
			resetprint("for '%s' ", qinfo[objectType].entered);
		resetprint("has failed, probably because a Directory \n");
		resetprint("server is unavailable.  ");
		if (objectType == ORGUNIT) {
			resetprint("In the meantime, displaying organisation details.\n");
			resetprint("For information on people or departments,");
			resetprint(" try again a little later.\n\n");
		} else if (objectType == PERSON) {
			if (qinfo[ORGUNIT].lp == NULLLIST)
				resetprint("In the meantime, displaying organisation details.\n");
			else
				resetprint("In the meantime, displaying department details.\n");
			resetprint("For information on people,");
			resetprint(" try again a little later.\n\n");
		}
	}
	if (objectType == ORGUNIT) {
		printNames(ORG);
		printDetails(ORG, qinfo[ORG].lp);
		return problem;
	}
	if (objectType == PERSON) {
		if (qinfo[ORGUNIT].lp != NULLLIST) {
			printNames(ORGUNIT);
			printDetails(ORGUNIT, qinfo[ORGUNIT].lp);
		} else {
			printNames(ORG);
			printDetails(ORG, qinfo[ORG].lp);
		}
		return problem;
	}
	return problem;
}

void
de_exit(exitCode)
int exitCode;
{
	void exit();

	if (byeByeMessage == TRUE)
		displayFile("byebye", FALSE); /* FALSE means not a help screen */
	exit(exitCode);
}
