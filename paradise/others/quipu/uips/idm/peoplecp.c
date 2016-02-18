
#include <signal.h>
#include "quipu/util.h"
#include "quipu/ds_search.h"
#include "quipu/dua.h"
#include "filter.h"
#include "demanifest.h"
#include "namelist.h"
#include "destrings.h"

extern struct ds_search_arg sarg;
extern struct ds_search_result sresult;
extern struct DSError serror;

extern int abandoned;
extern int highNumber;
extern int exactMatch;
extern char exactString[];

/* PRR stands fpr people, rooms and roles */

struct namelist * prratts;

struct ds_search_arg *fillMostPRRSearchArgCp();

void makeExplicitPRRFilter();
void prrFilter1(), prrFilter2(), prrFilter3(), prrFilter4();

VFP explicitPRRCp[] = {makeExplicitPRRFilter, NULLVFP};
VFP normalPRRCp[] = {prrFilter1, prrFilter2, prrFilter3, prrFilter4, NULLVFP};
/*VFP normalPRR[] = {prrFilter1, prrFilter3, NULLVFP};*/

int
listPRRCp(parentstr, thisstr, listp)
char * parentstr, * thisstr;
struct namelist ** listp;
{
	clearProblemFlags();
	initAlarm();
	if (exactMatch == PERSON)
		return(listExactPRRCp(exactString, listp));
	if (strcmp(thisstr, "*") == 0)
		return (listAllPRRCp(parentstr, listp));
	else
		return (listMatchingPRRCp(parentstr, thisstr, listp));
}

int
listAllPRRCp(parentstr, listp)
char * parentstr;
struct namelist ** listp;
{
	int ret;

	sarg = * fillMostPRRSearchArgCp(parentstr, SRA_WHOLESUBTREE);
	makeAllPRRFilter(&sarg.sra_filter);
	ret = makeListPRRs(listp, parentstr);
	if (ret != OK)
		logListSuccess(LIST_ERROR, "prr", 0);
	else
		logListSuccess(LIST_OK, "prr", listlen(*listp));
	freePRRSearchArgs();
	alarmCleanUp();
	return ret;
}

int
listMatchingPRRCp(parentstr, thisstr, listp)
char * parentstr, * thisstr;
struct namelist ** listp;
{
	VFP * filtarray;
	VFP filterfunc;
	int filtnumber;

	if (index(thisstr, '*') != NULLCP) { /* contains at least one asterisk */
		filtarray = explicitPRRCp;
		filtnumber = -1;
	} else {
		filtarray = normalPRRCp;
		filtnumber = 0;
	}
	sarg = * fillMostPRRSearchArgCp(parentstr, SRA_WHOLESUBTREE);
	while ((filterfunc = *filtarray++) != NULLVFP) {
		filtnumber++;
		filterfunc(thisstr, &sarg.sra_filter);
		if (sarg.sra_filter == NULLFILTER)
			continue;
		if (makeListPRRs(listp, parentstr) != OK) {
			freePRRSearchArgs();
			logSearchSuccess(SEARCH_ERROR, "prr", thisstr, filtnumber, 0);
			alarmCleanUp();
			return NOTOK;
		}
		if (*listp != NULLLIST)
			break;
	}
	if (*listp != NULLLIST)
		logSearchSuccess(SEARCH_OK, "prr", thisstr, filtnumber, listlen(*listp));
	else
		logSearchSuccess(SEARCH_FAIL, "prr", thisstr, filtnumber, 0);
	freePRRSearchArgs();
	alarmCleanUp();
	return OK;
}

int
listExactPRRCp(objectstr, listp)
char * objectstr;
struct namelist ** listp;
{
	int ret;

	sarg = * fillMostPRRSearchArgCp(objectstr, SRA_BASEOBJECT);
	makeExactPRRFilter(&sarg.sra_filter);
	ret = makeListPRRs(listp, objectstr);
	freePRRSearchArgs();
	alarmCleanUp();
	return ret;
}

struct ds_search_arg *
fillMostPRRSearchArgCp(parentstr, searchdepth)
char * parentstr;
int searchdepth;
{
	static struct ds_search_arg arg;
	Attr_Sequence * atl;
	AttributeType at;
	struct namelist * x;
	static CommonArgs sca = default_common_args;

	arg.sra_common = sca; /* struct copy */
	arg.sra_common.ca_aliased_rdns = TRUE;
	arg.sra_common.ca_servicecontrol.svc_options = SVC_OPT_PREFERCHAIN;
	arg.sra_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
	arg.sra_common.ca_servicecontrol.svc_sizelimit = SVC_NOSIZELIMIT;

	arg.sra_subset = searchdepth;
	arg.sra_baseobject = str2dn(parentstr);
	arg.sra_searchaliases = TRUE;
	/* specify attributes of interest */
	arg.sra_eis.eis_allattributes = FALSE;
	atl = &(arg.sra_eis.eis_select);
	for (x = prratts; x != NULLLIST; x = x->next) {
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
