/* psapminor2.c - PPM: respond to minorsyncs */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap2/RCS/psapminor2.c,v 9.0 1992/06/16 12:29:42 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap2/RCS/psapminor2.c,v 9.0 1992/06/16 12:29:42 isode Rel $
 *
 *
 * $Log: psapminor2.c,v $
 * Revision 9.0  1992/06/16  12:29:42  isode
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
#include "ppkt.h"

/*    P-MINOR-SYNC.RESPONSE */

int 
PMinSyncResponse (int sd, long ssn, PE *data, int ndata, struct PSAPindication *pi)
{
	SBV	    smask;
	int     len,
			result;
	char   *base,
		   *realbase;
	struct psapblk *pb;
	struct SSAPindication   sis;
	struct SSAPabort  *sa = &sis.si_abort;

	toomuchP (data, ndata, NPDATA, "minorsync");
	missingP (pi);

	smask = sigioblock ();

	psapPsig (pb, sd);

	if ((result = info2ssdu (pb, pi, data, ndata, &realbase, &base, &len,
							 "P-MINOR-SYNC user-data", PPDU_NONE)) != OK)
		goto out2;

	if ((result = SMinSyncResponse (sd, ssn, base, len, &sis)) == NOTOK)
		if (SC_FATAL (sa -> sa_reason))
			 ss2pslose (pb, pi, "SMinSyncResponse", sa);
		else {
			 ss2pslose (NULLPB, pi, "SMinSyncResponse", sa);
			goto out1;
		}

out2:
	;
	if (result == NOTOK)
		freepblk (pb);
	else if (result == DONE)
		result = NOTOK;
out1:
	;
	if (realbase)
		free (realbase);
	else if (base)
		free (base);

	 sigiomask (smask);

	return result;
}
