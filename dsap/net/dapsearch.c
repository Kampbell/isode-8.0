/* dapsearch.c - */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/dsap/net/RCS/dapsearch.c,v 9.0 1992/06/16 12:14:05 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/dsap/net/RCS/dapsearch.c,v 9.0 1992/06/16 12:14:05 isode Rel $
 *
 *
 * $Log: dapsearch.c,v $
 * Revision 9.0  1992/06/16  12:14:05  isode
 * Release 8.0
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


/* LINTLIBRARY */

#include "quipu/util.h"
#include "quipu/dap2.h"
#include "../x500as/DAS-types.h"

int
dap_search (int ad, int *id, struct ds_search_arg *arg, struct DSError *error, struct ds_search_result *result) {
	struct DAPindication	  di_s;
	struct DAPindication	* di = &(di_s);

	++(*id);

	DapSearch (ad, (*id), arg, di, ROS_INTR);

	error->dse_type = DSE_NOERROR;

	switch (di->di_type) {
	case DI_RESULT: {
		struct DAPresult	* dr = &(di->di_result);

		/* Nasty struct copy */
		(*result) = dr->dr_res.res_sr;      /* struct copy */
		dr->dr_res.result_type = -1;        /* Prevent freeing */
		DRFREE (dr);
		return (DS_OK);
	}

	case DI_ERROR: {
		struct DAPerror	* de = &(di->di_error);

		(*error) = de->de_err;	/* struct copy */
		return (DS_ERROR_REMOTE);
	}

	case DI_PREJECT:
		error->dse_type = DSE_REMOTEERROR;
		return (DS_ERROR_PROVIDER);

	case DI_ABORT:
		error->dse_type = DSE_REMOTEERROR;
		return (DS_ERROR_CONNECT);

	default:
		error->dse_type = DSE_REMOTEERROR;
		return (DS_ERROR_PROVIDER);
	}
}

int
DapSearch (int ad, int id, struct ds_search_arg *arg, struct DAPindication *di, int asyn) {
	PE                  arg_pe;

	if(encode_DAS_SearchArgument(&arg_pe,1,0,NULLCP,arg) != OK) {
		return(dapreject (di, DP_INVOKE, id, NULLCP, "Search argument encoding failed"));
	}

	return (DapInvokeReqAux (ad, id, OP_SEARCH, arg_pe, di, asyn));

}

