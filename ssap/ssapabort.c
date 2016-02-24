/* ssapabort.c - SPM: user abort */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/ssap/RCS/ssapabort.c,v 9.0 1992/06/16 12:39:41 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/ssap/RCS/ssapabort.c,v 9.0 1992/06/16 12:39:41 isode Rel $
 *
 *
 * $Log: ssapabort.c,v $
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
#include "tailor.h"

static int  SUAbortRequestAux ();

/*    S-U-ABORT.REQUEST */

int 
SUAbortRequest (int sd, char *data, int cc, struct SSAPindication *si)
{
	SBV	    smask;
	int     result;
	struct ssapblk *sb;

	missingP (si);

	smask = sigioblock ();

	if ((sb = findsblk (sd)) == NULL) {
		 sigiomask (smask);
		return ssaplose (si, SC_PARAMETER, NULLCP, "invalid session descriptor");
	}
	toomuchP (sb, data, cc, SA_SIZE, "abort");

	result = SUAbortRequestAux (sb, data, cc, si);

	 sigiomask (smask);

	return result;
}

/*  */

static int 
SUAbortRequestAux (struct ssapblk *sb, char *data, int cc, struct SSAPindication *si)
{
	int     result;
	struct ssapkt *s;
	struct TSAPdata txs;
	struct TSAPdata   *tx = &txs;
	struct TSAPdisconnect   tds;
	struct TSAPdisconnect *td = &tds;

	sb -> sb_flags &= ~(SB_ED | SB_EDACK | SB_ERACK);

	if ((sb -> sb_flags & SB_EXPD)
			&& sb -> sb_version >= SB_VRSN2
			&& cc > 9) {
		struct ssapkt *p;

		if (p = newspkt (SPDU_PR)) {
			p -> s_mask |= SMASK_PR_TYPE;
			p -> s_pr_type = PR_AB;
			result = spkt2sd (p, sb -> sb_fd, 1, si);
			freespkt (p);
			if (result == NOTOK)
				goto out1;
		}
	}

	if ((s = newspkt (SPDU_AB)) == NULL) {
		result = ssaplose (si, SC_CONGEST, NULLCP, "out of memory");
		goto out1;
	}

	s -> s_mask |= SMASK_SPDU_AB | SMASK_AB_DISC;
	s -> s_ab_disconnect = AB_DISC_RELEASE | AB_DISC_USER;

	if (cc > 0) {
		s -> s_mask |= SMASK_UDATA_PGI;
		s -> s_udata = data, s -> s_ulen = cc;
	} else
		s -> s_udata = NULL, s -> s_ulen = 0;
	result = spkt2sd (s, sb -> sb_fd, sb -> sb_flags & SB_EXPD ? 1 : 0, si);
	s -> s_mask &= ~SMASK_UDATA_PGI;
	s -> s_udata = NULL, s -> s_ulen = 0;

	freespkt (s);
	if (result == NOTOK)
		goto out1;

	if (ses_ab_timer >= 0)
		switch (TReadRequest (sb -> sb_fd, tx, ses_ab_timer, td)) {
		case OK:
		default: 		/* should be an ABORT ACCEPT, but who cares? */
			TXFREE (tx);
			break;

		case NOTOK:
			if (td -> td_reason != DR_TIMER)
				sb -> sb_fd = NOTOK;
			break;
		}
	result = OK;

out1:
	;
	freesblk (sb);

	return result;
}
