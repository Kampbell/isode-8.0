/* ssaptoken.c - SPM: tokens */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/ssap/RCS/ssaptoken.c,v 9.0 1992/06/16 12:39:41 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/ssap/RCS/ssaptoken.c,v 9.0 1992/06/16 12:39:41 isode Rel $
 *
 *
 * $Log: ssaptoken.c,v $
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
#include <signal.h>
#include "spkt.h"

/*    S-TOKEN-GIVE.REQUEST */

static int  SGTokenRequestAux ();
static int  SPTokenRequestAux ();

#define	dotoken(requires,shift,bit,type) \
{ \
    if (tokens & bit) { \
	if (!(sb -> sb_requirements & requires)) \
	    return ssaplose (si, SC_OPERATION, NULLCP, \
			"%s token unavailable", type); \
	if (!(sb -> sb_owned & bit)) \
	    return ssaplose (si, SC_OPERATION, NULLCP, \
			"%s token not owned by you", type); \
	settings |= bit; \
    } \
}

/*  */

int
SGTokenRequest (int sd, int tokens, struct SSAPindication *si) {
	SBV	    smask;
	int     result;
	struct ssapblk *sb;

	missingP (si);

	smask = sigioblock ();

	ssapXsig (sb, sd);

	result = SGTokenRequestAux (sb, tokens, si);

	sigiomask (smask);

	return result;
}

/*  */

static int
SGTokenRequestAux (struct ssapblk *sb, int tokens, struct SSAPindication *si) {
	int     result,
			settings;
	struct ssapkt *s;

	settings = 0;
	dotokens ();
	if (settings == 0)
		return ssaplose (si, SC_PARAMETER, NULLCP, "no tokens to give");

	if (sb -> sb_flags & SB_GTC)
		return ssaplose (si, SC_OPERATION, NULLCP, "give control request in progress");

	if (settings & ST_DAT_TOKEN)
		sb -> sb_flags &= ~(SB_EDACK | SB_ERACK);
	else if (sb -> sb_flags & (SB_EDACK | SB_ERACK))
		return ssaplose (si, SC_OPERATION, "exception in progress");

	if ((s = newspkt (SPDU_GT)) == NULL)
		return ssaplose (si, SC_CONGEST, NULLCP, "out of memory");

	s -> s_mask |= SMASK_SPDU_GT;

	s -> s_mask |= SMASK_GT_TOKEN;
	s -> s_gt_token = settings & 0xff;

	if ((result = spkt2sd (s, sb -> sb_fd, 0, si)) == NOTOK)
		freesblk (sb);
	else
		sb -> sb_owned &= ~s -> s_gt_token;

	freespkt (s);

	return result;
}

#undef	dotoken

/*    S-TOKEN-PLEASE.REQUEST */

#define	dotoken(requires,shift,bit,type) \
{ \
    if (tokens & bit) { \
	if (!(sb -> sb_requirements & requires)) \
	    return ssaplose (si, SC_OPERATION, NULLCP, \
			"%s token unavailable", type); \
	if (sb -> sb_owned & bit)    \
	    return ssaplose (si, SC_OPERATION, NULLCP, \
			"%s token owned by you", type);     \
	settings |= bit; \
    } \
}

/*  */

int
SPTokenRequest (int sd, int tokens, char *data, int cc, struct SSAPindication *si) {
	SBV	    smask;
	int     result;
	struct ssapblk *sb;

	missingP (si);

	smask = sigioblock ();

	ssapPsig (sb, sd);
	toomuchP (sb, data, cc, ST_SIZE, "token");

	result = SPTokenRequestAux (sb, tokens, data, cc, si);

	sigiomask (smask);

	return result;
}

/*  */

static int
SPTokenRequestAux (struct ssapblk *sb, int tokens, char *data, int cc, struct SSAPindication *si) {
	int     result,
			settings;
	struct ssapkt *s;

	settings = 0;
	dotokens ();
	if (settings == 0)
		return ssaplose (si, SC_PARAMETER, NULLCP, "no tokens to ask for");

	if (sb -> sb_flags & SB_GTC)
		return ssaplose (si, SC_OPERATION, NULLCP,
						 "give control request in progress");

	if ((s = newspkt (SPDU_PT)) == NULL)
		return ssaplose (si, SC_CONGEST, NULLCP, "out of memory");

	s -> s_mask |= SMASK_PT_TOKEN;
	s -> s_pt_token = settings & 0xff;

	if (cc > 0) {
		s -> s_mask |= SMASK_UDATA_PGI;
		s -> s_udata = data, s -> s_ulen = cc;
	} else
		s -> s_udata = NULL, s -> s_ulen = 0;
	if ((result = spkt2sd (s, sb -> sb_fd, 0, si)) == NOTOK)
		freesblk (sb);
	s -> s_mask &= ~SMASK_UDATA_PGI;
	s -> s_udata = NULL, s -> s_ulen = 0;

	freespkt (s);

	return result;
}

#undef	dotoken
