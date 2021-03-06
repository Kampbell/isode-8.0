/* ro2ssreleas1.c - initiate release */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/rosap/RCS/ro2ssreleas1.c,v 9.0 1992/06/16 12:37:02 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/rosap/RCS/ro2ssreleas1.c,v 9.0 1992/06/16 12:37:02 isode Rel $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log: ro2ssreleas1.c,v $
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
#include <signal.h>
#include "ropkt.h"

static int  RoEndRequestAux ();

/*    RO-END.REQUEST */

int
RoEndRequest (int sd, int priority, struct RoSAPindication *roi) {
	SBV	    smask;
	int     result;
	struct assocblk   *acb;

	missingP (roi);

	smask = sigioblock ();

	rosapPsig (acb, sd);

	result = RoEndRequestAux (acb, priority, roi);

	sigiomask (smask);

	return result;

}

/*  */

static int
RoEndRequestAux (struct assocblk *acb, int priority, struct RoSAPindication *roi) {
	int     result;
	struct SSAPindication   sis;
	struct SSAPindication *si = &sis;
	struct SSAPabort *sa = &si -> si_abort;
	struct SSAPrelease  srs;
	struct SSAPrelease *sr = &srs;

	if (acb -> acb_apdu)	/* ACB_CLOSING tested earlier in rosapPsig */
		return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);

	if (!(acb -> acb_flags & ACB_ROS))
		return rosaplose (roi, ROS_OPERATION, NULLCP,
						  "not an association descriptor for ROS");

	if (!(acb -> acb_flags & ACB_INIT))
		return rosaplose (roi, ROS_OPERATION, NULLCP, "not initiator");

	if (acb -> acb_ready
			&& !(acb -> acb_flags & ACB_TURN)
			&& (*acb -> acb_ready) (acb, priority, roi) == NOTOK)
		return NOTOK;

	if (SRelRequest (acb -> acb_fd, NULLCP, 0, NOTOK, sr, si) == NOTOK) {
		if (sa -> sa_peer)
			return ss2rosabort (acb, sa, roi);

		result = ss2roslose (acb, roi, "SRelRequest", sa);
	} else if (!sr -> sr_affirmative)
		result = ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
							"other side refused to release connection");
	else {
		acb -> acb_fd = NOTOK;
		result = OK;
	}

	freeacblk (acb);

	return result;
}
