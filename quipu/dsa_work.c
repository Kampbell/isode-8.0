/* dsa_work.c - do some work on an active task. */

#ifndef lint
static char *rcsid = "$Header: /xtel/isode/isode/quipu/RCS/dsa_work.c,v 9.0 1992/06/16 12:34:01 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/quipu/RCS/dsa_work.c,v 9.0 1992/06/16 12:34:01 isode Rel $
 *
 *
 * $Log: dsa_work.c,v $
 * Revision 9.0  1992/06/16  12:34:01  isode
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


#include "acsap.h"
#include "quipu/util.h"
#include "quipu/common.h"
#include "quipu/connection.h"

extern LLog * log_dsap;
extern char quipu_shutdown;

int
dsa_work (struct task_act *tk) {
	struct DSArgument   *arg;
	struct DSError      *err;
	struct DSResult     *res;
	DN                  orig = NULLDN;
	DN                  base = NULLDN;
	struct ds_search_task	**local;
	struct ds_search_task	**refer;
	int                 dsa_ret;
	struct di_block	    * di;
	char		    dsp;
	char		    authtype;

	arg = &(tk->tk_dx.dx_arg.dca_dsarg);
	res = &(tk->tk_resp.di_result.dr_res.dcr_dsres);
	err = &(tk->tk_resp.di_error.de_err);
	local = &(tk->local_st);
	refer = &(tk->refer_st);

	if (quipu_shutdown) {
		err->dse_type = DSE_SERVICEERROR;
		err->ERR_SERVICE.DSE_sv_problem = DSE_SV_UNAVAILABLE;
		tk->tk_resp.di_type = DI_ERROR;
		tk->tk_error = &(tk->tk_resp.di_error.de_err);
		task_conn_extract(tk);
		task_error(tk);
		if (tk->tk_conn->cn_ctx == DS_CTX_X500_DAP)
			force_close (tk->tk_conn->cn_ad,
						 (struct DSAPindication *)NULL);
		else
			conn_release(tk->tk_conn);

		task_extract(tk);

		return;
	}


	if(tk->tk_conn->cn_ctx == DS_CTX_X500_DAP) {
		orig = tk->tk_conn->cn_dn;
		dsp = FALSE;
	} else {
		if (tk->tk_dx.dx_arg.dca_charg.cha_originator == NULLDN) {
			switch (arg->arg_type) {
			case OP_READ:
				orig = arg->arg_rd.rda_common.ca_requestor;
				break;
			case OP_COMPARE:
				orig = arg->arg_cm.cma_common.ca_requestor;
				break;
			case OP_LIST:
				orig = arg->arg_ls.lsa_common.ca_requestor;
				break;
			case OP_SEARCH:
				orig = arg->arg_sr.sra_common.ca_requestor;
				break;
			case OP_ADDENTRY:
				orig = arg->arg_ad.ada_common.ca_requestor;
				break;
			case OP_REMOVEENTRY:
				orig = arg->arg_rm.rma_common.ca_requestor;
				break;
			case OP_MODIFYENTRY:
				orig = arg->arg_me.mea_common.ca_requestor;
				break;
			case OP_MODIFYRDN:
				orig = arg->arg_mr.mra_common.ca_requestor;
				break;
			default:
				orig = NULLDN;
				break;
			}
		} else
			orig = tk->tk_dx.dx_arg.dca_charg.cha_originator;

		if (tk->tk_dx.dx_arg.dca_charg.cha_target == NULLDN) {
			switch (arg->arg_type) {
			case OP_READ:
				base = arg->arg_rd.rda_object;
				break;
			case OP_COMPARE:
				base = arg->arg_cm.cma_object;
				break;
			case OP_LIST:
				base = arg->arg_ls.lsa_object;
				break;
			case OP_SEARCH:
				base = arg->arg_sr.sra_baseobject;
				break;
			case OP_ADDENTRY:
				base = arg->arg_ad.ada_object;
				break;
			case OP_REMOVEENTRY:
				base = arg->arg_rm.rma_object;
				break;
			case OP_MODIFYENTRY:
				base = arg->arg_me.mea_object;
				break;
			case OP_MODIFYRDN:
				base = arg->arg_mr.mra_object;
				break;
			case OP_GETEDB:
				base = arg->arg_ge.ga_entry;
				break;
			default:
				base = NULLDN;
				break;
			}
		} else
			base = tk->tk_dx.dx_arg.dca_charg.cha_target;

		dsp = TRUE;

	}


	DLOG (log_dsap,LLOG_TRACE,( "Apply operation"));

#ifndef NO_STATS
	if (*local == NULL_ST)
		log_x500_event (arg,tk->tk_conn->cn_ctx,orig,base,
						tk->tk_conn->cn_ad,tk);
#endif

	authtype = tk->tk_conn->cn_authen;
	if (!dsp && authtype == DBA_AUTH_NONE) {
		orig = NULLDN;
	}

	switch(arg->arg_type) {
	case OP_READ:
		dsa_ret = do_ds_read(&(arg->arg_rd), err, &(res->res_rd), orig, base,
							 &(di), dsp, tk->tk_conn->cn_ctx == DS_CTX_QUIPU_DSP, authtype);
		break;

	case OP_COMPARE:
		dsa_ret = do_ds_compare(&(arg->arg_cm), err, &(res->res_cm), orig,
								base, &(di), dsp, authtype);
		break;

	case OP_ABANDON:
		LLOG(log_dsap, LLOG_FATAL, ("Abandon being applied!"));
		dsa_ret = do_ds_abandon(&(arg->arg_ab), err);
		break;

	case OP_LIST:
		dsa_ret = do_ds_list(&(arg->arg_ls), err, &(res->res_ls), orig, base,
							 &(di), dsp, authtype);
		break;

	case OP_SEARCH:

		dsa_ret = do_ds_search(&(arg->arg_sr), err, &(res->res_sr), orig, base,
							   local, refer, &(di), dsp,
							   tk->tk_conn->cn_ctx == DS_CTX_QUIPU_DSP,
							   tk->tk_timed ? tk->tk_timeout : (time_t) 0,
							   tk->tk_dx.dx_arg.dca_charg.cha_entryonly, authtype);

		search_continue (tk);

		break;

	case OP_ADDENTRY:
		dsa_ret = do_ds_addentry(&(arg->arg_ad), err, orig, base, &(di),
								 dsp, authtype);
		break;

	case OP_REMOVEENTRY:
		dsa_ret = do_ds_removeentry(&(arg->arg_rm), err, orig, base, &(di),
									dsp, authtype);
		break;

	case OP_MODIFYENTRY:
		dsa_ret = do_ds_modifyentry(&(arg->arg_me), err, orig, base, &(di),
									dsp, authtype);
		break;

	case OP_MODIFYRDN:
		dsa_ret = do_ds_modifyrdn(&(arg->arg_mr), err, orig, base, &(di),
								  dsp, authtype);
		break;

	case OP_GETEDB:
		orig = tk->tk_conn->cn_dn;
		dsa_ret = do_get_edb (&(arg->arg_ge), err, &(res->res_ge), orig,
							  tk->tk_conn->cn_ad);
		break;

	default:
		LLOG (log_dsap,LLOG_EXCEPTIONS,( "Unknown operation type!"));
		break;
	}

	DLOG (log_dsap,LLOG_TRACE,( "Activity applied"));

	switch(dsa_ret) {
	case DS_OK:
		DLOG (log_dsap,LLOG_DEBUG,( "dsa_work - DS_OK"));
		/* Task completed successfully: send result */
		tk->tk_result = &(tk->tk_resp.di_result.dr_res);
		tk->tk_result->dcr_dsres.result_type = tk->tk_dx.dx_arg.dca_dsarg.arg_type;
		tk->tk_resp.di_type = DI_RESULT;

		if((tk->referred_st != NULL_ST) || (tk->tk_operlist != NULLOPER)) {
			tk->tk_state = TK_PASSIVE;
			break; /* Go wait for operations to return */
		}
		task_conn_extract(tk);
		task_result(tk);
		task_extract(tk);
		break;
	case DS_X500_ERROR:
		DLOG (log_dsap,LLOG_DEBUG,( "dsa_work - DS_X500_ERROR"));
		/* Task produced error: send error */
		tk->tk_resp.di_type = DI_ERROR;
		tk->tk_error = &(tk->tk_resp.di_error.de_err);
		task_conn_extract(tk);
		task_error(tk);
		if (tk->tk_dx.dx_arg.dca_dsarg.arg_type == OP_SEARCH) {
			free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_un.srr_unit);
			free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_next->srr_un.srr_unit);
			free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_next);
		}
		task_extract(tk);
		break;
	case DS_CONTINUE:
		DLOG (log_dsap,LLOG_DEBUG,( "dsa_work - DS_CONTINUE"));
