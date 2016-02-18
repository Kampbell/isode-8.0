#include <signal.h>
#include "quipu/util.h"
#include "quipu/read.h"
#include "quipu/list.h"
#include "quipu/ds_search.h"
#include "quipu/dua.h"
#include "demanifest.h"
#include "namelist.h"
#include "filter.h"
#include "destrings.h"

extern struct ds_list_arg larg;
extern struct ds_list_result lresult;

extern struct ds_search_arg sarg;
extern struct ds_search_result sresult;
extern struct DSError serror;

extern int abandoned;
extern int highNumber;
extern int numberType;
extern int exactMatch;
extern int quipuMastersCo;
extern int rfrl_msg;
extern char exactString[];

struct namelist * orgatts;

struct ds_list_arg *fillMostOrgListArgs();
struct ds_search_arg *fillMostOrgSearchArgs();

void makeExplicitOrgFilter();
void orgFilter1(), orgFilter2(), orgFilter3(), orgFilter4();

VFP explicitOrg[] = {makeExplicitOrgFilter, NULLVFP};
VFP normalOrg[] = {orgFilter1, orgFilter2, orgFilter3, orgFilter4, NULLVFP};

int
listOrgs(cstr, ostr, olistp)
char * cstr, * ostr;
struct namelist ** olistp;
{
	clearProblemFlags();
	initAlarm();
	if (exactMatch == ORG)
		return (readExactOrg(exactString, olistp));
	if (strcmp(ostr, "*") == 0)
		return (listAllOrgs(cstr, olistp));
	else
		return (listMatchingOrgs(cstr, ostr, olistp));
}

void
printListOrgs(org, olistp)
char * org;
struct namelist * olistp;
{
	struct namelist * x;
	int i;

	if (olistp == NULLLIST)
		if (strcmp(org, "*") == 0)
			pageprint("  No organisations found\n");
		else
			pageprint("  No organisations match string `%s'\n", org);
	else {
		for (i = 1, x = olistp; x != NULLLIST; i++, x = x->next)
			printLastComponent(INDENTON, x->name, ORG, i);
		/*		showAnyProblems(org); */
		showAnyProblems();
	}
}

void
freeOrgs(listpp)
struct namelist ** listpp;
{
	struct namelist * x, * y;

	x = *listpp;
	while (x != NULLLIST) {
		if (x->name != NULLCP)
			free(x->name);
		as_free(x->ats);
		y = x->next;
		free((char *)x);
		x = y;
	}
	*listpp = NULLLIST;
}

void
freeOrgListArgs() {
	dn_free(larg.lsa_object);
}

void
freeOrgSearchArgs() {
	/*
	Attr_Sequence atl, x;
	*/

	dn_free(sarg.sra_baseobject);
	as_free(sarg.sra_eis.eis_select);
	/*
	for (atl = sarg.sra_eis.eis_select; atl != NULLATTR; atl = x)
	{
		x = atl->attr_link;
		as_comp_free(atl);
	}
	*/
}


int
listAllOrgs(cstr, olistp)
char * cstr;
struct namelist ** olistp;
{
	int ret;

	if (quipuMastersCo == FALSE) {
		larg = * fillMostOrgListArgs(cstr);
		ret = reallyMakeListOrgs(olistp);
	} else {
		sarg = * fillMostOrgSearchArgs(cstr, SRA_ONELEVEL);
		makeAllOrgFilter(&sarg.sra_filter);
		ret = makeListOrganisations(olistp);
	}

	if (quipuMastersCo == FALSE) {
		freeOrgListArgs();
	} else {
		freeOrgSearchArgs();
	}
	alarmCleanUp();
	return ret;
}


int
readOrg(ostr, olistp)
char * ostr;
struct namelist ** olistp;
{
	static struct ds_read_arg rarg;
	static struct ds_read_result rresult;
	static struct DSError rerror;
	static CommonArgs sca = default_common_args;
	Attr_Sequence * atl;
	AttributeType at;
	struct namelist * x;
	int retval;

	rarg.rda_common =sca; /* struct copy */
	rarg.rda_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	rarg.rda_common.ca_servicecontrol.svc_sizelimit= SVC_NOSIZELIMIT;
	rarg.rda_object = str2dn(ostr);
	/* specify attributes of interest */
	rarg.rda_eis.eis_allattributes = FALSE;
	atl = &(rarg.rda_eis.eis_select);
	for (x = orgatts; x != NULLLIST; x = x->next) {
		if ((at = str2AttrT(x->name)) == NULLAttrT)
			continue;
		*atl = as_comp_alloc();
		(*atl)->attr_type = at;
		(*atl)->attr_value = NULLAV;
		atl = &(*atl)->attr_link;
	}
	*atl = NULLATTR;
	rarg.rda_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	retval = ds_read(&rarg, &rerror, &rresult);
	highNumber = 1;
	*olistp = list_alloc();
	(*olistp)->name = dn2pstr(rresult.rdr_entry.ent_dn);
	(*olistp)->ats = as_cpy(rresult.rdr_entry.ent_attr);
	(*olistp)->next = NULLLIST;
	return retval;
}

