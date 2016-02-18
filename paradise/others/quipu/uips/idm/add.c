#include "isoaddrs.h"
#include "quipu/util.h"
#include "demanifest.h"
#include "extmessages.h"
#include "namelist.h"
#include "quipu/ds_search.h"
#include "quipu/add.h"
#include <stdio.h>
extern char *oidtable;
extern char *tailfile;
extern char *dsa_address;
extern char *local_dit;
extern int  assoc;
extern int  boundToDSA;
extern int  exactMatch;
extern char default_person[];
extern char person[];
extern char posdit[];
extern char addorg_posdit[];
extern char country_mgr[];
extern char masterDSA[];
extern char org[];
extern char yes_string[];
extern char no_string[];
extern char prrOC[];
extern char rlOC[];
extern char rmOC[];
extern char init_user[];
extern struct namelist * orgatts;
extern struct namelist * prratts;
extern struct namelist * rlatts;
extern struct namelist * rmatts;

/* Global variables defined in this file below */

static Attr_Sequence as1;
static struct ds_addentry_arg		adarg;
static char * cn;
static char entry_dn[LINESIZE];
static char * lineptr[100];
static int av_count;
static int mhsUser;

char * mapAttName();

/* end file global variables */

int de_Add() {

	char * object_to_work;
	char * option;
	char * TidyString();
	char * tmp;
	char menu3defopt[LINESIZE];
	void searchFail(), onint1(), de_exit();

	int status;

	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();

	cn			= malloc(LINESIZE);
	object_to_work		= malloc(LINESIZE);
	option			= malloc(LINESIZE);
	tmp			= malloc(LINESIZE);

	as1 = as_comp_alloc();

	(void) sprintf(option, "%s", add_opt);
	(void) sprintf(menu3defopt, menu3defaddopt);

	ask_objectType(option, object_to_work);


	while ((strcmp(object_to_work, quit_String))) {

		(void) sprintf(tmp, "\n%s %s %s\n\n", enter_msg, quit_String, to_quit_anytime);
		(void) printf(tmp);

		if (!(strcmp(object_to_work, "1"))) {
			status = de_Add_person();
			if (status != OK) {
				(void) sprintf(object_to_work, "%s", quit_String);
				status = NOTOK;
				continue;
			}
			(void) sprintf(object_to_work, "%s", quit_String);
			continue;
		} else if (!(strcmp(object_to_work, "2"))) {
			status = de_Add_role();
			if (status != OK) {
				(void) sprintf(object_to_work, "%s", quit_String);
				status = NOTOK;
				continue;
			}
			(void) sprintf(object_to_work, "%s", quit_String);
			continue;
		} else if (!(strcmp(object_to_work, "3"))) {
			status = de_Add_room();
			if (status != OK) {
				(void) sprintf(object_to_work, "%s", quit_String);
				status = NOTOK;
				continue;
			}
			(void) sprintf(object_to_work, "%s", quit_String);
			continue;
		} else {
			(void) sprintf(tmp, "%s\n\n", inv_input_1to3);
			(void) printf(tmp);
		}
		ask_objectType(option, object_to_work);
	}

	free(cn);
	free(object_to_work);
	free(option);
	free(tmp);
	if (status == OK) {
		return OK;
	} else {
		return NOTOK;
	}

}


int prompt_atts(object, atts)
int object;
struct namelist * atts;
{
#define PADCHARS 22
	char * all_ok;
	char * cp;
	char * mark;
	char * p;
	char * ptr;
	char * str;
	char * tmp;
	char * addr_ptr[LINES_PA+1];
	char * tlx_ptr[LINES_PA+1];
	char buffer[LINESIZE];
	char dnstr[LINESIZE];
	char dnstr2[LINESIZE];
	Attr_Sequence as4;
	struct namelist * saveptr;
	int i;
	int last_line;
	int line_counter;
	int spaces;
	int status;
	extern int tmp_org;

	if (atts == NULLLIST)
		return OK;

	all_ok  = malloc(LINESIZE);
	cp      = malloc(LINESIZE);
	saveptr = (struct namelist *) malloc(sizeof(struct namelist));
	as4     = as_comp_alloc();
	as4     = NULLATTR;
	str     = malloc(LINESIZE);
	tmp     = malloc(LINESIZE);
	saveptr = atts;
	av_count = 0;

