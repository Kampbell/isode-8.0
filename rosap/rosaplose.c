/* rosaplose.c - ROPM: you lose */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/rosap/RCS/rosaplose.c,v 9.0 1992/06/16 12:37:02 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/rosap/RCS/rosaplose.c,v 9.0 1992/06/16 12:37:02 isode Rel $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log: rosaplose.c,v $
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
#include <stdarg.h>
#include "ropkt.h"
#include "tailor.h"

/*  */
#ifndef	lint
static int  _rosaplose ();
#endif

#ifndef	lint
int	ropktlose (struct assocblk*acb, ...)
{
	int	    reason,
	result,
	value;
	struct RoSAPindication *roi;
	struct RoSAPpreject *rop;
	va_list ap;

	va_start (ap, acb);

	roi = va_arg (ap, struct RoSAPindication *);
	reason = va_arg (ap, int);

	result = _rosaplose (roi, reason, ap);

	va_end (ap);

	if ((rop = &roi -> roi_preject) -> rop_cc > 0) {
		SLOG (rosap_log, LLOG_EXCEPTIONS, NULLCP,
		("ropktlose [%s] %*.*s", RoErrString (rop -> rop_reason),
		rop -> rop_cc, rop -> rop_cc, rop -> rop_data));
	} else
		SLOG (rosap_log, LLOG_EXCEPTIONS, NULLCP,
		("ropktlose [%s]", RoErrString (rop -> rop_reason)));

	if (acb == NULLACB
	|| acb -> acb_fd == NOTOK
	|| acb -> acb_ropktlose == NULLIFP)
		return result;

	switch (reason) {
	case ROS_PROTOCOL:
		value = ABORT_PROTO;
		break;

	case ROS_CONGEST:
		value = ABORT_TMP;
		break;

	default:
		value = ABORT_LSP;
		break;
	}

	(*acb -> acb_ropktlose) (acb, value);

	return result;
}
#else
/* VARARGS5 */

int 
ropktlose (struct assocblk *acb, struct RoSAPindication *roi, int reason, char *what, char *fmt)
{
	return ropktlose (acb, roi, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
int	rosapreject (struct assocblk*acb, ...)
{
	int	    reason,
	result;
	struct RoSAPindication  rois;
	struct RoSAPindication *roi;
	va_list ap;

	va_start (ap, acb);

	roi = va_arg (ap, struct RoSAPindication *);
	reason = va_arg (ap, int);

	result = _rosaplose (roi, reason, ap);

	va_end (ap);

	if (RoURejectRequestAux (acb, NULLIP, reason - REJECT_GENERAL_BASE,
	REJECT_GENERAL, 0, &rois) == NOTOK
	&& ROS_FATAL (rois.roi_preject.rop_reason)) {
		*roi = rois;		/* struct copy */
		result = NOTOK;
	}

	return result;
}
#else
/* VARARGS5 */

int 
rosapreject (struct assocblk *acb, struct RoSAPindication *roi, int reason, char *what, char *fmt)
{
	return rosapreject (acb, roi, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
int	rosaplose (struct RoSAPindication*roi, ...)
{
	int	    reason,
	result;
	va_list ap;

	va_start (ap, roi);

	reason = va_arg (ap, int);

	result = _rosaplose (roi, reason, ap);

	va_end (ap);

	return result;
}
#else
/* VARARGS4 */

int 
rosaplose (struct RoSAPindication *roi, int reason, char *what, char *fmt)
{
	return rosaplose (roi, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
static int 
_rosaplose (  /* what, fmt, args ... */
    struct RoSAPindication *roi,
    int reason,
    va_list ap
)
{
	char  *bp;
	char  *what;
	char  *fmt;
	char    buffer[BUFSIZ];
	struct RoSAPpreject *rop;

	if (roi) {
		bzero ((char *) roi, sizeof *roi);
		roi -> roi_type = ROI_PREJECT;
		rop = &roi -> roi_preject;

		what = va_arg(ap, char*);
		fmt = va_arg(ap, char*);
		_asprintf (bp = buffer, what, fmt, ap);
		bp += strlen (bp);

		rop -> rop_reason = reason;
		copyRoSAPdata (buffer, bp - buffer, rop);
	}

	return NOTOK;
}
#endif
