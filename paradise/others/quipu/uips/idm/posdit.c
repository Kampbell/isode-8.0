#include "extmessages.h"
#include "demanifest.h"
#include "destrings.h"
#include "quipu/util.h"
#include "namelist.h"
extern char yes_string[];
extern char no_string[];
extern int assoc;
extern char default_country[];
extern char default_organisation[];
extern char default_department[];
extern char co[];
extern char org[];
extern char change_posdit[];
extern char org_compel[];
extern char ou[];
extern char have_department[];
extern char person[];
extern char posdit[];
extern int highNumber;
extern int posdit_oc;
extern int quipuMastersCo;

int position_dit() {

	char * change_pos;
	char * mapCoName();
	DN dn;
	int objectType;

	void de_exit();
	void searchFail();

	change_pos = malloc(LINESIZE);

	if (strlen(posdit) > 0) {
		dn = NULLDN;
		dn = str2dn(posdit);
		if (dn == NULLDN) {
			printf(errmsg_invposditconf);
		} else {
			printf(curpos_msg);
			display_posdit(strlen(curpos_msg), posdit);
			determine_posdit(&objectType);
			posdit_oc = objectType;
		}
		de_prompt_yesno(change_pos_msg, change_pos, no_string);
		if (!(strcmp(change_pos, no_string))) {
			free(change_pos);
			return OK;
		} else {
			printf(enter_new_posdit);
			get_posdit();
			free(change_pos);
			return OK;
		}
	} else {
		printf(enter_posdit);
		get_posdit();
		free(change_pos);
		return OK;
	}
}

int display_posdit(length, str)
int length;
char str[];
{
	char * savestr;
	int first;
	int spaces;

	savestr = malloc(LINESIZE);

	sprintf(savestr, str);

	first = TRUE;

	pagerOn(NUMBER_NOT_ALLOWED);

	while (*savestr != NULL) {
		if (first == FALSE) {
			spaces = length - 1 ;
			printf("%*s", spaces, " ");
		}
		first = FALSE;
		printLastComponent(INDENTOFF, savestr, 0, FALSE);
		savestr = removeLastRDN(savestr);
	}

	free(savestr);
	return;
}