	(void) sprintf(buffer, atts->name);
	while ((strcmp(buffer, quit_String))) {
		*str = '\0';
		if (lineptr[av_count] != NULLCP) {
			if (is_postal_address(atts->name) == TRUE) {
				(void) sprintf(tmp, "%s: ", mapAttName(buffer));
				(void) printf(tmp);
				(void) sprintf(tmp, "(%d %s)\n", LINES_PA, idm_lines);
				(void) printf(tmp);
				status = ask_addr(buffer, lineptr[av_count]);
				if (status == NOTOK) {
					return NOTOK;
				}
				if (lineptr[av_count] != NULLCP) {
					(void) sprintf(str, lineptr[av_count]);
				}
			} else if (is_dn(atts->name) == TRUE) {
				(void) sprintf(str, lineptr[av_count]);
				(void) sprintf(dnstr2, str);
				(void) strcpy(dnstr, dnstr2);
				status = ask_dnhnc(str, atts->name);
				if (status != OK) {
					(void) sprintf(str, quit_String);
				}
			} else if (is_tlx(atts->name) == TRUE) {
				(void) sprintf(tmp, "%s (%s, %s %s %s)\n", mapAttName(atts->name), number_str,
							   country_str, and_str, ansBack_str);
				(void) printf(tmp);
				(void) sprintf(str, lineptr[av_count]);
				status = ask_tlx(atts->name, str);
				if (status != OK) {
					(void) sprintf(str, quit_String);
				}
			} else if (!(strcmp(atts->name, "objectClass"))) {
				/* Do nothing */
			} else {
				(void) printf("%s  [%s]: ", mapAttName(buffer), lineptr[av_count]);
				if (gets(str) == NULLCP) {
					/* behave as for an interrupt */
					clearerr(stdin);
					onint1();
				} else {
					str = TidyString(str);
				}
			}
		} else if (is_postal_address(atts->name) == TRUE) {
			(void) printf("%s: ", mapAttName(buffer));
			(void) printf("(%d %s) \n", LINES_PA, idm_lines);
			p = malloc(LINESIZE);
			*p = '\0';
			lineptr[av_count] = p;
			status = ask_addr(buffer, lineptr[av_count]);
			if (status == NOTOK) {
				return NOTOK;
			}
			if (lineptr[av_count] != NULLCP) {
				(void) sprintf(str, lineptr[av_count]);
			}
		} else if (is_dn(atts->name) == TRUE) {
			(void) sprintf(str, "");
			status = ask_dnhnc(str, atts->name);
			if (status != OK) {
				(void) sprintf(str, quit_String);
			}
		} else if (is_tlx(atts->name) == TRUE) {
			(void) printf("%s (%s, %s %s %s)\n", mapAttName(atts->name), number_str,
						  country_str, and_str, ansBack_str);
			p = malloc(LINESIZE);
			*p = '\0';
			lineptr[av_count] = p;
			status = ask_tlx(atts->name, lineptr[av_count]);
			if (status != OK) {
				(void) sprintf(str, quit_String);
			}
		} else if (!(strcmp(atts->name, "objectClass"))) {
			/* Do nothing */
		} else {
			(void) printf("%s: ", mapAttName(buffer));
			if (gets(str) == NULLCP) {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			} else {
				str = TidyString(str);
			}
		}

		str = TidyString(str);
		if (!(strcmp(str, quit_String))) {
			return NOTOK;
		}
		if (strlen(str) == 1 && index(str, '-') != 0) {
			/* A `-' was entered, delete default value */
			*str = '\0';
			lineptr[av_count] = NULLCP;
		}

		if (strlen(str) == 0 && lineptr[av_count] != NULLCP) {
			/* The default value was taken, copy it */
			(void) strcpy(str, lineptr[av_count]);
		}
		if (strlen(str) != 0) {
			if (!(strcmp(str, "?"))) {
				displayFile(buffer, FALSE);
				continue;
			}
			(void) sprintf(buffer, "%s=%s", buffer,str);
			if ((as4 = str2as(buffer)) == NULLATTR) {
				if (is_postal_address(atts->name) == TRUE) {
					/* It makes sense to save value for line by line editing */
					p = malloc(LINESIZE);
					(void) strcpy(p, str);
					lineptr[av_count] = p;
				}
				(void) printf(inv_value);
				(void) sprintf(buffer, atts->name);
				continue;
			} else if (is_phone_fax(atts->name) == TRUE &&
					   (strlen(str) > CHARS_PHONE)) {
				(void) sprintf(tmp, "%s %d %s\n", inv_length, CHARS_PHONE, characters);
				(void) printf(tmp);
				(void) printf(enter_again);
				(void) sprintf(buffer, atts->name);
				continue;
			} else {
				if (!(strcmp(atts->name, "mhsORAddresses"))) {
					mhsUser = TRUE;
				}
				as1 = as_merge(as1, as4);
			}
		}
		p = malloc(LINESIZE);
		(void) strcpy(p, str);
		lineptr[av_count++] = p;

		if (atts->next != NULL) {
			atts = atts->next;
			(void) sprintf(buffer, atts->name);
		} else {
			(void) strcpy(buffer, quit_String);
		}
	}

	spaces = PADCHARS -  strlen(name_of_entry);
	(void) printf("\n%s %*s - ", name_of_entry, spaces, " ");
	printLastComponent(INDENTOFF, entry_dn, PERSON, 0);
	if (object == PERSON) {
		spaces = PADCHARS -     strlen(full_name);
		(void) printf("%s %*s - %s\n", full_name, spaces, " ", cn);
	}
	/* Print attribute values */
	atts = saveptr;
	i = 0;
	while (i < av_count) {
		if (is_postal_address(atts->name) == TRUE) {
			ptr = lineptr[i];
			mark = lineptr[i];
			line_counter = 1;
			last_line = FALSE;
			while (line_counter <= LINES_PA) {
				/* a $ was found */
				if ((ptr = index(ptr, '$' )) == NULLCP) {
					ptr = index(lineptr[i], '\0');
					last_line = TRUE;
				}
				addr_ptr[line_counter] = malloc(LINESIZE);
				(void) sprintf(addr_ptr[line_counter], "");
				(void) strncpy(addr_ptr[line_counter], "", LINESIZE);
				(void) strncpy(addr_ptr[line_counter], mark, ptr-mark);
				if (line_counter == 1) {
					spaces = PADCHARS - strlen(mapAttName(atts->name));
					(void) printf("%s %*s - %s\n", mapAttName(atts->name), spaces, " ", addr_ptr[line_counter]);
				} else {
					(void) printf("%*s - %s\n", PADCHARS+1, " ", addr_ptr[line_counter]);
				}
				if (last_line == TRUE)
					break;
				ptr++;
				mark = ptr;
				line_counter++;
			}
		} else if (is_dn(atts->name) == TRUE) {
			spaces = PADCHARS - strlen(mapAttName(atts->name));
			(void) printf("%s %*s - ",mapAttName(atts->name), spaces, " ");
			if (tmp_org == TRUE) {
				cp = rindex(lineptr[i], '=');
				if (cp != NULLCP) {
					cp++;
					(void) printf(cp);
				}
			} else {
				display_posdit(PADCHARS+5, lineptr[i]);
			}
		} else if (is_tlx(atts->name) == TRUE) {
			ptr = lineptr[i];
			mark = lineptr[i];
			line_counter = 1;
			last_line = FALSE;
			while (line_counter <= LINES_TLX) {
				/* a $ was found */
				if ((ptr = index(ptr, '$' )) == NULLCP) {
					ptr = index(lineptr[i], '\0');
					last_line = TRUE;
				}
				tlx_ptr[line_counter] = malloc(LINESIZE);
				(void) sprintf(tlx_ptr[line_counter], "");
				(void) strncpy(tlx_ptr[line_counter], "", LINESIZE);
				(void) strncpy(tlx_ptr[line_counter], mark, ptr-mark);
				if (line_counter == 1) {
					spaces = PADCHARS - strlen(mapAttName(atts->name));
					(void) printf("%s %*s - %s\n", mapAttName(atts->name), spaces, " ", tlx_ptr[line_counter]);
				} else {
					(void) printf("%*s - %s\n", PADCHARS+1, " ", tlx_ptr[line_counter]);
				}
				if (last_line == TRUE)
					break;
				ptr++;
				mark = ptr;
				line_counter++;
			}
		} else if (!(strcmp(atts->name, "objectClass"))) {
			/* Don't print */
		} else {
			spaces = PADCHARS - strlen(mapAttName(atts->name));
			(void) printf("%s %*s - %s\n",mapAttName(atts->name), spaces, " ", lineptr[i]);
		}
		atts = atts->next;
		i++;
	}

