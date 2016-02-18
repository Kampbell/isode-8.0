/* loc.c - search for a locality */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/de/RCS/loc.c,v 9.1 1992/08/25 15:52:32 isode Exp $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/loc.c,v 9.1 1992/08/25 15:52:32 isode Exp $
 *
 *
 * $Log: loc.c,v $
 * Revision 9.1  1992/08/25  15:52:32  isode
 * *** empty log message ***
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


#include <signal.h>
#include "quipu/util.h"
#include "quipu/read.h"
#include "quipu/list.h"
#include "quipu/ds_search.h"
#include "quipu/dua.h"
#include "util.h"
#include "query.h"
#include "demanifest.h"
#include "namelist.h"
#include "filter.h"
#include "destrings.h"

extern struct ds_list_arg larg;
extern struct ds_list_result lresult;

extern struct ds_search_arg sarg;
extern struct ds_search_result sresult;
extern struct DSError serror;
extern struct query qinfo[];

extern int abandoned;
extern int highNumber;
extern int numberType;
extern int exactMatch;
extern int fuzzyMatching;
extern int quipuMastersCo;
extern char exactString[];

struct namelist * locatts;

struct ds_list_arg *fillMostLocListArgs();
struct ds_search_arg *fillMostLocSearchArgs();

static struct query savelocinfo;

void makeExplicitLocFilter();
void locFilter1(), locFilter2(), locFilter3(), locFilter4();


VFP explicitLoc[] = {makeExplicitLocFilter, NULLVFP};
VFP normalLoc[] = {locFilter1, locFilter2, locFilter3, locFilter4, NULLVFP};

void
saveLocs() {
	copyLocs(&savelocinfo, &qinfo[LOCALITY]);
}

void
restoreSavedLocs() {
	copyLocs(&qinfo[LOCALITY], &savelocinfo);
	highNumber = qinfo[LOCALITY].listlen;
}

void
copyLocs(a, b)
struct query * a, * b;
{
	struct namelist * lp, ** slp;

	(void) strcpy(a->defvalue, b->defvalue);
	(void) strcpy(a->entered, b->entered);
	(void) strcpy(a->displayed, b->displayed);
	a->listlen = 0;
	slp = &a->lp;
	for (lp = b->lp; lp != NULLLIST; lp = lp->next) {
		*slp = list_alloc();
		(*slp)->name = copy_string(lp->name);
		(*slp)->ats = as_cpy(lp->ats);
		slp = &(*slp)->next;
		a->listlen++;
	}
	*slp = NULLLIST;
}

int
listLocs(cstr, lstr, llistp)
char * cstr, * lstr;
struct namelist ** llistp;
{
	clearProblemFlags();
	initAlarm();
	if (exactMatch == LOCALITY)
		return (readExactLoc(exactString, llistp));
	if (strcmp(lstr, "*") == 0)
		return (listAllLocs(cstr, llistp));
	else
		return (listMatchingLocs(cstr, lstr, llistp));
}

void
printListLocs(loc, llistp)
char * loc;
struct namelist * llistp;
{
	struct namelist * x;
	int i;

	if (llistp == NULLLIST)
		if (strcmp(loc, "*") == 0)
			pageprint("  No localities found\n");
		else
			pageprint("  No localities match string `%s'\n", loc);
	else {
		for (i = 1, x = llistp; x != NULLLIST; i++, x = x->next)
			printLastComponent(INDENTON, x->name, LOCALITY, i);
		showAnyProblems(loc);
	}
}

void
freeSavedLocs() {
	freeLocs(&savelocinfo.lp);
}

void
freeLocs(listpp)
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
freeLocListArgs() {
	dn_free(larg.lsa_object);
}


