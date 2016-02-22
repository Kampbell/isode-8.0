/* asprintf.c - sprintf with errno */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/compat/RCS/asprintf.c,v 9.0 1992/06/16 12:07:00 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/compat/RCS/asprintf.c,v 9.0 1992/06/16 12:07:00 isode Rel $
 *
 *
 * $Log: asprintf.c,v $
 * Revision 9.0  1992/06/16  12:07:00  isode
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

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include "general.h"
#include "manifest.h"

#ifndef ASPRINTF
/*    DATA */


/*  */

void	asprintf (char*bp, char*what, char*fmt, ...)		/* fmt, args, ... */
{
	va_list ap;
	va_start(ap, fmt);
	_asprintf (bp, what, fmt, ap);
	va_end(ap);
}

#ifdef X25
unsigned char isode_x25_err[2];
char isode_x25_errflag = 0;
#endif

void	_asprintf (char*bp, char*what, char* fmt, va_list ap)	/* fmt, args, ... */
{
	register int    eindex;
	eindex = errno;

	*bp = NULL;

	if (fmt) {
#ifndef	VSPRINTF
		struct _iobuf iob;
#endif

#ifndef	VSPRINTF
#ifdef	pyr
		bzero ((char *) &iob, sizeof iob);
		iob._file = _NFILE;
#endif
		iob._flag = _IOWRT | _IOSTRG;
#if	!defined(vax) && !defined(pyr)
		iob._ptr = (unsigned char *) bp;
#else
		iob._ptr = bp;
#endif
		iob._cnt = BUFSIZ;
		_doprnt (fmt, ap, &iob);
		putc (NULL, &iob);
#else
		(void) vsprintf (bp, fmt, ap);
#endif
		bp += strlen (bp);

	}

	if (what) {
		if (*what) {
			(void) sprintf (bp, " %s: ", what);
			bp += strlen (bp);
		}
		(void) strcpy (bp, sys_errname (eindex));
		bp += strlen (bp);

#ifdef X25
		if (isode_x25_errflag) {
			(void) sprintf (bp, " (%02x %02x)",isode_x25_err[0],isode_x25_err[1]);
			bp += strlen (bp);
		}
#endif

	}

	errno = eindex;
}
#endif
