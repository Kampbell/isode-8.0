/* fdx2ps.c - full-duplex abstraction for PStreams */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/psap/RCS/fdx2ps.c,v 9.0 1992/06/16 12:25:44 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/psap/RCS/fdx2ps.c,v 9.0 1992/06/16 12:25:44 isode Rel $
 *
 *
 * $Log: fdx2ps.c,v $
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

static int  fdx_flush ();
/*    DATA */

struct ps_fdx {
	int	    ps_fd;

	struct ps_inout {
		char   *pio_base;
		int	pio_bufsiz;

		char   *pio_ptr;
		int	pio_cnt;
	}	    ps_input,
	  ps_output;

	int	    ps_nflush;
};


extern	IFP	set_check_fd ();

/* */

static int
fdx_prime (PS ps, int waiting) {
	struct ps_fdx *pt = (struct ps_fdx *) ps -> ps_addr;
	struct ps_inout *pi = &pt -> ps_input;

	return (waiting > 0 && pi -> pio_cnt > 0 ? DONE : OK);
}

/*  */

/* ARGSUSED */

static int
fdx_read (PS ps, PElementData data, PElementLen n, int in_line) {
	int	    cc;
	struct ps_fdx *pt = (struct ps_fdx *) ps -> ps_addr;
	struct ps_inout *pi = &pt -> ps_input;

	if ((cc = pi -> pio_cnt) <= 0) {
		if (n > pi -> pio_bufsiz) {
			if ((cc = read (pt -> ps_fd, (char *) data, n)) == NOTOK)
				return ps_seterr (ps, PS_ERR_IO, NOTOK);

			return cc;
		}

		if ((cc = read (pt -> ps_fd, pi -> pio_base, pi -> pio_bufsiz))
				== NOTOK)
			return ps_seterr (ps, PS_ERR_IO, NOTOK);
		pi -> pio_ptr = pi -> pio_base, pi -> pio_cnt = cc;
	}

	if (cc > n)
		cc = n;

	bcopy (pi -> pio_ptr, (char *) data, cc);
	pi -> pio_ptr += cc, pi -> pio_cnt -= cc;

	return cc;
}


/* ARGSUSED */

static int
fdx_write (PS ps, PElementData data, PElementLen n, int in_line) {
	int	    cc;
	struct ps_fdx *pt = (struct ps_fdx *) ps -> ps_addr;
	struct ps_inout *po = &pt -> ps_output;

#ifdef	oldef
	if (n > po -> pio_bufsiz) {
		if (fdx_flush (ps) == NOTOK
				|| (cc = write (pt -> ps_fd, (char *) data, n)) != n)
#else
	if (n > po -> pio_bufsiz && po -> pio_ptr <= po -> pio_base) {
		if ((cc = write (pt -> ps_fd, (char *) data, n)) != n)
#endif
			return ps_seterr (ps, PS_ERR_IO, NOTOK);

		return cc;
	}

	if (n > po -> pio_cnt)
		n = po -> pio_cnt;

	bcopy ((char *) data, po -> pio_ptr, n);
	po -> pio_ptr += n, po -> pio_cnt -= n;

	if (po -> pio_cnt <= 0 && fdx_flush (ps) == NOTOK)
		return ps_seterr (ps, PS_ERR_IO, NOTOK);

	return n;
}


static int  fdx_flush (ps)
PS	ps;
{
	int	    cc;
	struct ps_fdx *pt = (struct ps_fdx *) ps -> ps_addr;
	struct ps_inout *po = &pt -> ps_output;

	if ((cc = po -> pio_ptr - po -> pio_base) <= 0)
		return OK;
	pt -> ps_nflush++;

	if (write (pt -> ps_fd, po -> pio_base, cc) != cc)
		return ps_seterr (ps, PS_ERR_IO, NOTOK);
	po -> pio_ptr = po -> pio_base, po -> pio_cnt = po -> pio_bufsiz;

	return OK;
}


static int  fdx_close (ps)
PS	ps;
{
	struct ps_fdx *pt = (struct ps_fdx *) ps -> ps_addr;

	if (pt == NULL)
		return OK;

	if (pt -> ps_input.pio_base)
		free (pt -> ps_input.pio_base);
	if (pt -> ps_output.pio_base)
		free (pt -> ps_output.pio_base);

	set_check_fd (pt -> ps_fd, NULLIFP, NULLCP);

	free ((char *) pt);

	return OK;
}


/* ARGSUSED */
static int  fdx_check (fd, data)
int	fd;
caddr_t	data;
{
	return (ps_prime ((PS) data, 1) > 0 ? DONE : OK);
}

/*  */

int	fdx_open (ps)
PS	ps;
{
	ps -> ps_primeP = fdx_prime;
	ps -> ps_readP = fdx_read;
	ps -> ps_writeP = fdx_write;
	ps -> ps_flushP = fdx_flush;
	ps -> ps_closeP = fdx_close;

	return OK;
}


int	fdx_setup (ps, fd)
PS	ps;
int	fd;
{
	int	    pz;
	struct ps_fdx *pt;

	if ((pt = (struct ps_fdx *) calloc (1, sizeof *pt)) == NULL)
		return ps_seterr (ps, PS_ERR_NMEM, NOTOK);
	ps -> ps_addr = (caddr_t) pt;

	pt -> ps_fd = fd;

#ifdef	BSD42
	if ((pz = getpagesize ()) <= 0)
#endif
		pz = BUFSIZ;

	if ((pt -> ps_input.pio_base = malloc ((unsigned) pz)) == NULL
			|| (pt -> ps_output.pio_base = malloc ((unsigned) pz)) == NULL)
		return ps_seterr (ps, PS_ERR_NMEM, NOTOK);
	pt -> ps_input.pio_bufsiz = pz, pt -> ps_output.pio_cnt = 0;
	pt -> ps_input.pio_ptr = pt -> ps_input.pio_base;

	pt -> ps_output.pio_bufsiz = pt -> ps_output.pio_cnt = pz;
	pt -> ps_output.pio_ptr = pt -> ps_output.pio_base;

	set_check_fd (fd, fdx_check, (caddr_t) ps);

	return OK;
}


int	fdx_reset (ps)
PS	ps;
{
	struct ps_fdx *pt = (struct ps_fdx *) ps -> ps_addr;
	struct ps_inout *po = &pt -> ps_output;

	if (pt -> ps_nflush == 0)
		po -> pio_ptr = po -> pio_base, po -> pio_cnt = po -> pio_bufsiz;
	else
		return ps_seterr (ps, PS_ERR_NONE, NOTOK);

	return OK;

}
