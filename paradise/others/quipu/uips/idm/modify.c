#include "isoaddrs.h"
#include "quipu/util.h"
#include "demanifest.h"
#include "extmessages.h"
#include "namelist.h"
#include "destrings.h"
#include "quipu/ds_search.h"
#include "quipu/modify.h"
#include "quipu/read.h"
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
extern char no_string[];
extern char password[];
extern struct namelist * prratts;
extern struct namelist * rlatts;
extern struct namelist * rmatts;

/* Global variables for this file defined below */

Attr_Sequence as;
struct ds_modifyentry_arg modarg;
static struct namelist * olp = NULLLIST;
static struct namelist * oulp = NULLLIST;
static struct namelist * plp = NULLLIST;
static struct namelist * newatts = NULLLIST;
static int already_mhsUser;
char * mapAttName();
/* end file global variables */

int de_Modify() {

	char * more;
	char * rdn;
	char * TidyString();
	int objectType;
	int status;
	void searchFail(), onint1(), de_exit();

	int noEntries;

	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();


	more		= malloc(LINESIZE);
	rdn		= malloc(LINESIZE);
	highNumber = 0;
	noEntries = 0;
	as = as_comp_alloc();
	as = NULLATTR;

	sprintf(more, yes_string);  /* Enter loop */

	while (!(strcmp(more, yes_string))) {
		enterString(MODIFY, person, plp);

		if (!(strcmp(person, quit_String))) {
			sprintf(more, no_string);
			if (noEntries > 0) {
				freePRRs(&plp);
			}
			continue;
		}
		if (strlen(person) <= 0) {
			if (strlen(default_person) <= 0) {
				printf("%s %s %s\n", enter_entry_name, quit_String, to_quit);
				continue;
			} else {
				sprintf(person, "%s", default_person);
			}
		}
		printf("\n");

		if (noEntries > 0) {
			freePRRs(&plp);
		}
		if (listPRRs(posdit, person, &plp) != OK) {
			searchFail(person);
			de_exit(-1);
		} else {
			noEntries = listlen(plp);
			if (noEntries == 0) {
				printf("\n%s", no_ent_found);
				freePRRs(&plp);
			} else if (noEntries == 1) {
				highNumber = 0;
				rdn = copy_string(lastComponent(plp->name, PERSON));
				sprintf(default_person, "%s", rdn);
				pagerOn(NUMBER_NOT_ALLOWED);
				printLastComponent(INDENTON, plp->name, PERSON, 0);
				status = get_objectClassPRR(plp, &objectType);
				if (status != OK) {
					printf(no_p_rl_rm);
				} else {
					modify_av(plp->name, rdn, objectType);
				}
			} else {
				pagerOn(NUMBER_ALLOWED);
				printListPRRs(person, plp, COUNTRY, FALSE);
			}
		}
	}

	free(more);
	free(rdn);
	if (as != NULLATTR) {
		as_free(as);
	}
	return OK;
}

