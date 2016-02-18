/* deufn.c - some de specific ufn routines */

#ifndef lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/de/RCS/deufn.c,v 9.1 1992/08/25 15:52:32 isode Exp $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/deufn.c,v 9.1 1992/08/25 15:52:32 isode Exp $
 *
 *
 * $Log: deufn.c,v $
 * Revision 9.1  1992/08/25  15:52:32  isode
 * *** empty log message ***
 *
 */
/*

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "quipu/ufn.h"
#include "quipu/util.h"
#include "demanifest.h"
#include "util.h"
#include "destrings.h"
#include "tailor.h"
#include "query.h"

extern LLog *de_log;
extern char *TidyString();
void onint1(), de_exit();

extern struct query qinfo[];

extern int boundToDSA;
extern int deLogLevel;

void
doUfnSearch() {
	DNS ufn_interact();
	struct dn_seq  *ufnres, * dns;
	char buf[BUFSIZ];
	char * ufnargv[10];
	int ufnel;
	char * cp, * cp2, ** cpp;
	int res;

	if (boundToDSA == FALSE)
		if (de_bind(TRUE) == NOTOK) /* block until bound */
			de_exit(-1);

	if (deLogLevel)
		(void) ll_log (de_log, LLOG_NOTICE, NULLCP,
					   "Search: ufn=%s", qinfo[PERSON].entered);

	ufnel = 0;
	ufnres = NULL;
	(void) strcpy(buf, qinfo[PERSON].entered);
	cp = buf;
	for (cpp = &ufnargv[0]; ; cpp++) {
		*cpp = cp;
		ufnel++;
		cp2 = index(cp, ',');
		if (cp2 == NULL) {
			*(++cpp) = NULLCP;
			break;
		} else {
			*cp2 = '\0';
			cp = ++cp2;
		}
	}
	initAlarm();
	/* don't know what ufn_match returns when it has a crisis */
	if (ufn_match(ufnel, ufnargv, ufn_interact, &ufnres, NULLEL) == 0) {
		resetprint("Sorry, but there are no tailor files set up to support user-friendly\n");
		resetprint("naming - refer to your system administrator.  In the meantime, try and\n");
		resetprint("find the entry you want by responding to the series of prompts.\n\n");
		return;
	}
	if (ufnres == NULLDNS) {
		(void)printf ("\nNo matching entries found\n");
		return;
	}
	pagerOn(NUMBER_NOT_ALLOWED);
	for (dns = ufnres; dns != NULLDNS; dns = dns->dns_next) {
		res =readPerson(dn2str(dns->dns_dn), &qinfo[PERSON].lp);
		alarmCleanUp();
		if (res != OK) {
			resetprint("Couldn't read entry for `%s'\n",
					   lastComponent(dn2str(dns->dns_dn), PERSON));
		} else {
			(void) printUfnComponents(dn2str(dns->dns_dn));
			printDetails(PERSON, qinfo[PERSON].lp);
			freePRRs(&qinfo[PERSON].lp);
		}
		initAlarm();
	}
	alarmCleanUp();
	(void) strcpy(qinfo[PERSON].defvalue, qinfo[PERSON].entered);
	return;
}


/* ARGSUSED */
DNS ufn_interact (dns, dn, s)
DNS dns;
DN dn;
char * s;
{
	char    buf[LINESIZE], buf2[LINESIZE], penult[LINESIZE];
	DNS tmp, ptr;
	int i, j, n;
	char * cp;

	alarmCleanUp();
	if (dns == NULLDNS)
		return NULLDNS;
	pagerOn(NUMBER_ALLOWED);
	i = 0;
	for (ptr = dns; ptr; ptr = ptr -> dns_next) {
		i++;
		if (dns->dns_next != NULLDNS) { /* more than one in list */
			if (removeLastRDN(dn2str(ptr->dns_dn)) != NULLCP) {
				(void)strcpy(buf2, removeLastRDN(dn2str(ptr->dns_dn)));
				if (i == 1)
					j = printUfnComponents(buf2);
				else {
					if (strcmp(penult, lastRDN(buf2)) != 0) {
						(void)strcpy(buf, lastRDN(buf2));
						cp = index(buf, '=') + 1;
						pageprint("%-*s%s\n", j-1, "", cp);
					}
				}
				(void)strcpy(penult, lastRDN(buf2));
			} else
				j = 0; /* component under the root */
			(void)strcpy(buf, lastRDN(dn2str(ptr->dns_dn)));
			cp = index(buf, '=') + 1;
			pageprint("%-*s%3d %s\n", j, "", i, cp);
		}
	}
	if (i == 1)
		goto validNumber;
	if ((n = getpnum()) != -1)
		if (n > i)
			(void) fprintf(stderr,
						   "Invalid number entered (maximum = %d)\n\n", i);
		else
			goto validNumber;

reEnter:
	writeInverse("Enter number, or press RETURN if none of these entries is the one required\n: ");
	writeInverse(":- ");
	if (gets(buf) == NULLCP) { /* deal with control-D */
		clearerr(stdin);
		onint1();
	}
	cp = copy_string(TidyString(buf));
	if (strlen(cp) == 0) {
		free(cp);
		initAlarm();
		return NULLDNS;
	}
	n = atoi(cp);
	if ((n < 1) || (n > i)) {
		soundBell();
		goto reEnter;
	}
validNumber:
	for (ptr = dns, j = 1; j < n; j++) {
		tmp = ptr;
		ptr = ptr -> dns_next;
		tmp -> dns_next = NULL;
		dn_seq_free (tmp);
	}
	dn_seq_free(ptr->dns_next);
	ptr->dns_next = NULL;
	initAlarm();
	return(ptr);
}

int
printUfnComponents(str)
char * str;
{
	char buf[BUFSIZ];
	int j;
	char * cp, *cp2;

	if (str == NULLCP)
		return 0;
	(void)strcpy(buf, str);
	for (j = 0, cp = buf; cp != NULLCP; j++) {
		cp2 = index(cp, '@');
		if (cp2 != NULLCP)
			*cp2 = '\0';
		cp = index(cp, '=') + 1;
		pageprint("%*s%s\n", j, "", cp);
		if (cp2 != NULLCP)
			cp = cp2+1;
		else
			cp = NULLCP;
	}
	return j;
}
