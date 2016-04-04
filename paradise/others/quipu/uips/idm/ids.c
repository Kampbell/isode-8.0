#include "bind.h"
#include "extmessages.h"
#include "demanifest.h"
#include "quipu/util.h"
#include "namelist.h"
extern char no_string[];
extern int assoc;
extern int boundToDSA;
extern char change_posdit[];
extern char change_posdn[];
extern char org_compel[];
extern char search_mgr[];
extern char default_person[];
extern char default_country[];
extern char default_organisation[];
extern char default_department[];
extern char co[];
extern char org[];
extern char ou[];
extern char person[];
extern char yes_string[];
extern char no_string[];
extern int assoc;
extern char password[];
extern char username[];
extern char posdit[];
extern char addorg[];
extern char addorg_posdit[];
extern int pswd_intrctv;
extern int username_intrctv;
extern int highNumber;
extern int posdit_oc;
extern int tmp_org;
extern int orgEntered;
extern int quipuMastersCo;

char * add_entries;
int tmp_org_found;

int check_credentials() {

	void de_exit();
	char * getpass();
	char * temp2;
	int pswd_tries;
	int status;
	int username_tries;
	int wait_result;
	pswd_tries = 1;
	status = 0;
	add_entries = malloc(LINESIZE);
	temp2 = malloc(LINESIZE);
	username_tries = 1;

username_prompt:

	if (!(strcmp(username, "interactive"))) {
		sprintf(username, "");
		username[0] = 0;
		printf("%s   %s", connect_idm, please_wait);
		fflush(stdout);
		status = init_bind_to_ds(&assoc); /* Bind no authentication mode */
		if (status != OK) {
			de_exit(-1);
		}
		wait_result = wait_bind_to_ds(assoc, TRUE); /* block */
		if (wait_result != OK) {
			printf("\n%s", srvr_unava);
			de_exit(-1);
		} else {
			printf(done);
		}

		status = get_username();
		if (status != OK) {
			return NOTOK;
		}
		if (!(strcmp(add_entries, yes_string))) {
			if (tmp_org_found == TRUE) {
				de_unbind();
				pswd_intrctv = TRUE;
				if (status != OK) {
					de_exit(-1);
				}
				goto prompt_pswd;
			} else {
				de_unbind();
				pswd_intrctv = FALSE;
				status = init_bind_to_ds(&assoc);
				if (status != OK) {
					de_exit(-1);
				}
				return OK;
			}
		}
		if (username == quit_String) {
			de_exit(-1);
		}
		de_unbind();  /* Unbind from no authentication mode */
	}

prompt_pswd:
	if (pswd_intrctv == TRUE) {
		wait_result = NOTOK; /* Enter loop */
		while (wait_result != OK) {
			sprintf(temp2, "%s :  ", ask_password);
			sprintf(password, getpass(temp2));

			if (strlen(password) <= 0) {
				if (pswd_tries < PSWD_TRY_LIMIT) {
					printf(pswd_must_enter);
					pswd_tries ++;
					goto prompt_pswd;
				} else {
					printf(pswd_lmt_xcd);
					de_exit(-1);
				}
			} else if (!(strcmp(password, "?"))) {
				displayFile("userPassword", FALSE);
				goto prompt_pswd;
			}
			status = init_bind_to_ds(&assoc);
			if (status != OK) {
				de_exit(-1);
			}

			wait_result = wait_bind_to_ds(assoc, TRUE); /* block */
			if (wait_result == INV_PSWD) {
				if (pswd_tries < PSWD_TRY_LIMIT) {
					printf(pswd_tryagain);
					pswd_tries ++;
					sprintf(password, "interactive");
				} else {
					printf(pswd_lmt_xcd);
					de_exit(-1);
				}
			} else if (wait_result == NOTOK) {
				printf("%s   ...%s\n", nobind_user, exiting);
				de_exit(-1);
			}
		}
	} else if (pswd_intrctv == FALSE) {
		status = init_bind_to_ds(&assoc);
		if (status == INV_USERNAME) {
			if (username_intrctv == TRUE) {
				if (username_tries >= USERNAME_TRY_LIMIT) {
					printf(errmsg_invun);
					printf(username_lmt_xcd);
					de_exit(-1);
				}
				username_tries ++;
				printf(errmsg_invun);
				sprintf(username, "interactive");
				goto username_prompt;
			}
		} else if (status != OK) {
			de_exit(-1);
		}
	}

	free(temp2);
	return OK;
}

