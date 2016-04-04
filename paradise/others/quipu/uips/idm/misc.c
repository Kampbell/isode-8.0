#include "extmessages.h"
#include "demanifest.h"
#include "quipu/util.h"
#include "namelist.h"
extern char yes_string[];
extern char no_string[];
extern char default_country[];
extern char default_organisation[];
extern char default_department[];
extern char default_userdn[];
extern char co[];
extern char org[];
extern char ou[];
extern char person[];
extern char posdit[];
extern char org_compel[];
void onint1();
void searchFail();
char * mapAttName();
extern int orgEntered;

int de_prompt_yesno(prompt, value, default_string)
char prompt[];
char * value;
char default_string[];
{

	extern char no_string[];
	extern char yesnoonly[];
	extern char yes_string[];
	char * prompt2;
	char * temp;
	temp = malloc(LINESIZE);

	prompt2 = malloc(LINESIZE);

	for (;;) {
		sprintf(prompt2, "%s (%s/%s)", prompt, yes_string, no_string);
		sprintf(prompt2, "%s [%s]  ", prompt2, default_string);
		writeInverse(prompt2);
		putchar(' ');
		if (gets(value) == NULLCP) {
			/* behave as for an interrupt */
			clearerr(stdin);
			onint1();
		} else {
			value = TidyString(value);
		}
		if (!(strcmp(value, yes_string)))
			break;
		else if (!(strcmp(value, no_string)))
			break;
		else if (strlen(value) == 0) {
			sprintf(value, default_string);
			break;
		} else {
			sprintf(temp, yesnoonly);
			writeInverse(temp);
		}
	}

	free(temp);
	return;
}

int de_prompt_value(prompt, value)
char prompt[];
char * value;
{
	char * temp;
	temp = malloc(LINESIZE);
	sprintf(temp, prompt);
	writeInverse(temp);

	putchar(' ');

	if (gets(value) == NULLCP) {
		/* behave as for an interrupt */
		clearerr(stdin);
		onint1();
	} else {
		value = TidyString(value);
	}
	free(temp);
	return;
}

int ask_addr(attr_name, lines)
char * attr_name;
char * lines;
{

	char * addr_ptr[LINES_PA+1];
	char str_pa[LINESIZE];
	char * mark;
	char * ptr;
	char * str;

	int last;
	int length;
	int line_counter;
	int position;

	str = malloc(LINESIZE);
	strncpy(str_pa, "", LINESIZE);
	line_counter = 1;
	length = 0;
	position = 0;
	last = FALSE;
	strncpy(str_pa, "", LINESIZE);
	while (line_counter <= LINES_PA) {
		addr_ptr[line_counter] = malloc(LINESIZE);
		strncpy(addr_ptr[line_counter], "", LINESIZE);
		line_counter++;
	}

	if (strlen(lines) > 0) {
		ptr = lines;
		mark = lines;
		line_counter = 1;

		while (line_counter <= LINES_PA) {
			if (last == FALSE) {
				if ((ptr = index(ptr, '$')) == NULLCP) {
					ptr = index(lines, '\0');
					last = TRUE;
				}
				strncpy(addr_ptr[line_counter], mark, ptr-mark);
			}
prompt_value1:
			printf("   %s %d [%s]:  ", line_str, line_counter, addr_ptr[line_counter]);
			if (gets(str) == NULLCP) {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			} else {
				str = TidyString(str);
			}
			if (!(strcmp(str, quit_String))) {
				sprintf(lines, "q");
				return NOTOK;
			} else if (!(strcmp(str, "?"))) {
				displayFile(attr_name, FALSE);
				goto prompt_value1;
			}
			if (strlen(str) == 1 && index(str, '-') != 0) {
				/* A `-' was entered, delete default value */
				*str = '\0';
				addr_ptr[line_counter] = NULLCP;
			}
			if (strlen(str) > CHARS_PA) {
				printf("%s %d %s\n", max_len_line, CHARS_PA, characters);
				goto prompt_value1;
			}

			if (strlen(str) == 0 && addr_ptr[line_counter] != NULLCP) {
				/* The default value was taken, copy it */
				strcpy(str, addr_ptr[line_counter]);
			}

			if (line_counter == 1) {
				strcpy(str_pa, str);
			} else {
				strcat(str_pa, str);
			}
			if (line_counter < LINES_PA) { /* Don't add $ to last line */
				sprintf(str_pa, "%s$", str_pa);
			}
			if (last == FALSE) {
				ptr++; /* skip $ */
				mark = ptr;
			}
			line_counter++;
		}
	} else {
		line_counter = 1;
		while (line_counter <= LINES_PA) {
prompt_value2:
			printf("   %s %d:  ", line_str, line_counter);
			if (gets(str) == NULLCP) {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			} else {
				str = TidyString(str);
			}
			if (!(strcmp(str, quit_String))) {
				sprintf(lines, "q");
				while (line_counter <= LINES_PA) {
					/*                      free(addr_ptr[line_counter]); */
					line_counter++;
				}
				return NOTOK;
			} else if (!(strcmp(str, "?"))) {
				displayFile(attr_name, FALSE);
				goto prompt_value2;
			}
			if (strlen(str) > CHARS_PA) {
				printf("%s %d %s\n", max_len_line, CHARS_PA, characters);
				goto prompt_value2;
			}
			if (line_counter == 1) {
				strcpy(str_pa, str);
			} else {
				strcat(str_pa, str);
			}
			if (line_counter < LINES_PA) { /* Don't add $ to last line */
				sprintf(str_pa, "%s$", str_pa);
			}
			line_counter++;
		}
	}

	/* Strip trailing $ signs (looping backwards) */

	length = strlen(str_pa);
	position = length - 1;  /* 0 based */
	while (position >= 0 ) {
		if (str_pa[position] == '$') {
			str_pa[position] = '\0';
			position--;
		} else {
			break;
		}
	}
	sprintf(lines, str_pa);
	line_counter = 1;
	while (line_counter <= LINES_PA) {
		/*        free(addr_ptr[line_counter]); */
		line_counter++;
	}
	return OK;
}

