/* psaprelease1.c - PPM: initiate release */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap2/RCS/psaprelease1.c,v 9.0 1992/06/16 12:29:42 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap2/RCS/psaprelease1.c,v 9.0 1992/06/16 12:29:42 isode Rel $
 *
 *
 * $Log: psaprelease1.c,v $
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

static int  PRelRetryRequestAux ();

/*    P-RELEASE.REQUEST */

int
PRelRequest (int sd, PE *data, int ndata, int secs, struct PSAPrelease *pr, struct PSAPindication *pi) {
	SBV	    smask;
	int	    result;
	struct psapblk *pb;

	toomuchP (data, ndata, NPDATA, "release");
	missingP (pr);
	missingP (pi);

	smask = sigioblock ();

	psapPsig (pb, sd);

	switch (result = info2ssdu (pb, pi, data, ndata, &pb -> pb_realbase,
								&pb -> pb_retry, &pb -> pb_len,
								"P-RELEASE user-data", PPDU_NONE)) {
	case OK:
	default:
		result = PRelRetryRequestAux (pb, secs, pr, pi);
		goto out;

	case NOTOK:
		freepblk (pb), pb = NULLPB;
		break;

	case DONE:
		result = NOTOK;
		break;
	}

	if (pb) {
		if (pb -> pb_realbase)
			free (pb -> pb_realbase);
		else if (pb -> pb_retry)
			free (pb -> pb_retry);
		pb -> pb_realbase = pb -> pb_retry = NULL;
	}

out:
	;
	sigiomask (smask);

	return result;
}

/*    P-RELEASE-RETRY.REQUEST (pseudo) */

int
PRelRetryRequest (int sd, int secs, struct PSAPrelease *pr, struct PSAPindication *pi) {
	SBV	    smask;
	int	    result;
	struct psapblk *pb;

	missingP (pr);
	missingP (pi);

	smask = sigioblock ();

	if ((pb = findpblk (sd)) == NULL)
		result = psaplose (pi, PC_PARAMETER, NULLCP,
						   "invalid session descriptor");
	else if (!(pb -> pb_flags & PB_RELEASE))
		result = psaplose (pi, PC_OPERATION, "release not in progress");
	else
		result = PRelRetryRequestAux (pb, secs, pr, pi);

	sigiomask (smask);

	return result;
}

/*  */

static int
PRelRetryRequestAux (struct psapblk *pb, int secs, struct PSAPrelease *pr, struct PSAPindication *pi) {
	int	    result;
	char   *id = pb -> pb_flags & PB_RELEASE ? "SRelRetryRequest"
				 : "SRelRequest";
	struct SSAPrelease   srs;
	struct SSAPrelease   *sr = &srs;
	struct SSAPindication   sis;
	struct SSAPabort  *sa = &sis.si_abort;

	bzero ((char *) sr, sizeof *sr);

	if ((result = (pb -> pb_flags & PB_RELEASE)
				  ? SRelRetryRequest (pb -> pb_fd, secs, sr, &sis)
				  : SRelRequest (pb -> pb_fd, pb -> pb_retry,
								 pb -> pb_len, secs, sr, &sis))
			== NOTOK) {
		if (sa -> sa_reason == SC_TIMER) {
			pb -> pb_flags |= PB_RELEASE;

			return ss2pslose (NULLPB, pi, id, sa);
		}

		if (sa -> sa_peer) {
			ss2psabort (pb, sa, pi);
			goto out1;
		}
		if (SC_FATAL (sa -> sa_reason)) {
			ss2pslose (pb, pi, id, sa);
			goto out2;
		} else {
			ss2pslose (NULLPB, pi, id, sa);
			goto out1;
		}
	}

	bzero ((char *) pr, sizeof *pr);

	if ((result = ssdu2info (pb, pi, sr -> sr_data, sr -> sr_cc, pr -> pr_info,
							 &pr -> pr_ninfo, "P-RELEASE user-data", PPDU_NONE)) == NOTOK)
		goto out2;

	if (pr -> pr_affirmative = sr -> sr_affirmative) {
		pb -> pb_fd = NOTOK;
		result = OK;
	} else
		result = DONE;

out2:
	;
	if (result == DONE)
		result = OK;
	else
		freepblk (pb), pb = NULLPB;
out1:
	;
	SRFREE (sr);
	if (pb) {
		if (pb -> pb_realbase)
			free (pb -> pb_realbase);
		else if (pb -> pb_retry)
			free (pb -> pb_retry);
		pb -> pb_realbase = pb -> pb_retry = NULL;
	}

	return result;
}