int modify_av(dn, rdn, objectType)
char * dn;
char * rdn;
int  objectType;
{

	int addr_changed;
	int arenew;
	int attr_count;
	int count;
	int delete_count;
	int dn_changed;
	int isnum;
	int modify;
	int new;
	int new_count;
	int new_value;
	int n;
	int position;
	int status;
	int tlx_changed;
	struct entrymod *emnew, *ems_append(), *modify_avs();
	char * cp;
	char * cp2;
	char * str;
	char * val2str();
	char * val2dnstr();
	char addrstr[LINESIZE];
	char addrstr2[LINESIZE];
	char dnstr[LINESIZE];
	char dnstr2[LINESIZE];
	char attr_name[LINESIZE];
	char buffer[LINESIZE];
	AttributeType  at;
	Attr_Sequence  as_entry;
	AV_Sequence    avl;
	AttributeValue av;
	struct acl_info * acl;

	cp  = malloc(LINESIZE);
	cp2 = malloc(LINESIZE);
	str = malloc(LINESIZE);
	as_entry = as_comp_alloc();
	as_entry = NULLATTR;

	arenew		= TRUE;
	attr_count	= 1;
	count		= 0;
	new_count	= 0;
	new		= FALSE;
	delete_count	= 0;
	dn_changed	= FALSE;
	tlx_changed	= FALSE;

reprintDetails:		/* object class has not changed */
	status = existing_at(rdn, &count, objectType, TRUE);
	status = check_new_at(&new_count, objectType);
	if (count <= 0 && new_count <= 0) {
		printf(noAtt_mod);
		return OK;
	}

	if (count <= 0) {
		sprintf(cp, newAttr_str);
		goto only_new_attributes;
	} else {
		if (arenew == TRUE && new_count > 0 ) {
			printf("%s %s %s %s:  ", entAtt_new_nmb, quit_String,
				   to_quit, this_entry);
		} else {
			printf("%s %s %s %s:  ", entAtt_nmb, quit_String,
				   to_quit, this_entry);
		}
	}

get_atnumber:
	if (gets(cp) == NULLCP) {
		/* behave as for an interrupt */
		clearerr(stdin);
		onint1();
	} else {
		cp = TidyString(cp);
	}

only_new_attributes:
	if (!(strcmp(cp, newAttr_str))) {
		status = new_at(&count, objectType);
		if (status == NOTOK) {
			free(cp);
			free(cp2);
			free(str);
			if (as_entry != NULLATTR) {
				as_free(as_entry);
			}
			return OK;
		}
		if (count > 0) {
			printf("%s, %s %s %s:  ", entAtt_mod, quit_String,
				   to_quit, this_entry);
			new = TRUE;
			goto get_atnumber;
		} else {
			arenew = FALSE;
			goto reprintDetails;
		}
	}

	isnum = TRUE;
	for (cp2 = cp; *cp2 != '\0'; cp2++) {
		if (! isdigit(*cp2)) {
			isnum = FALSE;
			break;
		}
	}

	if (isnum) {
		n = atoi(cp);
		if ((n < 1 || n > count) ||
				((strlen(cp) <= 0) && (n == 0))) {
			if (count == 1) {
				printf("`%d' %s %s %s\n\n", n, is_inv1, quit_String, to_quit);
			} else if ((strlen(cp) <= 0) && (n == 0)) {
				printf("%s %s %s\n\n", is_invCR, quit_String, to_quit);
			} else {
				printf("`%d' %s %d, %s \n\n", n, is_invRng, count, enter_q_help);
			}
			if (new == FALSE) {
				goto reprintDetails;
			} else {
				status = new_at(&count, objectType);
				if (status == NOTOK) {
					free(cp);
					free(cp2);
					free(str);
					if (as_entry != NULLATTR) {
						as_free(as_entry);
					}
					return OK;
				}
				printf("%s %s %s %s:  ", entAtt_mod, quit_String,
					   to_quit, this_entry);
				goto get_atnumber;
			}
		} else {
			if (new == TRUE) {
				status = get_new_attr(dn, n);
				return OK;
			}
			printf(removeValue);

			if (objectType == PERSON ||
					objectType == ROLE   ||
					objectType == ROOM) {
				as_entry = as_cpy(plp->ats);
			} else if (objectType == ORG) {
				as_entry = as_cpy(olp->ats);
			} else if (objectType == ORGUNIT) {
				as_entry = as_cpy(oulp->ats);
			}

			position = n-1;
			if ((!(strcmp(attr2name(as_entry->attr_type, OIDPART), "objectClass")))) {
				as_entry = as_entry->attr_link;
			}
			while (position > 0) {
				/* circulate here, as many positions as required */
				as_entry = as_entry->attr_link;
				if ((strcmp(attr2name(as_entry->attr_type, OIDPART), "objectClass"))) {
					position--;
				}
			}

			if (!(strcmp(attr2name(as_entry->attr_type, OIDPART), "objectClass"))) {
				as_entry = as_entry->attr_link;
			}
			modarg.mea_changes = NULLMOD; /* initially no changes */
			modify = FALSE;

prompt_value:
			printf("\n");
			sprintf(attr_name, attr2name(as_entry->attr_type, OIDPART));
			addr_changed = FALSE;
			if (is_postal_address(attr_name) == TRUE) {
				printf("%s (%d %s)\n", mapAttName(attr_name), LINES_PA, idm_lines);
				str = val2str(&(as_entry->attr_value->avseq_av));
				rtnstr2dlrstr(str, addrstr2);
				strcpy(addrstr, addrstr2); /* save original */
				status = ask_addr(attr_name, addrstr2);
				if (!(strcmp(addrstr2, quit_String))) {
					sprintf(str, quit_String);
				} else if ((strcmp(addrstr, addrstr2))) {
					addr_changed = TRUE;
					if (strlen(addrstr2) > 0) {
						sprintf(str, addrstr2);
					} else {
						sprintf(str, "-"); /* Delete the value */
					}
				}

				if (status == NOTOK) {
					free(cp);
					free(cp2);
					free(str);
					if (as_entry != NULLATTR) {
						as_free(as_entry);
					}
					return NOTOK;
				}
			} else if (is_dn(attr_name) == TRUE) {
				str = val2dnstr(&(as_entry->attr_value->avseq_av));
				sprintf(dnstr2, str);
				strcpy(dnstr, dnstr2);
				status = ask_dnhnc(str, attr_name);
				if (status != OK) {
					sprintf(str, quit_String);
				}

				if ((strcmp(str, dnstr2))) {
					dn_changed = TRUE;
				}

			} else if (is_tlx(attr_name) == TRUE) {
				printf("%s (%s, %s %s %s)\n", mapAttName(attr_name), number_str,
					   country_str, and_str, ansBack_str);
				str = val2dnstr(&(as_entry->attr_value->avseq_av));
				sprintf(dnstr2, str);
				strcpy(dnstr, dnstr2);
				status = ask_tlx(attr_name, str);
				if (status != OK) {
					sprintf(str, quit_String);
				}
				if ((strcmp(str, dnstr2))) {
					tlx_changed = TRUE;
					if (strlen(str) <= 0) {
						sprintf(str, "-"); /* Delete the value */
					}
				}
			} else if (is_x400(attr_name) == TRUE) {
				str = val2dnstr(&(as_entry->attr_value->avseq_av));
				sprintf(dnstr2, str);
				strcpy(dnstr, dnstr2);  /* original EDB value to delete */
				printf("%s [%s]:  ", mapAttName(attr_name),
					   val2str(&(as_entry->attr_value->avseq_av)));
				if (gets(str) == NULLCP) {
					/* behave as for an interrupt */
					clearerr(stdin);
					onint1();
				}
			} else {
				printf("%s [%s]:  ", mapAttName(attr_name),
					   val2str(&(as_entry->attr_value->avseq_av)));
				if (gets(str) == NULLCP) {
					/* behave as for an interrupt */
					clearerr(stdin);
					onint1();
				} else {
					str = TidyString(str);
				}
			}
			if (!(strcmp(str, quit_String))) {
				printf(ent_not_mod);
				free(cp);
				free(cp2);
				free(str);
				if (as_entry != NULLATTR) {
					as_free(as_entry);
				}
				return OK;
			} else if (strlen(str) == 1 && index(str, '-') != 0) {
				/* We must delete this value */
				if (is_postal_address(attr_name) == TRUE) {
					sprintf(buffer, "%s=%s", attr_name, addrstr);
				} else if (is_dn(attr_name) == TRUE) {
					sprintf(buffer, "%s=%s", attr_name, dnstr);
				} else if (is_tlx(attr_name) == TRUE) {
					sprintf(buffer, "%s=%s", attr_name, dnstr);
				} else if (is_x400(attr_name) == TRUE) {
					sprintf(buffer, "%s=%s", attr_name, dnstr);
				} else {
					sprintf(buffer, "%s=%s", attr_name,
							val2str(&(as_entry->attr_value->avseq_av)));
				}
				delete_count++;
				if ((as = str2as(buffer)) == NULLATTR) {
					printf(fail_mod_buf);
					return NOTOK;
				}

				emnew = NULLMOD;
				emnew = em_alloc();
				emnew->em_type = EM_REMOVEVALUES;
				emnew->em_what = as_cpy(as);
				emnew->em_next = NULLMOD;
				modarg.mea_changes = ems_append (modarg.mea_changes,emnew);
				modify = TRUE;
			} else if (!(strcmp(str, "?"))) {
				displayFile(attr_name, FALSE);
				goto prompt_value;
			} else if (strlen(str) == 0) {
				/* No action, original value kept */
			} else {
				/* change a value (delete and add) */
				if (is_postal_address(attr_name) == TRUE) {
					if (addr_changed == TRUE) {
						sprintf(buffer, "%s=%s", attr_name, addrstr);
					} else {
						goto next_value;
					}
				} else if (is_dn(attr_name) == TRUE) {
					if (dn_changed == TRUE) {
						sprintf(buffer, "%s=%s", attr_name, dnstr);
					} else {
						goto next_value;
					}
				} else if (is_tlx(attr_name) == TRUE) {
					if (tlx_changed == TRUE) {
						sprintf(buffer, "%s=%s", attr_name, dnstr);
					} else {
						goto next_value;
					}
				} else if (is_x400(attr_name) == TRUE) {
					sprintf(buffer, "%s=%s", attr_name, dnstr);
				} else {
					sprintf(buffer, "%s=%s", attr_name,
							val2str(&(as_entry->attr_value->avseq_av)));
				}
				if ((as = str2as(buffer)) == NULLATTR) {
					printf(fail_mod_buf);
					return NOTOK;
				}

				emnew = NULLMOD;
				emnew = em_alloc();
				emnew->em_type = EM_REMOVEVALUES;
				emnew->em_what = as_cpy(as);
				sprintf(buffer, "%s=%s", attr_name, str);
				if ((as = str2as(buffer)) == NULLATTR) {
					printf(inv_value);
					goto prompt_value;
				} else if ((is_phone_fax(attr_name) == TRUE) &&
						   (strlen(str) > CHARS_PHONE)) {
					printf("%s %d %s\n", inv_length, CHARS_PHONE, characters);
					printf(enter_again);
					goto prompt_value;
				}
				emnew->em_next = em_alloc();
				emnew->em_next->em_type = EM_ADDVALUES;
				emnew->em_next->em_what = as_cpy(as);
				emnew->em_next->em_next = NULLMOD;
				modarg.mea_changes = ems_append(modarg.mea_changes, emnew);
				modify = TRUE;
			}

next_value:
			if (as_entry->attr_value->avseq_next == NULL) {
				/* Prompt for another value */
				status = prompt_new_value(attr_name, &new_value);
				if (status != OK) {
					free(cp);
					free(cp2);
					free(str);
					if (as_entry != NULLATTR) {
						as_free(as_entry);
					}
					return OK;
				}
				if (new_value == TRUE) {
					modify = TRUE;
				}
			} else {
				as_entry->attr_value = as_entry->attr_value->avseq_next;
				attr_count++;
				goto prompt_value;
			}
			if (modify == TRUE) {
				if (attr_count == delete_count) {
					/* Delete all values, therefore remove attribute */
					emnew->em_next = em_alloc();
					emnew->em_next->em_type = EM_REMOVEATTRIBUTE;
					at = str2AttrT(attr_name);
					as = as_comp_new(at, NULLAV, NULLACL_INFO);
					emnew->em_next->em_what = as_cpy(as);
					emnew->em_next->em_next = NULLMOD;
					if (is_x400(attr_name)) {
						/* Must delete mhsUser object class as well */
						emnew->em_next->em_next = em_alloc();
						emnew->em_next->em_next->em_type = EM_REMOVEVALUES;
						at = NULLAttrT;
						at = str2AttrT("objectClass");
						av = NULLAttrV;
						sprintf(buffer, "%s", "mhsUser");
						av = str2AttrV(buffer, at->oa_syntax);
						avl = avs_comp_new(av);
						acl = NULLACL_INFO;
						as = as_comp_new(at, avl, acl);
						if (as == NULLATTR) {
							printf(fail_mod_rmmhs);
							printf(err_rprtHlpdsk);
						}
						emnew->em_next->em_next->em_what = as_cpy(as);
						emnew->em_next->em_next->em_next = NULLMOD;
					}
					modarg.mea_changes = NULLMOD;
					modarg.mea_changes = ems_append(modarg.mea_changes, emnew);
				}

				dm_modentry(dn);
			}
		}
	} else {
		if (!(strcmp(cp, quit_String))) {
			free(cp);
			free(cp2);
			free(str);
			sprintf(default_person, "");	/* clear default value */
			if (as_entry != NULLATTR) {
				as_free(as_entry);
			}
			return NOTOK;
		} else if (!(strcmp(cp, "?"))) {
			displayFile("attr_numbers", FALSE);
			goto reprintDetails;
		} else {
			printf("`%s' %s %s, %s %s\n",
				   cp, is_inv, enter_q_help, quit_String, to_quit);
			goto reprintDetails;
		}
	}

	free(cp);
	free(cp2);
	free(str);
	if (as_entry != NULLATTR) {
		as_free(as_entry);
	}
	return OK;
}


