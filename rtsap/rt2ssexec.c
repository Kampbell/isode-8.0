/* rt2ssexec.c - RTPM: exec */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/rtsap/RCS/rt2ssexec.c,v 9.0 1992/06/16 12:37:45 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/rtsap/RCS/rt2ssexec.c,v 9.0 1992/06/16 12:37:45 isode Rel $
 *
 *
 * $Log: rt2ssexec.c,v $
 * Revision 9.0  1992/06/16  12:37:45  isode
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
#include "RTS-types.h"
#include "OACS-types.h"
#include "rtpkt.h"
#include "isoservent.h"
#include "tailor.h"
#include "internet.h"

/*    SERVER only */

int
RtExec (struct SSAPstart *ss, struct RtSAPindication *rti, char *arg1, char *arg2, IFP hook, IFP setperms) {
	int     result,
			result2;
	struct isoservent *is;
	PE	    pe;
	struct SSAPref ref;
	struct SSAPindication   sis;
	struct SSAPindication *si = &sis;
	struct type_OACS_PConnect	*pcon = NULL;

	missingP (ss);
	missingP (rti);
	missingP (arg1);
	missingP (arg2);

	/* acsap_conntype = CN_OPEN, acsap_data = NULLPE; */
	if ((pe = ssdu2pe (ss -> ss_data, ss -> ss_cc, NULLCP, &result)) == NULLPE
			|| parse_OACS_PConnect (pe, 1, NULLIP, NULLVP, &pcon) == NOTOK) {
		if (pe)
			pe_free (pe);
		if (result == PS_ERR_NMEM) {
congest:
			;
			result = SC_CONGESTION, result2 = RTS_CONGEST;
		} else
			result = SC_REJECTED, result2 = RTS_PROTOCOL;
		goto out;
	}

	PLOGP (rtsap_log,OACS_PConnect, pe, "PConnect", 1);

	pe_free (pe);

	if (pcon -> pUserData -> member_OACS_2 -> offset
			!= type_OACS_ConnectionData_open) {
		result = SC_REJECTED, result2 = RTS_ADDRESS;
		goto out;
	}
	if (is = getisoserventbyport ("rtsap",
								  (u_short) htons ((u_short) pcon -> pUserData -> applicationProtocol))) {
		*is -> is_tail++ = arg1;
		*is -> is_tail++ = arg2;
		*is -> is_tail = NULL;
	} else {
		result = SC_REJECTED, result2 = RTS_ADDRESS;
		goto out;
	}

	switch (hook ? (*hook) (is, rti) : OK) {
	case NOTOK:
		if (pcon)
			free_OACS_PConnect(pcon);
		return NOTOK;

	case DONE:
		if (pcon)
			free_OACS_PConnect(pcon);
		return OK;

	case OK:
		if (setperms)
			(*setperms) (is);
		execv (*is -> is_vec, is -> is_vec);/* fall */
		SLOG (rtsap_log, LLOG_FATAL, *is -> is_vec, ("unable to exec"));
	default:
		goto congest;
	}

out:
	;
	if (pcon)
		free_OACS_PConnect(pcon);
	SSFREE (ss);

	bzero ((char *) &ref, sizeof ref);
	SConnResponse (ss -> ss_sd, &ref, NULLSA, result, 0, 0,
				   SERIAL_NONE, NULLCP, 0, si);
	return rtsaplose (rti, result2, NULLCP, NULLCP);
}
