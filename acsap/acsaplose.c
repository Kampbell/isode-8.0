/* acsaplose.c - ACPM: you lose */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/acsap/RCS/acsaplose.c,v 9.0 1992/06/16 12:05:59 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/acsap/RCS/acsaplose.c,v 9.0 1992/06/16 12:05:59 isode Rel $
 *
 *
 * $Log: acsaplose.c,v $
 * Revision 9.0  1992/06/16  12:05:59  isode
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
#include "ACS-types.h"
#define	ACSE
#include "acpkt.h"
#include "tailor.h"

/*  */

#ifndef	lint
static int _acsaplose (struct AcSAPindication *aci, int reason, va_list ap); /* what, fmt, args ... */
#endif

int	acsaplose (struct AcSAPindication*aci, ...);

#ifndef	lint
int	acpktlose (struct assocblk*acb, ...)
{
	int	    reason,
	result;
	PE	    pe;
	struct AcSAPindication *aci;
	struct PSAPindication   pis;
	struct type_ACS_ABRT__apdu pdus;
	struct type_ACS_ABRT__apdu *pdu = &pdus;
	va_list ap;

	va_start (ap, acb);

	aci = va_arg (ap, struct AcSAPindication *);
	reason = va_arg (ap, int);

	result = _acsaplose (aci, reason, ap);

	va_end (ap);

	if (acb == NULLACB || acb -> acb_fd == NOTOK)
		return result;

	if (acb -> acb_sversion == 1) {
		if (PUAbortRequest (acb -> acb_fd, NULLPEP, 0, &pis) != NOTOK)
			acb -> acb_fd = NOTOK;

		return result;
	}

	pdu -> abort__source = int_ACS_abort__source_acse__service__provider;
	pdu -> user__information = NULL;

	pe = NULLPE;
	if (encode_ACS_ABRT__apdu (&pe, 1, 0, NULLCP, pdu) != NOTOK) {
		pe -> pe_context = acb -> acb_id;

		PLOGP (acsap_log,ACS_ACSE__apdu, pe, "ABRT-apdu", 0);

		if (PUAbortRequest (acb -> acb_fd, &pe, 1, &pis) != NOTOK)
			acb -> acb_fd = NOTOK;
	}
	if (pe)
		pe_free (pe);

	return result;
}
#else
/* VARARGS5 */

int 
acpktlose (struct assocblk *acb, struct AcSAPindication *aci, int reason, char *what, char *fmt)
{
	return acpktlose (acb, aci, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
int	acsaplose (struct AcSAPindication*aci, ...)
{
	int	    reason,
	result;
	va_list ap;

	va_start (ap, aci);

	reason = va_arg (ap, int);

	result = _acsaplose (aci, reason, ap);

	va_end (ap);

	return result;
}
#else
/* VARARGS4 */

int 
acsaplose (struct AcSAPindication *aci, int reason, char *what, char *fmt)
{
	return acsaplose (aci, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
static int _acsaplose (  struct AcSAPindication *aci, int reason, va_list ap) /* what, fmt, args ... */
{
	char  *bp;
	char  *what;
	char  *fmt;
	char    buffer[BUFSIZ];
	struct AcSAPabort *aca;

	if (aci) {
		bzero ((char *) aci, sizeof *aci);
		aci -> aci_type = ACI_ABORT;
		aca = &aci -> aci_abort;

		what = va_arg(ap, char*);
		fmt = va_arg(ap, char*);
		_asprintf (bp = buffer, what, fmt, ap);
		bp += strlen (bp);

		aca -> aca_source = ACA_LOCAL;
		aca -> aca_reason = reason;
		copyAcSAPdata (buffer, bp - buffer, aca);
	}

	return NOTOK;
}
#endif
