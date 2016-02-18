
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
/*#include <curses.h>
#include <termio.h> */
#include "quipu/util.h"
#include "demanifest.h"
#include "bind.h"
#include "namelist.h"
#include "cnamelist.h"
#include "mapatts.h"
#include "mapphone.h"
#include "messages.h"
#include "tailor.h"

static char *myname = "de";

LLog    _de_log = {
	"de.log", NULLCP, NULLCP, LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE,
	LLOG_NONE, -1, LLOGCLS | LLOGCRT | LLOGZER, NOTOK
};
LLog *de_log = &_de_log;


extern char *TidyString();
extern char *findHelp();
void searchFail(), onint1(), onint2(), de_exit();

int boundToDSA = FALSE;
int limitProblem = FALSE;
int browseMess = FALSE;
int notAllReached = FALSE;
int alarmCount = 0;
int abandoned = FALSE;
int searchfail = FALSE;
int byeByeMessage = FALSE;
int controlCtoQuit = TRUE;
int quipuMastersCo = FALSE;
int exactMatch;
int highNumber = 0;
int maxOrgs = 1;
int maxDepts = 1;
int maxPersons = 3;
int no_of_languages = 0;
int localAlarmTime = 15;
int remoteAlarmTime = 30;
int quitChars = 1;
int deLogLevel = 1;
int alarmLeft = 0;
int wanAccess = FALSE;
int rfrl_msg = TRUE;

struct cnamelist * cnamelp = NULLCNLIST;
struct mapnamelist * mapnamelp = NULLMNLIST;
struct mapphonelist * mapphonelp = NULLPHLIST;

struct namelist * coatts    = NULLLIST;
struct namelist * orgatts   = NULLLIST;
struct namelist * ouatts    = NULLLIST;
struct namelist * prratts   = NULLLIST;
struct namelist * rlatts    = NULLLIST;
struct namelist * rmatts    = NULLLIST;
struct namelist * option3lp = NULLLIST;
struct namelist * option4lp = NULLLIST;

/* Defaults for unspecified name parts */
char default_country[LINESIZE], default_organisation[LINESIZE],
	 default_option1[LINESIZE] = "1",
								 default_option2[LINESIZE] = "1",
										 default_option3[LINESIZE] = "1",
												 default_option4[LINESIZE] = "1",
														 default_option5[LINESIZE] = "1",
																 default_department[LINESIZE], default_person[LINESIZE],
																 default_userdn[LINESIZE], have_department[LINESIZE],
																 change_posdit[LINESIZE], change_posdn[LINESIZE],
																 org_compel[LINESIZE], search_mgr[LINESIZE], prrOC[LINESIZE],
																 rlOC[LINESIZE], rmOC[LINESIZE], addorg[LINESIZE],
																 country_mgr[LINESIZE], masterDSA[LINESIZE],
																 init_user[LINESIZE], init_pswd[LINESIZE], addorg_posdit[LINESIZE],
																 just_dn[LINESIZE],
																 option1[LINESIZE], option2[LINESIZE], option3[LINESIZE],
																 option4[LINESIZE], option5[LINESIZE], language[LINESIZE],
																 co[LINESIZE], org[LINESIZE], ou[LINESIZE], person[LINESIZE],
																 origDefaultCo[LINESIZE], origDefaultOrg[LINESIZE],
																 exactString[LINESIZE],
																 welcomeMessage[LINESIZE],
																 callingDteNumber[LINESIZE];
char username[LINESIZE];
char password[LINESIZE];
char posdit[LINESIZE];
char yes_string[LINESIZE];
char no_string[LINESIZE];
int pswd_intrctv = TRUE;
int username_intrctv = TRUE;
jmp_buf sjbuf;
int assoc;
int first_time=TRUE;
int posdit_oc;
int tmp_org=FALSE;
int orgEntered;