int just_bind() {

	int status;
	int wait_result;

	printf("%s   %s", connect_idm, please_wait);
	fflush(stdout);
	status = init_bind_to_ds(&assoc); /* Bind no authentication mode */
	if (status != OK) {
		de_exit(-1);
	}
	wait_result = wait_bind_to_ds(assoc, TRUE); /* block */
	if (wait_result != OK) {
		printf("\n%s", srvr_unava);
		return NOTOK;
	} else {
		printf(done);
	}
	return OK;
}

int get_username() {

	struct namelist * clp  = NULLLIST;
	struct namelist * olp  = NULLLIST;
	struct namelist * oulp = NULLLIST;
	struct namelist * plp  = NULLLIST;

	FILE * dn_file;

	int noCos;
	int noOrgs;
	int noOUs;
	int noPersons;
	int objectType;
	int status;
	void de_exit();
	char * add_org_ids;
	char * already_in;
	char * confirm_out;
	char * cp;
	char * full_c;
	char * mapCoName();
	char * str;
	char * tmp_search;
	char addthisorg[LINESIZE];
	char buffer[LINESIZE];
	char posdit_user[LINESIZE];
	char username_save[LINESIZE];
	char save_addorg_posdit[LINESIZE];
	extern char have_department[];
	extern char just_dn[];
	void searchFail();
	void onint1();
	void exit();
	Attr_Sequence at;

	add_org_ids = malloc(LINESIZE);
	already_in	= malloc(LINESIZE);
	confirm_out = malloc(LINESIZE);
	cp		= malloc(LINESIZE);
	full_c	= malloc(LINESIZE);
	str         = malloc(LINESIZE);
	tmp_search	= malloc(LINESIZE);
	orgEntered  = FALSE;
	tmp_org_found = FALSE;

	printf(enter_country);

	strcpy(username_save, username);

prompt_country:
	enterString(COUNTRY, co, clp);
	if (strlen(co) == 0) {
		if (!(strcmp(default_country, ""))) {
			printf("%s %s %s\n", reenter_c, quit_String, to_quit);
			goto prompt_country;
		} else {
			strcpy(co, default_country);
		}
	} else if (!(strcmp(co, quit_String))) {
		de_prompt_yesno(ask_ifout, confirm_out, no_string);
		if (!(strcmp(confirm_out, yes_string))) {
			de_exit(-1);
		} else {
			highNumber = 0;
			goto prompt_country;
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
		sprintf(posdit_user, "@%s", clp->name);
		str = copy_string(lastComponent(clp->name, COUNTRY));
		strcpy(default_country, str);
		sprintf(full_c, "%s -- %s", default_country, mapCoName(default_country));
		printf("\n%s\n", full_c);
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
				goto prompt_person;
			}
		} else {
			strcpy(org, default_organisation);
		}
	} else {
		if (!(strcmp(org, quit_String))) {
			de_prompt_yesno(ask_ifout, confirm_out, no_string);
			if (!(strcmp(confirm_out, yes_string))) {
				de_exit(-1);
			} else {
				highNumber = 0;
				pagerOn(NUMBER_NOT_ALLOWED);
				goto prompt_country;
			}
		}
	}
	if (listOrgs(posdit_user, org, &olp) != OK) {
		searchFail(org);
		de_exit(-1);
	}
	noOrgs = listlen(olp);
	if (noOrgs <= 0) {
		printf("%s `%s' \n\n", no_orgMatch, org);
		if (!(strcmp(addorg, yes_string))) {
			printf("%s   %s", srch_newOrg, please_wait);
			fflush(stdout);
			sprintf(save_addorg_posdit, "%s", addorg_posdit);
			sprintf(addorg_posdit, "%s@%s", addorg_posdit, clp->name);
			if (listOrgs(addorg_posdit, org, &olp) != OK) {
				printf("%s `%s'\n", unbl_newOrgC, full_c);
				printf(cntct_hlpdsk);
				de_exit(-1);
			} else {
				printf(done);
			}
			noOrgs = listlen(olp);
			if ( noOrgs <= 0 ) {
				sprintf(addthisorg, "%s`%s'?", ask_ifaddorg, org);
				printf("\n%s `%s' \n\n", no_orgMatch, org);
				de_prompt_yesno(addthisorg, add_org_ids, no_string);
				if (!(strcmp(add_org_ids, yes_string))) {
					displayFile("neworg_confirm", FALSE);
					sprintf(posdit, "%s", addorg_posdit);
prompt_name_org:
					sprintf(buffer, newOrgName1);
					printf(buffer);
					sprintf(buffer, "%s %s %s\n[%s]  : ", newOrgName2,
							quit_String, to_quit, org);
					printf(buffer);
					if (gets(cp) == NULLCP) {
						/* behave as for an interrupt */
						clearerr(stdin);
						onint1();
					}
					if (!(strcmp(cp, quit_String))) {
						de_exit(-1);
					} else if (strlen(cp) > 0) {
						sprintf(org, "%s", cp);
						printf("%s   ", check_notAlrIn);
						fflush(stdout);
						sprintf(addorg_posdit, "%s@%s", save_addorg_posdit, clp->name);
						if (listOrgs(addorg_posdit, org, &olp) != OK) {
							printf("%s `%s'\n", unbl_newOrgC, full_c);
							printf(cntct_hlpdsk);
							de_exit(-1);
						} else {
							printf(done);
						}
						noOrgs = listlen(olp);
						if ( noOrgs <= 0 ) {
							/* Everything OK */
						} else {
							printf("\n");
							printListOrgs(org, olp);
							de_prompt_yesno(ask_ifNotAlrIn, already_in, no_string);
							if (!strcmp(already_in, no_string)) {
								freeOrgs(&olp);
								goto prompt_org;
							}
						}
					}
					status = dm_Add_org();
					if (status != OK) {
						goto prompt_name_org;
					}
					tmp_org = TRUE;
					de_prompt_yesno(ask_ifaddent, add_entries, yes_string);
					if (!(strcmp(add_entries, yes_string))) {
						sprintf(change_posdit, "%s", no_string);
						sprintf(change_posdn, "%s", no_string);
						sprintf(username, "%s@cn=%s", posdit, dir_Mngr);
						orgEntered = TRUE;
						return OK;
					} else {
						displayFile("neworg_added", FALSE);
						de_exit(-1);
					}
				} else {
					sprintf(addorg_posdit, "%s", save_addorg_posdit);
					goto prompt_org;
				}
			} else if (noOrgs == 1) {
				str = copy_string(lastComponent(olp->name, ORG));
				sprintf(default_organisation, str);
				printf("\n%s\n", default_organisation);
				sprintf(posdit_user, "%s", olp->name);
				sprintf(org, "%s", str);
				orgEntered = TRUE;
				sprintf(posdit, "%s", addorg_posdit);
				sprintf(posdit, "%s@o=%s", posdit, str);
				sprintf(change_posdit, "%s", no_string);
				sprintf(change_posdn, "%s", no_string);
				sprintf(search_mgr, "%s", yes_string);
				tmp_org = TRUE;
				sprintf(add_entries, "%s", yes_string);
				highNumber = 0;
				pagerOn(NUMBER_NOT_ALLOWED);
				freeOrgs(&olp);
				tmp_org_found = TRUE;
				orgEntered = TRUE;
				goto prompt_person;
			} else {
				printf(got_match);
				printf(type_number);
				pagerOn(NUMBER_ALLOWED);
				printListOrgs(org, olp);
				goto prompt_org;
			}
		} else {
			goto prompt_org;
		}
	} else if (noOrgs == 1) {
		sprintf(posdit_user, "@%s", olp->name);
		str = copy_string(lastComponent(olp->name, ORG));
		sprintf(default_organisation, str);
		printf("\n%s\n", default_organisation);
		orgEntered = TRUE;
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListOrgs(org, olp);
		goto prompt_org;
	}
	pagerOn(NUMBER_NOT_ALLOWED);

	if (!(strcmp(have_department, no_string))) {
		goto prompt_person;
	} else if ((strcmp(have_department, yes_string))) {
		sprintf(ou, "%s", "*");
		if (listOUs(posdit_user, ou, &oulp) != OK) {
			searchFail(ou);
			de_exit(-1);
		}
		highNumber = 0;  /* disable entering a number, without displaying list */
		noOUs = listlen(oulp);
		if (noOUs <= 0) {
			goto prompt_person;
		} else {
			/*             sprintf(have_department, "%s", yes_string); */
			sprintf(change_posdit, "%s", yes_string);
		}
	}
