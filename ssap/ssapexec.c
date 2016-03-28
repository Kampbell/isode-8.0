/* ssapexec.c - SPM: exec */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/ssap/RCS/ssapexec.c,v 9.0 1992/06/16 12:39:41 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/ssap/RCS/ssapexec.c,v 9.0 1992/06/16 12:39:41 isode Rel $
 *
 *
 * $Log: ssapexec.c,v $
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
#include "spkt.h"
#include "isoservent.h"
#include "tailor.h"

/*    SERVER only */

int 
SExec (struct TSAPstart *ts, struct SSAPindication *si, IFP hook, IFP setperms)
{
	int	    sd;
	char   *cp;
	struct isoservent *is;
	struct ssapkt *s;
	struct TSAPdata txs;
	struct TSAPdata  *tx = &txs;
	struct TSAPdisconnect   tds;
	struct TSAPdisconnect *td = &tds;

	if (TReadRequest (sd = ts -> ts_sd, tx, NOTOK, td) == NOTOK)
		return ts2sslose (si, "TReadRequest", td);

	s = tsdu2spkt (&tx -> tx_qbuf, tx -> tx_cc, NULLIP);
	TXFREE (tx);

	if (s == NULL || s -> s_errno != SC_ACCEPT) {
		 spktlose (sd, si, (s ? s -> s_errno : SC_CONGEST) | SC_REFUSE, NULLCP, NULLCP);
		goto out1;
	}

	switch (s -> s_code) {
	case SPDU_CN:
		if ((s -> s_mask & SMASK_CN_VRSN) && !(s -> s_cn_version & SB_ALLVRSNS)) {
			spktlose (sd, si, SC_VERSION | SC_REFUSE, NULLCP, "version mismatch: expecting 0x%x, got 0x%x",	SB_ALLVRSNS, s -> s_cn_version);
			break;
		}

		if ((s -> s_mask & SMASK_CN_CALLED) && s -> s_calledlen > 0) {
			if ((is = getisoserventbyselector ("ssap", s -> s_called, s -> s_calledlen)) == NULL) {
				char buffer[BUFSIZ];

				buffer[explode (buffer, (u_char *) s -> s_called, s-> s_calledlen)] = NULL;
				spktlose (sd, si, SC_SSAPID | SC_REFUSE, NULLCP, "ISO service ssap/%s not found", buffer);
				break;
			}
		} else
		if ((is = getisoserventbyname ("presentation", "ssap"))  == NULL) {
			 spktlose (sd, si, SC_SSUSER | SC_REFUSE, NULLCP, "default presentation service not found");
			break;
		}

		if (TSaveState (sd, is -> is_tail, td) == NOTOK) {
			spktlose (sd, si, SC_CONGEST | SC_REFUSE, NULLCP, NULLCP);
			break;
		}
		cp = *is -> is_tail++;
		if ((*is -> is_tail++ = spkt2str (s)) == NULL) {
			 spktlose (sd, si, SC_CONGEST | SC_REFUSE, NULLCP, NULLCP);
			break;
		}
		*is -> is_tail = NULL;

		switch (hook ? (*hook) (is, si) : OK) {
		case NOTOK:
			return NOTOK;

		case DONE:
			return OK;

		case OK:
		default:
			if (setperms)
				 (*setperms) (is);
			execv (*is -> is_vec, is -> is_vec);
			SLOG (ssap_log, LLOG_FATAL, *is -> is_vec, ("unable to exec"));
			TRestoreState (cp, ts, td);
			spktlose (ts -> ts_sd, si, SC_CONGEST | SC_REFUSE, *is -> is_vec, "unable to exec");
			break;
		}
		break;

	default:
		 spktlose (sd, si, SC_PROTOCOL | SC_REFUSE, NULLCP, "session protocol mangled: expecting 0x%x, got 0x%x", SPDU_CN, s -> s_code);
		break;
	}

out1:
	;
	freespkt (s);

	return NOTOK;
}