int fillMostModArg() {
	modarg.mea_common.ca_servicecontrol.svc_options = SVC_OPT_CHAININGPROHIBIT;
	modarg.mea_common.ca_servicecontrol.svc_prio = SVC_PRIO_MED;
	modarg.mea_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	modarg.mea_common.ca_servicecontrol.svc_sizelimit = SVC_NOSIZELIMIT;
	modarg.mea_common.ca_servicecontrol.svc_scopeofreferral = SVC_REFSCOPE_COUNTRY;
	modarg.mea_common.ca_servicecontrol.svc_tmp = NULL;
	modarg.mea_common.ca_servicecontrol.svc_len = 0;
	modarg.mea_common.ca_requestor = NULL;
	modarg.mea_common.ca_progress.op_resolution_phase = OP_PHASE_NOTDEFINED;
	modarg.mea_common.ca_progress.op_nextrdntoberesolved = 0;
	modarg.mea_common.ca_aliased_rdns = NULL;
	modarg.mea_common.ca_security = (struct security_parms *)NULL;
	modarg.mea_common.ca_sig = (struct signature *) NULL;
	modarg.mea_common.ca_extensions = (struct extension *) NULL;
}

int dm_modentry(dn)
char * dn;
{

	extern int rfrl_msg;

	struct DSError error;

	int status;

	fillMostModArg();

	modarg.mea_object = str2dn(dn);
	printf("%s   %s ", mod_entry, please_wait);
	fflush(stdout);
	rfrl_msg = TRUE;

modify_entry:
	status = ds_modifyentry(&modarg, &error);
	if (status != OK) {
		status = check_error(error);
		if (status == REFERRAL) {
			rfrl_msg = FALSE;
			goto modify_entry;
		} else {
			printf(ent_not_mod);
		}
	} else {
		printf(done);
	}
	rfrl_msg = TRUE;
}