prompt_ou:
	printf("\n");
	enterString(ORGUNIT, ou, oulp);
	if (strlen(ou) == 0) {
		if (!(strcmp(default_department, ""))) {
			goto prompt_person;
		} else {
			strcpy(ou, default_department);
		}
	} else {
		if (!(strcmp(ou, quit_String))) {
			highNumber = 0;
			pagerOn(NUMBER_NOT_ALLOWED);
			goto prompt_country;
		}
	}
	if (listOUs(posdit_user, ou, &oulp) != OK) {
		searchFail(ou);
		de_exit(-1);
	}
	noOUs = listlen(oulp);
	if (noOUs <= 0) {
		printf("%s `%s' \n\n", no_ouMatch, ou);
		goto prompt_ou;
	} else if (noOUs == 1) {
		sprintf(posdit_user, "@%s", oulp->name);
		str = copy_string(lastComponent(oulp->name, ORGUNIT));
		strcpy(default_department, str);
		printf("\n%s\n", default_department);
		highNumber = 0;
		pagerOn(NUMBER_NOT_ALLOWED);
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListOUs(ou, oulp);
		goto prompt_ou;
	}

prompt_person:
	/* Copy to posdit, if not defined */

	if (strlen(posdit) <= 0) {
		sprintf(posdit, "%s", posdit_user);
		determine_posdit(&objectType);
		posdit_oc = objectType;
	}
	if (!(strcmp(search_mgr, no_string))) {
		goto prompt_individual;
	}

	sprintf(person, dir_Mngr);
	sprintf(tmp_search, "%s@cn=%s", posdit_user,person);
	if (listExactPRRcn(tmp_search, &plp) != OK) {
		searchFail(person);
		de_exit(-1);
	}
	noPersons = listlen(plp);
	if (noPersons <= 0) {
		goto prompt_individual;
	} else if (noPersons == 1) {
		sprintf(username, "@%s", plp->name);
		str = copy_string(lastComponent(plp->name, PERSON));
		free(confirm_out);
		free(full_c);
		free(str);
		highNumber = 0;
		if (!(strcmp(just_dn, yes_string))) {
			dn_file = fopen("/tmp/dn_user", "w");
			fprintf(dn_file, username);
			de_unbind();
			fclose(dn_file);
			exit(0);
		}
		return OK;
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListPRRs(person, plp, PERSON, FALSE);
		goto prompt_person;
	}


