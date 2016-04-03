/*
 * RFA - Remote File Access
 *
 * Access and Management for a partial file system tree that exists
 * at two sites either as master files or slave files
 *
 * ryresponder.c : responders interface to the ISODE Ry-Library
 *
 * Contributed by Oliver Wenzel, GMD Berlin, 1990
 *
 * $Header: /xtel/isode/isode/others/rfa/RCS/ryresponder.c,v 9.0 1992/06/16 12:47:25 isode Rel $
 *
 * $Log: ryresponder.c,v $
 * Revision 9.0  1992/06/16  12:47:25  isode
 * Release 8.0
 *
 */

#ifndef       lint
static char *rcsid = "$Header: /xtel/isode/isode/others/rfa/RCS/ryresponder.c,v 9.0 1992/06/16 12:47:25 isode Rel $";
#endif

/*
 *                              NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */

#include <stdio.h>
#include <setjmp.h>
#include <varargs.h>
#include "RFA-types.h"
#include "RFA-ops.h"
#include "ryresponder.h"
#include "tsap.h"		/* for listening */
#include "rfa.h"


int	debug = 0;
IFP	startfnx;
IFP	stopfnx;

static jmp_buf toplevel;




int
ros_init (int vecp, char **vec) {
	int	    reply,
			result,
			sd;
	struct AcSAPstart   acss;
	struct AcSAPstart *acs = &acss;
	struct AcSAPindication  acis;
	struct AcSAPindication *aci = &acis;
	struct AcSAPabort   *aca = &aci -> aci_abort;
	struct PSAPstart *ps = &acs -> acs_start;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;
	struct RoSAPpreject   *rop = &roi -> roi_preject;
	PE pe[1];

	if (AcInit (vecp, vec, acs, aci) == NOTOK) {
		acs_advise (aca, "initialization fails");
		return NOTOK;
	}
	advise (LLOG_NOTICE, NULLCP,
			"A-ASSOCIATE.INDICATION: <%d, %s, %s, %s, %d>",
			acs -> acs_sd, oid2ode (acs -> acs_context),
			sprintaei (&acs -> acs_callingtitle),
			sprintaei (&acs -> acs_calledtitle), acs -> acs_ninfo);

	sd = acs -> acs_sd;

	for (vec++; *vec; vec++)
		advise (LLOG_EXCEPTIONS, NULLCP, "unknown argument \"%s\"", *vec);

	reply = startfnx ? (*startfnx) (sd, acs, &pe[0]) : ACS_ACCEPT;

	result = AcAssocResponse (sd, reply,
							  reply != ACS_ACCEPT ? ACS_USER_NOREASON : ACS_USER_NULL,
							  NULLOID, NULLAEI, NULLPA, NULLPC, ps -> ps_defctxresult,
							  ps -> ps_prequirements, ps -> ps_srequirements, SERIAL_NONE,
							  ps -> ps_settings, &ps -> ps_connect, pe,
							  reply != ACS_ACCEPT ? 1 : 0, aci);

	ACSFREE (acs);

	if (result == NOTOK) {
		acs_advise (aca, "A-ASSOCIATE.RESPONSE");
		return NOTOK;
	}
	if (reply != ACS_ACCEPT)
		return NOTOK;

	if (RoSetService (sd, RoPService, roi) == NOTOK)
		ros_adios (rop, "set RO/PS fails");

	return sd;
}


int
ros_work (int fd) {
	int	    result;
	caddr_t out;
	struct AcSAPindication  acis;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi = &rois;
	struct RoSAPpreject   *rop = &roi -> roi_preject;

	switch (setjmp (toplevel)) {
	case OK:
		break;

	default:
		if (stopfnx)
			(*stopfnx) (fd, (struct AcSAPfinish *) 0);
	case DONE:
		AcUAbortRequest (fd, NULLPEP, 0, &acis);
		RyLose (fd, roi);
		return NOTOK;
	}

	switch (result = RyWait (fd, NULLIP, &out, OK, roi)) {
	case NOTOK:
		if (rop -> rop_reason == ROS_TIMER)
			break;
	case OK:
	case DONE:
		ros_indication (fd, roi);
		break;

	default:
		adios (NULLCP, "unknown return from RoWaitRequest=%d", result);
	}

	return OK;
}


