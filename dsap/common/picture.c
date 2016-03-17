/* picture.c - exec printing of external attributes */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/dsap/common/RCS/picture.c,v 9.0 1992/06/16 12:12:39 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/dsap/common/RCS/picture.c,v 9.0 1992/06/16 12:12:39 isode Rel $
 *
 *
 * $Log: picture.c,v $
 * Revision 9.0  1992/06/16  12:12:39  isode
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
#include "quipu/util.h"
#include "quipu/photo.h"
#include "quipu/attr.h"
#include "psap.h"
#include <errno.h>
#ifdef	BSD42
#include <sys/wait.h>
#endif
#include "quipu/syntaxes.h"

typedef struct childList {
	struct childList *next;
	int childpid;
} ChildList;

static ChildList *rootChildList = NULL;

char *
show_picture (char *picture, char *picture_process, int len)
{
	int     ret;
	int     pd[2];
	int     pd2[2];
	static char * buffer = NULLCP;
	char    * cp;
	char	* argv[NVEC];
	SFP	pstat;
	ChildList *cl;

	if (buffer == NULLCP)
		buffer = smalloc (BUFLEN);

	/* hide_picture (); */

	if (*picture == '\0')
		return ("(No data to pass !)");
	if (picture_process == NULLCP)
		return ("(No external process defined !)");

	 sstr2arg (picture_process, NVEC, argv, " \t");

	/* get next two file descriptors  used for data xfer */
	ret = pipe(pd);
	if (ret == -1)
		return ("ERROR: could not create pipe");

	ret = pipe(pd2);
	if (ret == -1) {
		 close (pd[1]);
		 close (pd[0]);
		return ("ERROR: could not create 2nd pipe");
	}


	pstat = signal (SIGPIPE, SIG_IGN);

	cl = (ChildList *) malloc(sizeof(ChildList));
	/* generate one parent and one child process */
	if ((cl->childpid = fork()) == -1) {
		 free ((char *) cl);
		 close (pd[1]);
		 close (pd[0]);
		 close (pd2[1]);
		 close (pd2[0]);
		 signal (SIGPIPE, pstat);
		return ("ERROR: could not fork");
	}
	if (rootChildList == NULL)
		cl->next = NULL;
	else
		cl->next = rootChildList;
	rootChildList = cl;

	if (cl->childpid != 0) {

		/* in parent process */
		 close (pd[0]);
		 close (pd2[1]);

		if (write (pd[1], picture, len) != len) {
			 close (pd[1]);
			 close (pd2[0]);
			 signal (SIGPIPE, pstat);
			return("ERROR: length error");
		}

		 close (pd[1]);

		for (cp = buffer, len = BUFLEN - 1; len > 0;) {
			if ((ret = read (pd2[0], cp, len)) <= 0)
				break;
			cp += ret;
			len -= ret;
		}
		if (cp > buffer) {
			if (*--cp != '\n')
				cp++;
			*cp = NULL;
		} else
			 sprintf (buffer, "%s invoked", argv[0]);

		 close (pd2[0]);

		 signal (SIGPIPE, pstat);

		if ( ret < 0 )
			return ("ERROR: read error");

		return (buffer);

	}

	/* you're in child process */
	 signal (SIGPIPE, pstat);

	if (dup2(pd[0], 0) == -1)
		_exit (-1);
	 close (pd[0]);
	 close (pd[1]);

	if (dup2(pd2[1], 1) == -1)
		_exit (-1);
	 close (pd2[0]);
	 close (pd2[1]);

	 execv (argv[0],argv);

	while (read (0, buffer, sizeof buffer) > 0)
		continue;
	 printf ("ERROR: can't execute '%s'",argv[0]);

	 fflush (stdout);
	/* safety catch */
	_exit (-1);
	/* NOTREACHED */
}


exec_print (ps,av,proc)
PS ps;
AttributeValue av;
char * proc;
{
	char * ptr;
	PS sps;
	PE pe, grab_pe();

	 ps_flush (ps);

	if ((sps = ps_alloc (str_open)) == NULLPS)
		return;
	if (str_setup (sps,NULLCP,LINESIZE,0) == NOTOK) {
		ps_free (sps);
		return;
	}

	pe = grab_pe (av);
	 pe2ps (sps,pe);

	ptr = show_picture (sps->ps_base,proc,ps_get_abs(pe));
	ps_print (ps,ptr);

	pe_free (pe);
	ps_free (sps);

}


int 
hide_picture (void) {
	int	    pid;
	int	status;

	ChildList *cl, *cl1;

	cl = rootChildList;
	while (cl != NULL) {
		 kill (cl->childpid, SIGTERM);
		while ((pid = wait (&status)) != NOTOK && cl->childpid != pid)
			continue;

		cl1 = cl;
		cl = cl->next;
		 free((char *) cl1);
	}
	rootChildList = NULL;
}

picture_print (ps,pe,format)
PS ps;
PE pe;
int format;
{
	if (format != READOUT)
		pe_print (ps,pe,format);
	else
		ps_print (ps,"(No display process defined)");
}

extern int quipu_pe_cmp();

int 
photo_syntax (void) {
	 add_attribute_syntax ("photo",
								 (IFP)pe_cpy,	NULLIFP,
								 NULLIFP,	picture_print,
								 (IFP)pe_cpy,	quipu_pe_cmp,
								 pe_free,	NULLCP,
								 NULLIFP,	TRUE );


	 add_attribute_syntax ("jpeg",
								 (IFP)pe_cpy,	NULLIFP,
								 NULLIFP,	picture_print,
								 (IFP)pe_cpy,	quipu_pe_cmp,
								 pe_free,	NULLCP,
								 NULLIFP,	TRUE );

}
