/* ssaplose.c - SPM: you lose */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/ssap/RCS/ssaplose.c,v 9.0 1992/06/16 12:39:41 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/ssap/RCS/ssaplose.c,v 9.0 1992/06/16 12:39:41 isode Rel $
 *
 *
 * $Log: ssaplose.c,v $
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
#include <stdarg.h>
#include "spkt.h"
#include "tailor.h"

/*  */

#ifndef	lint
static int  _ssaplose ();
#endif

#ifndef	lint
int	spktlose (int sd, ...)
{
	int	    reason,
	result,
	secs,
	value;
	struct ssapblk *sb;
	struct ssapkt *s;
	struct SSAPindication   sis;
	struct SSAPindication *si;
	struct SSAPabort *sa;
	struct TSAPdata txs;
	struct TSAPdata   *tx = &txs;
	struct TSAPdisconnect   tds;
	struct TSAPdisconnect *td = &tds;
	va_list ap;

	va_start (ap, sd);

	si = va_arg (ap, struct SSAPindication *);
	if (si == NULL)
		si = &sis;

	reason = va_arg (ap, int);
	value = reason & SC_REFUSE;
	reason &= ~SC_REFUSE;

	result = _ssaplose (si, reason, ap);

	va_end (ap);

	if ((sa = &si -> si_abort) -> sa_cc > 0) {
		SLOG (ssap_log, LLOG_EXCEPTIONS, NULLCP,
		("spktlose [%s] %*.*s", SErrString (sa -> sa_reason),
		sa -> sa_cc, sa -> sa_cc, sa -> sa_prdata));
	} else
		SLOG (ssap_log, LLOG_EXCEPTIONS, NULLCP,
		("spktlose [%s]", SErrString (sa -> sa_reason)));

	if (value && SC_OFFICIAL (reason)) {
		if ((s = newspkt (SPDU_RF)) == NULL)
			return result;

		s -> s_mask |= SMASK_RF_DISC;
		s -> s_rf_disconnect |= RF_DISC_RELEASE;

		if (reason == SC_VERSION) {
			s -> s_mask |= SMASK_RF_VRSN;
			s -> s_rf_version = SB_ALLVRSNS;
		}

		if (s -> s_rdata = malloc ((unsigned) (s -> s_rlen = 1)))
			*s -> s_rdata = reason & 0xff;

		secs = ses_rf_timer;
	} else {
		if ((s = newspkt (SPDU_AB)) == NULL)
			return result;

		s -> s_mask |= SMASK_SPDU_AB | SMASK_AB_DISC;
		s -> s_ab_disconnect = AB_DISC_RELEASE;
		switch (reason) {
		case SC_PROTOCOL:
		case SC_VERSION:
			s -> s_ab_disconnect |= AB_DISC_PROTO;
			break;

		case SC_CONGEST:
		case SC_SSAPID:
			break;

		default:
			s -> s_ab_disconnect |= AB_DISC_UNKNOWN;
			break;
		}

		secs = ses_ab_timer;
	}

	value = spkt2sd (s, sd,
	(sb = findsblk (sd)) && (sb -> sb_flags & SB_EXPD), si);

	freespkt (s);
	if (value == NOTOK)
		return result;

	if (secs >= 0)
		switch (TReadRequest (sd, tx, secs, td)) {
		case OK:
		default:
			TXFREE (tx);
			break;

		case NOTOK:
			break;
		}

	return result;
}
#else
/* VARARGS5 */

int 
spktlose (int sd, struct SSAPindication *si, int reason, char *what, char *fmt)
{
	return spktlose (sd, si, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
int	ssaplose (struct SSAPindication*si, ...)
{
	int	    reason,
	result;
	va_list ap;

	va_start (ap, si);

	reason = va_arg (ap, int);

	result = _ssaplose (si, reason, ap);

	va_end (ap);

	return result;
}
#else
/* VARARGS4 */

int 
ssaplose (struct SSAPindication *si, int reason, char *what, char *fmt)
{
	return ssaplose (si, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
static int 
_ssaplose (	/* what, fmt, args ... */
    struct SSAPindication *si,
    int reason,
    va_list ap
)
{
	char  *bp;
	char    buffer[BUFSIZ];
	struct SSAPabort *sa;

	if (si) {
		bzero ((char *) si, sizeof *si);
		si -> si_type = SI_ABORT;
		sa = &si -> si_abort;

		asprintf (bp = buffer, ap);
		bp += strlen (bp);

		sa -> sa_peer = 0;
		sa -> sa_reason = reason;

		if ((sa -> sa_cc = min (bp - buffer, sizeof sa -> sa_prdata)) > 0)
			bcopy (buffer, sa -> sa_prdata, sa -> sa_cc);
	}

	return NOTOK;
}
#endif