int
ros_indication (int sd, struct RoSAPindication *roi) {
	int	    reply,
			result;

	switch (roi -> roi_type) {
	case ROI_INVOKE:
	case ROI_RESULT:
	case ROI_ERROR:
		adios (NULLCP, "unexpected indication type=%d", roi -> roi_type);
		break;

	case ROI_UREJECT: {
		struct RoSAPureject   *rou = &roi -> roi_ureject;

		if (rou -> rou_noid)
			advise (LLOG_EXCEPTIONS, NULLCP, "RO-REJECT-U.INDICATION/%d: %s",
					sd, RoErrString (rou -> rou_reason));
		else
			advise (LLOG_EXCEPTIONS, NULLCP,
					"RO-REJECT-U.INDICATION/%d: %s (id=%d)",
					sd, RoErrString (rou -> rou_reason),
					rou -> rou_id);
	}
	break;

	case ROI_PREJECT: {
		struct RoSAPpreject   *rop = &roi -> roi_preject;

		if (ROS_FATAL (rop -> rop_reason))
			ros_adios (rop, "RO-REJECT-P.INDICATION");
		ros_advise (rop, "RO-REJECT-P.INDICATION");
	}
	break;

	case ROI_FINISH: {
		struct AcSAPfinish *acf = &roi -> roi_finish;
		struct AcSAPindication  acis;
		struct AcSAPabort *aca = &acis.aci_abort;

		advise (LLOG_NOTICE, NULLCP, "A-RELEASE.INDICATION/%d: %d",
				sd, acf -> acf_reason);

		reply = stopfnx ? (*stopfnx) (sd, acf) : ACS_ACCEPT;

		result = AcRelResponse (sd, reply, ACR_NORMAL, NULLPEP, 0,
								&acis);

		ACFFREE (acf);

		if (result == NOTOK)
			acs_advise (aca, "A-RELEASE.RESPONSE");
		else if (reply != ACS_ACCEPT)
			break;
		longjmp (toplevel, DONE);
	}
	/* NOTREACHED */

	default:
		adios (NULLCP, "unknown indication type=%d", roi -> roi_type);
	}
}


int
ros_lose (struct TSAPdisconnect *td) {
	if (td -> td_cc > 0)
		adios (NULLCP, "TNetAccept: [%s] %*.*s",
			   TErrString (td -> td_reason), td -> td_cc, td -> td_cc,
			   td -> td_data);
	else
		adios (NULLCP, "TNetAccept: [%s]", TErrString (td -> td_reason));
}


void
ros_adios (struct RoSAPpreject *rop, char *event) {
	ros_advise (rop, event);

	cleanup ();

	longjmp (toplevel, NOTOK);
}


void
ros_advise (struct RoSAPpreject *rop, char *event) {
	char    buffer[BUFSIZ];

	if (rop -> rop_cc > 0)
		sprintf (buffer, "[%s] %*.*s", RoErrString (rop -> rop_reason),
				 rop -> rop_cc, rop -> rop_cc, rop -> rop_data);
	else
		sprintf (buffer, "[%s]", RoErrString (rop -> rop_reason));

	advise (LLOG_EXCEPTIONS, NULLCP, "%s: %s", event, buffer);
}


void
acs_advise (struct AcSAPabort *aca, char *event) {
	char    buffer[BUFSIZ];

	if (aca -> aca_cc > 0)
		sprintf (buffer, "[%s] %*.*s",
				 AcErrString (aca -> aca_reason),
				 aca -> aca_cc, aca -> aca_cc, aca -> aca_data);
	else
		sprintf (buffer, "[%s]", AcErrString (aca -> aca_reason));

	advise (LLOG_EXCEPTIONS, NULLCP, "%s: %s (source %d)", event, buffer,
			aca -> aca_source);
}



/*--------------------------------------------------------------*/
/*  ureject							*/
/*--------------------------------------------------------------*/
int
ureject (int sd, int reason, struct RoSAPinvoke *rox, struct RoSAPindication *roi) {
	if (RyDsUReject (sd, rox -> rox_id, reason, ROS_NOPRIO, roi) == NOTOK)
		ros_adios (&roi -> roi_preject, "U-REJECT");

	return OK;
}

