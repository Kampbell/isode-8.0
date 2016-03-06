/* text2tpkt.c - test utilities for use with TPDU packets */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/tsap/RCS/text2tpkt.c,v 9.0 1992/06/16 12:40:39 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/tsap/RCS/text2tpkt.c,v 9.0 1992/06/16 12:40:39 isode Rel $
 *
 *
 * $Log: text2tpkt.c,v $
 * Revision 9.0  1992/06/16  12:40:39  isode
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
#include <ctype.h>
#include "tpkt.h"
#include "logger.h"
#include "internet.h"

/*  */

char *fgets ();
static	type_id ();
static	type_data ();

/*  */

#define	TPKT_TYPE(e)	 ll_printf (lp, "%sCODE/ %s\n", rw, e)


void 
tpkt2text (LLog *lp, struct tsapkt *t, int isread)
{
	char   *rw = isread ? "<--- " : "---> ";
	struct udvec *uv;

	LLOG (lp, LLOG_ALL, ("dump of TPKT 0x%x, errno=0x%x version=0x%x length=%d", t, t -> t_errno, t -> t_vrsn, t -> t_length));
	 ll_printf (lp, "%s(\n", rw);

	 ll_printf (lp, "%sLI/ %d\n", rw, t -> t_li);
	if (t -> t_vdata)
		type_data (lp, "VARIABLE", rw, t -> t_vlen, t -> t_vdata);

	switch (TPDU_CODE (t)) {
	case TPDU_CR:
	case TPDU_CC:
		TPKT_TYPE (TPDU_CODE (t) == TPDU_CR ? "CONNECT REQUEST" : "CONNECT CONFIRM");
		 ll_printf (lp, "%sDSTREF/ 0x%x\n", rw, ntohs (t -> t_cr.cr_dstref));
		 ll_printf (lp, "%sSRCREF/ 0x%x\n", rw, ntohs (t -> t_cr.cr_srcref));
		 ll_printf (lp, "%sCLASS/ 0x%x\n", rw, t -> t_cr.cr_class);
		if (t -> t_calledlen > 0)
			type_id (lp, "CALLED", rw, t -> t_called, t -> t_calledlen);
		if (t -> t_callinglen > 0)
			type_id (lp, "CALLING", rw, t -> t_calling, t -> t_callinglen);
		if (t -> t_tpdusize)
			 ll_printf (lp, "%sTPDUSIZE/ %d\n", rw, 1 << t -> t_tpdusize);
		 ll_printf (lp, "%sOPTIONS/ 0x%x\n", rw, t -> t_options);
		if (t -> t_alternate)
			 ll_printf (lp, "%sALTERNATES/ 0x%x\n", rw, t -> t_alternate);
		break;

	case TPDU_DR:
		TPKT_TYPE ("DISCONNECT REQUEST");
		 ll_printf (lp, "%sDSTREF/ 0x%x\n", rw, ntohs (t -> t_dr.dr_dstref));
		 ll_printf (lp, "%sSRCREF/ 0x%x\n", rw, ntohs (t -> t_dr.dr_srcref));
		 ll_printf (lp, "%sREASON/ 0x%x: %s\n", rw, t -> t_dr.dr_reason, TErrString ((int) t -> t_dr.dr_reason));
		break;

	case TPDU_DT:
	case TPDU_ED:
		TPKT_TYPE (TPDU_CODE (t) == TPDU_DT ? "DATA TRANSFER"
				   : "EXPEDITED DATA TRANSFER");
		 ll_printf (lp, "%sSEQUENCE/ %s0x%x\n", rw, t -> t_dt.dt_nr & DT_EOT ? "<EOT>+" : "", t -> t_dt.dt_nr & ~DT_EOT);
		break;

	case TPDU_ER:
		TPKT_TYPE ("ERROR");
		 ll_printf (lp, "%sDSTREF/ 0x%x\n", rw, ntohs (t -> t_er.er_dstref));
		 ll_printf (lp, "%sREJECT/ 0x%x\n", rw, t -> t_er.er_reject);
		break;

	default:
		 ll_printf (lp, "%sCODE/ 0x%x\n", rw, TPDU_CODE (t));
		break;
	}

	if (t -> t_qbuf && t -> t_qbuf -> qb_data)
		type_data (lp, "QBUF", rw, t -> t_qbuf -> qb_len,
				   t -> t_qbuf -> qb_data);
	for (uv = t -> t_udvec; uv -> uv_base; uv++)
		type_data (lp, "UVEC", rw, uv -> uv_len, uv -> uv_base);
	 ll_printf (lp, "%s)\n", rw);

	 ll_sync (lp);
}

/*  */

static 
type_id (LLog *lp, char *type, char *rw, char *selector, int len)
{
	char    buffer[BUFSIZ];

	buffer[explode (buffer, (u_char *) selector, len)] = NULL;

	 ll_printf (lp, "%s%s/ %d/\"%s\"\n", rw, type, len, buffer);
}


static 
type_data (LLog *lp, char *type, char *rw, int len, char *data)
{
	char    buffer[BUFSIZ];
	char *cp;
	int i;

	 ll_printf (lp, "%s%s DATA/ %d ", rw, type, len);
	for (cp = data; len > 0; ) {
		i = (sizeof buffer - 1) / 2;
		if (len < i)
			i = len;
		buffer[explode (buffer, (u_char *) cp, i)] = NULL;
		 ll_printf (lp, "%s", buffer);
		cp += i;
		len -= i;
	}
	 ll_printf (lp, "\n");
}

/*  */

