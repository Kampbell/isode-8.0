/* fred.c - FRont-End to Dish */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/fred/RCS/fred.c,v 9.0 1992/06/16 12:44:30 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/fred/RCS/fred.c,v 9.0 1992/06/16 12:44:30 isode Rel $
 *
 *
 * $Log: fred.c,v $
 * Revision 9.0  1992/06/16  12:44:30  isode
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


#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <varargs.h>
#include "fred.h"
#include "internet.h"

#include "sys.file.h"

#ifndef	R_OK
#define	R_OK	4
#endif

/*    DATA */

static char *myname = "fred";

static char **op = NULLVP;

static int alarming = 0;
static int logging = 0;
static int ontty;
static int armed;
static jmp_buf	alrmenv;
static jmp_buf	intrenv;
int	interrupted;

int	oneshot;

SFP	astat;
SFP	istat;
SFP	qstat;

SFD	alrmser ();
SFD	intrser ();

LLog    _fred_log = {
	"fred.log", NULLCP, NULLCP, LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE,
	LLOG_NONE, -1, LLOGCLS | LLOGCRT | LLOGZER, NOTOK
};
LLog *fred_log = &_fred_log;


static char *from = NULL;
static char *reply_to = NULL;
static char *sender = NULL;
static char *subject = NULL;

static struct pair {
	char   *p_name;
	char  **p_value;
} pairs[] = {
	"From:", &from,
	"Reply-To:", &reply_to,
	"Sender:", &sender,
	"Subject:", &subject,
	NULL, NULL
};

static	arginit ();
static int  fetchline ();
static	rcmap ();
static	log_utmp ();

/*    MAIN */

/* ARGSUSED  */

