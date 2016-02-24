/* getpassword.c - generic read-the-password-from-the-tty */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/compat/RCS/getpassword.c,v 9.0 1992/06/16 12:07:00 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/compat/RCS/getpassword.c,v 9.0 1992/06/16 12:07:00 isode Rel $
 *
 *
 * $Log: getpassword.c,v $
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

#include <signal.h>
#include <stdio.h>
#include "general.h"
#include "manifest.h"
#include "sys.file.h"

#ifdef	BSD44
char   *getpass ();
#endif

/*  */

/* roll our own since want to get past UNIX's limit of 8 octets... */

char *
getpassword (char *prompt)
{
#ifndef	BSD44
	int    c;
	int	    flags,
			isopen;
	char  *bp,
			 *ep;
#if	!defined(FEDORA) && !defined(SYS5) && !defined(XOS_2)
	struct sgttyb   sg;
#else
	struct termio   sg;
#endif
	SFP	    istat;
	FILE    *fp;
	static char buffer[BUFSIZ];

#ifdef SUNLINK_7_0
	fp = stdin, isopen = 0;	/* will help greatly to work off a script */
#else
	if ((c = open ("/dev/tty", O_RDWR)) != NOTOK && (fp = fdopen (c, "r")))
		setbuf (fp, NULLCP), isopen = 1;
	else {
		if (c != NOTOK)
			 close (c);

		fp = stdin, isopen = 0;
	}
#endif

	istat = signal (SIGINT, SIG_IGN);

#if	!defined(FEDORA) && !defined(SYS5) && !defined(XOS_2)
	 gtty (fileno (fp), &sg);
	flags = sg.sg_flags;
	sg.sg_flags &= ~ECHO;
	 stty (fileno (fp), &sg);
#else
	 ioctl (fileno (fp), TCGETA, (char *) &sg);
	flags = sg.c_lflag;
	sg.c_lflag &= ~ECHO;
	 ioctl (fileno (fp), TCSETAW, (char *) &sg);
#endif

#ifdef SUNLINK_7_0
	 fprintf (stdout, "%s", prompt);
	 fflush (stdout);
#else
	 fprintf (stderr, "%s", prompt);
	 fflush (stderr);
#endif

	for (ep = (bp = buffer) + sizeof buffer - 1; (c = getc (fp)) != EOF;)
#ifndef	apollo
		if (c == '\n')
#else
		if (c == '\n' || c == '\r')
#endif
			break;
		else if (bp < ep)
			*bp++ = c;
	*bp = NULL;

#ifdef SUNLINK_7_0
	 fprintf (stdout, "\n");
	 fflush (stdout);
#else
	 fprintf (stderr, "\n");
	 fflush (stderr);
#endif

#if	!defined(FEDORA) && !defined(SYS5) && !defined(XOS_2)
	sg.sg_flags = flags;
	 stty (fileno (fp), &sg);
#else
	sg.c_lflag = flags;
	 ioctl (fileno (fp), TCSETAW, (char *) &sg);
#endif

	 signal (SIGINT, istat);

	if (isopen)
		 fclose (fp);

	return buffer;
#else
	return getpass (prompt);
#endif
}