#ifdef DEBUG
		di_list_log(di);
#endif
		/* Task produced list of dsas: chain operation or send referral */
		if(task_chain(tk, di) != OK) {
			task_conn_extract(tk);
			task_error(tk);
			if (tk->tk_dx.dx_arg.dca_dsarg.arg_type == OP_SEARCH) {
				free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_un.srr_unit);
				free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_next->srr_un.srr_unit);
				free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_next);
			}
			task_extract(tk);
		} else {
			tk->tk_state = TK_PASSIVE;
		}
		break;
	case DS_SUSPEND:
		DLOG (log_dsap,LLOG_DEBUG,( "dsa_work - DS_SUSPEND"));
		/* Task has suspended itself to check network and other tasks */
		if((tk->referred_st != NULL_ST) || (tk->tk_operlist != NULLOPER))
			/* doing things over the net - no need to hurry !!! */
			tk->tk_prio = DSA_PRIO_LOW;
		tk->tk_state = TK_SUSPEND;
		break;
	default:
		/* Local error */
		LLOG(log_dsap, LLOG_EXCEPTIONS, ("dsa_work - do_ds_OP() failed"));
		tk->tk_resp.di_error.de_err.dse_type = DSE_SERVICEERROR;
		tk->tk_resp.di_error.de_err.ERR_SERVICE.DSE_sv_problem = DSE_SV_DITERROR;
		task_conn_extract(tk);
		task_error(tk);
		if (tk->tk_dx.dx_arg.dca_dsarg.arg_type == OP_SEARCH) {
			free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_un.srr_unit);
			free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_next->srr_un.srr_unit);
			free ((char *)tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr.srr_next);
		}
		task_extract(tk);
		break;
	}

}