int main(argc, argv)
int argc;
char *argv[];
{

	struct namelist * option5lp = NULLLIST;

	SFD cleanupok();

	char * buf;

	int display_posdit();
	int objectType;
	int status;
	int wait_result;

	(void) signal(SIGINT, de_exit);

	/*prratts = (struct namelist *) malloc(sizeof(struct namelist)); */

	(void) printf("\n");

	(void) sprintf(username, "%s", "interactive");
	(void) sprintf(password, "%s", "interactive");
	(void) sprintf(welcomeMessage, "%s", wlcm_msg);

	if (initialisations(argc, argv) != OK) {
		de_exit(-1);
	}

	(void) displayFile("helloMessage", FALSE);

	status = check_credentials();

	if (status == NOTOK) {
		de_exit(-1);
	}

	if (boundToDSA == FALSE) {
		if (username_intrctv == TRUE) {
			(void) printf("%s   %s", reconnect, please_wait);
		} else {
			(void) printf("%s   %s", connect_idm, please_wait);
		}
		(void) fflush(stdout);

		wait_result = wait_bind_to_ds(assoc, TRUE);  /* block */
		if (wait_result == NOTOK) {
			de_exit(-1);
		} else if (wait_result == INV_PSWD) {
			(void) printf(inv_pswd_exit);
			de_exit(-1);
		} else {
			(void) printf(done);
		}
	}

	(void) tailorHelp();

	ll_hdinit(de_log, myname);

	if (strlen(posdit) > 0) {
		if (!(strcmp(change_posdit, yes_string))) {
			(void) printf(curpos_msg);
			display_posdit(strlen(curpos_msg), posdit);
		}
		(void) determine_posdit(&objectType);
		posdit_oc = objectType;
	} else {
		(void) position_dit();
	}

	(void) setjmp(sjbuf);
	(void) signal(SIGINT, onint1);

	for(;;) {
		(void) printf(menutopline);
		(void) printf(menu5line1);
		if (posdit_oc == ORG) {
			(void) printf(menu5line2);
		} else if (posdit_oc == ORGUNIT) {
			(void) printf(menu5line2ou);
		} else {
			(void) printf(menu5line2na);
		}
		(void) printf(menu5line3);
		(void) printf(menu5line4);

		enterString(OPTION5, default_option5, option5lp);

		(void) signal(SIGINT, onint2);

		if (!(strcmp(option5, "1"))) {
			(void) de_Action();
		} else if (!(strcmp(option5, "2"))) {
			(void) dm_ModifyOrg();
		} else if (!(strcmp(option5, "3"))) {
			/* modify password */
			(void) prompt_new_password();
		} else if (!(strcmp(option5, "4"))) {
			/* modify password */
			(void) displayFile("help", FALSE);
			(void) printf(press_CR);
			buf = malloc(LINESIZE);
			if (gets(buf) == NULLCP) {
				free(buf);
				de_exit(-1);
			} else {
				free(buf);
			}
		} else if (!(strcmp(option5, quit_String))) {
			break;
		}
	}

	if (boundToDSA == TRUE) {
#ifdef SPEC_MALL
		stop_malloc_trace();
#endif
		(void) de_unbind();
	}
	de_exit(0);
	return 0;
	/* and that's all for now */
}

int de_Action() {
	struct namelist * option2lp = NULLLIST;

	int return_status;

	for(;;) {
		/*  (void) setjmp(sjbuf);
		  (void) signal(SIGINT, onint1); */
		(void) printf(menutopline);
		(void) printf(menu2line1);
		(void) printf(menu2line2);
		(void) printf(menu2line3);
		(void) printf(menu2line4);
		(void) printf(menu2line5);
		if (!(strcmp(change_posdit, yes_string))) {
			(void) printf(menu2line6);
		}

		if (tmp_org == TRUE) {
			(void) sprintf(default_option2, "%s", "3");
		}

		enterString(OPTION2, default_option2, option2lp);


		if (!(strcmp(option2, "1")))
			return_status = dm_List();
		else if (!(strcmp(option2, "2")))
			return_status = de_Read();
		else if (!(strcmp(option2, "3")))
			return_status = de_Add();
		else if (!(strcmp(option2, "4")))
			return_status = de_Modify();
		else if (!(strcmp(option2, "5")))
			return_status = de_Delete();
		else if (!(strcmp(option2, "6")))
			return_status = position_dit();
		else if (!(strcmp(option2, quit_String)))
			break;

		return_status = OK;

		if (return_status == INV_PSWD) {
			return INV_PSWD;
		}

	}

	return OK;
}