int 
main (int argc, char **argv, char **envp)
{
	int	    eof,
			status,
			vecp;
	char   *cp;
	char    address[BUFSIZ],
			buffer[BUFSIZ],
			*vec[NVEC + 1];
	struct sockaddr_in in_socket,
			*isock = &in_socket;

	arginit (argv);

	rcinit ();

	rcfile (isodefile ("fredrc", 0), 0, 1);
	if (init_ufnrc () == OK)
		area_quantum = -1;

	status = 0;

	if (mail) {
		int   c;
		char *ep;
		struct pair *p;
		FILE   *fp;

		for (;;) {
			ep = (cp = buffer) + sizeof buffer - 1;

			while ((c = getc (stdin)) != EOF)
				if (c == '\n') {
					if ((c = getc (stdin)) == ' ' || c == '\t') {
						*cp++ = ' ';
						while ((c = getc (stdin)) == ' ' || c == '\t')
							continue;
						if (c != EOF)
							 ungetc (c, stdin);
						else {
							c = NULL;
							break;
						}
					} else {
						if (c == EOF)
							c = NULL;
						else
							 ungetc (c, stdin);
						break;
					}
				} else if (cp < ep)
					*cp++ = c != '\t' ? c & 0xff : ' ';

			if (cp == buffer)
				break;
			*cp = NULL;

			for (p = pairs; p -> p_name; p++)
				if (lexnequ (buffer, p -> p_name, c = strlen (p -> p_name))
						== 0) {
					if (*p -> p_value == NULL) {
						for (cp = buffer + c; *cp == ' ' || *cp == '\t'; cp++)
							continue;
						if (*cp) {
							ep = cp + strlen (cp) - 1;
							while (*ep == ' ')
								ep--;
							*++ep = NULL;
							*p -> p_value = strdup (cp);
						}
					}
					break;
				}

		}

		if (!from && !reply_to && !sender)
			adios (NULLCP, "unable to determine return address");

		 sprintf (buffer, "/bin/mail \"%s\"",
						ep = reply_to ? reply_to : from ? from : sender);
		if (watch) {
			 fprintf (stderr, "%s\n", buffer);
			 fflush (stderr);
		}

		 signal (SIGPIPE, SIG_IGN);
		if ((fp = popen (buffer, "w")) == NULL)
			adios ("failed", "popen");

		stdfp = errfp = fp;

		 fprintf (stdfp, "To: %s\nSubject: Re: %s\n\n",
						ep, subject ? subject : "white pages query");
		 fflush (stdfp);

		if (f_bind (NULLVP) == NOTOK)
			adios (NULLCP, "unable to open the white pages");

		if (subject) {
			 strcpy (buffer, subject);
			 ll_log (fred_log, LLOG_NOTICE, NULLCP, "%s asks: %s",
						   ep, buffer);

			bzero ((char *) vec, sizeof vec);
			if (str2vecY (buffer, vec) < 1)
				 f_help (NULLVP);
			else if (fredloop (vec, NOTOK) != OK)
				status = 1;
		} else {
			int	    didone = 0;

			while ((c = getc (stdin)) != EOF)
				if (c != ' ' || c != '\t' || c != '\n')
					break;

			if (c != EOF)
				while (fgets (buffer, sizeof buffer, stdin)) {
					if (cp = index (buffer, '\n'))
						*cp = NULL;
					if (buffer[0] == NULL)
						break;

					 ll_log (fred_log, LLOG_NOTICE, NULLCP,
								   "%s asks: %s", ep, buffer);

					 fprintf (stdfp, "%s>>> %s\n", didone ? "\n\n" : "", buffer);

					bzero ((char *) vec, sizeof vec);
					if (str2vecY (buffer, vec) < 1)
						break;

					if (fredloop (vec, NOTOK) != OK) {
						status = 1;
						break;
					}
					didone = 1;
				}

			if (!didone)
				 f_help (NULLVP);
		}

		 fclose (fp);

		stdfp = stdout;

		goto were_out_of_here;
	}

	if (network) {
		int    len;

		if (getpeername (fileno (stdin), (struct sockaddr *) isock,
						 (len = sizeof *isock, &len)) != NOTOK) {
			 sprintf (address, "%s/%d",
							inet_ntoa (isock -> sin_addr),
							ntohs (isock -> sin_port));

			rcmap (isock);
		} else {
			 ll_log (fred_log, LLOG_EXCEPTIONS, "failed", "getpeername");
			 strcpy (address, "peer");
		}
	} else {
		struct hostent *hp;

		 strcpy (address, getlocalhost ());

		if (hp = gethostbystring (address)) {
			bzero ((char *) isock, sizeof *isock);
			isock -> sin_family = hp -> h_addrtype;
			inaddr_copy (hp, isock);
			rcmap (isock);
		} else
			advise (NULLCP, "%s: unknown host", address);

	}

	if (!fflag) {
		 sprintf (buffer, "%s/.fredrc", myhome);
		rcfile (buffer, op ? 1 : 0, 0);
	}

	if (f_bind (NULLVP) == NOTOK)
		adios (NULLCP, "unable to open the white pages");

	if (network) {
		errfp = stdout;

		 strcpy (buffer, "whois ");
		switch (fetchline (buffer + (sizeof "whois " - 1),
						   sizeof buffer - (sizeof "whois " - 1), stdin)) {
		case NOTOK:
			adios (NULLCP, "error reading query from %s", address);
		/* NOTREACHED */

		case DONE:
			buffer[0] = NULL;
		/* and fall... */

		case OK:
		default:
			break;
		}
		if (cp = index (buffer, '\r'))
			*cp = NULL;
		if (cp = index (buffer, '\n'))
			*cp = NULL;

		 ll_log (fred_log, LLOG_NOTICE, NULLCP, "%s asks: %s",
					   address, buffer);

		bzero ((char *) vec, sizeof vec);
		if (str2vecY (buffer, vec) < 2)
			 f_help (NULLVP);
		else if (fredloop (vec, NOTOK) != OK)
			status = 1;

		goto were_out_of_here;
	}

	if (op) {
		vecp = 0;
		if (nametype > 1
				&& (strcmp (myname, "whois") == 0 || (*op && **op == 'w'))) {
			char **pp;
			char    *bp = "";

			if (strcmp (myname, "whois") && *op)
				op++;

			for (pp = op; *pp; pp++)
				if (!test_ufn (*pp)) {
					bp = "\"";
					break;
				}

			for (cp = buffer; *op; cp += strlen (cp))
				 sprintf (cp, " %s%s%s", bp, *op++, bp);

			vec[vecp++] = "whois";
			vec[vecp++] = buffer;
		} else {
			if (strcmp (myname, "whois") == 0)
				vec[vecp++] = myname;

			while (*op)
				vec[vecp++] = *op++;
		}
		vec[vecp] = NULL;

		if (fredloop (vec, NOTOK) != NOTOK)
			status = 1;

		goto were_out_of_here;
	}

	istat = signal (SIGINT, intrser);

	eof = 0;
	for (interrupted = 0;; interrupted = 0) {
		if (alarming) {
			astat = signal (SIGALRM, alrmser);

			switch (setjmp (alrmenv)) {
			case OK:
				 alarm ((unsigned) 300);
				break;

			default:
				adios (NULLCP, "timed out due to inactivity");
			}
		}

		if (getline ("%s> ", buffer) == NOTOK) {
			if (eof)
				break;

			eof = 1;
			continue;
		}
		eof = 0;

		if (alarming)
			 alarm ((unsigned) 0);

		if (logging)
			 ll_log (fred_log, LLOG_NOTICE, NULLCP, "command: %s",
						   buffer);

		bzero ((char *) vec, sizeof vec);
		if ((vecp = str2vecY (buffer, vec)) < 1)
			continue;

		switch (fredloop (vec, OK)) {
		case NOTOK:
			status = 1;
			break;

		case OK:
		default:
			continue;

		case DONE:
			status = 0;
			break;
		}
		break;
	}

	 signal (SIGINT, istat);

were_out_of_here:
	;
	 f_quit (NULLVP);

	exit (mail ? 0 : status);		/* NOTREACHED */
}

