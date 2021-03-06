/* ssapcapd1.c - SPM: write capability data */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/ssap/RCS/ssapcapd1.c,v 9.0 1992/06/16 12:39:41 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/ssap/RCS/ssapcapd1.c,v 9.0 1992/06/16 12:39:41 isode Rel $
 *
 *
 * $Log: ssapcapd1.c,v $
 * Revision 9.0  1992/06/16  12:39:41  isode
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
#include "spkt.h"

/*    S-CAPABILITY-DATA.REQUEST */

static  int SCapdRequestAux ();

int
SCapdRequest (int sd, char *data, int cc, struct SSAPindication *si) {
	SBV	    smask;
	int     result;
	struct ssapblk *sb;

	missingP (si);

	smask = sigioblock ();

	ssapPsig (sb, sd);
	toomuchP (sb, data, cc, SX_CDSIZE, "capability");

	result = SCapdRequestAux (sb, data, cc, si);

	sigiomask (smask);

	return result;
}

/*  */

static int
SCapdRequestAux (struct ssapblk *sb, char *data, int cc, struct SSAPindication *si) {
	int     result;

	if (!(sb -> sb_requirements & SR_CAPABILITY))
		return ssaplose (si, SC_OPERATION, NULLCP,
						 "capability data exchange service unavailable");
	if (SDoActivityAux (sb, si, 1, 0) == NOTOK)
		return NOTOK;
	if (sb -> sb_flags & SB_CD)
		return ssaplose (si, SC_OPERATION, NULLCP,
						 "capability data request in progress");
	sb -> sb_flags |= SB_CD;

	if ((result = SWriteRequestAux (sb, SPDU_CD, data, cc, 0, 0L, 0, NULLSD,
									NULLSD, NULLSR, si)) == NOTOK)
		freesblk (sb);

	return result;
}