int get_posdit() {
	struct namelist * clp = NULLLIST;
	struct namelist * olp = NULLLIST;
	struct namelist * oulp = NULLLIST;
	int noCos;
	int noOrgs;
	int noOUs;
	char posdit_save[LINESIZE];
	char * confirm_out;
	char * full_c;
	char * rdn;
	Attr_Sequence at;

	rdn		= malloc(LINESIZE);

	strcpy(posdit_save, posdit);

prompt_country:
	enterString(COUNTRY, co, clp);
	if (strlen(co) == 0) {
		if (!(strcmp(default_country, ""))) {
			printf("%s %s %s\n", reenter_c, quit_String, to_quit);
			goto prompt_country;
		} else {
			strcpy(co, default_country);
		}
	} else {
		if (!(strcmp(co, quit_String))) {
			if (strlen(posdit_save) <= 0) {
				confirm_out = malloc(LINESIZE);
				de_prompt_yesno(ask_ifout, confirm_out, no_string);
				if (!(strcmp(confirm_out, yes_string))) {
					de_exit(-1);
				} else {
					free(confirm_out);
					goto prompt_country;
				}
			}
			strcpy(posdit, posdit_save);
			printf(samepos_msg);
			display_posdit(strlen(samepos_msg), posdit);
			free(rdn);
			highNumber = 0;
			pagerOn(NUMBER_NOT_ALLOWED);
			return OK;
		} else {
			sprintf(default_organisation, "");
		}
	}

	if (listCos(co, &clp) != OK) {
		searchFail(co);
		de_exit(-1);
	}
	noCos = listlen(clp);
	if (noCos <= 0) {
		printf("%s `%s' \n\n", no_cMatch, co);
		goto prompt_country;
	} else if (noCos == 1) {
		sprintf(posdit, "@%s", clp->name);
		rdn = copy_string(lastComponent(clp->name, COUNTRY));
		strcpy(default_country, rdn);
		full_c	= malloc(LINESIZE);
		sprintf(full_c, "%s -- %s", default_country, mapCoName(default_country));
		printf("%s\n", full_c);
		free(full_c);
		for (at = clp->ats; at != NULLATTR; at = at->attr_link) {
			if (strcmp(attr2name(at->attr_type, OIDPART), "masterDSA") == 0) {
				quipuMastersCo = TRUE;
				break;
			}
		}
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListCos(clp);
		goto prompt_country;
	}
prompt_org:
	printf("\n");
	enterString(ORG, org, olp);
	if (strlen(org) == 0) {
		if (!(strcmp(default_organisation, ""))) {
			if ((strcmp(org_compel, no_string))) {
				printf("%s %s %s\n", enter_org, quit_String, to_quit);
				goto prompt_org;
			} else {
				printf(newpos_msg);
				display_posdit(strlen(newpos_msg), posdit);
				free(rdn);
				return OK;
			}
		} else {
			sprintf(org, "%s", default_organisation);
		}
	} else {
		if (!(strcmp(org, quit_String))) {
			if (strlen(posdit_save) <= 0) {
				confirm_out = malloc(LINESIZE);
				de_prompt_yesno(ask_ifout, confirm_out, no_string);
				if (!(strcmp(confirm_out, yes_string))) {
					de_exit(-1);
				} else {
					free(confirm_out);
					highNumber = 0;
					pagerOn(NUMBER_NOT_ALLOWED);
					goto prompt_country;
				}
			}
			if (strcmp(org_compel, no_string)) {
				strcpy(posdit, posdit_save);
				printf(samepos_msg);
				display_posdit(strlen(samepos_msg), posdit);
				free(rdn);
				return OK;
			} else {
				printf(newpos_msg);
				display_posdit(strlen(newpos_msg), posdit);
				posdit_oc = COUNTRY;
				free(rdn);
				return OK;
			}
		} else {
			sprintf(default_department, "");
		}
	}
	if (listOrgs(posdit, org, &olp) != OK) {
		searchFail(org);
		strcpy(posdit, posdit_save);
		printf(samepos_msg);
		display_posdit(strlen(samepos_msg), posdit);
		free(rdn);
		return OK;
	}
	noOrgs = listlen(olp);
	if (noOrgs <= 0) {
		printf("%s `%s' \n\n", no_orgMatch, org);
		goto prompt_org;
	} else if (noOrgs == 1) {
		sprintf(posdit, "@%s", olp->name);
		rdn = copy_string(lastComponent(olp->name, ORG));
		strcpy(default_organisation, rdn);
		printf("%s\n", default_organisation);
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
		posdit_oc = ORG;
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListOrgs(org, olp);
		goto prompt_org;
	}

	pagerOn(NUMBER_NOT_ALLOWED);

	if (!(strcmp(have_department, no_string))) {
		free(rdn);
		return OK;
	} else if ((strcmp(have_department, yes_string))) {
		/* if different from yes (and no) */
		sprintf(ou, "%s", "*");
		if (listOUs(posdit, ou, &oulp) != OK) {
			searchFail("departments");
			strcpy(posdit, posdit_save);
			printf(samepos_msg);
			display_posdit(strlen(samepos_msg), posdit);
			free(rdn);
			return NOTOK;
		}
		highNumber = 0; /* disable entering a number, without displaying it */
		noOUs = listlen(oulp);
		if (noOUs <= 0) {
			printf(newpos_msg);
			display_posdit(strlen(newpos_msg), posdit);
			free(rdn);
			return OK;
		} else {
			sprintf(change_posdit, "%s", yes_string);
		}
	}
	highNumber = 0; /* disable entering a number, without displaying it */
	pagerOn(NUMBER_NOT_ALLOWED);
prompt_ou:
	printf("\n");
	enterString(ORGUNIT, ou, oulp);
	if (strlen(ou) == 0) {
		if (!(strcmp(default_department, ""))) {
			printf(newpos_msg);
			display_posdit(strlen(newpos_msg), posdit);
			free(rdn);
			return OK;
		} else {
			strcpy(ou, default_department);
		}
	} else {
		if (!(strcmp(ou, quit_String)) ||
				!(strcmp(ou, "-"))) {
			printf(newpos_msg);
			display_posdit(strlen(newpos_msg), posdit);
			free(rdn);
			highNumber = 0;
			pagerOn(NUMBER_NOT_ALLOWED);
			return OK;
		}
	}
	if (listOUs(posdit, ou, &oulp) != OK) {
		searchFail(ou);
		strcpy(posdit, posdit_save);
		printf(samepos_msg);
		display_posdit(strlen(samepos_msg), posdit);
		free(rdn);
		return OK;
	}
	noOUs = listlen(oulp);
	if (noOUs <= 0) {
		printf("%s `%s' \n\n", no_ouMatch, ou);
		goto prompt_ou;
	} else if (noOUs == 1) {
		sprintf(posdit, "@%s", oulp->name);
		rdn = copy_string(lastComponent(oulp->name, ORGUNIT));
		strcpy(default_department, rdn);
		printf("%s\n", default_department);
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
		posdit_oc = ORGUNIT;
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListOUs(ou, oulp);
		goto prompt_ou;
	}
	printf(newpos_msg);
	display_posdit(strlen(newpos_msg), posdit);
	free(rdn);
	return OK;
}

int determine_posdit(objectType)
int * objectType;
{
	char * cp;
	char * savestr;
	char * temprdn;

	savestr = malloc(LINESIZE);
	temprdn = malloc(LINESIZE);

	sprintf(savestr, posdit);

	cp = rindex(savestr, '@');
	cp++;
	if (strncmp(cp, SHORT_OU, strlen(SHORT_OU)) == 0) {
		*objectType = ORGUNIT;
		free(temprdn);
		free(savestr);
		return OK;
	} else if (strncmp(cp, SHORT_ORG, strlen(SHORT_ORG)) == 0) {
		*objectType = ORG;
		free(temprdn);
		free(savestr);
		return OK;
	} else if (strncmp(cp, SHORT_CO, strlen(SHORT_CO)) == 0) {
		*objectType = COUNTRY;
		cp++;
		if (!(strncmp(cp, "-", 1))) {
			free(temprdn);
			free(savestr);
			return OK;
		} else {
			/* Something is fishy , return ENTRY */
			*objectType = ENTRY;
			free(temprdn);
			free(savestr);
			return OK;
		}
	} else {
		free(temprdn);
		free(savestr);
		return OK;
	}
}
