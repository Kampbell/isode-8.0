/* ro2ps.c - ROPM: PSAP interface */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/rosap/RCS/ro2ps.c,v 9.0 1992/06/16 12:37:02 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/rosap/RCS/ro2ps.c,v 9.0 1992/06/16 12:37:02 isode Rel $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log: ro2ps.c,v $
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
#include "ropkt.h"
#include "tailor.h"

/*    DATA */

static int	acslose ();

static int	pslose ();
static int	psDATAser (), psTOKENser (), psSYNCser (), psACTIVITYser (),
		psREPORTser (), psFINISHser (), psABORTser ();

static int  doPSdata ();
static int  doPStokens ();
static int  doPSsync ();
static int  doPSactivity ();
static int  doPSreport ();
static int  doPSfinish ();
static int  doPSabort ();

/*    bind underlying service */

int
RoPService (struct assocblk *acb, struct RoSAPindication *roi) {
	if (!(acb -> acb_flags & ACB_ACS) || (acb -> acb_flags & ACB_RTS))
		return rosaplose (roi, ROS_OPERATION, NULLCP,
						  "not an association descriptor for ROS on presentation");

	acb -> acb_putosdu = ro2pswrite;
	acb -> acb_rowaitrequest = ro2pswait;
	acb -> acb_ready = NULLIFP;
	acb -> acb_rosetindications = ro2psasync;
	acb -> acb_roselectmask = ro2psmask;
	acb -> acb_ropktlose = NULLIFP;

	return OK;
}

/*    define vectors for INDICATION events */

#define	e(i)	(indication ? (i) : NULLIFP)


/* ARGSUSED */