int ask_tlx(attr_name, lines)
char * attr_name;
char * lines;
{

	char * tlx_ptr[LINES_TLX+1];
	char str_pa[LINESIZE];
	char * mark;
	char * prompt;
	char * ptr;
	char * str;

	int last;
	int length;
	int line_counter;
	int position;

	prompt	= malloc(LINESIZE);
	str		= malloc(LINESIZE);

	strncpy(str_pa, "", LINESIZE);
	line_counter = 1;
	length = 0;
	position = 0;
	last = FALSE;
	strncpy(str_pa, "", LINESIZE);
	while (line_counter <= LINES_TLX) {
		tlx_ptr[line_counter] = malloc(LINESIZE);
		strncpy(tlx_ptr[line_counter], "", LINESIZE);
		line_counter++;
	}

	if (strlen(lines) > 0) {
		ptr = lines;
		mark = lines;
		line_counter = 1;

		while (line_counter <= LINES_TLX) {
			if (last == FALSE) {
				if ((ptr = index(ptr, '$')) == NULLCP) {
					ptr = index(lines, '\0');
					last = TRUE;
				}
				strncpy(tlx_ptr[line_counter], mark, ptr-mark);
			}
prompt_value1:
			if (line_counter == 1) {
				sprintf(prompt, "   %s", number_str);
			} else if (line_counter == 2) {
				sprintf(prompt, "   %s", country_str);
			} else if (line_counter == 3) {
				sprintf(prompt, "   %s", ansBack_str);
			}
			printf("%s [%s]:  ", prompt, tlx_ptr[line_counter]);
			if (gets(str) == NULLCP) {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			} else {
				str = TidyString(str);
			}
			if (!(strcmp(str, quit_String))) {
				sprintf(lines, "q");
				return NOTOK;
			} else if (!(strcmp(str, "?"))) {
				displayFile(attr_name, FALSE);
				goto prompt_value1;
			}
			if (strlen(str) == 1 && index(str, '-') != 0) {
				/* A `-' was entered, delete default value */
				*str = '\0';
				tlx_ptr[line_counter] = NULLCP;
			}

			if (strlen(str) == 0 && tlx_ptr[line_counter] != NULLCP) {
				/* The default value was taken, copy it */
				strcpy(str, tlx_ptr[line_counter]);
			}

			if (line_counter == 1) {
				strcpy(str_pa, str);
			} else {
				strcat(str_pa, str);
			}
			if (line_counter < LINES_TLX) { /* Don't add $ to last line */
				sprintf(str_pa, "%s$", str_pa);
			}
			if (last == FALSE) {
				ptr++; /* skip $ */
				mark = ptr;
			}
			line_counter++;
		}
	} else {
		line_counter = 1;
		while (line_counter <= LINES_TLX) {
prompt_value2:
			if (line_counter == 1) {
				sprintf(prompt, "   %s", number_str);
			} else if (line_counter == 2) {
				sprintf(prompt, "   %s", country_str);
			} else if (line_counter == 3) {
				sprintf(prompt, "   %s", ansBack_str);
			}
			printf("%s:  ", prompt);
			if (gets(str) == NULLCP) {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			} else {
				str = TidyString(str);
			}
			if (!(strcmp(str, quit_String))) {
				sprintf(lines, "q");
				while (line_counter <= LINES_TLX) {
					/*                      free(tlx_ptr[line_counter]); */
					line_counter++;
				}
				return NOTOK;
			} else if (!(strcmp(str, "?"))) {
				displayFile(attr_name, FALSE);
				goto prompt_value2;
			}

			if (line_counter == 1) {
				if (strlen(str) <= 0) {
					sprintf(lines, "");
					return OK;
				} else {
					strcpy(str_pa, str);
				}
			} else {
				strcat(str_pa, str);
			}
			if (line_counter < LINES_TLX) { /* Don't add $ to last line */
				sprintf(str_pa, "%s$", str_pa);
			}
			line_counter++;
		}
	}

	/* Strip trailing $ signs (looping backwards) */

	length = strlen(str_pa);
	position = length - 1;  /* 0 based */
	while (position >= 0 ) {
		if (str_pa[position] == '$') {
			str_pa[position] = '\0';
			position--;
		} else {
			break;
		}
	}
	sprintf(lines, str_pa);
	line_counter = 1;
	while (line_counter <= LINES_TLX) {
		/*        free(tlx_ptr[line_counter]); */
		line_counter++;
	}
	return OK;
}


