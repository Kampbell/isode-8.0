/* ssdu2pe.c - read a PE from SSDU */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap/RCS/ssdu2pe.c,v 9.0 1992/06/16 12:25:44 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap/RCS/ssdu2pe.c,v 9.0 1992/06/16 12:25:44 isode Rel $
 *
 *
 * $Log: ssdu2pe.c,v $
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
#include "tailor.h"

/*  */

PE
ssdu2pe (char *base, int len, char *realbase, int *result) {
	PE pe;
	PS ps;

	if ((ps = ps_alloc (str_open)) == NULLPS) {
		*result = PS_ERR_NMEM;
		return NULLPE;
	}
	if (str_setup (ps, base, len, 1) == OK) {
		if (!realbase)
			ps -> ps_inline = 0;
		if (pe = ps2pe (ps)) {
			if (realbase)
				pe -> pe_realbase = realbase;

			ps -> ps_errno = PS_ERR_NONE;
		} else if (ps -> ps_errno == PS_ERR_NONE)
			ps -> ps_errno = PS_ERR_EOF;
	}

	*result = ps -> ps_errno;

	ps -> ps_inline = 1;
	ps_free (ps);

#ifdef	DEBUG
	if (pe && (psap_log -> ll_events & LLOG_PDUS))
		pe2text (psap_log, pe, 1, len);
#endif

	return pe;
}