struct entrymod * ems_append (a,b)
struct entrymod *a;
struct entrymod *b;
{
	struct entrymod *ptr;

	if ((ptr = a) == NULLMOD)
		return b;

	for ( ; ptr->em_next != NULLMOD; ptr = ptr->em_next)
		;

	ptr->em_next = b;
	return a;
}

int rtnstr2dlrstr(rtnstr, dlrstr)
char * rtnstr;
char dlrstr[];
{

	char * cp;
	char * start;

	int first_time;
	int spaces;

	if (strlen(rtnstr) <= 0) {
		strcpy(dlrstr, "");
		return OK;
	}

	start = rtnstr;
	cp = start;
	strcpy(dlrstr, "");
	first_time = TRUE;

	cp = index(cp, '\n');

	while ((cp != NULLCP)) {
		if (first_time == TRUE) {
			first_time = FALSE; /* First line does not have spaces */
			strncat(dlrstr, start, cp-start);
		} else {
			spaces = strspn(start, " ");
			strncat(dlrstr, start+spaces, cp-start-spaces);
		}
		strcat(dlrstr, "$");
		cp++;
		start = cp;
		cp = index(cp, '\n');
	}

	if (first_time == TRUE) {
		/* Only one line was entered */
		strcpy(dlrstr, start); /* Copy the entire string */
	} else {
		spaces = strspn(start, " ");
		strcat(dlrstr, start+spaces); /* Concatenate last part */
	}
	return OK;
}



int new_at(atcount, objectType)
int * atcount;
int objectType;
{

	extern struct namelist * orgatts;
	extern struct namelist * ouatts;

	int count;

	if (objectType == PERSON) {
		pagerOn(NUMBER_NOT_ALLOWED);
		printEmptyAttributes(plp, prratts, &count);
	} else if (objectType == ROLE) {
		pagerOn(NUMBER_NOT_ALLOWED);
		printEmptyAttributes(plp, rlatts, &count);
	} else if (objectType == ROOM) {
		pagerOn(NUMBER_NOT_ALLOWED);
		printEmptyAttributes(plp, rmatts, &count);
	} else if (objectType == ORG) {
		pagerOn(NUMBER_NOT_ALLOWED);
		printEmptyAttributes(olp, orgatts, &count);
	} else if (objectType == ORGUNIT) {
		pagerOn(NUMBER_NOT_ALLOWED);
		printEmptyAttributes(oulp, ouatts, &count);
	}

	*atcount = count;

	return OK;
}

