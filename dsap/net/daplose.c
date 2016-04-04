/* daplose.c - DAP: Support for directory protocol mappings */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/dsap/net/RCS/daplose.c,v 9.0 1992/06/16 12:14:05 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/dsap/net/RCS/daplose.c,v 9.0 1992/06/16 12:14:05 isode Rel $
 *
 *
 * $Log: daplose.c,v $
 * Revision 9.0  1992/06/16  12:14:05  isode
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
#include "tailor.h"
#include "quipu/dap2.h"

#ifndef	lint
static int  _daplose ();
static int  _dapreject ();
#endif

/*  */

#ifndef	lint
int	daplose (struct DAPindication *di, ...) {
	int	    reason,
			result;

	va_list ap;

	va_start (ap, di);

	reason = va_arg (ap, int);

	result = _daplose (di, reason, ap);

	va_end (ap);

	return result;
}
#else
/* VARARGS4 */

int
daplose (struct DAPindication *di, int reason, char *what, char *fmt) {
	return daplose (di, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
static int
_daplose (  /* what, fmt, args ... */
	struct DAPindication *di,
	int reason,
	va_list ap
) {
	char  *bp;
	char    buffer[BUFSIZ];
	struct DAPabort	* da;

	if (di) {
		bzero ((char *) di, sizeof *di);
		di->di_type = DI_ABORT;
		da = &(di->di_abort);
		da->da_reason = reason;

		asprintf (bp = buffer, ap);
		bp += strlen (bp);

		copyDAPdata (buffer, bp - buffer, da);
	}

	return NOTOK;
}
#endif

#ifndef	lint
int	dapreject (struct DAPindication *di, ...) {
	int	    reason,
			id,
			result;

	va_list ap;

	va_start (ap, di);

	reason = va_arg (ap, int);
	id = va_arg (ap, int);

	result = _dapreject (di, reason, id, ap);

	va_end (ap);

	return result;
}
#else
/* VARARGS4 */

int
dapreject (struct DAPindication *di, int reason, int id, char *what, char *fmt) {
	return dapreject (di, reason, id, what, fmt);
}
#endif

/*  */

#ifndef	lint
static int
_dapreject (  /* what, fmt, args ... */
	struct DAPindication *di,
	int reason,
	int id,
	va_list ap
) {
	char  *bp;
	char    buffer[BUFSIZ];
	struct DAPpreject	* dp;

	if (di) {
		bzero ((char *) di, sizeof *di);
		di->di_type = DI_PREJECT;
		dp = &(di->di_preject);
		dp->dp_id = id;
		dp->dp_reason = reason;

		asprintf (bp = buffer, ap);
		bp += strlen (bp);

		copyDAPdata (buffer, bp - buffer, dp);
	}

	return (NOTOK);
}
#endif
