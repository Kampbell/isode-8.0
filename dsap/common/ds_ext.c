/* ds_ext.c - */

#ifndef lint
static char *rcsid = "$Header: /xtel/isode/isode/dsap/common/RCS/ds_ext.c,v 9.0 1992/06/16 12:12:39 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/dsap/common/RCS/ds_ext.c,v 9.0 1992/06/16 12:12:39 isode Rel $
 *
 *
 * $Log: ds_ext.c,v $
 * Revision 9.0  1992/06/16  12:12:39  isode
 * Release 8.0
 *
 */

/*
 *                                NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include "quipu/util.h"
#include "quipu/common.h"

extern	LLog	* log_dsap;

int
subords_free (struct subordinate *subp) {
	if(subp == NULLSUBORD)
		return;
	subords_free(subp->sub_next);
	rdn_free(subp->sub_rdn);
	free((char *)subp);
}

int
ems_free (struct entrymod *emp) {
	if(emp == NULLMOD)
		return;
	ems_free(emp->em_next);
	as_free(emp->em_what);
	free((char *)emp);
}

int
aps_free (struct access_point *app) {
	if(app == NULLACCESSPOINT)
		return;

	aps_free(app->ap_next);
	dn_free(app->ap_name);
	if (app->ap_address)
		psap_free (app->ap_address);

	free((char *)app);
}

crefs_free(crefp)
ContinuationRef crefp;
{
	if(crefp == NULLCONTINUATIONREF)
		return;
	crefs_free(crefp->cr_next);
	dn_free(crefp->cr_name);
	aps_free(crefp->cr_accesspoints);
	free((char *)crefp);
}