int check_new_at(atcount, objectType)
int * atcount;
int objectType;
{

	extern struct namelist * orgatts;
	extern struct namelist * ouatts;

	int count;

	if (objectType == PERSON) {
		checkForEmptyAttributes(plp, prratts, &count);
	} else if (objectType == ROLE) {
		checkForEmptyAttributes(plp, rlatts, &count);
	} else if (objectType == ROOM) {
		checkForEmptyAttributes(plp, rmatts, &count);
	} else if (objectType == ORG) {
		checkForEmptyAttributes(olp, orgatts, &count);
	} else if (objectType == ORGUNIT) {
		checkForEmptyAttributes(oulp, ouatts, &count);
	}

	*atcount = count;

	return OK;
}


int existing_at(rdn, atcount, objectType, numbers)
char * rdn;
int * atcount;
int objectType;
int numbers;				/* Unfortunately a control flag,
                                           to indicate if to print numbers
					   alongside attributes or not */
{

	struct namelist * olp = NULLLIST;
	struct namelist * oulp = NULLLIST;

	char * str;
	char * tmp_posdit;
	char * val2str();
	int count;

	count = 0;
	str = malloc(LINESIZE);

	if (objectType == PERSON) {
		pagerOn(NUMBER_NOT_ALLOWED);
		if (listPRRs(posdit, rdn, &plp) != OK) {
			searchFail(rdn);
			free(str);
			return NOTOK;
		}
		if (numbers == TRUE) {
			printDetailsNumbers(PERSON, plp, &count);
		} else {
			printDetails(PERSON, plp);
		}
	} else if (objectType == ROLE) {
		pagerOn(NUMBER_NOT_ALLOWED);
		if (listPRRRl(posdit, rdn, &plp) != OK) {
			searchFail(rdn);
			free(str);
			return NOTOK;
		}
		if (numbers == TRUE) {
			printDetailsNumbers(PERSON, plp, &count);
		} else {
			printDetails(PERSON, plp);
		}
	} else if (objectType == ROOM) {
		pagerOn(NUMBER_NOT_ALLOWED);
		if (listPRRRm(posdit, rdn, &plp) != OK) {
			searchFail(rdn);
			free(str);
			return NOTOK;
		}
		if (numbers == TRUE) {
			printDetailsNumbers(PERSON, plp, &count);
		} else {
			printDetails(PERSON, plp);
		}
	} else if (objectType == ORG) {
		tmp_posdit = malloc(LINESIZE);
		sprintf(tmp_posdit, posdit);
		tmp_posdit = removeLastRDN(tmp_posdit);
		if (listOrgs(tmp_posdit, rdn, &olp) != OK) {
			searchFail(rdn);
			free(tmp_posdit);
			free(str);
			return NOTOK;
		}
		free(tmp_posdit);
		pagerOn(NUMBER_NOT_ALLOWED);
		if (numbers == TRUE) {
			printDetailsNumbers(ORG, olp, &count);
		} else {
			printDetails(ORG, olp);
		}
	} else if (objectType == ORGUNIT) {
		tmp_posdit = malloc(LINESIZE);
		sprintf(tmp_posdit, posdit);
		tmp_posdit = removeLastRDN(tmp_posdit);
		if (listOUs(posdit, rdn, &oulp) != OK) {
			searchFail(rdn);
			free(str);
			free(tmp_posdit);
			return NOTOK;
		}
		free(tmp_posdit);
		pagerOn(NUMBER_NOT_ALLOWED);
		if (numbers == TRUE) {
			printDetailsNumbers(ORGUNIT, oulp, &count);
		} else {
			printDetails(ORGUNIT, oulp);
		}

	}

	if (numbers == TRUE) {
		*atcount = count;
	}

	free(str);
	return OK;
}

printEmptyAttributes(lp, objatts, count)
struct namelist * lp;
struct namelist * objatts;
int * count;
{
	Attr_Sequence at;

	struct namelist * saveprr;
	char * name;

	saveprr = objatts;
	name = objatts->name;

	(*count) = 0;

	while (name != NULLCP) {
		for (at = lp->ats; at != NULLATTR; at = at->attr_link) {
			if (!(strcmp(name, attr2name(at->attr_type, OIDPART)))) {
				/* it exists, skip it */
				break;
			}
		}
		if (at == NULLATTR) {
			/* it was not found, print it */
			(*count)++;
			if ((*count) == 1) {
				printf("\n");
			}
			printf("        %d  %s\n", *count, mapAttName(name));
		}
		if (objatts->next != NULL) {
			objatts = objatts->next;
			name = objatts->name;
		} else {
			break;
		}
	}
	objatts = saveprr;
	if ((*count) == 0) {
		printf(all_attVal);
	}
}
checkForEmptyAttributes(lp, objatts, count)
struct namelist * lp;
struct namelist * objatts;
int * count;
{
	Attr_Sequence at;

	struct namelist * saveprr;
	char * name;

	saveprr = objatts;
	newatts = NULLLIST; /* Initialize list of attributes */
	name = objatts->name;

	(*count) = 0;

	while (name != NULLCP) {
		for (at = lp->ats; at != NULLATTR; at = at->attr_link) {
			if (!(strcmp(name, attr2name(at->attr_type, OIDPART)))) {
				/* it exists, skip it */
				break;
			}
		}
		if (at == NULLATTR) {
			/* it was not found, there are empty attributes
			   change count and return */
			(*count)++;
			addToList(&newatts, name);
		}
		if (objatts->next != NULL) {
			objatts = objatts->next;
			name = objatts->name;
		} else {
			break;
		}
	}
	objatts = saveprr;
}