struct ds_list_arg *
fillMostOrgListArgs(str)
char * str;
{
	static struct ds_list_arg arg;
	static CommonArgs sca = default_common_args;

	arg.lsa_common =sca; /* struct copy */
	arg.lsa_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	arg.lsa_common.ca_servicecontrol.svc_sizelimit= SVC_NOSIZELIMIT;
	arg.lsa_object = str2dn(str);
	return (&arg);
}


int
readExactOrg(ostr, olistp)
char * ostr;
struct namelist ** olistp;
{
	int ret;

	ret = readOrg(ostr, olistp);
	alarmCleanUp();
	return ret;
}

int
reallyMakeListOrgs(olistp)
struct namelist ** olistp;
{
	struct subordinate * x;
	int retval;
	char * cp;
	DN dn1, dn2;
	int status;

list_orgs:
	status = ds_list(&larg, &serror, &lresult);
	if (status != OK) {
		status = check_error(serror);
		if (status == REFERRAL) {
			rfrl_msg = FALSE;
			goto list_orgs;
		} else {
			return NOTOK;
		}
	}

	highNumber = 0;
	for (x = lresult.lsr_subordinates; x != (struct subordinate *) NULL;
			x = x->sub_next) {
		/* only add orgs to the list */
		cp = strdup(attr2name(x->sub_rdn->rdn_at, OIDPART));
		if (strcmp(cp, DE_ORGANISATION_NAME) == 0) {
			*olistp = list_alloc();
			dn1 = dn_cpy(larg.lsa_object);
			dn2 = dn_comp_new(x->sub_rdn);
			dn_append(dn1, dn2);
			(*olistp)->name = dn2pstr(dn1);
			(*olistp)->ats = NULLATTR;
			olistp = &(*olistp)->next;
			highNumber++;
		}
		free(cp);
	}
	*olistp = NULLLIST;
	/* free something */
	dn_free (lresult.lsr_object);
	/* free something else */
	return OK;
}


int
listMatchingOrgs(cstr, ostr, olistp)
char * cstr, * ostr;
struct namelist ** olistp;
{
	VFP * filtarray;
	VFP filterfunc;
	int filtnumber;
	char rstr[LINESIZE];

	if (index(ostr, '*') != NULLCP) { /* contains at least one asterisk */
		filtarray = explicitOrg;
		filtnumber = -1;
	} else {
		filtarray = normalOrg;
		filtnumber = 0;
	}

	if (quipuMastersCo ==FALSE) { /* try read first */
		(void)sprintf(rstr, "%s@o=%s", cstr, ostr);
		if (readExactOrg(rstr, olistp) == OK) {
			return OK;
		}
	}

	sarg = * fillMostOrgSearchArgs(cstr, SRA_ONELEVEL);
	while ((filterfunc = *filtarray++) != NULLVFP) {
		filtnumber++;
		filterfunc(ostr, &sarg.sra_filter);
		if (makeListOrganisations(olistp) != OK) {
			freeOrgSearchArgs();
			logSearchSuccess(SEARCH_ERROR, "org", ostr, filtnumber, 0);
			alarmCleanUp();
			return NOTOK;
		}
		if (*olistp != NULLLIST)
			break;
	}
	if (*olistp != NULLLIST)
		logSearchSuccess(SEARCH_OK, "org", ostr, filtnumber, listlen(*olistp));
	else
		logSearchSuccess(SEARCH_FAIL, "org", ostr, filtnumber, 0);
	freeOrgSearchArgs();
	alarmCleanUp();
	return OK;
}

int
listExactOrgs(ostr, olistp)
char * ostr;
struct namelist ** olistp;
{
	int ret;

	sarg = * fillMostOrgSearchArgs(ostr, SRA_BASEOBJECT);
	makeAllOrgFilter(&sarg.sra_filter);
	ret = makeListOrganisations(olistp);
	freeOrgSearchArgs();
	alarmCleanUp();
	return ret;
}

int
makeListOrganisations(olistp)
struct namelist ** olistp;
{
	entrystruct * x;
	int retval;

