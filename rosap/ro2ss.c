/* ro2ss.c - ROPM: SSAP interface */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/rosap/RCS/ro2ss.c,v 9.0 1992/06/16 12:37:02 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/rosap/RCS/ro2ss.c,v 9.0 1992/06/16 12:37:02 isode Rel $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log: ro2ss.c,v $
 * Revision 9.0  1992/06/16  12:37:02  isode
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

#include <stdio.h>
#include "ROS-types.h"
#include "../acsap/OACS-types.h"
#include "ropkt.h"
#include "tailor.h"

/*    DATA */

#define	doSSabort	ss2rosabort


static int	ssDATAser (), ssTOKENser (), ssSYNCser (), ssACTIVITYser (),
		ssREPORTser (), ssFINISHser (), ssABORTser ();

static int  doSSdata ();
static int  doSStokens ();
static int  doSSsync ();
static int  doSSactivity ();
static int  doSSreport ();
static int  doSSfinish ();

/*    local stub routine for psap/qbuf2pe */

static PE
qbuf2pe_local (struct qbuf *qb, int len, int *result) {
	return(qbuf2pe(qb, len, result));
}

/*    bind underlying service */

int
RoSService (struct assocblk *acb, struct RoSAPindication *roi) {
	if (acb -> acb_flags & (ACB_ACS | ACB_RTS))
		return rosaplose (roi, ROS_OPERATION, NULLCP,
						  "not an association descriptor for ROS on session");

	acb -> acb_putosdu = ro2sswrite;
	acb -> acb_rowaitrequest = ro2sswait;
	acb -> acb_getosdu = qbuf2pe_local;
	acb -> acb_ready = ro2ssready;
	acb -> acb_rosetindications = ro2ssasync;
	acb -> acb_roselectmask = ro2ssmask;
	acb -> acb_ropktlose = ro2sslose;

	return OK;
}

/*    define vectors for INDICATION events */

#define	e(i)	(indication ? (i) : NULLIFP)


/* ARGSUSED */

int
ro2ssasync (struct assocblk *acb, IFP indication, struct RoSAPindication *roi) {
	struct SSAPindication   sis;
	struct SSAPabort  *sa = &sis.si_abort;

	if (acb -> acb_rosindication = indication)
		acb -> acb_flags |= ACB_ASYN;
	else
		acb -> acb_flags &= ~ACB_ASYN;

	if (SSetIndications (acb -> acb_fd, e (ssDATAser), e (ssTOKENser),
						 e (ssSYNCser), e (ssACTIVITYser), e (ssREPORTser),
						 e (ssFINISHser), e (ssABORTser), &sis) == NOTOK) {
		acb -> acb_flags &= ~ACB_ASYN;
		switch (sa -> sa_reason) {
		case SC_WAITING:
			return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);

		default:
			ss2roslose (acb, roi, "SSetIndications", sa);
			freeacblk (acb);
			return NOTOK;
		}
	}


	return OK;
}

#undef	e

/*    map association descriptors for select() */

/* ARGSUSED */

int
ro2ssmask (struct assocblk *acb, fd_set *mask, int *nfds, struct RoSAPindication *roi) {
	struct SSAPindication   sis;
	struct SSAPabort  *sa = &sis.si_abort;

	if (SSelectMask (acb -> acb_fd, mask, nfds, &sis) == NOTOK)
		switch (sa -> sa_reason) {
		case SC_WAITING:
			return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);

		default:
			ss2roslose (acb, roi, "SSelectMask", sa);
			freeacblk (acb);
			return NOTOK;
		}

	return OK;
}

/*    protocol-level abort */

int
ro2sslose (struct assocblk *acb, int result) {
	int     len;
	char   *base;
	PE	    pe;
	struct SSAPindication   sis;

	base = NULL, len = 0;
	/* begin AbortInformation PSDU (pseudo) */
	if (pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SET)) {
		if (set_add (pe, num2prim ((integer) result, PE_CLASS_CONT, 0))
				!= NOTOK)
			pe2ssdu (pe, &base, &len);

		PLOGP (rosap_log,OACS_AbortInformation, pe, "AbortInformation",
			   0);

		pe_free (pe);
	}
	/* end AbortInformation PSDU */

	SUAbortRequest (acb -> acb_fd, base, len, &sis);
	acb -> acb_fd = NOTOK;

	if (base)
		free (base);
}

/*    SSAP interface */