	(void) printf("\n");

	atts = saveptr;
	de_prompt_yesno(ask_ifallok, all_ok, no_string);
	free(str);

	if (!(strcmp(all_ok, no_string))) {
		free(all_ok);
		free(tmp);
		return NOTOK;
	} else {
		/* Clear attribute values */
		i = 0;
		while (i <= av_count) {
			lineptr[i] = NULLCP;
			i++;
		}
		free(all_ok);
		free(tmp);
		return OK;
	}
}

int dm_addentry() {
	extern int assoc;
	extern int rfrl_msg;
	struct	DSError		error;
	Attr_Sequence as_cpy();
	int status;

	rfrl_msg = TRUE;
	adarg.ada_entry = as_cpy(as1);

add_again:

	rfrl_msg = TRUE;
	status = ds_addentry(&adarg, &error);
	if (status != DS_OK) {
		status = check_error(error);
		if (status == REFERRAL) {
			rfrl_msg = FALSE;
			goto add_again;
		} else {
			(void) printf(add_failure);
			return NOTOK;
		}
	} else {
		(void) printf(done);
		(void) printf("\n");
	}
	rfrl_msg = TRUE;
	return OK;
}

int bind_to_initial_DSA(initial_dsa_address)
char * initial_dsa_address;
{

	int status;

	(void) printf(set_init_DSA);
	dsa_address = malloc(LINESIZE);
	(void) sprintf(dsa_address, initial_dsa_address);
	status = init_bind_to_ds(&assoc);
	if (status != OK) {
		(void) printf(err_init_DSA);
		(void) printf(exiting);
		boundToDSA = FALSE;
		de_exit(-1);
	} else {
		status = wait_bind_to_ds(assoc, TRUE); /* block */
		if (status != OK) {
			(void) printf(err_init_DSA);
			(void) printf(exiting);
			boundToDSA = FALSE;
			de_exit(-1);
		}
	}
	return OK;
}


