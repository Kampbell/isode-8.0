/* rtsaplose.c - RTPM: you lose */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/rtsap/RCS/rtsaplose.c,v 9.0 1992/06/16 12:37:45 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/rtsap/RCS/rtsaplose.c,v 9.0 1992/06/16 12:37:45 isode Rel $
 *
 *
 * $Log: rtsaplose.c,v $
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
#include <stdarg.h>
#include "rtpkt.h"
#include "tailor.h"

#ifndef	lint
static int  _rtsaplose ();
#endif

/*  */

#ifndef	lint
int	rtpktlose (struct assocblk*acb, ...)
{
	int	    reason,
	result,
	value;
	struct RtSAPindication *rti;
	struct RtSAPabort *rta;
	va_list ap;

	va_start (ap, acb);

	rti = va_arg (ap, struct RtSAPindication *);
	reason = va_arg (ap, int);

	result = _rtsaplose (rti, reason, ap);

	va_end (ap);

	if ((rta = &rti -> rti_abort) -> rta_cc > 0) {
		SLOG (rtsap_log, LLOG_EXCEPTIONS, NULLCP,
		("rtpktlose [%s] %*.*s", RtErrString (rta -> rta_reason),
		rta -> rta_cc, rta -> rta_cc, rta -> rta_data));
	} else
		SLOG (rtsap_log, LLOG_EXCEPTIONS, NULLCP,
		("rtpktlose [%s]", RtErrString (rta -> rta_reason)));

	if (acb == NULLACB
	|| acb -> acb_fd == NOTOK
	|| acb -> acb_rtpktlose == NULLIFP)
		return result;

	switch (reason) {
	case RTS_PROTOCOL:
		value = ABORT_PROTO;
		break;

	case RTS_CONGEST:
		value = ABORT_TMP;
		break;

	default:
		value = ABORT_LSP;
		break;
	}

	(*acb -> acb_rtpktlose) (acb, value);

	return result;
}
#else
/* VARARGS5 */

int 
rtpktlose (struct assocblk *acb, struct RtSAPindication *rti, int reason, char *what, char *fmt)
{
	return rtpktlose (acb, rti, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
int	rtsaplose (struct RtSAPindication*rti, ...)
{
	int	    reason,
	result;
	va_list ap;

	va_start (ap, rti);

	reason = va_arg (ap, int);

	result = _rtsaplose (rti, reason, ap);

	va_end (ap);

	return result;
}
#else
/* VARARGS4 */

int 
rtsaplose (struct RtSAPindication *rti, int reason, char *what, char *fmt)
{
	return rtsaplose (rti, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
static int 
_rtsaplose (  /* what, fmt, args ... */
    struct RtSAPindication *rti,
    int reason,
    va_list ap
)
{
	char  *bp;
	char  *what;
	char  *fmt;
	char    buffer[BUFSIZ];
	struct RtSAPabort *rta;

	if (rti) {
		bzero ((char *) rti, sizeof *rti);
		rti -> rti_type = RTI_ABORT;
		rta = &rti -> rti_abort;

		asprintf (bp = buffer, what, fmt, ap);
		bp += strlen (bp);

		rta -> rta_peer = 0;
		rta -> rta_reason = reason;
		copyRtSAPdata (buffer, bp - buffer, rta);
	}

	return NOTOK;
}
#endif