int ask_objectType(string, object_to_work)
char * string;
char * object_to_work;
{

	char * temp1;
	char * temp2;
	char * temp3;
	char * temp4;

	temp1 = malloc(LINESIZE);
	temp2 = malloc(LINESIZE);
	temp3 = malloc(LINESIZE);
	temp4 = malloc(LINESIZE);

	(void) sprintf(temp1, "%s", menutopline);
	(void) sprintf(temp2, "             1  %s %s\n", string, menutypeperson);
	(void) sprintf(temp3, "             2  %s %s\n", string, menutyperole);
	(void) sprintf(temp4, "             3  %s %s\n", string, menutyperoom);
	(void) printf(temp1);
	(void) printf(temp2);
	(void) printf(temp3);
	(void) printf(temp4);

	enterString(OPTION3, default_option3, option3lp);
	(void) sprintf(object_to_work, "%s", option3);
	free(temp1);
	free(temp2);
	free(temp3);
	free(temp4);
}

enterString(objectType,defaultValue, lp)
int objectType;
char * defaultValue;
struct namelist * lp;
{
	char prompt[LINESIZE];
	static char prstr[] = ":";

	switch(objectType) {
	case OPTION1:
		/*      (void) sprintf(prompt, menubotpart); */
		if (strlen(defaultValue) != 0)
			(void) sprintf(prompt, "%s %s%s", menubotpart1, menu1defopt,
						   menubotpart2);
		enterAndValidate(prompt, option1, objectType, defaultValue, lp);
		break;
	case OPTION2:
		if (strlen(defaultValue) != 0) {
			if (tmp_org == FALSE) {
				(void) sprintf(prompt, "%s %s  %s: ", menubotpart1, menubotpart2,
							   menu2defopt);
			} else {
				(void) sprintf(prompt, "%s %s  %s: ", menubotpart1, menubotpart2,
							   menu2defoptneworg);
			}
		}
		enterAndValidate(prompt, option2, objectType, defaultValue, lp);
		break;
	case OPTION3:
		if (strlen(defaultValue) != 0)
			(void) sprintf(prompt, "%s %s%s%s  %s: ", menubotpart1,
						   "or ", quit_String, " to quit to previous menu", menu3defaddopt);
		enterAndValidate(prompt, option3, objectType, defaultValue, lp);
		break;
	case OPTION4:
		if (strlen(defaultValue) != 0)
			(void) sprintf(prompt, "%s %s%s", menubotpart1, menu4defopt,
						   menubotpart2);
		enterAndValidate(prompt, option4, objectType, defaultValue, lp);
		break;
	case OPTION5:
		if (strlen(defaultValue) != 0)
			(void) sprintf(prompt, "%s %s  %s: ", menubotpart1, menubotpart2,
						   menu5defopt);
		enterAndValidate(prompt, option5, objectType, defaultValue, lp);
		break;
	case PERSON:
		(void) sprintf(prompt, "%s * %s %s %s %s : ", person_name,
					   to_list, quit_String, to_quit, to_previous);
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case ENTRY:
		(void) sprintf(prompt, "%s, * %s %s %s %s ", name_entry,
					   to_list, quit_String, to_quit, to_previous);
		if (strlen(default_person) > 0) {
			(void) sprintf(prompt, "%s [%s]", prompt, default_person);
		}
		(void) sprintf(prompt, "%s :\n", prompt);
		objectType = PERSON;   /* treat as person */
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case MODIFY:
		(void) sprintf(prompt, "%s %s, %s %s %s ", name_entry,
					   to_modify, quit_String, to_quit, to_previous);
		if (strlen(default_person) > 0) {
			(void) sprintf(prompt, "%s [%s]", prompt, default_person);
		}
		(void) sprintf(prompt, "%s : ", prompt);
		objectType = PERSON;   /* treat as person */
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case DELETE:
		(void) sprintf(prompt, "%s %s, %s %s %s ", name_entry,
					   to_delete, quit_String, to_quit, to_previous);
		if (strlen(default_person) > 0) {
			(void) sprintf(prompt, "%s [%s]", prompt, default_person);
		}
		(void) sprintf(prompt, "%s : ", prompt);
		objectType = PERSON;   /* treat as person */
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case USER:
		(void) sprintf(prompt, "%s, * %s  : ", your_name, to_list);
		objectType = PERSON; /* treat as a person */
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case USERDN:
		if (strlen(default_userdn) > 0) {
			(void) sprintf(prompt, "[%s] : ", default_userdn);
		} else {
			(void) sprintf(prompt, "%s", ": ");
		}
		objectType = PERSON;   /* treat as person */
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case ROLE:
		(void) sprintf(prompt, "%s, * %s, %s %s %s", role_name,
					   to_list, quit_String, to_quit, to_previous);
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case ROOM:
		(void) sprintf(prompt, "%s, * %s, %s %s %s", room_name,
					   to_list, quit_String, to_quit, to_previous);
		enterAndValidate(prompt, person, objectType, defaultValue, lp);
		break;
	case ORGUNIT:
		(void) sprintf(prompt, "%s, * %s ", opt_dept_name,
					   list_depts);
		if (strlen(default_department) > 0) {
			(void) sprintf(prompt, "%s [%s] ", prompt, default_department);
		}
		(void) sprintf(prompt, "%s %s", prompt, prstr);
		enterAndValidate(prompt, ou, objectType, defaultValue, lp);
		break;
	case ORG:
		(void) printf(org_manage);
		(void) sprintf(prompt, "%s, * %s ", org_name, list_orgs);
		if (strlen(default_organisation) > 0) {
			(void) sprintf(prompt, "%s [%s] ", prompt, default_organisation);
		}
		(void) sprintf(prompt, "%s %s", prompt, prstr);
		enterAndValidate(prompt, org, objectType, defaultValue, lp);
		break;
	case COUNTRY:
		(void) sprintf(prompt, "%s, * %s ", c_name, list_c);
		if (strlen(default_country) > 0) {
			(void) sprintf(prompt, "%s  [%s] ", prompt, default_country);
		}
		(void) sprintf(prompt, "%s %s", prompt, prstr);
		enterAndValidate(prompt, co, objectType, defaultValue, lp);
		break;
	default:
		(void) fprintf(stderr, "%s", unk_type_eS);
		break;
	}
}


