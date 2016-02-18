/****************************************************************************

  init.c - Do some initialising
           Read tailor files
           Eventually do argument parsing

*****************************************************************************/

/* #include <stdlib.h> */
#include "types.h"
#include "util.h"
#include "quipu/util.h"
#include "tailor.h"
#include "logger.h"
#include "namelist.h"
#include "demanifest.h"
#include "destrings.h"
#include "extmessages.h"
extern int print_parse_errors;          /* Quipu variable to disable
                                           printing of errors */

extern LLog * de_log;


extern char *oidtable,   /* Name of oidtable to use */
	   *tailfile,   /* Name of dsaptailor file to use*/
	   username[],
	   password[],
	   posdit[],
	   yes_string[],
	   no_string[],
	   *dsa_address,
	   *local_dit;

extern int preferInvVideo;
extern int greyBook;
extern int browseMess;
extern int maxOrgs;
extern int maxDepts;
extern int maxPersons;
extern int no_of_languages;
extern int localAlarmTime;
extern int remoteAlarmTime;
extern int byeByeMessage;
extern int quitChars;
extern int deLogLevel;
extern int controlCtoQuit;

extern char callingDteNumber[],
	   default_country[],
	   default_organisation[],
	   default_department[],
	   default_person[],
	   prrOC[],
	   rlOC[],
	   rmOC[],
	   have_department[],
	   change_posdit[],
	   change_posdn[],
	   org_compel[],
	   search_mgr[],
	   addorg[],
	   country_mgr[],
	   masterDSA[],
	   init_user[],
	   init_pswd[],
	   addorg_posdit[],
	   just_dn[],
	   language[],
	   origDefaultCo[],
	   origDefaultOrg[],
	   welcomeMessage[];

extern int pswd_intrctv;
extern int username_intrctv;

static void read_de_option();

extern struct namelist * coatts;
extern struct namelist * orgatts;
extern struct namelist * ouatts;
extern struct namelist * prratts;
extern struct namelist * rlatts;
extern struct namelist * rmatts;

int initialisations(argc, argv)
int argc;
char ** argv;
{
	FILE *config_file;
	char linebuf[LINESIZE];
	char * cp;
	char * home;
	char tailorfile[LINESIZE];
	char *getenv();
	void de_exit();
	print_parse_errors = FALSE;   /* Stop auto printing of errors */
	quipu_syntaxes();
	dsa_address = malloc(LINESIZE);

#ifdef USE_PP
	pp_quipu_init ("de");
#endif

	/* Initialize dsap library. */
	dsap_init((int *)NULL, (char ***)NULL);

	specialSyntaxHandlers();

#ifdef USE_PP
	pp_quipu_run ();
#endif

	dsa_address = NULLCP;

	default_country[0] = '\0';
	default_organisation[0] = '\0';
	default_department[0] = '\0';
	default_person[0] = '\0';

	home = malloc(LINESIZE);

	/* turn off logging that we are not interested in */
	(void) isodesetvar("compatlevel", "none", 0);
	(void) isodesetvar("addrlevel", "none", 0);
	(void) isodesetvar("tsaplevel", "none", 0);
	(void) isodesetvar("ssaplevel", "none", 0);
	(void) isodesetvar("psaplevel", "none", 0);
	(void) isodesetvar("psap2level", "none", 0);
	(void) isodesetvar("acsaplevel", "none", 0);
	(void) isodesetvar("rosaplevel", "none", 0);
	/*  isodexport("idm"); */

	addToList(&coatts, "masterDSA");
	addToList(&coatts, DE_COUNTRY_NAME);


	/* Read in dsaptailor file */
	if ((config_file = fopen(isodefile(tailfile, 0), "r")) == (FILE *) NULL) {
		(void) fprintf(stderr, dsap_notOpen);
	} else {
		while(fgets(linebuf, sizeof(linebuf), config_file) != NULLCP)
			if ((*linebuf != '#') && (*linebuf != '\n')) (void) tai_string (linebuf);

		(void) fclose(config_file);
	}

	if ((home = getenv("HOME")) != NULLCP) {
		(void) sprintf(tailorfile, "%s%s", home, "/.dmtailor");
		if ((config_file = fopen(isodefile(tailorfile, 0), "r")) == (FILE *) NULL) {
			if ((config_file = fopen(isodefile("dm/dmtailor", 0), "r")) == (FILE *) NULL) {
				(void) fprintf(stderr, dm_notOpen);
				(void) de_exit(-1);
			} else {
				while(fgets(linebuf, sizeof(linebuf), config_file) != NULLCP) {
					if ((*linebuf != '#') && (*linebuf != '\n')) read_de_option(linebuf);
				}
				(void) fclose(config_file);
			}
		} else {
			while(fgets(linebuf, sizeof(linebuf), config_file) != NULLCP) {
				if ((*linebuf != '#') && (*linebuf != '\n')) read_de_option(linebuf);
			}
			(void) fclose(config_file);
		}
	} else {
		if ((config_file = fopen(isodefile("dm/dmtailor", 0), "r")) == (FILE *) NULL) {
			(void) fprintf(stderr, dm_notOpen);
			(void) pagerOn(NUMBER_NOT_ALLOWED);
			(void) de_exit(-1);
		} else {
			while(fgets(linebuf, sizeof(linebuf), config_file) != NULLCP) {
				if ((*linebuf != '#') && (*linebuf != '\n')) read_de_option(linebuf);
			}
			(void) fclose(config_file);
		}
	}
	/*
	  no_of_languages = 0;

	  if ((config_file = fopen(isodefile("./GBwelcome",0), "r")) == (FILE *) NULL)
	    {
	    }
	  else
	    {
	      no_of_languages ++;
	    }
	  if ((config_file = fopen(isodefile("./ESwelcome",0), "r")) == (FILE *) NULL)
	    {
	    }
	  else
	    {
	      no_of_languages ++;
	    }
	  if ((config_file = fopen(isodefile("./FRwelcome",0), "r")) == (FILE *) NULL)
	    {
	    }
	  else
	    {
	      no_of_languages ++;
	    }

	*/
	for (argc--, argv++; argc > 0; ) {
		cp = *argv;
		if (*cp == '-')
			switch (*++cp) {
			case 'h':
				argc--;
				argv++;
				if (argv != (char **)NULL) {
					(void) strcpy(callingDteNumber, *argv);
					argc--;
					argv++;
				} else
					(void) strcpy(callingDteNumber, "");
				break;
			default:
				(void)fprintf(stderr, "%s %c %s", flag_prob, *cp, flag);
				/* let's show bravado - carry on regardless!!! */
			}
	}


	(void) printf("%*s%s\n\n", (80 - strlen(welcomeMessage)) / 2, "",
				  welcomeMessage);

	initVideo();

	return OK;
}