int
ro2sswait (struct assocblk *acb, int *invokeID, int secs, struct RoSAPindication *roi) {
	int     result;
	struct SSAPdata sxs;
	struct SSAPdata   *sx = &sxs;
	struct SSAPindication   sis;
	struct SSAPindication *si = &sis;

	if (acb -> acb_apdu) {
		result = acb2osdu (acb, NULLIP, acb -> acb_apdu, roi);
		acb -> acb_apdu = NULLPE;

		return result;
	}

	for (;;) {
		switch (result = SReadRequest (acb -> acb_fd, sx, secs, si)) {
		case NOTOK:
			return doSSabort (acb, &si -> si_abort, roi);

		case OK:
			if ((result = doSSdata (acb, invokeID, sx, roi)) != OK)
				return (result != DONE ? result : OK);
			continue;

		case DONE:
			switch (si -> si_type) {
			case SI_TOKEN:
				if (doSStokens (acb, &si -> si_token, roi) == NOTOK)
					return NOTOK;
				continue;

			case SI_SYNC:
				if (doSSsync (acb, &si -> si_sync, roi) == NOTOK)
					return NOTOK;
				continue;

			case SI_ACTIVITY:
				if (doSSactivity (acb, &si -> si_activity, roi) == NOTOK)
					return NOTOK;
				continue;

			case SI_REPORT:
				if (doSSreport (acb, &si -> si_report, roi) == NOTOK)
					return NOTOK;
				continue;

			case SI_FINISH:
				if (doSSfinish (acb, &si -> si_finish, roi) == NOTOK)
					return NOTOK;
				return DONE;

			default:
				ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
						   "unknown indication (0x%x) from session",
						   si -> si_type);
				break;
			}
			break;

		default:
			ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
					   "unexpected return from SReadRequest=%d", result);
			break;
		}
		break;
	}

	freeacblk (acb);

	return NOTOK;
}

/*  */

/* ARGSUSED */

int
ro2ssready (struct assocblk *acb, int priority, struct RoSAPindication *roi) {
	int     result;
	PE	    pe;
	struct SSAPdata sxs;
	struct SSAPdata *sx = &sxs;
	struct  SSAPindication sis;
	struct  SSAPindication *si = &sis;
	struct  SSAPabort *sa = &si -> si_abort;

	if (acb -> acb_apdu || (acb -> acb_flags & ACB_CLOSING))
		return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);

	if (!(acb -> acb_requirements & SR_HALFDUPLEX)
			|| (acb -> acb_flags & ACB_TURN))
		return OK;

	if (!(acb -> acb_flags & ACB_PLEASE)) {
		if (SPTokenRequest (acb -> acb_fd, ST_DAT_TOKEN, NULLCP, 0, si)
				== NOTOK) {
			ss2roslose (acb, roi, "SPTokenRequest", sa);
			goto out;
		}

		acb -> acb_flags |= ACB_PLEASE;
	}

	for (;;) {
		switch (result = SReadRequest (acb -> acb_fd, sx, NOTOK, si)) {
		case NOTOK:
			return doSSabort (acb, &si -> si_abort, roi);

		case OK:
			if (sx -> sx_type != SX_NORMAL) {
				ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
						   "unexpected data indication (0x%x)",
						   sx -> sx_type);
				goto bad_sx;
			}
			if (pe = qbuf2pe (&sx -> sx_qbuf, sx -> sx_cc, &result)) {
				acb -> acb_apdu = pe;
				return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);
			}
			if (result != PS_ERR_NMEM) {
				rosapreject (acb, roi, ROS_GP_STRUCT, NULLCP, "%s",
							 ps_error (result));
				continue;
			}

			ropktlose (acb, roi, ROS_PROTOCOL, NULLCP, "%s",
					   ps_error (result));
bad_sx:
			;
			SXFREE (sx);
			goto out;

		case DONE:
			switch (si -> si_type) {
			case SI_TOKEN:
				if (doSStokens (acb, &si -> si_token, roi) == NOTOK)
					return NOTOK;
				return OK;

			case SI_SYNC:
				if (doSSsync (acb, &si -> si_sync, roi) == NOTOK)
					return NOTOK;
				continue;

			case SI_ACTIVITY:
				if (doSSactivity (acb, &si -> si_activity, roi) == NOTOK)
					return NOTOK;
				continue;

			case SI_REPORT:
				if (doSSreport (acb, &si -> si_report, roi) == NOTOK)
					return NOTOK;
				continue;

			case SI_FINISH:
				if (doSSfinish (acb, &si -> si_finish, roi) == NOTOK)
					return NOTOK;
				return DONE;

			default:
				ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
						   "unknown indication (0x%x) from session",
						   si -> si_type);
				break;
			}
			break;

		default:
			ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
					   "unexpected return from SReadRequest=%d", result);
			break;
		}
		break;
	}