	retval = ds_search(&sarg, &serror, &sresult);
	if ((retval == DSE_INTR_ABANDONED) &&
			(serror.dse_type == DSE_ABANDONED))
		abandoned = TRUE;
	if (retval != OK)
		return NOTOK;
	correlate_search_results (&sresult);

	setProblemFlags(sresult);

	highNumber = 0;
	for (x = sresult.CSR_entries; x != NULLENTRYINFO; x = x->ent_next) {
		*olistp = list_alloc();
		(*olistp)->name = dn2pstr(x->ent_dn);
		(*olistp)->ats = as_cpy(x->ent_attr);
		olistp = &(*olistp)->next;
		highNumber++;
	}
	*olistp = NULLLIST;
	entryinfo_free(sresult.CSR_entries, 0);
	dn_free (sresult.CSR_object);
	crefs_free (sresult.CSR_cr);
	filter_free(sarg.sra_filter);
	return OK;
}

struct ds_search_arg *
fillMostOrgSearchArgs(cstr, searchdepth)
char * cstr;
int searchdepth;
{
	static struct ds_search_arg arg;
	Attr_Sequence * atl;
	AttributeType at;
	struct namelist * x;
	static CommonArgs sca = default_common_args;

	arg.sra_common = sca; /* struct copy */
	arg.sra_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	arg.sra_common.ca_servicecontrol.svc_sizelimit= SVC_NOSIZELIMIT;

	/*        arg.sra_common.ca_servicecontrol.svc_options = (SVC_OPT_CHAININGPROHIBIT |
		                                                SVC_OPT_DONTUSECOPY); */

	arg.sra_subset = searchdepth;
	if (strcmp(cstr, "root") == 0)
		arg.sra_baseobject = NULLDN;
	else
		arg.sra_baseobject = str2dn(cstr);
	arg.sra_searchaliases = TRUE;
	/* specify attributes of interest */
	arg.sra_eis.eis_allattributes = FALSE;
	atl = &(arg.sra_eis.eis_select);
	for (x = orgatts; x != NULLLIST; x = x->next) {
		if ((at = str2AttrT(x->name)) == NULLAttrT)
			continue;
		*atl = as_comp_alloc();
		(*atl)->attr_type = at;
		(*atl)->attr_value = NULLAV;
		atl = &(*atl)->attr_link;
	}
	*atl = NULLATTR;
	arg.sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	return (&arg);
}

makeAllOrgFilter(fpp)
struct s_filter ** fpp;
{
	*fpp = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_ORGANISATION);
}

void
makeExplicitOrgFilter(ostr, fpp)
char * ostr;
struct s_filter ** fpp;
{
	struct s_filter * fp;
	int wildcardtype;
	char * ostr1, * ostr2;

	wildcardtype = starstring(ostr, &ostr1, &ostr2);
	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_ORGANISATION);
	switch (wildcardtype) {
	case LEADSUBSTR: /* fall through */
	case TRAILSUBSTR: /* fall through */
	case ANYSUBSTR:
		fp = fp->flt_next = subsfilter(wildcardtype,
									   DE_ORGANISATION_NAME, ostr1);
		break;
	case LEADANDTRAIL:
		fp = fp->flt_next = subsfilter(LEADSUBSTR,
									   DE_ORGANISATION_NAME, ostr1);
		fp = fp->flt_next = subsfilter(TRAILSUBSTR,
									   DE_ORGANISATION_NAME, ostr2);
		break;
	}
	fp->flt_next = NULLFILTER;
}

void
orgFilter1(ostr, fpp)
char * ostr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_ORGANISATION);
	fp = fp->flt_next = eqfilter(FILTERITEM_EQUALITY, DE_ORGANISATION_NAME, ostr);
	fp->flt_next = NULLFILTER;
}

void
orgFilter2(ostr, fpp)
char * ostr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_ORGANISATION);
	fp = fp->flt_next = subsfilter(LEADSUBSTR, DE_ORGANISATION_NAME, ostr);
	fp->flt_next = NULLFILTER;
}

void
orgFilter3(ostr, fpp)
char * ostr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_ORGANISATION);
	fp = fp->flt_next = subsfilter(ANYSUBSTR, DE_ORGANISATION_NAME, ostr);
	fp->flt_next = NULLFILTER;
}

void
orgFilter4(ostr, fpp)
char * ostr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_ORGANISATION);
	fp = fp->flt_next = eqfilter(FILTERITEM_APPROX, DE_ORGANISATION_NAME, ostr);
	fp->flt_next = NULLFILTER;
}