int de_Add_person() {
	char * already_in;
	char * first_name;
	char * more;
	char * surname;
	char * TidyString();
	void searchFail(), onint1(), de_exit();
	char ask_cn_string[LINESIZE];
	char buffer[LINESIZE];
	char cn_default[LINESIZE];
	char first_name_default[LINESIZE];
	char init_value[LINESIZE];
	char ask_1stname_string[LINESIZE];
	char surname_default[LINESIZE];
	char ask_surname_string[LINESIZE];
	int changed;
	int noPersons;
	int status;
	int i;

	AttributeType	at;
	AttributeValue	av;
	AV_Sequence	avl;
	Attr_Sequence	as2;
	Attr_Sequence	as3;
	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();
	struct acl_info * acl;
	struct namelist *plp = NULLLIST;

	already_in		= malloc(LINESIZE);
	first_name		= malloc(LINESIZE);
	more			= malloc(LINESIZE);
	surname		= malloc(LINESIZE);

	acl = (struct acl_info *) malloc(sizeof(struct acl_info));
	plp = (struct namelist *) malloc(sizeof(struct namelist));
	as2 = as_comp_alloc();
	as2 = NULLATTR;
	as3 = as_comp_alloc();
	as3 = NULLATTR;
	av  = AttrV_alloc();
	avl = NULLAV;
	(void) sprintf(cn_default, "");
	(void) sprintf(surname_default, "");
	(void) sprintf(first_name_default, "");
	(void) sprintf(init_value, "");
	status = OK;

	set_adarg();
	(void) sprintf(more, yes_string);  /* Enter loop */

	while (!(strcmp(more,  yes_string))) {
prompt_surname:
		changed = TRUE;
		if (strlen(surname_default) == 0) {
			(void) sprintf(ask_surname_string,
						   "%s, %s %s",
						   ask_surname, quit_String, quit_add_prr );
		} else {
			(void) sprintf(ask_surname_string,
						   "%s, %s %s [%s]: ",
						   ask_surname,
						   quit_String,
						   quit_add_prr,
						   surname_default);
		}
		de_prompt_value(ask_surname_string, surname);
		if (!(strcmp(surname, quit_String))) {
			(void) strcpy(more, no_string);
			continue;
		}
		if (!(strcmp(surname, "?"))) {
			displayFile("surname", FALSE);
			continue;
		}
		if (strlen(surname) == 0) {
			if (strlen(surname_default) == 0) {
				(void) sprintf(buffer, "%s %s %s\n", enter_sn, quit_String, to_quit);
				(void) printf(buffer);
				goto prompt_surname;
			} else {
				/* copy the default */
				(void) sprintf(surname, surname_default);
				changed = FALSE;
			}
		} else if (strlen(surname) == 1 && index(surname, '-') != 0) {
			/* A `-' was entered, delete default value */
			(void) sprintf(buffer, "%s %s %s\n", enter_sn, quit_String, to_quit);
			(void) sprintf(surname_default, "");
			(void) printf(buffer);
			goto prompt_surname;
		} else {
			(void) sprintf(surname_default, surname);
		}

prompt_1stname:

		if (strlen(first_name_default) == 0) {
			(void) sprintf(ask_1stname_string, "%s: ", ask_1stname);
		} else {
			(void) sprintf(ask_1stname_string, "%s  [%s]: ", ask_1stname, first_name_default);
		}
		de_prompt_value(ask_1stname_string, first_name);
		if (!(strcmp(first_name, quit_String))) {
			goto prompt_surname;
		}
		if (!(strcmp(first_name, "?"))) {
			displayFile("firstname", FALSE);
			goto prompt_1stname;
		}
		if (strlen(first_name) == 0) {
			if (strlen(first_name_default) == 0) {
				(void) sprintf(buffer, "%s %s %s\n", enter_fn, quit_String, to_quit);
				(void) printf(buffer);
				goto prompt_1stname;
			} else {
				/* copy the default */
				(void) sprintf(first_name, first_name_default);
			}
		} else if (strlen(first_name) == 1 && index(first_name, '-') != 0) {
			/* A `-' was entered, delete default value */
			(void) sprintf(buffer, "%s %s %s\n", enter_fn, quit_String, to_quit);
			(void) sprintf(first_name_default, "");
			(void) printf(buffer);
			goto prompt_1stname;
		} else {
			(void) sprintf(first_name_default, first_name);
			changed = TRUE;
		}

		(void) strcpy(person, first_name);
		(void) strcat(person, " ");
		(void) strcat(person, surname);
		if (changed == TRUE) {
			(void) printf(check_notAlrIn);
			(void) fflush(stdout);
			if (listPRRs(posdit, person, &plp) != OK) {
				(void) printf("\n");
				(void) searchFail(person);
				(void) de_exit(-1);
			} else {
				noPersons = listlen(plp);
				if (noPersons <= 0) {
					/* OK */
					(void) printf(done);
				} else {
					(void) printf("\n");
					pagerOn(NUMBER_NOT_ALLOWED);
					printListPRRs(person, plp, ORG, FALSE); /* False = don't print details */
					de_prompt_yesno(ask_ifNotAlrIn, already_in, no_string);
					if (!strcmp(already_in, no_string)) {
						freePRRs(&plp);
						(void) sprintf(cn_default, "");
						(void) sprintf(surname_default, "");
						(void) sprintf(first_name_default, "");
						continue;
					}
				}
			}
			freePRRs(&plp);
		}

prompt_cn:
		if (changed == TRUE) {
			(void) sprintf(cn_default, "%s %s", first_name, surname);
		}

		(void) sprintf(ask_cn_string, "%s [%s]: ", ask_cn, cn_default);

		de_prompt_value(ask_cn_string, cn);
		if (!(strcmp(cn, quit_String))) {
			goto prompt_surname;
		}
		if (!(strcmp(cn, "?"))) {
			displayFile("cn", FALSE);
			goto prompt_cn;
		}
		if (strlen(cn) == 0) {
			/* copy the default */
			(void) sprintf(cn, cn_default);
		} else {
			(void) sprintf(cn_default, cn);
		}
		at = NULLAttrT;
		at = str2AttrT("surname");
		av = NULLAttrV;
		av = str2AttrV(surname, at->oa_syntax);
		avl = avs_comp_new(av);
		acl = NULLACL_INFO;
		as1 = as_comp_new(at, avl, acl);
		if (strlen(cn) == 0) {
			(void) sprintf(buffer, "cn=%s %s", first_name, surname);
		} else {
			(void) sprintf(buffer, "cn=%s", cn);
		}
		if ((as3 = str2as(buffer)) == NULLATTR) {
			(void) printf(fail_add_prrcn);
			status = NOTOK;
			(void) sprintf(more, "%s", no_string);
			continue;
		}
		as1 = as_merge(as1, as3);
		(void) sprintf(buffer, "%s@cn=%s", posdit, person);
		(void) sprintf(entry_dn, buffer);
		adarg.ada_object = str2dn(entry_dn);
		mhsUser = FALSE;  /* May be modified in prompt_atts */
		status = prompt_atts(PERSON, prratts);
		if (status == NOTOK) {
			(void) printf(removeValue);
			continue;
		}
		if (mhsUser == FALSE) {
			(void) sprintf(buffer, "ObjectClass=person&quipuobject&newPilotPerson&organizationalPerson");
		} else {
			(void) sprintf(buffer, "ObjectClass=person&quipuobject&newPilotPerson&organizationalPerson&mhsUser");
		}
		if (strlen(prrOC) > 0) {
			(void) sprintf(buffer, "%s&%s", buffer, prrOC);
		}
		if ((as2 = str2as(buffer)) == NULLATTR) {
			(void) printf(fail_add_prroc);
			status = NOTOK;
			(void) sprintf(more, "%s", no_string);
			continue;
		}
		as1 = as_merge(as1, as2);
		(void) printf("\n%s `%s %s'   %s", add_entry,
					  first_name, surname, please_wait);
		(void) fflush(stdout);
		status = dm_addentry();
		if (status != DS_OK) {
			status = NOTOK;
			(void) sprintf(more, "%s", no_string);
			continue;
		} else {
			(void) sprintf(default_person, "%s %s", first_name, surname);
		}
		(void) sprintf(first_name_default, "");
		(void) sprintf(surname_default, "");
		(void) sprintf(cn_default, "");
	}
	i = 0;
	while (i <= av_count) {
		lineptr[i] = NULLCP;
		i++;
	}

	free(already_in);
	free(first_name);
	free(more);
	free(surname);

	if (status == OK) {
		return OK;
	} else {
		return NOTOK;
	}
}