/*
 * - read_de_option() -
 * Read in an option from detailor or derc file.
 *
 */
static void read_de_option(line)
char *line;
{
	char *part1, *part2;
	extern char *TidyString(), *SkipSpace();
	extern int wanAccess;
	int n;

	part1 = SkipSpace(line);

	if ((part2 = index(part1, ':')) == NULLCP) return;

	*part2++ = '\0';
	part2 = TidyString(part2);

	if (lexequ(part1, "welcomeMessage") == 0) {
		(void) strcpy(welcomeMessage, part2);
	} else if (lexequ(part1, "default_country") == 0) {
		(void) strcpy(default_country, part2);
	} else if (lexequ(part1, "default_org") == 0) {
		(void) strcpy(default_organisation, part2);
	} else if (lexequ(part1, "default_dept") == 0) {
		(void) strcpy(default_department, part2);
	} else if (lexequ(part1, "prrOC") == 0) {
		(void) strcpy(prrOC, part2);
	} else if (lexequ(part1, "rlOC") == 0) {
		(void) strcpy(rlOC, part2);
	} else if (lexequ(part1, "rmOC") == 0) {
		(void) strcpy(rmOC, part2);
	} else if (lexequ(part1, "dept") == 0) {
		(void) strcpy(have_department, part2);
	} else if (lexequ(part1, "change_posdit") == 0) {
		(void) strcpy(change_posdit, part2);
	} else if (lexequ(part1, "change_posdn") == 0) {
		(void) strcpy(change_posdn, part2);
	} else if (lexequ(part1, "org_compel") == 0) {
		(void) strcpy(org_compel, part2);
	} else if (lexequ(part1, "search_mgr") == 0) {
		(void) strcpy(search_mgr, part2);
	} else if (lexequ(part1, "addorg") == 0)  {
		(void) strcpy(addorg, part2);
	} else if (lexequ(part1, "country_mgr") == 0)  {
		(void) strcpy(country_mgr, part2);
	} else if (lexequ(part1, "masterDSA") == 0)  {
		(void) strcpy(masterDSA, part2);
	} else if (lexequ(part1, "init_user") == 0) {
		(void) strcpy(init_user, part2);
	} else if (lexequ(part1, "init_pswd") == 0) {
		(void) strcpy(init_pswd, part2);
	} else if (lexequ(part1, "addorg_posdit") == 0) {
		(void) strcpy(addorg_posdit, part2);
	} else if (lexequ(part1, "just_dn") == 0) {
		(void) strcpy(just_dn, part2);
	} else if (lexequ(part1, "wanAccess") == 0) {
		if (lexequ(part2, "off") == 0) {
			wanAccess = FALSE;
		} else {
			wanAccess = TRUE;
		}
	} else if (lexequ(part1, "language") == 0) {
		(void) strcpy(language, part2);
	} else if (lexequ(part1, "dsa_address") == 0) {
		dsa_address = copy_string(part2);
	} else if (lexequ(part1, "username") == 0) {
		(void) sprintf(username, part2);
		username_intrctv = FALSE;
	} else if (lexequ(part1, "password") == 0) {
		(void) sprintf(password, part2);
		pswd_intrctv = FALSE;
	} else if (lexequ(part1, "posdit") == 0) {
		(void) sprintf(posdit, part2);
	} else if (lexequ(part1, "yes") == 0) {
		(void) sprintf(yes_string, part2);
	} else if (lexequ(part1, "no") == 0) {
		(void) sprintf(no_string, part2);
	} else if (lexequ(part1, "delogfile") == 0) {
		de_log->ll_file = copy_string(part2);
	} else if (lexequ(part1, "byeByeMessage") == 0) {
		if (lexequ(part2, "off") == 0)
			byeByeMessage = FALSE;
		else
			byeByeMessage = TRUE;
	} else if (lexequ(part1, "inverseVideo") == 0) {
		if (lexequ(part2, "off") == 0)
			preferInvVideo = FALSE;
		else
			preferInvVideo = TRUE;
	} else if (lexequ(part1, "greyBook") == 0) {
		if (lexequ(part2, "off") == 0)
			greyBook = FALSE;
		else
			greyBook = TRUE;
	} else if (lexequ(part1, "browseMess") == 0) {
		if (lexequ(part2, "off") == 0)
			browseMess = FALSE;
		else
			browseMess = TRUE;
	} else if (lexequ(part1, "allowControlCtoQuit") == 0) {
		if (lexequ(part2, "off") == 0)
			controlCtoQuit = FALSE;
		else
			controlCtoQuit = TRUE;
	} else if (lexequ(part1, "country") == 0) {
		addToCoList(part2);
	} else if (lexequ(part1, "mapattname") == 0) {
		addToAttList(part2);
	} else if (lexequ(part1, "commonatt") == 0) {
		if ((str2AttrT(part2)) == NULLAttrT) {
			(void) printf("%s   %s -- %s", inv_attrT, part2, ignored);
		} else {
			addToList(&rmatts, part2);
			addToList(&rlatts, part2);
			addToList(&prratts, part2);
			addToList(&ouatts, part2);
			addToList(&orgatts, part2);
		}
	} else if (lexequ(part1, "orgatt") == 0) {
		if ((str2AttrT(part2)) == NULLAttrT) {
			(void) printf("%s   %s -- %s", inv_attrT, part2, ignored);
		} else {
			addToList(&orgatts, part2);
		}
	} else if (lexequ(part1, "ouatt") == 0) {
		if ((str2AttrT(part2)) == NULLAttrT) {
			(void) printf("%s   %s -- %s", inv_attrT, part2, ignored);
		} else {
			addToList(&ouatts, part2);
		}
	} else if (lexequ(part1, "prratt") == 0) {
		if ((str2AttrT(part2)) == NULLAttrT) {
			(void) printf("%s   %s -- %s", inv_attrT, part2, ignored);
		} else {
			addToList(&prratts, part2);
		}
	} else if (lexequ(part1, "rlatt") == 0) {
		if ((str2AttrT(part2)) == NULLAttrT) {
			(void) printf("%s   %s -- %s", inv_attrT, part2, ignored);
		} else {
			addToList(&rlatts, part2);
		}
	} else if (lexequ(part1, "rmatt") == 0) {
		if ((str2AttrT(part2)) == NULLAttrT) {
			(void) printf("%s   %s -- %s", inv_attrT, part2, ignored);
		} else {
			addToList(&rmatts, part2);
		}
	} else if (lexequ(part1, "mapphone") == 0) {
		addToPhoneList(part2);
	}
	/*
	  else if (lexequ(part1, "maxOrgs") == 0) {
	    n = atoi(part2);
	    if (n > 0)
	      maxOrgs = n;
	  }
	  else if (lexequ(part1, "maxDepts") == 0) {
	    n = atoi(part2);
	    if (n > 0)
	      maxDepts = n;
	  }
	*/
	else if (lexequ(part1, "maxPersons") == 0) {
		n = atoi(part2);
		if (n > 0)
			maxPersons = n;
	} else if (lexequ(part1, "localAlarmTime") == 0) {
		n = atoi(part2);
		if (n > 0)
			localAlarmTime = n;
	} else if (lexequ(part1, "remoteAlarmTime") == 0) {
		n = atoi(part2);
		if (n > 0)
			remoteAlarmTime = n;
	} else if (lexequ(part1, "quitChars") == 0) {
		n = atoi(part2);
		if (n > 0)
			quitChars = n;
	} else if (lexequ(part1, "logLevel") == 0) {
		deLogLevel = atoi(part2);
	}
} /* read_de_option */

welcome() {
	FILE * welcome_file;
	char linebuf[LINESIZE];
	char welcome_lang[LINESIZE];

	(void) strcpy(welcome_lang, "./");
	(void) strcat(welcome_lang, language);
	(void) strcat(welcome_lang, "welcome");
	if ((welcome_file = fopen(isodefile(welcome_lang, 0), "r")) == (FILE *) NULL)
		(void) fprintf(stderr, wlcm_notOpen);
	else {
		while(fgets(linebuf, sizeof(linebuf), welcome_file) != NULLCP)
			(void) fputs(linebuf, stdout);
		(void) fclose(welcome_file);
	}
}

