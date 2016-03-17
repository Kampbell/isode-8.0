/* ftamlose.c - FPM: you lose */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/ftam/RCS/ftamlose.c,v 9.0 1992/06/16 12:14:55 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/ftam/RCS/ftamlose.c,v 9.0 1992/06/16 12:14:55 isode Rel $
 *
 *
 * $Log: ftamlose.c,v $
 * Revision 9.0  1992/06/16  12:14:55  isode
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
#include "fpkt.h"

static int _ftamoops (struct FTAMindication *fti, int reason, int fatal, int observer, int source, va_list ap);

/*  */

#ifndef	lint
int	fpktlose (struct ftamblk *fsb, ...)
{
	int     observer,
	reason,
	result,
	source;
	struct FTAMindication   ftis;
	struct FTAMindication *fti;
	va_list	ap;

	va_start (ap, fsb);

	fti = va_arg (ap, struct FTAMindication *);
	reason = va_arg (ap, int);

	if (fsb -> fsb_flags & FSB_INIT)
		observer = EREF_IFPM, source = EREF_RFPM;
	else
		observer = EREF_RFPM, source = EREF_IFPM;

	result = _ftamoops (fti, reason, 1, observer, source, ap);

	va_end (ap);

	if (fsb -> fsb_fd == NOTOK)
		return result;

	 FAbortRequestAux (fsb, type_FTAM_PDU_f__p__abort__request,
	FACTION_PERM, fti -> fti_abort.fta_diags, 1,
	&ftis);

	return result;
}
#else
/* VARARGS5 */

int 
fpktlose (struct ftamblk *fsb, struct FTAMindication *fti, int reason, char *what, char *fmt)
{
	return fpktlose (fsb, fti, reason, what, fmt);
}
#endif

/*  */

#ifndef	lint
int	ftamlose (struct FTAMindication *fti, ...)
{
	int     fatal,
	reason,
	result;
	
	va_list	ap;

	va_start (ap, fti);

	reason = va_arg (ap, int);
	fatal = va_arg (ap, int);

	result = _ftamoops (fti, reason, fatal, EREF_NONE, EREF_NONE, ap);

	va_end (ap);

	return result;
}
#else
/* VARARGS4 */

int 
ftamlose (struct FTAMindication *fti, int reason, int fatal, char *what, char *fmt)
{
	return ftamlose (fti, reason, fatal, what, fmt);
}
#endif

/*  */

#ifndef	lint
int	ftamoops (struct FTAMindication *fti, ...)
{
	int	    reason,
	result,
	fatal,
	observer,
	source;
	
	va_list ap;

	va_start (ap, fti);

	reason = va_arg (ap, int);
	fatal = va_arg (ap, int);
	observer = va_arg (ap, int);
	source = va_arg (ap, int);

	result = _ftamoops (fti, reason, fatal, source, observer, ap);

	va_end (ap);

	return result;
}


static int _ftamoops (struct FTAMindication *fti, int reason, int fatal, int observer, int source, va_list ap)
{
	char  *bp;
	char    buffer[BUFSIZ];
	struct FTAMabort  *fta;
	struct FTAMdiagnostic *ftd;

	if (fti) {
		bzero ((char *) fti, sizeof *fti);
		fti -> fti_type = FTI_ABORT;
		fta = &fti -> fti_abort;

		asprintf (bp = buffer, ap);
		bp += strlen (bp);

		fta -> fta_peer = 0;
		fta -> fta_action = fatal ? FACTION_PERM : FACTION_TRANS;

		ftd = &fta -> fta_diags[0];
		ftd -> ftd_type = fatal ? DIAG_PERM : DIAG_TRANS;
		ftd -> ftd_identifier = reason;
		ftd -> ftd_observer = observer;
		ftd -> ftd_source = source;
		ftd -> ftd_delay = DIAG_NODELAY;
		copyFTAMdata (buffer, bp - buffer, ftd);

		fta -> fta_ndiag = 1;
	}

	return NOTOK;
}
#else
/* VARARGS7 */

int 
ftamoops (struct FTAMindication *fti, int reason, int fatal, int observer, int source, char *what, char *fmt)
{
	return ftamoops (fti, reason, fatal, observer, source, what, fmt);
}
#endif