enterAndValidate(prompt, buf, objectType, defaultValue, lp)
char * prompt, * buf;
int objectType;
char * defaultValue;
struct namelist * lp;
{
	char * cp, * cp2;
	int i, n, isnum;

	/* this picks up any number that was entered at the pager prompt */
	if ((n = getpnum()) != -1) {
		if ((n > highNumber) || (n < 1)) { /* check number against valid range */
			if (highNumber > 0) {
				if (n <= 0) {
					(void) printf("%d %s = %d)\n\n", n, inv_no_neg, highNumber);
				} else {
					(void) fprintf(stderr, "%d %s = %d)\n\n", n, inv_no_max, highNumber);
				}
			} else {
				(void) fprintf(stderr, no_list_cur);
			}
		} else { /* valid number */
			for (i = 1; i < n; i++, lp = lp->next) {};
			cp = copy_string(lastComponent(lp->name, objectType));
			exactMatch = objectType;
			(void) strcpy(exactString, lp->name);
			(void) strcpy(buf, cp);
			free(cp);
			return;
		}
	}

	for (;;) {
		exactMatch = -1;
		writeInverse(prompt);
		(void) putchar(' ');

		if (gets(buf) == NULLCP) {
			if (objectType == OPTION5) {
				/* exit the program */
				cleanup(0);
				(void) de_exit(-1);
			} else {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			}
		}

		cp = copy_string(TidyString(buf));
		if (strlen(cp) == 0) { /* default accepted */
			free(cp);
			cp = copy_string(defaultValue);
			break;
		}
		/* if "-" entered, convert this to a null entry, unless entering a
		   country.  In this case, "-" is treated as search international orgs */
		/*    if (objectType != COUNTRY)
		      if (strcmp(cp, "-") == 0)
		      {
		        *cp ='\0';
			break;
		      }    *** Removed for dm to allow deletion of dn values */
		if (strcmp(cp, "?") == 0) { /* help on current input requested */
			switch (objectType) {
			case OPTION1:
				displayFile("option1", FALSE);
				break;
			case OPTION2:
				if (!(strcmp(change_posdit, yes_string))) {
					displayFile("option2", FALSE);
				} else {
					displayFile("option2noc", FALSE);
				}
				break;
			case OPTION3:
				displayFile("option3", FALSE);
				break;
			case OPTION4:
				displayFile("option4", FALSE);
				break;
			case OPTION5:
				displayFile("option5", FALSE);
				break;
			case PERSON:
				displayFile("name", FALSE);
				break;
			case ENTRY:
				displayFile("entry", FALSE);
				break;
			case ORGUNIT:
				displayFile("department", FALSE);
				break;
			case ORG:
				displayFile("organisation", FALSE);
				break;
			case COUNTRY:
				displayFile("country", FALSE);
				break;
			}
			continue;
		}
		if (*cp == '?') { /* help on some other specific topic requested */
			for (cp++; *cp == ' '; cp++) {};
			if ((lexequ(cp, "?") == 0) || (lexequ(cp, "help") == 0)) /* help about help requested */
				displayHelp("help");
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
			if (objectType == OPTION1)
				highNumber = 2;
			else if (objectType == OPTION2) {
				if (!(strcmp(change_posdit, yes_string))) {
					highNumber = 6;
				} else {
					highNumber = 5;
				}
			} else if (objectType == OPTION3)
				highNumber = 3;
			else if (objectType == OPTION4)
				highNumber = 4;
			else if (objectType == OPTION5)
				highNumber = 4;
			if ((n > highNumber) || (n < 1)) { /* check number against valid range */
				if (highNumber > 0) {
					if (n <= 0) {
						(void) printf("%d %s = %d)\n\n", n, inv_no_neg, highNumber);
					} else {
						(void) fprintf(stderr, "%d %s = %d)\n\n", n, inv_no_max, highNumber);
					}
				} else if (objectType != OPTION1 && objectType != OPTION2  &&
						   objectType != OPTION3 && objectType != OPTION4  &&
						   objectType != OPTION5) {
					(void) fprintf(stderr, no_list_cur);
				}
				continue;
			} else if (objectType != OPTION1 && objectType != OPTION2 &&
					   objectType != OPTION3 && objectType != OPTION4 &&
					   objectType != OPTION5) {
				for (i = 1; i < n; i++, lp = lp->next) {};
				free(cp);
				cp = copy_string(lastComponent(lp->name, objectType));
				exactMatch = objectType;
				(void) strcpy(exactString, lp->name);
			}
			if (objectType == OPTION1 || objectType == OPTION2 ||
					objectType == OPTION5) {
				(void) strcpy(buf, cp);
				free(cp);
				return;
			}
		}

		if (index(cp, '*') == 0) { /* no wild cards */
			if (isnum == FALSE && (strcmp(cp, quit_String)) &&
					(objectType == OPTION1 || objectType == OPTION2
					 || objectType == OPTION3
					 || objectType == OPTION4
					 || objectType == OPTION5) ) {
				(void) printf("`%s' %s", cp, inv_no_pos);
			}
			break;
		}

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

displayValidWildCards() {
	(void) printf(wildcard1);
	(void) printf(wildcard2);
}

void
onint1() {
	(void) putchar('\n');
	(void) de_exit(-1);
}
void
onint2() {
	(void) putchar('\n');
	/* simulate search failure -
	   this ensures that the "country question" is asked */
	searchfail = TRUE;
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

void
onalarm() {

	(void) signal(SIGALRM, (VFP) onalarm);
	(void) alarm(2);
	switch (alarmCount) {
	case 0:
		(void) printf(some_time);
		break;
	case 1:
		(void) printf(still_try);
		(void) fflush(stdout);
		break;
	default:
		(void) printf(".");
		(void) fflush(stdout);
		break;
	}
	alarmCount++;
}

void
searchFail(str)
char * str;
{
	searchfail = TRUE;
	if (abandoned)
		(void) printf(srch_abandoned);
	else {
		(void) printf(the_search);
		if (strcmp(str, "*") != 0)
			(void) printf("%s '%s' ", for_str, str);
		(void) printf(has_failed);
		(void) printf(srvr_unava);
	}
}

void
de_exit(exitCode)
int exitCode;
{
	void exit();

	/*  if (byeByeMessage == TRUE) */
	pagerOn(NUMBER_NOT_ALLOWED);
	displayFile("byebye", FALSE); /* FALSE means not a help screen */
	exit(exitCode);
}
