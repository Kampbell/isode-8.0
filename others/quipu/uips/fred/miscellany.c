/* miscellany.c - fred miscellaneous functions */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/fred/RCS/miscellany.c,v 9.0 1992/06/16 12:44:30 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/fred/RCS/miscellany.c,v 9.0 1992/06/16 12:44:30 isode Rel $
 *
 *
 * $Log: miscellany.c,v $
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
#include "fred.h"

/*    DATA */

int	area_quantum = 0;

struct area_guide areas[] = {
	W_ORGANIZATION,
	"organization",	"-singlelevel",	"organization",		"o",	NULL,

	W_UNIT,
	"unit",		"-subtree",	"organizationalUnit",	"ou",	NULL,

	W_LOCALITY,
	"locality",	"-singlelevel", "locality",		"l",	NULL,

	W_PERSON,
	"person",	"-subtree",	"person",		"cn",	NULL,

	W_DSA,
	"dsa",		"-singlelevel",	"dsa",			"cn",	NULL,

	W_ROLE,
	"role",		"-subtree",	"organizationalRole",	"cn",	NULL,

	NULL
};

/*    ALIAS */

/* ARGSUSED */

int
f_alias (char **vec) {
	char   *cp,
		   buffer[BUFSIZ];

	if ((cp = *++vec) == NULL)
		return dish ("squid -fred -sequence default", 0);

	if (strcmp (cp, "-help") == 0) {
		fprintf (stdfp, "alias [name]\n");
		fprintf (stdfp, "    with no arguments, reports on active aliases\n");
		fprintf (stdfp,
				 "    with one argument, defines an alias for the given name\n");

		return OK;
	}

	sprintf (buffer, "squid -fred -alias \"%s\"", cp);
	return dish (buffer, runcom);
}

/*    AREA */

int
f_area (char **vec) {
	int	    status;
	char   *cp,
		   *dp,
		   *ep,
		   buffer[BUFSIZ];
	struct area_guide *ag;

	if ((cp = *++vec) == NULL) {
		if (myarea == NULL) {
			if (dish ("moveto -pwd", 0) == NOTOK)
				return NOTOK;
		} else
			fprintf (stdfp, "                     default area %s\n", myarea);

		for (ag = areas; ag -> ag_record; ag++)
			if (ag -> ag_area)
				fprintf (stdfp, "area for record-type %-12.12s %s\n",
						 ag -> ag_key, ag -> ag_area);

		return OK;
	}

	if (strcmp (cp, "-help") == 0) {
		fprintf (stdfp, "area [[record] location]\n");
		fprintf (stdfp,
				 "    with no arguments, lists areas current defined for various searches\n");
		fprintf (stdfp,
				 "    with one argument, sets the default area for general searches\n");
		fprintf (stdfp,
				 "    with two arguments, sets the default area for the given record type\n");

		return OK;
	}

	if ((dp = *++vec) == NULL) {
		sprintf (buffer, "moveto -pwd \"%s\"", cp);
		if (dish (buffer, 1) == NOTOK) {
			advise (NULLCP, "bad area: \"%s\"", cp);
			return NOTOK;
		}
		if (!runcom)
			fprintf (stdfp, "%s\n", myarea);

		area_quantum++;
		return OK;
	}

	if ((ep = index (dp, '=')) && *++ep == '@' && runcom) {
		ep = isodefile ("fredrc", 0);
		adios (NULLCP,
			   "%s is not configured!\n\tif this is your first system to run fred, read the Admin. Guide\n\totherwise, get a copy of %s from your first system",
			   ep, ep);
	}

	for (ag = areas; ag -> ag_record; ag++)
		if (strcmp (ag -> ag_key, cp) == 0)
			break;
	if (!ag -> ag_record) {
		advise (NULLCP, "invalid record-type: \"%s\"", cp);
		return NOTOK;
	}

	if (cp = myarea)
		myarea = NULL;

	sprintf (buffer, "moveto -pwd \"%s\"", dp);
	if ((status = dish (buffer, 1)) == OK) {
		if (ag -> ag_area)
			free (ag -> ag_area);
		ag -> ag_area = myarea;

		if (!runcom)
			fprintf (stdfp, "area for record-type %s: %s\n",
					 ag -> ag_key, ag -> ag_area);
	} else {
		advise (NULLCP, "bad area: \"%s\"", dp);
		if (myarea)
			free (myarea), myarea = NULL;
	}

	if (myarea = cp) {
		sprintf (buffer, "moveto -pwd \"%s\"", myarea);
		dish (buffer, 1);
	}

	area_quantum++;
	return status;
}

/*    DISH */

int
f_dish (char **vec) {
	char *bp,
		 *cp;
	char    buffer[BUFSIZ];

	if ((cp = *++vec) == NULL)
		return dish ("squid -fred", 0);
	if (strcmp (cp, "-help") == 0) {
		fprintf (stdfp, "dish [command [arguments ...]]\n");
		fprintf (stdfp, "    with no arguments, reports on status of dish\n");
		fprintf (stdfp, "    with arguments, passes those directly to dish\n");

		return OK;
	}

	strcpy (bp = buffer, cp);
	bp += strlen (bp);

	while (cp = *++vec) {
		sprintf (bp, " \"%s\"", cp);
		bp += strlen (bp);
	}

	return dish (buffer, runcom);
}