int is_postal_address(str)
char * str;
{
	if (!(strcmp(str, "postalAddress"))     ||
			!(strcmp(str, "homePostalAddress")) ||
			!(strcmp(str, "registeredAddress")) ||
			!(strcmp(str, "streetAddress"))) {
		return TRUE;
	} else {
		return FALSE;
	}
}


int is_dn(str)
char * str;
{
	if (!(strcmp(str, "roleOccupant"))		||
			!(strcmp(str, "secretary"))		||
			!(strcmp(str, "aliasedObjectName"))	||
			!(strcmp(str, "seeAlso"))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int is_phone_fax(str)
char * str;
{
	if (!(strcmp(str, "telephoneNumber"))		||
			!(strcmp(str, "homePhone"))			||
			!(strcmp(str, "facsimileTelephoneNumber"))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int is_tlx(str)
char * str;
{
	if (!(strcmp(str, "telexNumber"))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int is_x400(str)
char * str;
{
	if (!(strcmp(str, "mhsORAddresses"))) {
		return TRUE;
	} else {
		return FALSE;
	}
}


int ask_dnhnc(str, attr_name)
char * str;  /* Should have been malloc'd before */
char attr_name[];
{

	struct namelist *plp = NULLLIST;

	static char posdn[LINESIZE];
	extern char change_posdn[];
	char ask_ifhere[LINESIZE];
	char posDNattr[LINESIZE];
	char * cont_add_role;
	char * cp;
	char * here;
	char * rdn;
	char * tmp_search;

	int new;
	int noPersons;
	int objectType;
	int status;

	here	= malloc(LINESIZE);
	rdn		= malloc(LINESIZE);
	tmp_search	= malloc(LINESIZE);
	new = FALSE;

	if (((!(strcmp(change_posdn, yes_string))) &&
			(strlen(posdn) <= 0)) ||
			(strcmp(change_posdn, yes_string))) {
		sprintf(posdn, posdit);
	}

prompt_where:
	if (!(strcmp(change_posdn, yes_string))) {
		printf("                                   ");
		display_posdit(36, posdn);
		printf("\n");
		objectType = 0;
		cp = rindex(posdn, '@');
		status = get_objectClass(cp, &objectType);
		if (status != OK) {
			printf(org_unk_type);
			printf("\n%s", err_rprtHlpdsk);
			sprintf(posDNattr, "position"); /* continue */
		} else {
			if (objectType == COUNTRY) {
				sprintf(posDNattr, country2_str);
			} else if (objectType == ORG) {
				sprintf(posDNattr, org_str);
			} else if (objectType == ORGUNIT) {
				sprintf(posDNattr, ou_str);
			}
		}
		sprintf(ask_ifhere, "%s %s %s %s", is_the, mapAttName(attr_name), posDNattr, above_str);
		de_prompt_yesno(ask_ifhere, here, yes_string);
	} else {
		sprintf(here, yes_string);
	}
prompt_dn:

	if (!(strcmp(here, yes_string))) {
		if (strlen(str) <= 0) {
			sprintf(default_userdn, "");
		} else {
			rdn = copy_string(lastComponent(str, PERSON));
			sprintf(default_userdn, rdn);
		}

		printf("%s %s ", enter_msg, mapAttName(attr_name));
		noPersons = 0;
		enterString(USERDN, person, plp);

		if (!(strcmp(person, quit_String))) {
			if (noPersons > 0) {
				freePRRs(&plp);
			}
			return NOTOK;
		}

		if (strlen(person) == 1 && index(person, '-') != 0) {
			sprintf(str, "%s", "-");
			return OK;
		} else if (strlen(person) <= 0) {
			/* Return was entered */
			if (strlen(default_userdn) <= 0) {
				if (strlen(str) <= 0) {
					/* no value entered and no default*/
					sprintf(str, "");
					return OK;
				} else {
					return OK;
				}
			} else {
				if (new == FALSE) {
					return OK;
				} else {
					/* set person, in order to search in new posdn */
					sprintf(person, "%s", default_userdn);
				}
			}
		}

		if (noPersons > 0) {
			freePRRs(&plp);
		}

		if (orgEntered == FALSE) {
			sprintf(tmp_search, "%s@cn=%s", posdn,person);
			if (listXctCpPRRcn(tmp_search, &plp) != OK) {
				searchFail(person);
				cont_add_role = malloc(LINESIZE);
				de_prompt_yesno(cnt_nosrv_posdn, cont_add_role, no_string);
				if (!(strcmp(cont_add_role, yes_string))) {
					/* return an empty string */
					sprintf(str, "");
					free(cont_add_role);
					return OK;
				} else {
					sprintf(str, "");
					free(cont_add_role);
					return NOTOK;
				}
			}
		} else {
			if (listPRRCp(posdn, person, &plp) != OK) {
				searchFail(person);
				cont_add_role = malloc(LINESIZE);
				de_prompt_yesno(cnt_nosrv_posdn, cont_add_role, no_string);
				if (!(strcmp(cont_add_role, yes_string))) {
					/* return an empty string */
					sprintf(str, "");
					free(cont_add_role);
					return OK;
				} else {
					sprintf(str, "");
					free(cont_add_role);
					return NOTOK;
				}
			}
		}
		noPersons = listlen(plp);
		if (noPersons <= 0) {
			printf("%s `%s'\n\n", no_ent_match, person);
			goto prompt_where;
		} else if (noPersons == 1) {
			sprintf(str, "@%s", plp->name);
			return OK;
		} else {
			printf(got_match);
			printf(type_number);
			pagerOn(NUMBER_ALLOWED);
			printListPRRs(person, plp, PERSON, FALSE);
			goto prompt_dn;
		}
	} else {
		orgEntered = FALSE;
		status = get_posdn(posdn, &new);
		if (status != OK) {
			cont_add_role = malloc(LINESIZE);
			de_prompt_yesno(cnt_nosrv_posdn, cont_add_role, no_string);
			if (!(strcmp(cont_add_role, yes_string))) {
				/* return an empty string */
				sprintf(str, "");
				free(cont_add_role);
				return OK;
			} else {
				sprintf(str, "");
				free(cont_add_role);
				return NOTOK;
			}
		} else {
			sprintf(here, yes_string);
			goto prompt_dn;
		}
	}
}


int get_posdn(posdn, new)
char posdn[];
int * new;
{
	struct namelist * clp = NULLLIST;
	struct namelist * olp = NULLLIST;
	struct namelist * oulp = NULLLIST;
	int noCos;
	int noOrgs;
	int noOUs;
	extern char have_department[];
	extern int highNumber;
	char * rdn;

	rdn = malloc(LINESIZE);

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
			free(rdn);
			highNumber = 0;
			pagerOn(NUMBER_NOT_ALLOWED);
			return NOTOK;
		} else {
			sprintf(default_organisation, "");
			*new = TRUE;
		}
	}

	if (listCos(co, &clp) != OK) {
		searchFail(co);
		return NOTOK;
	}
	noCos = listlen(clp);
	if (noCos <= 0) {
		printf("%s `%s' \n\n", no_cMatch, co);
		goto prompt_country;
	} else if (noCos == 1) {
		sprintf(posdn, "@%s", clp->name);
		rdn = copy_string(lastComponent(clp->name, COUNTRY));
		strcpy(default_country, rdn);
		printf("%s\n", default_country);
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
				free(rdn);
				return OK;
			}
		} else {
			strcpy(org, default_organisation);
		}
	} else {
		if (!(strcmp(org, quit_String))) {
			if (strcmp(org_compel, no_string)) {
				free(rdn);
				return NOTOK;
			} else {
				free(rdn);
				highNumber = 0;
				pagerOn(NUMBER_NOT_ALLOWED);
				return OK;
			}
		} else {
			sprintf(default_department, "");
			*new = TRUE;
		}
	}
	if (listOrgs(posdn, org, &olp) != OK) {
		searchFail(org);
		return NOTOK;
	}
	noOrgs = listlen(olp);
	if (noOrgs <= 0) {
		printf("%s `%s' \n\n", no_orgMatch, org);
		goto prompt_org;
	} else if (noOrgs == 1) {
		sprintf(posdn, "@%s", olp->name);
		rdn = copy_string(lastComponent(olp->name, ORG));
		strcpy(default_organisation, rdn);
		printf("%s\n", default_organisation);
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
		orgEntered = TRUE;
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
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
		return OK;
	} else if ((strcmp(have_department, yes_string))) {
		/* if something different from yes or no (default) */
		sprintf(ou, "%s", "*");
		if (listOUs(posdn, ou, &oulp) != OK) {
			searchFail("departments");
			return NOTOK;
		}
		noOUs = listlen(oulp);
		if (noOUs <= 0) {
			free(rdn);
			return OK;
		}
	}

	highNumber = 0;
	pagerOn(NUMBER_NOT_ALLOWED);

prompt_ou:
	printf("\n");
	enterString(ORGUNIT, ou, oulp);
	if (strlen(ou) == 0) {
		if (!(strcmp(default_department, ""))) {
			free(rdn);
			return OK;
		} else {
			strcpy(ou, default_department);
		}
	} else {
		if (!(strcmp(ou, quit_String))) {
			free(rdn);
			highNumber = 0;
			pagerOn(NUMBER_NOT_ALLOWED);
			return OK;
		} else {
			*new = TRUE;
		}
	}
	if (listOUs(posdn, ou, &oulp) != OK) {
		searchFail(ou);
		return NOTOK;
	}
	noOUs = listlen(oulp);
	if (noOUs <= 0) {
		printf("%s `%s' \n\n", no_ouMatch, ou);
		goto prompt_ou;
	} else if (noOUs == 1) {
		sprintf(posdn, "@%s", oulp->name);
		rdn = copy_string(lastComponent(oulp->name, ORGUNIT));
		strcpy(default_department, rdn);
		printf("%s\n", default_department);
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListOUs(ou, oulp);
		goto prompt_ou;
	}
	free(rdn);
	return OK;
}

int ask_password_entry(pswd)
char pswd[];
{
	char * getpass();
	char * temp2;
	char new_pswd[LINESIZE];
	char vfy_pswd[LINESIZE];

	temp2 = malloc(LINESIZE);

ask_again:
	sprintf(temp2, "%s :  ", ask_org_password);
	sprintf(new_pswd, getpass(temp2));
	if ((!(strcmp(new_pswd, quit_String))) || (strlen(new_pswd) <= 0)) {
		printf(pswd_must_enter);
		goto ask_again;
	} else if (!(strcmp(new_pswd, "?"))) {
		displayFile("userPassword", FALSE);
		goto ask_again;
	}
	sprintf(temp2, "%s :  ", ask_vfy_org_pswd);
	sprintf(vfy_pswd, getpass(temp2));
	if ((!(strcmp(vfy_pswd, quit_String))) || (strlen(new_pswd) <= 0)) {
		printf(pswd_must_enter);
		goto ask_again;
	} else if (!(strcmp(vfy_pswd, "?"))) {
		displayFile("userPassword", FALSE);
		goto ask_again;
	}
	if (!(strcmp(new_pswd, vfy_pswd))) {
		sprintf(pswd, new_pswd);
		return OK;
	} else {
		printf(vfy_pswd_fail);
		goto ask_again;
	}
}
