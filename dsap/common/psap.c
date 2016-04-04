/* psap.c - General PSAP utility routines */

#ifndef lint
static char *rcsid = "$Header: /xtel/isode/isode/dsap/common/RCS/psap.c,v 9.0 1992/06/16 12:12:39 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/dsap/common/RCS/psap.c,v 9.0 1992/06/16 12:12:39 isode Rel $
 *
 *
 * $Log: psap.c,v $
 * Revision 9.0  1992/06/16  12:12:39  isode
 * Release 8.0
 *
 */

/*
 *                                NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include "quipu/util.h"
#include "quipu/attr.h"
#include "psap.h"
#include "isoaddrs.h"
#include "../x500as/DO-types.h"

extern LLog * log_dsap;

int
psap_free (struct PSAPaddr *psap) {
	free ((char *)psap) ;
}

struct PSAPaddr *
psap_cpy (struct PSAPaddr *a) {
	struct PSAPaddr * r;

	r = (struct PSAPaddr *) smalloc (sizeof (struct PSAPaddr));
	bzero ((char *) r,sizeof (struct PSAPaddr));

	*r = *a;        /* struct copy */

	return (r);
}

int
psap_dup (struct PSAPaddr *r, struct PSAPaddr *a) {
	*r = *a;    /* struct copy */
}

static
psap_cmp (struct PSAPaddr *r, struct PSAPaddr *a) {
	return (bcmp ((char *) r, (char *) a, sizeof *a) ? (-1) : 0);
}

static PE
psap_enc (struct PSAPaddr *p) {
	PE ret_pe;

	if (build_DSE_PSAPaddr (&ret_pe,0,0,NULLCP,p) == NOTOK ) {
		ret_pe = NULLPE;
		LLOG (log_dsap,LLOG_EXCEPTIONS, ("Failed to encode PSAP"));
	}
	return (ret_pe);
}

static struct PSAPaddr *
psap_dec (PE pe) {
	struct PSAPaddr *psap;

	psap = (struct PSAPaddr *) smalloc (sizeof *psap);

	if (parse_DSE_PSAPaddr (pe,1,NULLIP,NULLVP,psap) == NOTOK) {
		free ((char *)psap);
		return (NULLPA);
	}

	return (psap);
}

static struct PSAPaddr *
psap_parse (char *s) {
	struct PSAPaddr *pa;
	struct PSAPaddr *psap;

	psap = (struct PSAPaddr *) calloc (1,sizeof (struct PSAPaddr));
	if (pa=str2paddr(s)) {
		*psap = *pa;  /* struct copy */
		return (psap);
	} else {
		parse_error ("invalid presentation address %s",s);
		free ((char *)psap);
		return (NULLPA);
	}
}

static
psap_print (PS ps, struct PSAPaddr *p, int format) {
	if (format != READOUT)
		ps_printf (ps, "%s", _paddr2str(p,NULLNA,-1));
	else
		ps_printf (ps, "%s", paddr2str(p,NULLNA));

}

int
psap_syntax (void) {
	add_attribute_syntax ("presentationAddress",
						  (IFP) psap_enc,		(IFP) psap_dec,
						  (IFP) psap_parse,	psap_print,
						  (IFP) psap_cpy,		psap_cmp,
						  psap_free,		NULLCP,
						  NULLIFP,		TRUE );
}