int de_Add_role() {

	char * already_in;
	char * more;
	char * TidyString();
	void searchFail(), onint1(), de_exit();
	char buffer[LINESIZE];
	char role[LINESIZE];
	char role_default[LINESIZE];
	char ask_role_string[LINESIZE];
	int changed;
	int i;
	int noPersons;
	int status;

	Attr_Sequence	as2;
	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();
	struct namelist *plp = NULLLIST;

	already_in		= malloc(LINESIZE);
	more			= malloc(LINESIZE);

	as2 = as_comp_alloc();
	as2 = NULLATTR;
	(void) sprintf(role_default, "");
	status = OK;

	set_adarg();

	(void) sprintf(more, yes_string);  /* Enter loop */
	while (!(strcmp(more, yes_string))) {
		changed = TRUE;
		if (strlen(role_default) == 0) {
			(void) sprintf(ask_role_string, "%s: ", ask_role);
		} else {
			(void) sprintf(ask_role_string, "%s  [%s]: ", ask_role, role_default);
		}
		de_prompt_value(ask_role_string, role);
		if (!(strcmp(role, quit_String))) {
			(void) strcpy(more, no_string);
			(void) sprintf(role_default, "");
			continue;
		}
		if (!(strcmp(role, "?"))) {
			displayFile("role", FALSE);
			continue;
		}
		if (strlen(role) == 0) {
			if (strlen(role_default) == 0) {
				(void) sprintf(buffer, "%s %s %s \n", enter_role, quit_String, to_quit);
				(void) printf(buffer);
				continue;
			} else {
				/* copy the default */
				(void) sprintf(role, role_default);
				changed = FALSE;
			}
		} else if (strlen(role) == 1 && index(role, '-') != 0) {
			/* A `-' was entered, delete default value */
			(void) sprintf(buffer, "%s %s %s\n", enter_role, quit_String, to_quit);
			(void) sprintf(role_default, "");
			(void) printf(buffer);
			continue;
		} else {
			/* save new default */
			(void) sprintf(role_default, role);
		}

		(void) strcpy(person, role);

		if (changed == TRUE) {
			(void) printf(check_notAlrIn);
			(void) fflush(stdout);
			if (listPRRs(posdit, person, &plp) != OK) {
				(void) printf("\n");
				(void) searchFail(person);
				(void) de_exit(-1);
			} else {
				noPersons = listlen(plp);
				if (noPersons <= 0) {
					/* OK */
					(void) printf(done);
				} else {
					(void) printf("\n");
					pagerOn(NUMBER_NOT_ALLOWED);
					printListPRRs(person, plp, ORG, FALSE); /* False = don't print details */
					de_prompt_yesno(ask_ifNotAlrIn, already_in, no_string);
					if (!strcmp(already_in, no_string)) {
						freePRRs(&plp);
						(void) sprintf(role_default, "");
						continue;
					}
				}
			}
			freePRRs(&plp);
		}
		(void) sprintf(buffer, "cn=%s", role);
		if ((as1 = str2as(buffer)) == NULLATTR) {
			(void) printf(fail_add_rl);
			(void) sprintf(more, "%s", no_string);
			status = NOTOK;
			continue;
		}
		(void) sprintf(buffer, "%s@cn=%s", posdit, role);
		(void) sprintf(entry_dn, buffer);
		adarg.ada_object = str2dn(entry_dn);
		mhsUser = FALSE;  /* may be modified in prompt_atts */
		status = prompt_atts(ROLE, rlatts);
		if (status == NOTOK) {
			continue;
		}
		if (mhsUser == FALSE) {
			(void) sprintf(buffer, "ObjectClass=quipuobject&organizationalRole");
		} else {
			(void) sprintf(buffer, "ObjectClass=quipuobject&organizationalRole&mhsUser");
		}
		if (strlen(rlOC) > 0) {
			(void) sprintf(buffer, "%s&%s", buffer, rlOC);
		}
		if ((as2 = str2as(buffer)) == NULLATTR) {
			(void) printf(fail_add_rloc);
			(void) sprintf(more, "%s", no_string);
			status = NOTOK;
			continue;
		}
		as1 = as_merge(as1, as2);
		(void) printf("\n%s `%s'   %s ", add_entry, role, please_wait);
		(void) fflush(stdout);
		status = dm_addentry();
		if (status != DS_OK) {
			(void) sprintf(more, "%s", no_string);
			status = NOTOK;
			continue;
		} else {
			(void) sprintf(default_person, role);
		}
		(void) sprintf(role_default, "");
	}
	i = 0;
	while (i <= av_count) {
		lineptr[i] = NULLCP;
		i++;
	}

	free(already_in);
	free(more);

	if (status == OK) {
		return OK;
	} else {
		return NOTOK;
	}

}


int de_Add_room() {
	char * already_in;
	char * more;
	char * TidyString();
	void searchFail(), onint1(), de_exit();
	char buffer[LINESIZE];
	char room[LINESIZE];
	char room_default[LINESIZE];
	char ask_room_string[LINESIZE];

	int changed;
	int i;
	int noPersons;
	int status;

	Attr_Sequence	as2;
	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();
	struct namelist *plp = NULLLIST;

	already_in		= malloc(LINESIZE);
	more			= malloc(LINESIZE);

	(void) sprintf(room_default, "");
	as2 = as_comp_alloc();
	as2 = NULLATTR;
	(void) sprintf(ask_ifNotAlrIn, "%s", sure_not_in);
	status = OK;

	set_adarg();

	/* room */
	(void) sprintf(more, yes_string);  /* Enter loop */
	while (!(strcmp(more, yes_string))) {
		changed = TRUE;
		if (strlen(room_default) == 0) {
			(void) sprintf(ask_room_string, "%s: ", ask_room);
		} else {
			(void) sprintf(ask_room_string, "%s  [%s]: ", ask_room, room_default);
		}
		de_prompt_value(ask_room_string, room);
		if (!(strcmp(room, quit_String))) {
			(void) strcpy(more, no_string);
			(void) sprintf(room_default, "");
			continue;
		}
		if (!(strcmp(room, "?"))) {
			displayFile("roomNumber", FALSE);
			continue;
		}
		if (strlen(room) == 0) {
			if (strlen(room_default) == 0) {
				(void) sprintf(buffer, "%s %s %s \n", enter_room, quit_String, to_quit);
				(void) printf(buffer);
				continue;
			} else {
				/* copy the default */
				(void) sprintf(room, room_default);
				changed = FALSE;
			}
		} else if (strlen(room) == 1 && index(room, '-') != 0) {
			/* A `-' was entered, delete default value */
			(void) sprintf(buffer, "%s %s %s\n", enter_room, quit_String, to_quit);
			(void) sprintf(room_default, "");
			(void) printf(buffer);
			continue;
		} else {
			/* save new default */
			(void) sprintf(room_default, room);
		}
		(void) strcpy(person, room);

		if (changed == TRUE) {
			(void) printf(check_notAlrIn);
			(void) fflush(stdout);
			if (listPRRs(posdit, person, &plp) != OK) {
				(void) printf("\n");
				(void) searchFail(person);
				(void) de_exit(-1);
			} else {
				noPersons = listlen(plp);
				if (noPersons <= 0) {
					/* OK */
					(void) printf(done);
				} else {
					(void) printf("\n");
					pagerOn(NUMBER_NOT_ALLOWED);
					printListPRRs(person, plp, ORG, FALSE); /* False = don't print details */
					de_prompt_yesno(ask_ifNotAlrIn, already_in, no_string);
					if (!strcmp(already_in, no_string)) {
						freePRRs(&plp);
						(void) sprintf(room_default, "");
						continue;
					}
				}
			}
			freePRRs(&plp);
		}
		(void) sprintf(buffer, "cn=%s", room);
		if ((as1 = str2as(buffer)) == NULLATTR) {
			(void) printf(fail_add_rm);
			(void) sprintf(more, "%s", no_string);
			status = NOTOK;
			continue;
		}
		(void) sprintf(buffer, "%s@cn=%s", posdit, room);
		(void) sprintf(entry_dn, buffer);
		adarg.ada_object = str2dn(entry_dn);
		mhsUser = FALSE;  /* May be modified in prompt_atts */
		status = prompt_atts(ROOM, rmatts);
		if (status == NOTOK)
			continue;
		if (mhsUser == FALSE) {
			(void) sprintf(buffer, "ObjectClass=quipuobject&room");
		} else {
			(void) sprintf(buffer, "ObjectClass=quipuobject&room&mhsUser");
		}
		if (strlen(rmOC) > 0) {
			(void) sprintf(buffer, "%s&%s", buffer, rmOC);
		}
		if ((as2 = str2as(buffer)) == NULLATTR) {
			(void) printf(fail_add_rmoc);
			(void) sprintf(more, "%s", no_string);
			status = NOTOK;
			continue;
		}
		as1 = as_merge(as1, as2);
		(void) printf("\n%s `%s'   %s ", add_entry, room, please_wait);
		(void) fflush(stdout);
		status = dm_addentry();
		if (status != OK) {
			(void) sprintf(more, "%s", no_string);
			status = NOTOK;
			continue;
		} else {
			(void) sprintf(default_person, room);
		}
		(void) sprintf(room_default, "");
	}
	i = 0;
	while (i <= av_count) {
		lineptr[i] = NULLCP;
		i++;
	}

	free(already_in);
	free(more);

	return OK;
}

