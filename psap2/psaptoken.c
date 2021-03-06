/* psaptoken.c - PPM: tokens */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap2/RCS/psaptoken.c,v 9.0 1992/06/16 12:29:42 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap2/RCS/psaptoken.c,v 9.0 1992/06/16 12:29:42 isode Rel $
 *
 *
 * $Log: psaptoken.c,v $
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

/*    P-TOKEN-GIVE.REQUEST */

int
PGTokenRequest (int sd, int tokens, struct PSAPindication *pi) {
	SBV	    smask;
	int     result;
	struct psapblk *pb;
	struct SSAPindication   sis;
	struct SSAPabort  *sa = &sis.si_abort;

	missingP (pi);

	smask = sigioblock ();

	psapPsig (pb, sd);

	if ((result = SGTokenRequest (sd, tokens, &sis)) == NOTOK)
		if (SC_FATAL (sa -> sa_reason))
			ss2pslose (pb, pi, "SGTokenRequest", sa);
		else {
			ss2pslose (NULLPB, pi, "SGTokenRequest", sa);
			goto out1;
		}
	else
		pb -> pb_owned &= ~tokens;

	if (result == NOTOK)
		freepblk (pb);
out1:
	;
	sigiomask (smask);

	return result;
}

/*    P-TOKEN-PLEASE.REQUEST */

int
PPTokenRequest (int sd, int tokens, PE *data, int ndata, struct PSAPindication *pi) {
	SBV	    smask;
	int     len,
			result;
	char   *base,
		   *realbase;
	struct psapblk *pb;
	struct SSAPindication   sis;
	struct SSAPabort  *sa = &sis.si_abort;

	toomuchP (data, ndata, NPDATA, "token");
	missingP (pi);

	smask = sigioblock ();

	psapPsig (pb, sd);

	if ((result = info2ssdu (pb, pi, data, ndata, &realbase, &base, &len,
							 "P-TOKEN-PLEASE user-data", PPDU_NONE)) != OK)
		goto out2;

	if ((result = SPTokenRequest (sd, tokens, base, len, &sis)) == NOTOK)
		if (SC_FATAL (sa -> sa_reason))
			ss2pslose (pb, pi, "SPTokenRequest", sa);
		else {
			ss2pslose (NULLPB, pi, "SPTokenRequest", sa);
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