/*    ARGINIT */

static 
arginit (char **vec)
{
	char  *ap;

	if (myname = rindex (*vec, '/'))
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = *vec;

	if (strcmp (myname, "in.whitepages") == 0)
		network++, fflag++;

	isodetailor (myname, 1);
	ll_hdinit (fred_log, myname);

	if (ontty = isatty (fileno (stdin)))
		verbose++;
	oneshot = 0;

	for (vec++; ap = *vec; vec++) {
		if (*ap == '-') {
			while (*++ap)
				switch (*ap) {
				case 'a':
					alarming++;
					break;

				case 'd':
					debug++;
					break;

				case 'm':
					mail++;
				/* and fall... */

				case 'n':
					network++;
				/* and fall... */

				case 'f':
					fflag++;
					break;

				case 'k':
					kflag++;
					break;

				case 'l':
					logging++;
					break;

				case 'r':
					readonly++;
					pager = "internal";
					break;

				case 'v':
					verbose = 1;
					break;

				case 'w':
					watch++;
					break;

				default:
					adios (NULLCP, "unknown switch -%c", *ap);
				}
			continue;
		}
		if (op == NULL) {
			op = vec;
			oneshot = 1;
			break;
		}
	}

	if (debug)
		ll_dbinit (fred_log, myname);
	if (logging)
		log_utmp ();
}

/*    INTERACTIVE */

int 
getline (char *prompt, char *buffer)
{
	int    i;
	char  *cp,
			 *ep;
	static int  sticky = 0;

	if (interrupted) {
		interrupted = 0;
		return NOTOK;
	}

	if (sticky) {
		sticky = 0;
		return NOTOK;
	}

	switch (setjmp (intrenv)) {
	case OK:
		armed++;
		break;

	case NOTOK:
		if (ontty)
			 printf ("\n");	/* and fall */
	default:
		armed = 0;
		return NOTOK;
	}

	if (ontty) {
		 printf (prompt, myname);
		 fflush (stdout);
	}

	for (ep = (cp = buffer) + BUFSIZ - 1; (i = getchar ()) != '\n';) {
		if (i == EOF) {
			if (ontty)
				 printf ("\n");
			clearerr (stdin);
			if (cp == buffer)
				longjmp (intrenv, DONE);

			sticky++;
			break;
		}

		if (cp < ep)
			*cp++ = i;
	}
	*cp = NULL;

	armed = 0;

	return OK;
}

/*  */

#ifndef	IAC
#define	IAC	255
#endif