int
ro2psasync (struct assocblk *acb, IFP indication, struct RoSAPindication *roi) {
	struct PSAPindication   pis;
	struct PSAPabort  *pa = &pis.pi_abort;

	if (acb -> acb_rosindication = indication)
		acb -> acb_flags |= ACB_ASYN;
	else
		acb -> acb_flags &= ~ACB_ASYN;

	if (PSetIndications (acb -> acb_fd, e (psDATAser), e (psTOKENser),
						 e (psSYNCser), e (psACTIVITYser), e (psREPORTser),
						 e (psFINISHser), e (psABORTser), &pis) == NOTOK) {
		acb -> acb_flags &= ~ACB_ASYN;
		switch (pa -> pa_reason) {
		case PC_WAITING:
			return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);

		default:
			pslose (acb, roi, "PSetIndications", pa);
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
ro2psmask (struct assocblk *acb, fd_set *mask, int *nfds, struct RoSAPindication *roi) {
	struct PSAPindication   pis;
	struct PSAPabort   *pa = &pis.pi_abort;

	if (PSelectMask (acb -> acb_fd, mask, nfds, &pis) == NOTOK)
		switch (pa -> pa_reason) {
		case PC_WAITING:
			return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);

		default:
			pslose (acb, roi, "PSelectMask", pa);
			freeacblk (acb);
			return NOTOK;
		}

	return OK;
}

/*    AcSAP interface */

static int
acslose (struct assocblk *acb, struct RoSAPindication *roi, char *event, struct AcSAPabort *aca) {
	int     reason;
	char   *cp,
		   buffer[BUFSIZ];

	if (event)
		SLOG (rosap_log, LLOG_EXCEPTIONS, NULLCP,
			  (aca -> aca_cc > 0 ? "%s: %s [%*.*s]": "%s: %s", event,
			   AcErrString (aca -> aca_reason), aca -> aca_cc, aca -> aca_cc,
			   aca -> aca_data));

	cp = "";
	switch (aca -> aca_reason) {
	case ACS_ADDRESS:
		reason = ROS_ADDRESS;
		break;

	case ACS_REFUSED:
		reason = ROS_REFUSED;
		break;

	case ACS_CONGEST:
		reason = ROS_CONGEST;
		break;

	default:
		sprintf (cp = buffer, " (%s at association control)",
				 AcErrString (aca -> aca_reason));
	case ACS_PRESENTATION:
		reason = ROS_ACS;
		break;
	}

	if (aca -> aca_cc > 0)
		return ropktlose (acb, roi, reason, NULLCP, "%*.*s%s",
						  aca -> aca_cc, aca -> aca_cc, aca -> aca_data, cp);
	else
		return ropktlose (acb, roi, reason, NULLCP, "%s", cp);
}

/*    PSAP interface */

int
ro2pswait (struct assocblk *acb, int *invokeID, int secs, struct RoSAPindication *roi) {
	int     result;
	struct PSAPdata pxs;
	struct PSAPdata   *px = &pxs;
	struct PSAPindication   pis;
	struct PSAPindication *pi = &pis;

	for (;;) {
		switch (result = PReadRequest (acb -> acb_fd, px, secs, pi)) {
		case NOTOK:
			return doPSabort (acb, &pi -> pi_abort, roi);

		case OK:
			if ((result = doPSdata (acb, invokeID, px, roi)) != OK)
				return (result != DONE ? result : OK);
			continue;

		case DONE:
			switch (pi -> pi_type) {
			case PI_TOKEN:
				if (doPStokens (acb, &pi -> pi_token, roi) == NOTOK)
					return NOTOK;
				continue;

			case PI_SYNC:
				if (doPSsync (acb, &pi -> pi_sync, roi) == NOTOK)
					return NOTOK;
				continue;

			case PI_ACTIVITY:
				if (doPSactivity (acb, &pi -> pi_activity, roi) == NOTOK)
					return NOTOK;
				continue;

			case PI_REPORT:
				if (doPSreport (acb, &pi -> pi_report, roi) == NOTOK)
					return NOTOK;
				continue;

			case PI_FINISH:
				if (doPSfinish (acb, &pi -> pi_finish, roi) == NOTOK)
					return NOTOK;
				return DONE;

			default:
				ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
						   "unknown indication (0x%x) from presentation",
						   pi -> pi_type);
				break;
			}
			break;

		default:
			ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
					   "unexpected return from PReadRequest=%d", result);
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
ro2pswrite (struct assocblk *acb, PE pe, PE fe, int priority, struct RoSAPindication *roi) {
	int	    result;
	struct PSAPindication   pis;
	struct PSAPabort  *pa = &pis.pi_abort;

	pe -> pe_context = acb -> acb_rosid;

	PLOGP (rosap_log,ROS_ROSEapdus, pe, "ROSEapdus", 0);

	if ((result = PDataRequest (acb -> acb_fd, &pe, 1, &pis)) == NOTOK) {
		pslose (acb, roi, "PDataRequest", pa);
		freeacblk (acb);
	}

	if (fe)
		pe_extract (pe, fe);

	pe_free (pe);

	return result;
}

/*  */

static int
doPSdata (struct assocblk *acb, int *invokeID, struct PSAPdata *px, struct RoSAPindication *roi) {
	PE	    pe;

	if (px -> px_type != SX_NORMAL) {
		ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
				   "unexpected data indication (0x%x)", px -> px_type);
		PXFREE (px);

		freeacblk (acb);
		return NOTOK;
	}

	pe = px -> px_info[0], px -> px_info[0] = NULLPE;
	PXFREE (px);

	return acb2osdu (acb, invokeID, pe, roi);
}

/*  */

static int
doPStokens (struct assocblk *acb, struct PSAPtoken *pt, struct RoSAPindication *roi) {
	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected token indication (0x%x)", pt -> pt_type);
	PTFREE (pt);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doPSsync (struct assocblk *acb, struct PSAPsync *pn, struct RoSAPindication *roi) {
	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected sync indication (0x%x)", pn -> pn_type);
	PNFREE (pn);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doPSactivity (struct assocblk *acb, struct PSAPactivity *pv, struct RoSAPindication *roi) {
	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected activity indication (0x%x)", pv -> pv_type);
	PVFREE (pv);

	freeacblk (acb);
	return NOTOK;
}

/*  */

static int
doPSreport (struct assocblk *acb, struct PSAPreport *pp, struct RoSAPindication *roi) {
	ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
			   "unexpected exception report indication (0x%x)", pp -> pp_peer);
	PPFREE (pp);

	freeacblk (acb);
	return NOTOK;
}

/*  */

/* ARGSUSED */

static int
doPSfinish (struct assocblk *acb, struct PSAPfinish *pf, struct RoSAPindication *roi) {
	struct AcSAPindication acis;
	struct AcSAPabort *aca = &acis.aci_abort;

	if (acb -> acb_flags & ACB_INIT) {
		ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
				   "association management botched");
		PFFREE (pf);
		freeacblk (acb);
		return NOTOK;
	}

	roi -> roi_type = ROI_FINISH;
	{
		struct AcSAPfinish   *acf = &roi -> roi_finish;

		if (AcFINISHser (acb -> acb_fd, pf, &acis) == NOTOK)
			return acslose (acb, roi, "AcFINISHser", aca);

		*acf = acis.aci_finish;	/* struct copy */
	}

	return DONE;
}

/*  */

static int
doPSabort (struct assocblk *acb, struct PSAPabort *pa, struct RoSAPindication *roi) {
	struct AcSAPindication acis;
	struct AcSAPabort *aca = &acis.aci_abort;
	struct RoSAPpreject *rop = &roi -> roi_preject;

	if (!pa -> pa_peer && pa -> pa_reason == PC_TIMER)
		return rosaplose (roi, ROS_TIMER, NULLCP, NULLCP);

	if (AcABORTser (acb -> acb_fd, pa, &acis) == NOTOK) {
		acslose (acb, roi, "AcABORTser", aca);
		goto out;
	}

	if (aca -> aca_source != ACA_USER)
		acslose (acb, roi, NULLCP, aca);
	else
		rosaplose (roi, ROS_ABORTED, NULLCP, NULLCP);

	if (aca->aca_ninfo > 0) {
		rop -> rop_apdu = aca->aca_info[0];
		aca -> aca_info[0] = NULLPE;
	}
	ACAFREE (aca);

out:
	;
	acb -> acb_fd = NOTOK;
	freeacblk (acb);

	return NOTOK;
}

/*  */

static int
psDATAser (int sd, struct PSAPdata *px) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doPSdata (acb, NULLIP, px, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
psTOKENser (int sd, struct PSAPtoken *pt) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doPStokens (acb, pt, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
psSYNCser (int sd, struct PSAPsync *pn) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doPSsync (acb, pn, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
psACTIVITYser (int sd, struct PSAPactivity *pv) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doPSactivity (acb, pv, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
psREPORTser (int sd, struct PSAPreport *pp) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	if (doPSreport (acb, pp, roi) != OK)
		(*handler) (sd, roi);
}

/*  */

static int
psFINISHser (int sd, struct PSAPfinish *pf) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	doPSfinish (acb, pf, roi);

	(*handler) (sd, roi);
}

/*  */

static int
psABORTser (int sd, struct PSAPabort *pa) {
	IFP	    handler;
	struct assocblk   *acb;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;

	if ((acb = findacblk (sd)) == NULL)
		return;
	handler = acb -> acb_rosindication;

	doPSabort (acb, pa, roi);

	(*handler) (sd, roi);
}

/*  */

static int
pslose (struct assocblk *acb, struct RoSAPindication *roi, char *event, struct PSAPabort *pa) {
	int     reason;
	char   *cp,
		   buffer[BUFSIZ];

	if (event)
		SLOG (rosap_log, LLOG_EXCEPTIONS, NULLCP,
			  (pa -> pa_cc > 0 ? "%s: %s [%*.*s]": "%s: %s", event,
			   PErrString (pa -> pa_reason), pa -> pa_cc, pa -> pa_cc,
			   pa -> pa_data));

	cp = "";
	switch (pa -> pa_reason) {
	case PC_ADDRESS:
		reason = ROS_ADDRESS;
		break;

	case PC_REFUSED:
		reason = ROS_REFUSED;
		break;

	case PC_CONGEST:
		reason = ROS_CONGEST;
		break;

	default:
		sprintf (cp = buffer, " (%s at presentation)",
				 PErrString (pa -> pa_reason));
	case PC_SESSION:
		reason = ROS_PRESENTATION;
		break;
	}

	if (pa -> pa_cc > 0)
		return ropktlose (acb, roi, reason, NULLCP, "%*.*s%s",
						  pa -> pa_cc, pa -> pa_cc, pa -> pa_data, cp);
	else
		return ropktlose (acb, roi, reason, NULLCP, "%s", *cp ? cp + 1 : cp);
}