void 
text2tpkt (struct tsapkt *t)
{
	char buffer[80],                            /* Working input buffer */
		 *bptr;                                 /* Pointer to our buffer */
	int data;

	 printf("Packet Length [%d]: ", data = t -> t_length);
	 fflush(stdout);
	 fgets(buffer, sizeof buffer, stdin);
	 sscanf(buffer, "%d", &data);
	t -> t_length = data;
	 printf("Packet Version [%02x]: ", data = t -> t_vrsn);
	 fflush(stdout);
	 fgets(buffer, sizeof buffer, stdin);
	 sscanf(buffer, "%x", &data);
	t -> t_vrsn = data;
	 printf("Packet Errno [%02x]: ", data = t -> t_errno);
	 fflush(stdout);
	 fgets(buffer, sizeof buffer, stdin);
	 sscanf(buffer, "%d", &data);
	t -> t_errno = data;
	 printf("TPDU Code [%02x]: ", data = t -> t_code);
	 fflush(stdout);
	bptr = fgets(buffer, sizeof buffer, stdin);
	while (isspace((u_char) *bptr) && (*bptr != '\0')) ++bptr;
	if (toupper(*bptr) == 'C') {
		if (toupper(*(bptr + 1)) == 'R') {
			data = 0xE0;
		} else if (toupper(*(bptr + 1)) == 'C') {
			data = 0xD0;
		} else  sscanf(buffer, "%x", &data);
	} else if (toupper(*bptr) == 'D') {
		if (toupper(*(bptr + 1)) == 'R') {
			data = 0x80;
		} else if (toupper(*(bptr + 1)) == 'T') {
			data = 0xF0;
		} else  sscanf(buffer, "%x", &data);
	} else  sscanf(buffer, "%x", &data);
	t -> t_code = data;
	switch (TPDU_CODE(t)) {
	case TPDU_CR:
	case TPDU_CC:
		t -> t_li = TPDU_MINLEN(t, CR);
		 printf("TPDU Fixed Length (LI) [%02x]: ", data = t -> t_li);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_li = data;
		 printf("Destination Reference [%04x]: ",
					  data = ntohs (t -> t_cr.cr_dstref));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_dstref = htons((u_short)data);
		 printf("Source Reference [%04x]: ", data = ntohs(t -> t_cr.cr_srcref));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_srcref = htons ((u_short)data);
		 printf("Class/Options [%02x]: ", data = t-> t_cr.cr_class);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_class = data;
		 printf("TPDU size [%02x]: ",
					  data = t -> t_cr.cr_tpdusize);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_tpdusize = data;
		 printf("Real Options [%02x]: ",data = ntohs(t -> t_cr.cr_options));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_options = htons((u_short)data);
		 printf("Alternate classes [%02x]: ",
					  data = t -> t_cr.cr_alternate);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_alternate = data;
		break;
	case TPDU_DR:
		t -> t_li = TPDU_MINLEN(t, DR);
		 printf("TPDU Fixed Length (LI) [%02x]: ", data = t -> t_li);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_li = data;
		 printf("Destination Reference [%04x]: ",
					  data = ntohs(t -> t_dr.dr_dstref));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_dr.dr_dstref = htons((u_short)data);
		 printf("Source Reference [%04x]: ", data = ntohs(t -> t_dr.dr_srcref));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_dr.dr_srcref = htons((u_short)data);
		 printf("Disconnect Reason [%02x]: ", data = t-> t_dr.dr_reason);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_dr.dr_reason = data;
		break;
	case TPDU_DT:
	case TPDU_ED:
		t -> t_li = TPDU_MINLEN(t, DT);
		 printf("TPDU Fixed Length (LI) [%02x]: ", data = t -> t_li);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_li = data;
		 printf("EOT/Sequence [%04x]: ", data = t -> t_dt.dt_nr);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_dt.dt_nr = data;
		break;
	case TPDU_ER:
		t -> t_li = TPDU_MINLEN(t, ER);
		 printf("TPDU Fixed Length (LI) [%02x]: ", data = t -> t_li);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_li = data;
		 printf("Destination Reference [%04x]: ",
					  data = ntohs(t -> t_er.er_dstref));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_er.er_dstref = htons((u_short)data);
		 printf("Reject Cause [%02x]: ", data = t-> t_er.er_reject);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_er.er_reject = data;
		break;
	default:
		t -> t_li = TPDU_MINLEN(t, CR);
		 printf("TPDU Fixed Length (LI) [%02x]: ", data = t -> t_li);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_li = data;
		 printf("Octets 3-4 [%04x]: ",
					  data = ntohs(t -> t_cr.cr_dstref));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_dstref = htons((u_short)data);
		 printf("Octets 5-6 [%04x]: ",
					  data = ntohs(t -> t_cr.cr_srcref));
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_srcref = htons((u_short)data);
		 printf("Octet 7 [%02x]: ", data = t-> t_cr.cr_class);
		 fflush(stdout);
		 fgets(buffer, sizeof buffer, stdin);
		 sscanf(buffer, "%x", &data);
		t -> t_cr.cr_class = data;
		break;
	}
#ifdef	notdef			/* Dwight can fix this... */
	 printf("Calling TSAP ID [%d]: ",
				  ntohs (t -> t_calling));
	 fflush(stdout);
	 fgets(buffer, sizeof buffer, stdin);
	 sscanf(buffer, "%d", &data);
	t -> t_calling = htons((u_short)data);
	 printf("Called TSAP ID [%d]: ",
				  ntohs (t -> t_called));
	 fflush(stdout);
	 fgets(buffer, sizeof buffer, stdin);
	 sscanf(buffer, "%d", &data);
	t -> t_called = htons((u_short)data);
#endif
}