/*    EDIT */

int
f_edit (char **vec) {
	int	    result;
	char    buffer[BUFSIZ];

	if (*++vec != NULL && strcmp (*vec, "-help") == 0) {
		fprintf (stdfp, "edit\n");
		fprintf (stdfp, "    edit entry in the white pages\n");

		return OK;
	}

	if (readonly) {
		advise (NULLCP, "read only system ... edits not allowed ...");
		return NOTOK;
	}

	if (mydn == NULL) {
		advise (NULLCP, "who are you?  use the \"thisis\" command first...");
		return NOTOK;
	}

	sprintf (buffer, "modify -nocache -dontusecopy -newdraft \"%s\"",
			 mydn);
	dontpage = 1;
	result = dish (buffer, 0);
	dontpage = 0;

	if (result != OK)
		return result;

	sprintf (buffer, "showentry \"%s\" -fred -nocache -dontusecopy",
			 mydn);
	dish (buffer, 0);
	return OK;
}

/*    MANUAL */

int
f_manual (char **vec) {
	char   buffer[BUFSIZ];
	FILE  *fp;

	if (*++vec != NULL && strcmp (*vec, "-help") == 0) {
		fprintf (stdfp, "manual\n");
		fprintf (stdfp, "    print detailed information\n");

		return OK;
	}

	strcpy (buffer, isodefile ("fred.0", 0));
	if (fp = fopen (buffer, "r")) {
		while (fgets (buffer, sizeof buffer, fp))
			paginate (stdfp, buffer, strlen (buffer));
		paginate (stdfp, NULLCP, 0);

		fclose (fp);
	} else
		advise (buffer, "unable to open");

	return OK;
}

/*    REPORT */

int
f_report (char **vec) {
	char   *bp;
	char   *cp,
		   buffer[BUFSIZ];

	if (*++vec != NULL && strcmp (*vec, "-help") == 0) {
		fprintf (stdfp, "report [subject]\n");
		fprintf (stdfp, "    send report to white pages manager\n");

		return OK;
	}

	if (readonly)
		strcpy (buffer, _isodefile (isodebinpath, "mhmail"));
	bp = buffer;
	cp = strcmp (manager, "internal") ? manager : "wpp-manager@psi.com";

	if (!readonly || access (buffer, 0x01) == NOTOK) {
		strcpy (bp, "/usr/ucb/Mail ");
		bp += strlen (bp);

		if (debug) {
			sprintf (bp, "-v ");
			bp += strlen (bp);
		}

		if (readonly) {
			sprintf (bp, "-r \"%s\" ", cp);
			bp += strlen (bp);
		}

		sprintf (bp, "-s");
	} else {
		bp += strlen (bp);

		sprintf (bp, " -subject");
	}
	bp += strlen (bp);

	sprintf (bp, " \"%s\" \"%s\"",
			 *vec ? *vec : "White Pages report", cp);
	bp += strlen (bp);


	fprintf (stdfp, "End report with CTRL-D%s.\n",
			 readonly ? ", it will then take 30 seconds to post message" : "");
	if (readonly)
		fprintf (stdfp,
				 "In the message please specify your e-mail address, so a reply may be made.\n");
	fflush (stdfp);

	if (watch) {
		fprintf (stderr, "%s\n", buffer);
		fflush (stderr);
	}
	if (system (buffer))
		advise (NULLCP, "problem sending report");

	return OK;
}

/*    THISIS */

int
f_thisis (char **vec) {
	char   *bp;
	char   *cp,
		   buffer[BUFSIZ];

again:
	;
	if ((cp = *++vec) == NULL) {
		if (mydn == NULL) {
			advise (NULLCP, "who are you?");
			return NOTOK;
		}

		printf ("you are \"%s\"\n", mydn);
		return OK;
	}

	if (strcmp (cp, "-help") == 0) {
		fprintf (stdfp, "thisis [name]\n");
		fprintf (stdfp,
				 "    with no arguments, lists your name in the white pages\n");
		fprintf (stdfp,
				 "    with one argument, identifies you in the white pages\n");

		return OK;
	}

	if (strcmp (cp, "is") == 0)
		goto again;

	if (*cp == '!')
		cp++;
	for (bp = cp; isdigit (*bp); bp++)
		continue;
	if (*bp && (index (cp, '@') == NULL || index (cp, '=') == NULL)) {
		advise (NULLCP,
				"expecting a distinguished name (if you don't know what this is, punt)"
			   );
		return NOTOK;
	}

	bp = buffer;

	sprintf (bp, "bind -simple -user \"%s\"", cp);
	bp += strlen (bp);

	if (*++vec) {
		if (runcom && (rcmode & 077))
			adios (NULLCP,
				   "incorrect mode for runcom file -- use \"chmod 0600 $HOME/.fredrc\"");

		sprintf (bp, " -password \"%s\"", *vec);
		bp += strlen (bp);
	}

	if (dish (buffer, 0) != OK) {
		f_quit (NULLVP);
		exit (1);	/* NOT REACHED */
	}

	if (runcom)
		didbind = 1;

	dish ("squid -user", 1);

	return OK;
}