int
search_continue (struct task_act *tk) {
	struct ds_search_result * tk_sr = &(tk->tk_resp.di_result.dr_res.dcr_dsres.res_sr);

	/* Set up next part of search result to collate remote sub-searches */
	if(tk_sr->srr_next == NULLSRR) {
		DLOG(log_dsap, LLOG_DEBUG, ("Allocating a search result"));
		tk_sr->srr_next = (struct ds_search_result *) calloc(1, sizeof(struct ds_search_result));
		tk_sr->srr_next->srr_correlated = TRUE;
		tk_sr->srr_next->srr_un.srr_unit = (struct ds_search_unit *) calloc(1, sizeof(struct ds_search_unit));
		tk_sr->srr_next->CSR_limitproblem = LSR_NOLIMITPROBLEM;
	}

	/* Map any new elements in the refer list onto opers */
	subtask_chain(tk);

	if((tk->local_st != NULL_ST) && (tk->tk_state == TK_PASSIVE)) {
		tk->tk_state = TK_ACTIVE;
	}
}


#ifndef NO_STATS
int
log_x500_event (struct DSArgument *arg, int context, DN orig, DN dsptarget, int ad, struct task_act *tk) {
	extern LLog * log_stat;
	char * op;
	DN daptarget = NULLDN;
	char buf [LINESIZE];

	switch (arg->arg_type) {
	case OP_READ:
		op = "Read";
		daptarget = arg->arg_rd.rda_object;
		break;
	case OP_COMPARE:
		op = "Compare";
		daptarget = arg->arg_cm.cma_object;
		break;
	case OP_ABANDON:
		op = "Abandon";
		break;
	case OP_LIST:
		op = "List";
		daptarget = arg->arg_ls.lsa_object;
		break;
	case OP_SEARCH:
		op = "Search";
		daptarget = arg->arg_sr.sra_baseobject;
		break;
	case OP_ADDENTRY:
		op = "Add";
		daptarget = arg->arg_ad.ada_object;
		break;
	case OP_REMOVEENTRY:
		op = "Remove";
		daptarget = arg->arg_rm.rma_object;
		break;
	case OP_MODIFYENTRY:
		op = "Modify";
		daptarget = arg->arg_me.mea_object;
		break;
	case OP_MODIFYRDN:
		op = "Modifyrdn";
		daptarget = arg->arg_mr.mra_object;
		break;
	case OP_GETEDB:
		if (arg->arg_ge.ga_maxEntries == 0)
			op = "Getedb";
		else
			op = "Getedb slice";
		if (dsptarget == NULLDN) {
			dsptarget = arg->arg_ge.ga_entry;
		}
		break;
	default:
		op = "Unknown op";
		break;
	}

	if (log_stat -> ll_events & LLOG_DEBUG)
		sprintf (buf,"%s (%d) [%d]",op,ad,tk->tk_dx.dx_id);
	else
		sprintf (buf,"%s (%d)",op,ad);

	if (context == DS_CTX_X500_DAP)
		pslog (log_stat,LLOG_NOTICE,buf,(IFP)dn_print,(caddr_t)daptarget);
	else {
		pslog (log_stat,LLOG_NOTICE,buf,(IFP)dn_print,(caddr_t)dsptarget);
		if (arg->arg_type != OP_GETEDB)
			pslog (log_stat,LLOG_TRACE,"DAP Originator",(IFP)dn_print,(caddr_t)orig);
	}
}
#endif