out:
	;
	freeacblk (acb);

	return NOTOK;
}

/*  */

/* ARGSUSED */

int
ro2sswrite (struct assocblk *acb, PE pe, PE fe, int priority, struct RoSAPindication *roi) {
	int	    result;
	struct udvec *vv;
	struct udvec *uv;
	struct SSAPindication   sis;
	struct SSAPindication *si = &sis;
	struct SSAPabort  *sa = &si -> si_abort;

	PLOGP (rosap_log,ROS_OPDU, pe, "OPDU", 0);

	uv = NULL;
	if ((result = pe2uvec (pe, &uv)) == NOTOK)
		rosaplose (roi, ROS_CONGEST, NULLCP, "out of memory");
	else if ((result = SWriteRequest (acb -> acb_fd, 0, uv, si)) == NOTOK)
		ss2roslose (acb, roi, "SWriteRequest", sa);
	else
		result = OK;

	if (fe)
		pe_extract (pe, fe);
	pe_free (pe);

	if (uv) {
		for (vv = uv; vv -> uv_base; vv++)
			if (!vv -> uv_inline)
				free ((char *) vv -> uv_base);
		free ((char *) uv);
	}

	if (result == NOTOK)
		freeacblk (acb);

	return result;
}

/*  */

static int
doSSdata (struct assocblk *acb, int *invokeID, struct SSAPdata *sx, struct RoSAPindication *roi) {
	int     result;
	PE	    pe;

	if (sx -> sx_type != SX_NORMAL) {
		ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
				   "unexpected data indication (0x%x)", sx -> sx_type);
		goto out;
	}

	if (pe = (*acb -> acb_getosdu) (&sx -> sx_qbuf, sx -> sx_cc, &result))
		return acb2osdu (acb, invokeID, pe, roi);

	if (result != PS_ERR_NMEM)
		return rosapreject (acb, roi, ROS_GP_STRUCT, NULLCP, "%s",
							ps_error (result));

	ropktlose (acb, roi, ROS_CONGEST, NULLCP, "%s",
			   ps_error (result));