static int  fetchline (s, n, iop)
char  *s;
int	n;
FILE  *iop;
{
	int    c;
	char  *p;

	p = s;
	while (--n > 0 && (c = getc (iop)) != EOF) {
		while (c == IAC) {
			 getc (iop);
			c = getc (iop);
		}
		if ((*p++ = c) == '\n')
			break;
	}
	if (ferror (iop))
		return NOTOK;
	if (c == EOF && p == s)
		return DONE;
	*p++ = NULL;

	return OK;
}

/*  */

/* ARGSUSED */

static	SFD alrmser (sig)
int	sig;
{
#ifndef	BSDSIGS
	 signal (SIGALRM, alrmser);
#endif

	longjmp (alrmenv, NOTOK);
}

/*  */

/* ARGSUSED */

static	SFD intrser (sig)
int	sig;
{
#ifndef	BSDSIGS
	 signal (SIGINT, intrser);
#endif

	if (armed)
		longjmp (intrenv, NOTOK);

	interrupted++;
}

/*  */

#ifndef	lint
int	ask (va_alist)
va_dcl {
	int     x,
	y,
	result;
	char    buffer[BUFSIZ];
	va_list ap;

	if (interrupted) {
		interrupted = 0;
		return DONE;
	}

	if (!ontty)
		return OK;

	switch (setjmp (intrenv)) {
	case OK:
		armed++;
		break;

	case NOTOK:
	default:
		 printf ("\n");
		armed = 0;
		return DONE;
	}

	va_start (ap);

	_asprintf (buffer, NULLCP, ap);

	va_end (ap);

again:
	;
	 printf ("%s", buffer);

	x = y = getchar ();
	while (y != '\n' && y != EOF)
		y = getchar ();

	switch (x) {
	case 'y':
	case '\n':
		result = OK;
		break;

	case 'n':
		result = NOTOK;
		break;

	case EOF:
		clearerr (stdin);
		result = DONE;
		break;

	default:
		goto again;
	}

	armed = 0;

	return result;
}
#else
/* VARARGS */

int 
ask (char *fmt)
{
	return ask (fmt);
}
#endif

/*  */

int 
str2vecY (char *buffer, char **vec)
{
	int	    i;
	char *cp,
			 *dp;

	if (nametype <= 1) {
normal:
		;
		return str2vec (buffer, vec);
	}

	for (cp = buffer; isspace (*cp); cp++)
		continue;
	for (dp = cp++; !isspace (*cp); cp++)
		if (!*cp)
			goto normal;
	*cp = NULL;
	if ((i = strlen (dp)) > 5 || strncmp ("whois", dp, i)) {
		*cp = ' ';
		goto normal;
	}

	vec[0] = dp;
	vec[1] = ++cp;
	vec[2] = NULL;

	return 2;
}

/*    MAPPING */

/* reads fred's IP-address to DN mapping file.

   for environments like Rutgers where IP-addresses can be more or less
   trusted, this allows an easy mechanism for mapping local Rutgers users into
   some DN other than the NULL user

   Syntax:

   	<addrmask>	<netaddr>	<dn>	<password>

	Each token is separated by LWSP, though double-quotes may be used to
	prevent separation.

 */

static 
rcmap (struct sockaddr_in *isock)
{
	u_long	hostaddr,
			netmask,
			netaddr;
	char *cp;
	char    buffer[BUFSIZ + 1],
			*vec[NVEC + 1];
	FILE   *fp;

	if ((fp = fopen (isodefile ("fredmap", 0), "r")) == NULL)
		goto done;

	hostaddr = isock -> sin_addr.s_addr;

	while (fgets (buffer, sizeof buffer, fp)) {
		if (*buffer == '#')
			continue;
		if (cp = index (buffer, '\n'))
			*cp = NULL;
		bzero ((char *) vec, sizeof vec);
		if (str2vec (buffer, vec) != 4)
			continue;
		netmask = inet_addr (vec[0]);
		if ((netaddr = inet_network (vec[1])) == NOTOK)
			continue;
		netaddr = ntohl (netaddr);
		if (!(netaddr & 0xff000000))
			netaddr <<=   (netaddr & 0x00ff0000) ? 8
						  : (netaddr & 0x0000ff00) ? 16
						  :			   24;
		netaddr = htonl (netaddr);
		if ((hostaddr & netmask) != netaddr)
			continue;

		vec[1] = "thisis";

		runcom = 1, rcmode = 0400;
		if (f_thisis (vec + 1) == NOTOK)
			adios (NULLCP, "unable to bind as %s for %s",
				   vec[2], inet_ntoa (isock -> sin_addr));

		runcom = 0;
		break;
	}

	 fclose (fp);

done:
	;
	 setgid (getgid ());
	 setuid (getuid ());
}