int set_adarg() {
	adarg.ada_common.ca_servicecontrol.svc_options = SVC_OPT_CHAININGPROHIBIT;
	adarg.ada_common.ca_servicecontrol.svc_prio = SVC_PRIO_MED;
	adarg.ada_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	adarg.ada_common.ca_servicecontrol.svc_sizelimit = SVC_NOSIZELIMIT;
	adarg.ada_common.ca_servicecontrol.svc_scopeofreferral = SVC_REFSCOPE_COUNTRY;
	adarg.ada_common.ca_servicecontrol.svc_tmp = NULL;
	adarg.ada_common.ca_servicecontrol.svc_len = 0;
	adarg.ada_common.ca_requestor = NULL;
	adarg.ada_common.ca_progress.op_resolution_phase = OP_PHASE_NOTDEFINED;
	adarg.ada_common.ca_progress.op_nextrdntoberesolved = 0;
	adarg.ada_common.ca_aliased_rdns = NULL;
	adarg.ada_common.ca_security = (struct security_parms *)NULL;
	adarg.ada_common.ca_sig = (struct signature *) NULL;
	adarg.ada_common.ca_extensions = (struct extension *) NULL;
}

int check_error(error)
struct	DSError		error;
{
	static int first_time = TRUE;
	int status;
	int wait_result;
	static char * initial_dsa_address;
	char * temp_dsa_address;

	temp_dsa_address = malloc(LINESIZE);

	if (error.dse_type == DSE_ATTRIBUTEERROR ) {
		/* Attribute Error */
		if (error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what == DSE_AT_NOSUCHATTRIBUTE) {
			(void) printf(err_noSuchAttV);
		} else if (error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what == DSE_AT_INVALIDATTRIBUTESYNTAX) {
			(void) printf(err_invAtSntx);
		} else if (error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what == DSE_AT_UNDEFINEDATTRIBUTETYPE) {
			(void) printf(err_undAtType);
		} else if (error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what == DSE_AT_INAPPROPRIATEMATCHING) {
			(void) printf(err_inMtch);
		} else if (error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what == DSE_AT_CONSTRAINTVIOLATION) {
			(void) printf(err_cnstVio);
		} else if (error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what == DSE_AT_TYPEORVALUEEXISTS) {
			(void) printf(err_tVExist);
		} else {
			(void) printf("%s %d\n", err_atEN, error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what);
		}
		return NOTOK;
	} else if (error.dse_type == DSE_NAMEERROR ) {
		/* name error */
		(void) printf("%s %d\n", err_nEN, error.dse_un.dse_un_name.DSE_na_problem);
		(void) printf("\n");
		return NOTOK;
	} else if (error.dse_type == DSE_SERVICEERROR ) {
		/* service errors */
		(void) printf(err_tPS);
		if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_BUSY ) {
			(void) printf(err_sBusy);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_UNAVAILABLE ) {
			(void) printf(err_sUnav);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_UNWILLINGTOPERFORM ) {
			(void) printf(err_uPrfrm);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_CHAININGREQUIRED ) {
			(void) printf(err_chRqrd);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_UNABLETOPROCEED ) {
			(void) printf(err_unblPrcd);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_INVALIDREFERENCE ) {
			(void) printf(err_invRfrnc);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_TIMELIMITEXCEEDED ) {
			(void) printf(err_tmLmtXcd);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_ADMINLIMITEXCEEDED ) {
			(void) printf(err_admLmtXcd);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_LOOPDETECT ) {
			(void) printf(err_loopDtctd);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_UNAVAILABLECRITICALEXTENSION ) {
			(void) printf(err_unvCriExt);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_OUTOFSCOPE ) {
			(void) printf(err_outScope);
		} else if (error.dse_un.dse_un_service.DSE_sv_problem == DSE_SV_DITERROR ) {
			(void) printf(err_ditError);
		} else {
			(void) printf("%s = %d \n", err_errCode, error.dse_un.dse_un_service.DSE_sv_problem);
		}
		return NOTOK;
	} else if (error.dse_type == DSE_REFERRAL ) {
		/* referral error */
		if (first_time) {
			first_time= FALSE;
			initial_dsa_address = malloc(LINESIZE);
			(void) sprintf(initial_dsa_address, dsa_address);
		}
		(void) sprintf(temp_dsa_address,
					   pa2str(error.dse_un.dse_un_referral.DSE_ref_candidates->cr_accesspoints->ap_address));
		(void) de_unbind();
		/*          (void) printf("Current DSA is not the master for this entry\n"); */
		/*	  (void) printf("\nAttempting connection to master data in progress, ...please wait\n\n"); */
		dsa_address = malloc(LINESIZE);
		(void) sprintf(dsa_address, temp_dsa_address);
		status = init_bind_to_ds(&assoc);
		if (status != OK) {
			(void) printf(err_newDSANotA);
			status = bind_to_initial_DSA(initial_dsa_address);
		} else {
			wait_result = wait_bind_to_ds(assoc, TRUE); /* block */
			if (wait_result != OK) {
				(void) printf(err_bindNewDSA);
				status = bind_to_initial_DSA(initial_dsa_address);
			} else {
				/*                  (void) printf("Successful bind to new DSA \n"); */
				return REFERRAL;
			}
		}
	} else if (error.dse_type == DSE_ABANDONED) {
		/* Abandoned (probably the user typed cntrl-C) */
		(void) printf(err_abandoned);
		return NOTOK;
	} else if (error.dse_type == DSE_SECURITYERROR) {
		/* security problem */
		if (error.dse_un.dse_un_security.DSE_sc_problem == DSE_SC_AUTHENTICATION) {
			(void) printf(err_authProb);
			return NOTOK;
		} else if (error.dse_un.dse_un_security.DSE_sc_problem == DSE_SC_INVALIDCREDENTIALS) {
			(void) printf(err_invCred);
			return NOTOK;
		} else if (error.dse_un.dse_un_security.DSE_sc_problem = DSE_SC_ACCESSRIGHTS) {
			(void) printf(err_acRgtsProb);
			return NOTOK;
		} else {
			(void) printf("%s %d\n", err_secProbNo, error.dse_un.dse_un_security.DSE_sc_problem);
			return NOTOK;
		}
	} else if (error.dse_type == DSE_ABANDON_FAILED) {
		/* abandon problem */
		(void) printf(err_abndnProb);
		if (error.dse_un.dse_un_abandon_fail.DSE_ab_problem == DSE_AB_TOOLATE) {
			(void) printf(err_2lateAbndn);
			return NOTOK;
		} else if (error.dse_un.dse_un_abandon_fail.DSE_ab_problem == DSE_AB_CANNOTABANDON) {
			(void) printf(err_canNotAbndn);
			return NOTOK;
		} else {
			(void) printf(err_noSuchOper);
			return NOTOK;
		}
	} else if (error.dse_type == DSE_UPDATEERROR) {
		/* update problem */
		if (error.dse_un.dse_un_update.DSE_up_problem == DSE_UP_NAMINGVIOLATION) {
			(void) printf(err_namVioProb);
			(void) printf(err_chkTreStr);
			return NOTOK;
		} else if (error.dse_un.dse_un_update.DSE_up_problem == DSE_UP_OBJECTCLASSVIOLATION) {
			(void) printf(err_objClsVio);
			(void) printf(err_rprtHlpdsk);
			return NOTOK;
		} else if (error.dse_un.dse_un_update.DSE_up_problem == DSE_UP_ALREADYEXISTS) {
			(void) printf(err_entAlrExst);
			(void) printf(err_difEnt1);
			(void) printf(err_difEnt2);
			return NOTOK;
		} else {
			(void) printf("%s %d \n", err_updProbNo, error.dse_un.dse_un_update.DSE_up_problem);
			return NOTOK;
		}
	} else if (error.dse_type == DSE_LOCALERROR) {
		/* local error in the dua */
		(void) printf(err_idmErr1);
		(void) printf(err_idmErr2);
	} else if (error.dse_type == DSE_REMOTEERROR) {
		/* remote error in the dsa */
		(void) printf(err_dsaErr1);
		(void) printf(err_dsaErr2);
		(void) de_exit(-1);
	} else {
		/* Error is something different */
		(void) printf("%s %d \n", err_unknown, error.dse_type);
		return NOTOK;
	}
	return NOTOK;
}