prompt_individual:
	enterString(USER, person, plp);
	if (strlen(person) == 0) {
		if (!(strcmp(default_person, ""))) {
			printf("%s %s %s\n", enter_pName, quit_String, to_quit);
			goto prompt_individual;
		} else {
			strcpy(person, default_person);
		}
	} else if (!(strcmp(person, quit_String))) {
		de_prompt_yesno(ask_ifout, confirm_out, no_string);
		if (!(strcmp(confirm_out, yes_string))) {
			de_exit(-1);
		} else {
			highNumber = 0;
			goto prompt_country;
		}
	}
	if (orgEntered == FALSE) {
		sprintf(tmp_search, "%s@cn=%s", posdit_user,person);
		if (listExactPRRcn(tmp_search, &plp) != OK) {
			searchFail(person);
			de_exit(-1);
		}
	} else {
		if (listPRRs(posdit_user, person, &plp) != OK) {
			searchFail(person);
			de_exit(-1);
		}
	}
	noPersons = listlen(plp);
	if (noPersons <= 0) {
		printf("%s `%s' \n\n", no_pMatch, person);
		goto prompt_individual;
	} else if (noPersons == 1) {
		sprintf(username, "@%s", plp->name);
		str = copy_string(lastComponent(plp->name, PERSON));
		highNumber = 0;
		if (!(strcmp(just_dn, yes_string))) {
			dn_file = fopen("/tmp/dn_user", "w");
			fprintf(dn_file, username);
			de_unbind();
			fclose(dn_file);
			exit(0);
		}
	} else {
		printf(got_match);
		printf(type_number);
		pagerOn(NUMBER_ALLOWED);
		printListPRRs(person, plp, PERSON, FALSE);
		goto prompt_individual;
	}
	printf(user_identified);
	display_posdit(strlen(user_identified), username);
	free(confirm_out);
	free(full_c);
	free(str);
	free(already_in);
	return OK;
}
