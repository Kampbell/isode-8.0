/* objectbyname.c - getisobjectbyname */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap/RCS/objectbyname.c,v 9.0 1992/06/16 12:25:44 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap/RCS/objectbyname.c,v 9.0 1992/06/16 12:25:44 isode Rel $
 *
 *
 * $Log: objectbyname.c,v $
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

struct isobject *
getisobjectbyname (char *descriptor) {
	struct isobject   *io;

	isodetailor (NULLCP, 0);
#ifdef	DEBUG
	SLOG (addr_log, LLOG_TRACE, NULLCP,
		  ("getisobjectbyname \"%s\"", descriptor));
#endif

	setisobject (0);
	while (io = getisobject ())
		if (strcmp (descriptor, io -> io_descriptor) == 0)
			break;
	endisobject ();

	if (io) {
#ifdef	DEBUG
		SLOG (addr_log, LLOG_DEBUG, NULLCP,
			  ("\tODE: \"%s\"\tOID: %s",
			   io -> io_descriptor, sprintoid (&io -> io_identity)));
#endif
	} else
		SLOG (addr_log, LLOG_EXCEPTIONS, NULLCP,
			  ("lookup of object \"%s\" failed", descriptor));

	return io;
}