int dm_Add_org() {

	void searchFail(), onint1(), de_exit();
	char buffer[LINESIZE];
	char dir_mgr[LINESIZE];
	char iattr[LINESIZE];

	int i;
	int status;

	AttributeType	at;
	AttributeValue	av;
	AV_Sequence	avl;
	Attr_Sequence	as2;
	Attr_Sequence  as3;
	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();
	struct acl_info * acl_old;
	struct acl * acl_entry;
	struct acl_info *ac;

	/* as1 = as_comp_alloc();
	 as2 = as_comp_alloc();
	 as3 = as_comp_alloc();
	 as1 = NULLATTR;
	 as2 = NULLATTR;
	 as3 = NULLATTR; */
	set_adarg();

	(void) sprintf(buffer, "o=%s", org);
	if ((as1 = str2as(buffer)) == NULLATTR) {
		(void) printf(fail_addOrg);
		return NOTOK;
	}
	(void) sprintf(buffer, "%s@o=%s", addorg_posdit, org);
	(void) sprintf(entry_dn, buffer);
	(void) sprintf(dir_mgr, "%s@cn=%s", buffer, dir_Mngr);
	adarg.ada_object = str2dn(entry_dn);
	mhsUser = FALSE;  /* May be modified in prompt_atts */
	status = prompt_atts(ORG, orgatts);
	if (status == NOTOK) {
		return NOTOK;
	}
	if (mhsUser == FALSE) {
		(void) sprintf(buffer, "ObjectClass=top&quipuobject&organization&quipuNonLeafObject");
	} else {
		(void) sprintf(buffer, "ObjectClass=top&quipuObject&organization&mhsUser&quipuNonLeafObject");
	}

	if ((as2 = str2as(buffer)) == NULLATTR) {
		(void) printf(fail_addOrgOC);
		return NOTOK;
	}
	as1 = as_merge(as1, as2);

	if (strlen(masterDSA) <= 0) {
		(void) printf("%s   ...%s", mstr_rqrd, exiting);
		return NOTOK;
	}

	(void) sprintf(buffer, "%s %s", "masterDSA=", masterDSA);

	as2 = NULLATTR;
	if ((as2 = str2as(buffer)) == NULLATTR) {
		(void) printf(fail_mstrDSA);
		return NOTOK;
	}
	as1 = as_merge(as1, as2);

	(void) sprintf(buffer, "%s", "acl= self # write # child");
	(void) sprintf(buffer, "%s\n%s", buffer, "acl= others # read # child");
	(void) sprintf(buffer, "%s\n%s%s%s", buffer, "acl= group # ", dir_mgr, " # write # child");
	(void) sprintf(buffer, "%s\n%s", buffer, "acl= self # write # entry");
	(void) sprintf(buffer, "%s\n%s", buffer, "acl= others # read # entry");
	(void) sprintf(buffer, "%s\n%s%s%s", buffer, "acl= group # ", dir_mgr, " # write # entry");
	(void) sprintf(buffer, "%s\n%s", buffer, "acl= self # write # default");
	(void) sprintf(buffer, "%s\n%s", buffer, "acl= others # read # default");
	(void) sprintf(buffer, "%s\n%s%s%s", buffer, "acl= group # ", dir_mgr, " # write # default");
	(void) sprintf(iattr, "top $ DEFAULT (");
	(void) sprintf(iattr, "%s\n%s\n%s", iattr, buffer,")");
	/* at = NULLAttrT; */
	at = str2AttrT("inheritedAttribute");
	/* av = AttrV_alloc(); */
	/* av = NULLAttrV; */
	av = str2AttrV(iattr, at->oa_syntax);
	/* avl = NULLAV; */
	avl = avs_comp_new(av);
	acl_old = NULLACL_INFO;
	as3 = as_comp_new(at, avl, acl_old);
	as1 = as_merge(as1, as3);

	/* at = NULLAttrT; */
	at = str2AttrT("acl");

	acl_entry = acl_alloc();
	acl_entry->ac_child = acl_info_alloc();
	acl_entry->ac_child->acl_categories = ACL_READ;
	acl_entry->ac_child->acl_selector_type = ACL_OTHER;
	acl_entry->ac_child->acl_name = NULLDNSEQ;
	acl_entry->ac_child->acl_next = ac = acl_info_alloc();
	ac->acl_categories = ACL_WRITE;
	ac->acl_selector_type = ACL_GROUP;
	ac->acl_name = dn_seq_alloc();
	ac->acl_name->dns_dn = dn_comp_alloc();
	ac->acl_name->dns_dn = str2dn(init_user);
	ac->acl_name->dns_next = dn_seq_alloc();
	ac->acl_name->dns_next->dns_dn = str2dn(dir_mgr);
	ac->acl_name->dns_next->dns_next = NULLDNSEQ;
	acl_entry->ac_entry = acl_info_alloc();
	acl_entry->ac_entry->acl_categories = ACL_READ;
	acl_entry->ac_entry->acl_selector_type = ACL_OTHER;
	acl_entry->ac_entry->acl_name = NULLDNSEQ;
	acl_entry->ac_entry->acl_next = ac = acl_info_alloc();
	ac->acl_categories = ACL_WRITE;
	ac->acl_selector_type = ACL_GROUP;
	ac->acl_name = dn_seq_alloc();
	ac->acl_name->dns_dn = str2dn(init_user);
	ac->acl_name->dns_next = dn_seq_alloc();
	ac->acl_name->dns_next->dns_dn = str2dn(dir_mgr);
	ac->acl_name->dns_next->dns_next = NULLDNSEQ;
	acl_entry->ac_default = acl_info_alloc();
	acl_entry->ac_default->acl_categories = ACL_READ;
	acl_entry->ac_default->acl_selector_type = ACL_OTHER;
	acl_entry->ac_default->acl_name = NULLDNSEQ;
	acl_entry->ac_default->acl_next = ac = acl_info_alloc();
	ac->acl_categories = ACL_WRITE;
	ac->acl_selector_type = ACL_GROUP;
	ac->acl_name = dn_seq_alloc();
	ac->acl_name->dns_dn = str2dn(init_user);
	ac->acl_name->dns_next = dn_seq_alloc();
	ac->acl_name->dns_next->dns_dn = str2dn(dir_mgr);
	ac->acl_name->dns_next->dns_next = NULLDNSEQ;
	acl_entry->ac_attributes = NULLACL_ATTR;

	av = AttrV_alloc();
	av->av_syntax  = at->oa_syntax;
	av->av_struct  = (caddr_t) acl_entry;
	avl = avs_comp_new(av);
	acl_old = NULLACL_INFO;
	/* as3 = NULLATTR; */
	as3 = as_comp_new(at, avl, acl_old);

	as1 = as_merge(as1, as3);

	(void) printf("\n%s `%s'   %s... ", add_org, org, please_wait);
	(void) fflush(stdout);
	status = dm_addentry();
	if (status != OK) {
		return NOTOK;
	}
	(void) sprintf(posdit, "%s", addorg_posdit);
	(void) sprintf(posdit, "%s@o=%s", posdit, org);
	status = dm_Add_mgr();
	if (status != OK) {
		return NOTOK;
	}

	i = 0;
	while (i <= av_count) {
		lineptr[i] = NULLCP;
		i++;
	}
	if (status != OK) {
		return NOTOK;
	}
	return OK;
}