out:
	;
	SXFREE (sx);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doSStokens (struct assocblk *acb, struct SSAPtoken *st, struct RoSAPindication *roi) {
	int     result = DONE;
	struct SSAPindication   sis;
	struct SSAPindication  *si = &sis;
	struct SSAPabort   *sa = &si -> si_abort;

	switch (st -> st_type) {
	case ST_CONTROL:
		break;

	case ST_PLEASE:
		if (!(acb -> acb_requirements & SR_HALFDUPLEX))
			break;
		if (!(acb -> acb_flags & ACB_TURN))
			break;		/* error - do not have turn */

		result = SGTokenRequest (acb -> acb_fd, ST_DAT_TOKEN, si);

		if (result == NOTOK) {
			ss2roslose (acb, roi, "SGTokenRequest", sa);
			goto out;
		}
		acb -> acb_flags &= ~ACB_TURN;
		STFREE (st);
		return result;

	case ST_GIVE:
		if (!(acb -> acb_requirements & SR_HALFDUPLEX))
			break;
		if (acb -> acb_flags & ACB_TURN)
			break;		/* error - have turn already */

		if (st -> st_tokens & ST_DAT_TOKEN) {
			acb -> acb_flags |= ACB_TURN;
			acb -> acb_flags &= ~ACB_PLEASE;
		}
		return result;

	default:
		break;
	}

	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected token indication (0x%x)", st -> st_type);

out:
	;
	STFREE (st);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doSSsync (struct assocblk *acb, struct SSAPsync *sn, struct RoSAPindication *roi) {
	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected sync indication (0x%x)", sn -> sn_type);

	SNFREE (sn);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doSSactivity (struct assocblk *acb, struct SSAPactivity *sv, struct RoSAPindication *roi) {
	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected activity indication (0x%x)", sv -> sv_type);

	SVFREE (sv);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doSSreport (struct assocblk *acb, struct SSAPreport *sp, struct RoSAPindication *roi) {
	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected exception report indication (0x%x)", sp -> sp_peer);

	SPFREE (sp);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doSSfinish (struct assocblk *acb, struct SSAPfinish *sf, struct RoSAPindication *roi) {
	SFFREE (sf);

	if (acb -> acb_flags & ACB_INIT) {
		ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
				   "association management botched");
		freeacblk (acb);
		return NOTOK;
	}

	acb -> acb_flags |= ACB_FINN;
	roi -> roi_type = ROI_END;
	{
		struct RoSAPend   *roe = &roi -> roi_end;

		bzero ((char *) roe, sizeof *roe);
	}

	return DONE;
}

/*  */

int
ss2rosabort (struct assocblk *acb, struct SSAPabort *sa, struct RoSAPindication *roi) {
	int	    result;
	PE	pe;
	struct type_OACS_AbortInformation *pabort
		= (struct type_OACS_AbortInformation *) 0;
	int acsap_abort = -1;

	if (!sa -> sa_peer) {
		if (sa -> sa_reason == SC_TIMER)
			return rosaplose (roi, ROS_TIMER, NULLCP, NULLCP);

		ss2roslose (acb, roi, NULLCP, sa);
		goto out;
	}

	if (sa -> sa_cc == 0) {
		rosaplose (roi, ROS_ABORTED, NULLCP, NULLCP);
		goto out;
	}

	if ((pe = ssdu2pe (sa -> sa_info, sa -> sa_cc, NULLCP, &result))
			== NULLPE) {
		rosaplose (roi, ROS_PROTOCOL, NULLCP, NULLCP);
		goto out;
	}
	result = parse_OACS_AbortInformation (pe, 1, NULLIP, NULLVP, NULLCP);

#ifdef	DEBUG
	if (result != NOTOK && (rosap_log -> ll_events & LLOG_PDUS))
		pvpdu (rosap_log, print_OACS_AbortInformation_P, pe,
			   "AbortInformation", 1);
#endif

	pe_free (pe);
	if (result == NOTOK) {
		rosaplose (roi, ROS_PROTOCOL, "%s", PY_pepy);
		goto out;
	}
	acsap_abort = pabort->member_OACS_6->parm;
	switch (acsap_abort) {
	case ABORT_LSP:
	case ABORT_TMP:
		result = ROS_REMOTE;
		break;

	default:
		result = ROS_PROTOCOL;
		break;
	}
	rosaplose (roi, result, NULLCP, NULLCP);

out:
	;
	SAFREE (sa);
	acb -> acb_fd = NOTOK;
	freeacblk (acb);
	if (pabort)
		free_OACS_AbortInformation(pabort);

	return NOTOK;
}

/*  */

static int
ssDATAser (int sd, struct SSAPdata *sx) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doSSdata (acb, NULLIP, sx, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
ssTOKENser (int sd, struct SSAPtoken *st) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doSStokens (acb, st, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
ssSYNCser (int sd, struct SSAPsync *sn) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doSSsync (acb, sn, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
ssACTIVITYser (int sd, struct SSAPactivity *sv) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doSSactivity (acb, sv, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
ssREPORTser (int sd, struct SSAPreport *sp) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doSSreport (acb, sp, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
ssFINISHser (int sd, struct SSAPfinish *sf) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	doSSfinish (acb, sf, roi);

	(*handler) (sd, roi);
}

/*  */

static int
ssABORTser (int sd, struct SSAPabort *sa) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	doSSabort (acb, sa, roi);

	(*handler) (sd, roi);
}

/*  */

int
ss2roslose (struct assocblk *acb, struct RoSAPindication *roi, char *event, struct SSAPabort *sa) {
	int     reason;
	char   *cp,
		   buffer[BUFSIZ];

	if (event)
		SLOG (rosap_log, LLOG_EXCEPTIONS, NULLCP,
			  (sa -> sa_cc > 0 ? "%s: %s [%*.*s]": "%s: %s", event,
			   SErrString (sa -> sa_reason), sa -> sa_cc, sa -> sa_cc,
			   sa -> sa_prdata));

	cp = "";
	switch (sa -> sa_reason) {
	case SC_SSAPID:
	case SC_SSUSER:
	case SC_ADDRESS:
		reason = ROS_ADDRESS;
		break;

	case SC_REFUSED:
		reason = ROS_REFUSED;
		break;

	case SC_CONGEST:
		reason = ROS_CONGEST;
		break;

	default:
		sprintf (cp = buffer, " (%s at session)",
				 SErrString (sa -> sa_reason));
	case SC_TRANSPORT:
	case SC_ABORT:
		reason = ROS_SESSION;
		break;
	}

	if (sa -> sa_cc > 0)
		return ropktlose (acb, roi, reason, NULLCP, "%*.*s%s",
						  sa -> sa_cc, sa -> sa_cc, sa -> sa_prdata, cp);
	else
		return ropktlose (acb, roi, reason, NULLCP, "%s", *cp ? cp + 1 : cp);
}