void
freeLocSearchArgs() {
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
listAllLocs(cstr, llistp)
char * cstr;
struct namelist ** llistp;
{
	int ret;

	if (quipuMastersCo == FALSE) {
		larg = * fillMostLocListArgs(cstr);
		ret = reallyMakeListLocs(llistp);
	} else {
		sarg = * fillMostLocSearchArgs(cstr, SRA_ONELEVEL);
		makeAllLocFilter(&sarg.sra_filter);
		ret = makeListLocanisations(llistp);
	}
	if (ret != OK)
		logListSuccess(LIST_ERROR, "loc", 0);
	else
		logListSuccess(LIST_OK, "loc", listlen(*llistp));
	if (quipuMastersCo == FALSE)
		freeLocListArgs();
	else
		freeLocSearchArgs();
	alarmCleanUp();
	return ret;
}

int
listMatchingLocs(cstr, lstr, llistp)
char * cstr, * lstr;
struct namelist ** llistp;
{
	VFP * filtarray;
	VFP filterfunc;
	int filtnumber;
	char rstr[BUFSIZ];

	if (index(lstr, '*') != NULLCP) { /* contains at least one asterisk */
		filtarray = explicitLoc;
		filtnumber = -1;
	} else {
		filtarray = normalLoc;
		filtnumber = 0;
	}
	if (quipuMastersCo ==FALSE) { /* try read first */
		(void)sprintf(rstr, "%s@l=%s", cstr, lstr);
		if (readExactLoc(rstr, llistp) == OK) {
			logReadSuccess(READ_OK, "loc");
			return OK;
		} else
			/* else try searches */
			logReadSuccess(READ_FAIL, "loc");
	}
	sarg = * fillMostLocSearchArgs(cstr, SRA_ONELEVEL);
	while ((filterfunc = *filtarray++) != NULLVFP) {
		filtnumber++;
		filterfunc(lstr, &sarg.sra_filter);
		if (sarg.sra_filter == NULLFILTER)
			continue;
		if (makeListLocanisations(llistp) != OK) {
			freeLocSearchArgs();
			logSearchSuccess(SEARCH_ERROR, "loc", lstr, filtnumber, 0);
			alarmCleanUp();
			return NOTOK;
		}
		if (*llistp != NULLLIST)
			break;
	}
	if (*llistp != NULLLIST)
		logSearchSuccess(SEARCH_OK, "loc", lstr, filtnumber, listlen(*llistp));
	else
		logSearchSuccess(SEARCH_FAIL, "loc", lstr, filtnumber, 0);
	freeLocSearchArgs();
	alarmCleanUp();
	return OK;
}

int
readExactLoc(lstr, llistp)
char * lstr;
struct namelist ** llistp;
{
	int ret;

	ret = readLoc(lstr, llistp);
	alarmCleanUp();
	return ret;
}

int
reallyMakeListLocs(llistp)
struct namelist ** llistp;
{
	struct subordinate * x;
	int retval;
	char * cp;
	DN dn1, dn2;

	if (rebind() != OK)
		return NOTOK;
	retval = ds_list(&larg, &serror, &lresult);
	if ((retval == DSE_INTR_ABANDONED) &&
			(serror.dse_type == DSE_ABANDONED))
		abandoned = TRUE;

	/* setProblemFlags(sresult); */
	highNumber = 0;
	for (x = lresult.lsr_subordinates; x != (struct subordinate *) NULL;
			x = x->sub_next) {
		/* only add locs to the list */
		cp = strdup(attr2name(x->sub_rdn->rdn_at, OIDPART));
		if (strcmp(cp, DE_LOCALITY_NAME) == 0) {
			*llistp = list_alloc();
			dn1 = dn_cpy(larg.lsa_object);
			dn2 = dn_comp_new(x->sub_rdn);
			dn_append(dn1, dn2);
			(*llistp)->name = dn2pstr(dn1);
			(*llistp)->ats = NULLATTR;
			llistp = &(*llistp)->next;
			highNumber++;

		}
		free(cp);
	}
	*llistp = NULLLIST;
	/* free something */
	dn_free (lresult.lsr_object);
	/* free something else */
	return OK;
}

int
makeListLocanisations(llistp)
struct namelist ** llistp;
{
	entrystruct * x;
	int retval;

	if (rebind() != OK)
		return NOTOK;
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
		*llistp = list_alloc();
		(*llistp)->name = dn2pstr(x->ent_dn);
		(*llistp)->ats = as_cpy(x->ent_attr);
		llistp = &(*llistp)->next;
		highNumber++;
	}
	*llistp = NULLLIST;
	entryinfo_free(sresult.CSR_entries, 0);
	dn_free (sresult.CSR_object);
	crefs_free (sresult.CSR_cr);
	filter_free(sarg.sra_filter);
	return OK;
}

