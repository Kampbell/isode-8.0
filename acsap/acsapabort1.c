/* acsapabort1.c - ACPM: user abort */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/acsap/RCS/acsapabort1.c,v 9.0 1992/06/16 12:05:59 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/acsap/RCS/acsapabort1.c,v 9.0 1992/06/16 12:05:59 isode Rel $
 *
 *
 * $Log: acsapabort1.c,v $
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
#include <signal.h>
#include "ACS-types.h"
#define	ACSE
#include "acpkt.h"
#include "tailor.h"

/*    A-ABORT.REQUEST */

int
AcUAbortRequest (int sd, PE *data, int ndata, struct AcSAPindication *aci) {
	SBV     smask;
	int     result;
	struct assocblk  *acb;
	PE	    pe;
	struct PSAPindication pis;
	struct PSAPindication *pi = &pis;
	struct PSAPabort  *pa = &pi -> pi_abort;
	struct type_ACS_ABRT__apdu *pdu;

	toomuchP (data, ndata, NACDATA, "release");
	missingP (aci);

	smask = sigioblock ();

	if ((acb = findacblk (sd)) == NULL) {
		sigiomask (smask);
		return acsaplose (aci, ACS_PARAMETER, NULLCP,
						  "invalid association descriptor");
	}

	pdu = NULL;
	pe = NULLPE;
	if (acb -> acb_sversion == 1) {
		if ((result = PUAbortRequest (acb -> acb_fd, data, ndata, pi))
				== NOTOK) {
			ps2acslose (acb, aci, "PUAbortRequest", pa);
			if (PC_FATAL (pa -> pa_reason))
				goto out2;
			else
				goto out1;
		}

		result = OK;
		acb -> acb_fd = NOTOK;
		goto out2;
	}

	if ((pdu = (struct type_ACS_ABRT__apdu *) calloc (1, sizeof *pdu))
			== NULL) {
		result = acsaplose (aci, ACS_CONGEST, NULLCP, "out of memory");
		goto out2;
	}
	pdu -> abort__source = int_ACS_abort__source_acse__service__user;
	if (data
			&& ndata > 0
			&& (pdu -> user__information = info2apdu (acb, aci, data, ndata))
			== NULL)
		goto out2;

	result = encode_ACS_ABRT__apdu (&pe, 1, 0, NULLCP, pdu);

	free_ACS_ABRT__apdu (pdu);
	pdu = NULL;

	if (result == NOTOK) {
		acsaplose (aci, ACS_CONGEST, NULLCP, "error encoding PDU: %s",
				   PY_pepy);
		goto out2;
	}
	pe -> pe_context = acb -> acb_id;

	PLOGP (acsap_log,ACS_ACSE__apdu, pe, "ABRT-apdu", 0);

	if ((result = PUAbortRequest (acb -> acb_fd, &pe, 1, pi)) == NOTOK) {
		ps2acslose (acb, aci, "PUAbortRequest", pa);
		if (PC_FATAL (pa -> pa_reason))
			goto out2;
		else
			goto out1;
	}

	result = OK;
	acb -> acb_fd = NOTOK;

out2:
	;
	freeacblk (acb);

out1:
	;
	if (pe)
		pe_free (pe);
	if (pdu)
		free_ACS_ABRT__apdu (pdu);

	sigiomask (smask);

	return result;
}
