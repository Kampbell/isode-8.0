/* ps_get_abs.c - get absolute length */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap/RCS/ps_get_abs.c,v 9.0 1992/06/16 12:25:44 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap/RCS/ps_get_abs.c,v 9.0 1992/06/16 12:25:44 isode Rel $
 *
 *
 * $Log: ps_get_abs.c,v $
 * Revision 9.0  1992/06/16  12:25:44  isode
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
#include "psap.h"

/*    DATA */

int	ps_len_strategy = PS_LEN_SPAG;
static int  ps_get_id ();
static int  ps_get_len ();

/*  */

int
ps_get_abs (PE pe) {
	PElementLen len;
	PE p;

	switch (pe -> pe_form) {
	case PE_FORM_PRIM:
		len = pe -> pe_len;
		break;

	case PE_FORM_CONS:
		len = 0;
		for (p = pe -> pe_cons; p; p = p -> pe_next)
			len += ps_get_abs (p);

		switch (ps_len_strategy) {
		case PS_LEN_SPAG:
		default:
			if (len <= PE_LEN_SMAX) {
				pe -> pe_len = len;
				break;
			}		/* else fall */

		case PS_LEN_INDF:
			pe -> pe_len = PE_LEN_INDF;
			len += 2;	/* for EOC */
			break;

		case PS_LEN_LONG:
			pe -> pe_len = len;
			break;
		}
		break;

	case PE_FORM_ICONS:
		return pe -> pe_len;
	}

	return (ps_get_id (pe) + ps_get_len (pe) + len);
}

/*  */

static int
ps_get_id (PE pe) {
	int    i;
	PElementID id;

	if ((id = pe -> pe_id) < PE_ID_XTND)
		return 1;

	for (i = 1; id != 0; id >>= PE_ID_SHIFT)
		i++;
	return i;
}

/*  */

static int
ps_get_len (PE pe) {
	int    i;
	PElementLen len;

	if ((len = pe -> pe_len) == PE_LEN_INDF || len <= PE_LEN_SMAX)
		return 1;

	for (i = 1; len > 0; len >>= 8)
		i++;
	return i;
}