int get_new_attr(dn, at_number)
char * dn;
int at_number;
{

	struct entrymod *emnew, *ems_append(), *modify_avs();
	Attr_Sequence as_mhs;
	AttributeType at_mhs;
	AttributeValue av_mhs;
	AV_Sequence avs_mhs;
	struct acl_info * acl_mhs;
	struct namelist * tmp;
	int new_value;
	int position;
	int status;
	char addrstr2[LINESIZE];
	char attr_name[LINESIZE];
	char buffer[LINESIZE];
	char buffer_mhs[LINESIZE];
	char * str;

	as_mhs = as_comp_alloc();
	at_mhs = NULLAttrT;
	av_mhs = AttrV_alloc();
	avs_mhs = NULLAV;
	acl_mhs = NULLACL_INFO;
	str = malloc(LINESIZE);

	*str = '\0';

	tmp = newatts;

	position = at_number-1;
	while (position > 0) {
		/* circulate here, as many positions as required */
		tmp = tmp->next;
		position--;
	}

	sprintf(attr_name, tmp->name);

prompt_attr:
	if (is_postal_address(attr_name) == TRUE) {
		printf("%s %s (%d %s)\n", enter_msg, mapAttName(attr_name),
			   LINES_PA, idm_lines);
		sprintf(addrstr2, "");
		status = ask_addr(attr_name, addrstr2);
		if (!(strcmp(addrstr2, quit_String))) {
			sprintf(str, quit_String);
		} else {
			sprintf(str, addrstr2);
		}
	} else if (is_dn(attr_name) == TRUE) {
		status = ask_dnhnc(str, attr_name);
		if (status != OK) {
			sprintf(str, quit_String);
		}
	} else if (is_tlx(attr_name) == TRUE) {
		printf("%s (%s, %s %s %s)\n", mapAttName(attr_name), number_str,
			   country_str, and_str, ansBack_str);
		status = ask_tlx(attr_name, str);
		if (status != OK) {
			sprintf(str, quit_String);
		}
	} else {
		printf("%s %s  :  ", enter_msg, mapAttName(attr_name));

		if (gets(str) == NULLCP) {
			/* behave as for an interrupt */
			clearerr(stdin);
			onint1();
		} else {
			str = TidyString(str);
		}
	}

	if (!(strcmp(str, quit_String))) {
		return OK;
	} else if (!(strcmp(str, "?"))) {
		displayFile(attr_name, FALSE);
		goto prompt_attr;
	} else if ((strlen(str)) <= 0) {
		printf("%s %s %s %s", ent_val, quit_String, to_quit, enting_val);
		goto prompt_attr;
	}

	sprintf(buffer, "%s=%s", attr_name, str);
	if ((as = str2as(buffer)) == NULLATTR) {
		printf("%s, %s\n", err_invAtSntx, enter_q_help);
		goto prompt_attr;
	} else {
		if (!(strcmp(attr_name, "mhsORAddresses")) &&
				already_mhsUser == FALSE) {
			/* Not and mhsUser, and adding the x.400 address, add new objectClass */
			at_mhs = NULLAttrT;
			at_mhs = str2AttrT("objectClass");
			av_mhs = NULLAttrV;
			sprintf(buffer_mhs, "%s", "mhsUser");
			av_mhs = str2AttrV(buffer_mhs, at_mhs->oa_syntax);
			avs_mhs = avs_comp_new(av_mhs);
			acl_mhs = NULLACL_INFO;
			as_mhs = as_comp_new(at_mhs, avs_mhs, acl_mhs);
			if (as_mhs == NULLATTR) {
				printf(fail_mod_adMHS);
				printf(err_rprtHlpdsk);
			}
		}
		modarg.mea_changes = NULLMOD; /* initially no changes */
		emnew = NULLMOD;
		emnew = em_alloc();
		emnew->em_type = EM_ADDATTRIBUTE;
		emnew->em_what = as_cpy(as);
		if (!(strcmp(attr_name, "mhsORAddresses")) &&
				already_mhsUser == FALSE) {
			emnew->em_next = em_alloc();
			emnew->em_next->em_type = EM_ADDVALUES;
			emnew->em_next->em_what = as_cpy(as_mhs);
			emnew->em_next->em_next = NULLMOD;
		} else {
			emnew->em_next = NULLMOD;
		}

		modarg.mea_changes = ems_append(modarg.mea_changes, emnew);
		status = prompt_new_value(attr_name, &new_value);
		if (status != OK) {
			free(str);
			return NOTOK;
		} else {
			dm_modentry(dn);
		}
	}

	free(str);
	return OK;
}


