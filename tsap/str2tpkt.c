/* str2tpkt.c - read/write a TPDU thru a string */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/tsap/RCS/str2tpkt.c,v 9.0 1992/06/16 12:40:39 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/tsap/RCS/str2tpkt.c,v 9.0 1992/06/16 12:40:39 isode Rel $
 *
 *
 * $Log: str2tpkt.c,v $
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
#include "tpkt.h"
#include "tailor.h"


static int	readfnx (), getfnx (), writefnx (), putfnx ();

/*  */

char *
tpkt2str (struct tsapkt *t)
{
	int	    cc;
	char    packet[BUFSIZ];
	static char buffer[2 * sizeof packet + 1];

	 writefnx ((struct tsapblk *) NOTOK, packet, 0);
	if (tpkt2fd ((struct tsapblk *) 0, t, putfnx) == NOTOK)
		return NULLCP;

	cc = writefnx ((struct tsapblk *) NOTOK, NULLCP, 0);
	if (t -> t_qbuf) {
		bcopy (t -> t_qbuf -> qb_data, packet + cc, t -> t_qbuf -> qb_len);
		cc += t -> t_qbuf -> qb_len;
	}
	buffer[explode (buffer, (u_char *) packet, cc)] = NULL;

	DLOG (tsap_log, LLOG_PDUS,
		  ("write %d bytes, \"%s\"", strlen (buffer), buffer));

	return buffer;
}

/*  */

struct tsapkt *
str2tpkt (char *buffer)
{
	char    packet[BUFSIZ];
	struct tsapkt *t;

	DLOG (tsap_log, LLOG_PDUS,
		  ("read %d bytes, \"%s\"", strlen (buffer), buffer));

	 getfnx (NOTOK, NULLPKT, packet,
				   implode ((u_char *) packet, buffer, strlen (buffer)));
	t = fd2tpkt (0, getfnx, readfnx);

	return t;
}

/*  */

static int 
getfnx (int fd, struct tsapkt *t, char *buffer, int n)
{
	static int  cc;

	if (fd == NOTOK) {
		 readfnx (NOTOK, buffer, cc = n);
		return OK;
	}

	t -> t_length = cc + sizeof t -> t_pkthdr;
	t -> t_vrsn = TPKT_VRSN;

	if (readfnx (fd, (char *) &t -> t_li, sizeof t -> t_li)
			!= sizeof t -> t_li)
		return DR_LENGTH;

	if (readfnx (fd, (char *) &t -> t_code, sizeof t -> t_code)
			!= sizeof t -> t_code)
		return DR_LENGTH;

	return OK;
}


static int 
readfnx (int fd, char *buffer, int n)
{
	int    i;
	static int  cc;
	static char *bp;

	if (fd == NOTOK) {
		bp = buffer, cc = n;

		return OK;
	}

	if ((i = min (cc, n)) > 0) {
		bcopy (bp, buffer, n);
		bp += i, cc -= i;
	}

	return i;
}

/*  */

static int 
putfnx (struct tsapblk *tb, struct tsapkt *t, char *cp, int n)
{
	int    cc;
	struct udvec  *uv;

	cc = sizeof t -> t_li;
	if (writefnx (tb, (char *) &t -> t_li, cc) != cc)
		return NOTOK;

	if (writefnx (tb, (char *) &t -> t_code, sizeof t -> t_code)
			!= sizeof t -> t_code)
		return NOTOK;
	cc += sizeof t -> t_code;

	if (writefnx (tb, cp, n) != n)
		return NOTOK;
	cc += n;

	if (t -> t_vdata
			&& writefnx (tb, t -> t_vdata, t -> t_vlen) != t -> t_vlen)
		return NOTOK;
	cc += t -> t_vlen;

	for (uv = t -> t_udvec; uv -> uv_base; uv++) {
		if (writefnx (tb, uv -> uv_base, uv -> uv_len) != uv -> uv_len)
			return NOTOK;
		cc += uv -> uv_len;
	}

	return cc;
}

/*  */

static int 
writefnx (struct tsapblk *tb, char *buffer, int n)
{
	static int  cc;
	static char *bp;

	if (tb) {
		if (buffer == NULLCP)
			return cc;
		bp = buffer, cc = 0;

		return OK;
	}

	bcopy (buffer, bp, n);
	bp += n, cc += n;

	return n;
}