/*    ERRORS */

#ifndef	lint
void	_advise ();


void	adios (va_alist)
va_dcl {
	va_list ap;
	static int	latch = 0;

	if (latch)
		_exit (1);
	latch = 1;

	va_start (ap);

	if (network)
		 _ll_log (fred_log, LLOG_FATAL, ap);

	_advise (ap);

	va_end (ap);

	 f_quit (NULLVP);

	_exit (1);
}
#else
/* VARARGS */

void 
adios (char *what, char *fmt)
{
	adios (what, fmt);
}
#endif


#ifndef	lint
void	advise (va_alist)
va_dcl {
	va_list ap;

	va_start (ap);

	_advise (ap);

	va_end (ap);
}


static void  _advise (ap)
va_list	ap;
{
	char    buffer[BUFSIZ];
	FILE   *fp = network ? stdfp : stderr;

	asprintf (buffer, ap);

	 fflush (stdfp);

	 fprintf (fp, "%s: ", myname);
	 fputs (buffer, fp);
	 fputs (EOLN, fp);

	 fflush (fp);
}
#else
/* VARARGS */

void 
advise (char *what, char *fmt)
{
	advise (what, fmt);
}
#endif

/*  */

#include <utmp.h>

#ifdef	sun
#define	BSD42
#undef	SYS5
#endif

#ifdef	bsd43_ut_host
#undef	BSD42
#define	SYS5
#endif

#ifdef	BSD42
#define	HMAX	(sizeof (ut -> ut_host))
#endif
#define	LMAX	(sizeof (ut -> ut_line))
#define	NMAX	(sizeof (ut -> ut_name))

#define SCPYN(a, b)	strncpy(a, b, sizeof (a))
#define SCMPN(a, b)	strncmp(a, b, sizeof (a))


#ifdef	SYS5
struct utmp *getutent ();
#endif

char *ttyname ();


static	log_utmp () {
#ifndef	SYS5
	int	    ud;
#endif
	char   *line;
	struct utmp uts;
	struct utmp *ut = &uts;

	if ((line = ttyname (fileno (stdin))) == NULL)
		return;
	if (strncmp (line, "/dev/", sizeof "/dev/" - 1) == 0)
		line += sizeof "/dev/" - 1;
#ifndef	SYS5
	if ((ud = open ("/etc/utmp", 0)) == NOTOK)
		return;
	while (read (ud, (char *) ut, sizeof *ut) == sizeof *ut) {
		if (ut -> ut_name[0] == NULL || SCMPN (ut -> ut_line, line))
			continue;
#ifndef	BSD42
		 ll_log (fred_log, LLOG_NOTICE, NULLCP, "%.*s on %.*s",
					   NMAX, ut -> ut_name, LMAX, ut -> ut_line);
#else
		 ll_log (fred_log, LLOG_NOTICE, NULLCP,
					   "%.*s on %.*s (%.*s)",
					   NMAX, ut -> ut_name, LMAX, ut -> ut_line,
					   HMAX, ut -> ut_host);
#endif
		break;
	}
	 close (ud);
#else
	setutent ();
	while (ut = getutent ()) {
		if (ut -> ut_type != USER_PROCESS || SCMPN (ut -> ut_line, line))
			continue;

		 ll_log (fred_log, LLOG_NOTICE, NULLCP, "%.*s on %.*s",
					   NMAX, ut -> ut_name, LMAX, ut -> ut_line);
		break;
	}
	endutent ();
#endif
}


#ifdef	bsd43_ut_host
#define	BSD42
#undef	SYS5
#endif