int
readLoc(lstr, llistp)
char * lstr;
struct namelist ** llistp;
{
	static struct ds_read_arg rarg;
	static struct ds_read_result rresult;
	static struct DSError rerror;
	static CommonArgs sca = default_common_args;
	Attr_Sequence * atl;
	AttributeType at;
	struct namelist * x;
	int retval;

	if (rebind() != OK)
		return NOTOK;
	rarg.rda_common =sca; /* struct copy */
	rarg.rda_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	rarg.rda_common.ca_servicecontrol.svc_sizelimit= SVC_NOSIZELIMIT;
	rarg.rda_object = str2dn(lstr);
	/* specify attributes of interest */
	rarg.rda_eis.eis_allattributes = FALSE;
	atl = &(rarg.rda_eis.eis_select);
	for (x = locatts; x != NULLLIST; x = x->next) {
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
	if ((retval == DSE_INTR_ABANDONED) &&
			(rerror.dse_type == DSE_ABANDONED))
		abandoned = TRUE;
	if (retval != OK)
		return NOTOK;
	/* setProblemFlags(sresult); */
	highNumber = 1;
	*llistp = list_alloc();
	(*llistp)->name = dn2pstr(rresult.rdr_entry.ent_dn);
	(*llistp)->ats = as_cpy(rresult.rdr_entry.ent_attr);
	(*llistp)->next = NULLLIST;
	return retval;
}

struct ds_list_arg *
fillMostLocListArgs(str)
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

struct ds_search_arg *
fillMostLocSearchArgs(cstr, searchdepth)
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

	arg.sra_subset = searchdepth;
	if (strcmp(cstr, "root") == 0)
		arg.sra_baseobject = NULLDN;
	else
		arg.sra_baseobject = str2dn(cstr);
	arg.sra_searchaliases = TRUE;
	/* specify attributes of interest */
	arg.sra_eis.eis_allattributes = FALSE;
	atl = &(arg.sra_eis.eis_select);
	for (x = locatts; x != NULLLIST; x = x->next) {
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

makeAllLocFilter(fpp)
struct s_filter ** fpp;
{
	*fpp = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_LOCALITY);
}

void
makeExplicitLocFilter(lstr, fpp)
char * lstr;
struct s_filter ** fpp;
{
	struct s_filter * fp;
	int wildcardtype;
	char * lstr1, * lstr2;

	wildcardtype = starstring(lstr, &lstr1, &lstr2);
	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_LOCALITY);
	switch (wildcardtype) {
	case LEADSUBSTR: /* fall through */
	case TRAILSUBSTR: /* fall through */
	case ANYSUBSTR:
		fp = fp->flt_next = subsfilter(wildcardtype,
									   DE_LOCALITY_NAME, lstr1);
		break;
	case LEADANDTRAIL:
		fp = fp->flt_next = subsfilter(LEADSUBSTR,
									   DE_LOCALITY_NAME, lstr1);
		fp = fp->flt_next = subsfilter(TRAILSUBSTR,
									   DE_LOCALITY_NAME, lstr2);
		break;
	}
	fp->flt_next = NULLFILTER;
}

void
locFilter1(lstr, fpp)
char * lstr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_LOCALITY);
	fp = fp->flt_next = eqfilter(FILTERITEM_EQUALITY, DE_LOCALITY_NAME, lstr);
	fp->flt_next = NULLFILTER;
}

void
locFilter2(lstr, fpp)
char * lstr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_LOCALITY);
	fp = fp->flt_next = subsfilter(LEADSUBSTR, DE_LOCALITY_NAME, lstr);
	fp->flt_next = NULLFILTER;
}

void
locFilter3(lstr, fpp)
char * lstr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_LOCALITY);
	fp = fp->flt_next = subsfilter(ANYSUBSTR, DE_LOCALITY_NAME, lstr);
	fp->flt_next = NULLFILTER;
}

void
locFilter4(lstr, fpp)
char * lstr;
struct s_filter ** fpp;
{
	struct s_filter * fp;

	if (fuzzyMatching == FALSE) {
		*fpp = NULLFILTER;
		return;
	}
	*fpp = andfilter();
	fp = (*fpp)->FUFILT = eqfilter(FILTERITEM_EQUALITY, DE_OBJECT_CLASS, DE_LOCALITY);
	fp = fp->flt_next = eqfilter(FILTERITEM_APPROX, DE_LOCALITY_NAME, lstr);
	fp->flt_next = NULLFILTER;
}