int prompt_new_value(attr_name, new_value)
char attr_name[];
int * new_value;
{

	struct entrymod *emnew, *ems_append(), *modify_avs();

	char addrstr2[LINESIZE];
	char buffer[LINESIZE];
	char prompt[LINESIZE];
	char * more;
	char * str;

	more = malloc(LINESIZE);
	str  = malloc(LINESIZE);

	sprintf(str, "Enterloop");
	*new_value = FALSE;

	while (strlen(str) > 0) {
		if (is_postal_address(attr_name) == TRUE) {
			sprintf(prompt, "%s %s", have_another, mapAttName(attr_name));
			de_prompt_yesno(prompt, more, no_string);
			if (!(strcmp(more, no_string))) {
				return OK;
			}
			printf("%s %s (%d %s)\n", opt_add, mapAttName(attr_name),
				   LINES_PA, idm_lines);
			sprintf(addrstr2, "");
			ask_addr(attr_name, addrstr2);
			sprintf(str, addrstr2);
		} else if (is_dn(attr_name) == TRUE) {
			sprintf(prompt, "%s %s", have_another, mapAttName(attr_name));
			de_prompt_yesno(prompt, more, no_string);
			if (!(strcmp(more, no_string))) {
				return OK;
			}
			sprintf(str, "");
			printf("\n");
			ask_dnhnc(str, attr_name);
		} else if (is_tlx(attr_name) == TRUE) {
			sprintf(prompt, "%s %s", have_another, mapAttName(attr_name));
			de_prompt_yesno(prompt, more, no_string);
			if (!(strcmp(more, no_string))) {
				return OK;
			}
			sprintf(str, "");
			printf("\n");
			printf("%s %s (%s, %s %s %s)\n", opt_add, mapAttName(attr_name),
				   number_str, country_str, and_str, ansBack_str);
			ask_tlx(attr_name, str);
		} else {
			printf("%s %s:  ", opt_add, mapAttName(attr_name));
			if (gets(str) == NULLCP) {
				/* behave as for an interrupt */
				clearerr(stdin);
				onint1();
			} else {
				str = TidyString(str);
			}
		}

		if (!(strcmp(str, quit_String))) {
			printf("\n%s", ent_not_mod);
			return NOTOK;
		} else if (!(strcmp(str, "?"))) {
			displayFile(attr_name, FALSE);
		} else if (strlen(str) > 0) {
			sprintf(buffer, "%s=%s", attr_name, str);
			if ((as = str2as(buffer)) == NULLATTR) {
				printf(inv_Value);
				continue;
			} else if ((is_phone_fax(attr_name) == TRUE) &&
					   (strlen(str) > CHARS_PHONE)) {
				printf("%s %d %s\n", inv_length, CHARS_PHONE, characters);
				printf(enter_again);
				continue;
			}
			emnew = NULLMOD;
			emnew = em_alloc();
			emnew->em_type = EM_ADDVALUES;
			emnew->em_what = as_cpy(as);
			emnew->em_next = NULLMOD;
			modarg.mea_changes = ems_append(modarg.mea_changes, emnew);
			*new_value = TRUE;
		}
	}

	free(str);
	return OK;
}


int get_objectClass(rdn,objectType)
char * rdn;
int * objectType;
{
	char * cp;
	char * savestr;
	char * temprdn;

	savestr	= malloc(LINESIZE);
	temprdn	= malloc(LINESIZE);

	sprintf(savestr, rdn);
	/*    temprdn = copy_string(lastComponent(savestr, ORGUNIT));
	    if (!(strcmp(temprdn, rdn)))
	      { /* Same, modifying C, ORG or ORGUNIT, check which one it is */
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
		if (!(strncmp(cp, "=", 1))) {
			free(temprdn);
			free(savestr);
			return OK;
		} else {
			/* Something is suspicious, may be a person is a
			non-leaf node (or wrong posdit in tailor file) */
			return NOTOK;
		}
	}
	/*      }
	    return NOTOK; */
}



get_objectClassPRR(lp, objectType)
struct namelist * lp;
int * objectType;
{
	Attr_Sequence at;
	AV_Sequence av;
	char * str;

	str = malloc(LINESIZE);

	already_mhsUser = FALSE;

	for (at = lp->ats; at != NULLATTR; at = at->attr_link) {

		if (!(strcmp(attr2name(at->attr_type, OIDPART), "objectClass"))) {
			/* Only interested in object class */
			for (av = at->attr_value; av != NULLAV; av = av->avseq_next) {
				str = val2str(&(av->avseq_av));
				if (!strcmp(str, "mhsUser")) {
					already_mhsUser = TRUE;
				} else if (!strcmp(str, "person")) {
					*objectType = PERSON;
				} else if (!(strcmp(str, "organizationalRole"))) {
					*objectType = ROLE;
				} else if (!(strcmp(str, "room"))) {
					*objectType = ROOM;
				} else if (!(strcmp(str, "organization"))) {
					*objectType = ORG;
				} else if (!(strcmp(str, "organizationalUnit"))) {
					*objectType = ORGUNIT;
				}
			}
			return OK;
		}
	}
	return NOTOK;
}


