/* uvec2ps.c - uvec-backed abstraction for PStreams */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap/RCS/uvec2ps.c,v 9.0 1992/06/16 12:25:44 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap/RCS/uvec2ps.c,v 9.0 1992/06/16 12:25:44 isode Rel $
 *
 *
 * $Log: uvec2ps.c,v $
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


#define	NPSUV		10     	/* really should be NSPUV - 2 */

#define	PSDU_MAGIC	64	/* threshold for scattering */

/*  */

static int
uvec_write (PS ps, PElementData data, PElementLen n, int in_line) {
	struct udvec *uv;

	if (in_line && n < PSDU_MAGIC)
		in_line = 0;
	if (in_line && ps -> ps_cur -> uv_base)
		ps -> ps_cur++;
	if (ps -> ps_cur >= ps -> ps_end) {
		SLOG (psap_log, LLOG_DEBUG, NULLCP,
			  ("%d elements not enough for pe2uvec", ps -> ps_elems));
		ps -> ps_elems += NPSUV;
		if ((uv = (struct udvec *) realloc ((char *) ps -> ps_head,
											(unsigned) (ps -> ps_elems
													* sizeof *uv)))
				== NULL)
			return ps_seterr (ps, PS_ERR_NMEM, NOTOK);

		ps -> ps_cur = uv + (ps -> ps_cur - ps -> ps_head);
		ps -> ps_end = (ps -> ps_head = uv) + ps -> ps_elems - 1;
	}

	uv = ps -> ps_cur;
	if (in_line) {
		if (ps -> ps_cc == 0) {
			SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
				  ("first write in pe2uvec is inline"));

			return ps_seterr (ps, PS_ERR_EOF, NOTOK);
		}

		uv -> uv_base = (char *) data, uv -> uv_len = n;
		uv -> uv_inline = 1;
		(++ps -> ps_cur) -> uv_base = NULL;
	} else {
		if (n > ps -> ps_slop) {
			SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
				  ("insufficient slop in pe2uvec, at least %d octets short",
				   n - ps -> ps_slop));

			return ps_seterr (ps, PS_ERR_EOF, NOTOK);
		}

		if (uv -> uv_base == NULL) {
			uv -> uv_base = ps -> ps_extra, uv -> uv_len = 0;
			uv -> uv_inline = ps -> ps_cc > 0 ? 1 : 0;
		}

		uv -> uv_len += n;
		bcopy ((char *) data, ps -> ps_extra, n);
		ps -> ps_extra += n, ps -> ps_slop -= n;
	}
	ps -> ps_cc += n;

	return n;
}


static int
uvec_flush (PS ps) {
	if (ps -> ps_cur) {
		if (ps -> ps_cur -> uv_base)
			ps -> ps_cur++;
		ps -> ps_cur -> uv_base = NULL;
	}

	if (ps -> ps_slop != 0)
		SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
			  ("%d octets of slop remaining on pe2uvec flush", ps -> ps_slop));

	return OK;
}


static int
uvec_close (PS ps) {
	struct udvec *uv;

	if (ps -> ps_head) {
		for (uv = ps -> ps_head; uv -> uv_base; uv++)
			if (!uv -> uv_inline)
				free (uv -> uv_base);
		free ((char *) ps -> ps_head);
	}

	if (ps -> ps_extra && ps -> ps_cc == 0)
		free (ps -> ps_extra);

	return OK;
}

/*  */

int
uvec_open (PS ps) {
	ps -> ps_writeP = uvec_write;
	ps -> ps_flushP = uvec_flush;
	ps -> ps_closeP = uvec_close;

	return OK;
}


int
uvec_setup (PS ps, int len) {
	struct udvec *uv;

	ps -> ps_elems = NPSUV;
	if ((uv = (struct udvec *) calloc ((unsigned) ps -> ps_elems, sizeof *uv))
			== NULL)
		return ps_seterr (ps, PS_ERR_NMEM, NOTOK);
	ps -> ps_end = (ps -> ps_head = ps -> ps_cur = uv) + ps -> ps_elems - 1;

	ps -> ps_cc = 0;

	if ((ps -> ps_slop = len) <= 0)
		SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
			  ("bad initial slop in pe2uvec, %d octets", len));

	if ((ps -> ps_extra = malloc ((unsigned) len)) == NULL)
		return ps_seterr (ps, PS_ERR_NMEM, NOTOK);

	return OK;
}

/*  */

int
ps_get_plen (PE pe) {
	PElementLen len;
	PE	    p;

	len = 0;
	switch (pe -> pe_form) {
	case PE_FORM_PRIM:
	case PE_FORM_ICONS:
		if (pe -> pe_len >= PSDU_MAGIC)
			len = pe -> pe_len;
		break;

	case PE_FORM_CONS:
		for (p = pe -> pe_cons; p; p = p -> pe_next)
			len += ps_get_plen (p);
		break;
	}

	return len;
}