int dm_Add_mgr() {

	extern char password[];

	void searchFail(), onint1(), de_exit();
	char buffer[LINESIZE];
	char pswd_local[LINESIZE];
	char role[LINESIZE];
	int status;

	Attr_Sequence	as2;
	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();

	as1 = as_comp_alloc();
	as1 = NULLATTR;
	as2 = as_comp_alloc();
	as2 = NULLATTR;
	status = OK;

	set_adarg();

	(void) sprintf(role, dir_Mngr);
	(void) sprintf(buffer, "cn=%s", role);
	if ((as1 = str2as(buffer)) == NULLATTR) {
		(void) printf(fail_addMngr);
		return NOTOK;
	}
	(void) sprintf(buffer, "%s@cn=%s", posdit, role);
	(void) sprintf(entry_dn, buffer);
	adarg.ada_object = str2dn(entry_dn);
	mhsUser = FALSE;  /* may be modified in prompt_atts */
	(void) sprintf(buffer, "ObjectClass=quipuobject&organizationalRole&simpleSecurityObject");
	if (strlen(rlOC) > 0) {
		(void) sprintf(buffer, "%s&%s", buffer, rlOC);
	}
	if ((as2 = str2as(buffer)) == NULLATTR) {
		(void) printf(fail_addMngrOC);
		return NOTOK;
	}
	as1 = as_merge(as1, as2);
	as2 = NULLATTR;
	status = ask_password_entry(pswd_local);
	(void) sprintf(password, "%s", pswd_local); /* for rebinding */
	(void) sprintf(buffer, "userPassword=%s", password);
	if ((as2 = str2as(buffer)) == NULLATTR) {
		(void) printf(fail_addMngrUP);
		return NOTOK;
	}
	as1 = as_merge(as1, as2);
	(void) printf("%s   %s... ", add_entMngr, please_wait);
	(void) fflush(stdout);
	status = dm_addentry();
	if (status != DS_OK) {
		status = NOTOK;
	} else {
		(void) sprintf(default_person, role);
	}

	if (status == OK) {
		return OK;
	} else {
		return NOTOK;
	}

}