int dm_ModifyOrg() {

	char * cp;
	char * more;
	char * rdn;
	char * savestr;
	char * TidyString();
	void searchFail(), onint1(), de_exit();

	int ignore;
	int noEntries;
	int objectType;
	int status;

	Attr_Sequence	as_comp_new();
	Attr_Sequence	as_combine();


	more		= malloc(LINESIZE);
	rdn		= malloc(LINESIZE);
	savestr	= malloc(LINESIZE);
	highNumber = 0;
	as = as_comp_alloc();
	as = NULLATTR;

	sprintf(savestr, posdit);
	savestr = removeLastRDN(savestr);

	rdn = copy_string(lastComponent(posdit, ORGUNIT));

	objectType = 0;
	cp = rindex(posdit, '@');
	status = get_objectClass(cp, &objectType);
	if (status != OK) {
		printf(org_unk_type);
		printf("\n %s", err_rprtHlpdsk);
		return OK;  /* Treat as if everything went OK */
	}
	sprintf(more, yes_string);	/* Enter loop */

	while (!(strcmp(more, yes_string))) {
		if (objectType == ORG) {
			if (listOrgs(savestr, rdn, &olp) != OK) {
				searchFail(rdn);
				de_exit(-1);
			} else {
				noEntries = listlen(olp);
				if (noEntries == 0) {
					printf(no_orgFound);
				} else if (noEntries == 1) {
					printf("\n");
					pagerOn(NUMBER_NOT_ALLOWED);
					printLastComponent(INDENTON, olp->name, ORG, FALSE);
					printf("\n");
					rdn = copy_string(lastComponent(olp->name, ORG));
					status = modify_av(olp->name, rdn, objectType);
					if (status != OK) {
						break;
					}
					highNumber = 0;
					if (listOrgs(savestr, rdn, &olp) != OK) {
						searchFail(rdn);
						de_exit(-1);
					}
					printf("\n");
					pagerOn(NUMBER_NOT_ALLOWED);
					printLastComponent(INDENTON, olp->name, ORG, FALSE);
					printf("\n");
					status = existing_at(rdn, &ignore, objectType, FALSE);
				} else {
					pagerOn(NUMBER_ALLOWED);
					printListOrgs(rdn, olp);
				}
			}
		} else if (objectType == ORGUNIT) {
			if (listOUs(savestr, rdn, &oulp) != OK) {
				searchFail(rdn);
				de_exit(-1);
			} else {
				noEntries = listlen(oulp);
				if (noEntries == 0) {
					printf(no_ouFound);
				} else if (noEntries == 1) {
					printf("\n");
					pagerOn(NUMBER_NOT_ALLOWED);
					printLastComponent(INDENTON, oulp->name, ORGUNIT, FALSE);
					printf("\n");
					rdn = copy_string(lastComponent(oulp->name, ORGUNIT));
					status = modify_av(oulp->name, rdn, objectType);
					if (status != OK) {
						break;
					}
					highNumber = 0;
					if (listOUs(savestr, rdn, &oulp) != OK) {
						searchFail(rdn);
						de_exit(-1);
					}
					printf("\n");
					pagerOn(NUMBER_NOT_ALLOWED);
					printLastComponent(INDENTON, oulp->name, ORGUNIT, FALSE);
					printf("\n");
					status = existing_at(rdn, &ignore, objectType, FALSE);
				} else {
					pagerOn(NUMBER_ALLOWED);
					printListOUs(rdn, oulp);
				}
			}
		} else {
			printf(org_ou_only);
			break;
		}
		de_prompt_yesno(ask_ifmorechngs, more, yes_string);
	}

	free(more);
	free(rdn);
	if (as != NULLATTR) {
		as_free(as);
	}
	return OK;
}

prompt_new_password() {

#include "bind.h"

	char * getpass();
	char * temp2;
	char new_pswd[LINESIZE];
	char old_pswd[LINESIZE];
	char vfy_pswd[LINESIZE];

	int count;
	int status;

	temp2 = malloc(LINESIZE);

	count = 1;

get_old_pswd:
	count++;
	sprintf(temp2, "%s :  ", ask_old_password);
	sprintf(old_pswd, getpass(temp2));
	if (!(strcmp(old_pswd, quit_String))) {
		printf(pswd_not_changed);
		free(temp2);
		return OK;
	}

	if (!(strcmp(old_pswd, password))) {
get_new_pswd:
		sprintf(temp2, "%s :  ", ask_new_password);
		sprintf(new_pswd, getpass(temp2));
		if (!(strcmp(new_pswd, quit_String))) {
			printf(pswd_not_changed);
			free(temp2);
			return OK;
		}
		sprintf(temp2, "%s :  ", ask_vfy_password);
		sprintf(vfy_pswd, getpass(temp2));
		if (!(strcmp(vfy_pswd, quit_String))) {
			printf(pswd_not_changed);
			free(temp2);
			return OK;
		}
		if (!(strcmp(new_pswd, vfy_pswd))) {
			status = modify_pswd(old_pswd, new_pswd);
			if (status != OK) {
				goto get_new_pswd;
			} else {
				/* update memory variable */
				sprintf(password, "%s", new_pswd);
			}
		} else {
			printf(vfy_pswd_fail);
			goto get_new_pswd;
		}
	} else {
		printf(wrong_old_pswd);
		printf(pswd_tryagain);
		if (count > PSWD_TRY_LIMIT) {
			printf(pswd_lmt_xcd);
			de_exit(-1);
		} else {
			goto get_old_pswd;
		}
	}
	return OK;
}

modify_pswd(old_pswd, new_pswd)
char old_pswd[];
char new_pswd[];
{

	extern char username[];

	struct entrymod *emnew, *ems_append(), *modify_avs();

	char buffer[LINESIZE];

	sprintf(buffer, "userPassword=%s", old_pswd);

	if ((as = str2as(buffer)) == NULLATTR) {
		printf("\n%s", fail_mod_buf);
		return NOTOK;
	}
	modarg.mea_changes = NULLMOD; /* initially no changes */
	emnew = NULLMOD;
	emnew = em_alloc();
	emnew->em_type = EM_REMOVEVALUES;
	emnew->em_what = as_cpy(as);
	sprintf(buffer, "userPassword=%s", new_pswd);
	if ((as = str2as(buffer)) == NULLATTR) {
		printf("%s, %s %s\n", inv_Value, quit_String, to_quit);
		return NOTOK;
	}
	emnew->em_next = em_alloc();
	emnew->em_next->em_type = EM_ADDVALUES;
	emnew->em_next->em_what = as_cpy(as);
	emnew->em_next->em_next = NULLMOD;
	modarg.mea_changes = ems_append(modarg.mea_changes, emnew);
	dm_modentry(username);
	return OK;
}
