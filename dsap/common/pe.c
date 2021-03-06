/* pe.c - General PE utility routines */

#ifndef lint
static char *rcsid = "$Header: /xtel/isode/isode/dsap/common/RCS/pe.c,v 9.0 1992/06/16 12:12:39 isode Rel $";
#endif

/*
 * $Header: /xtel/isode/isode/dsap/common/RCS/pe.c,v 9.0 1992/06/16 12:12:39 isode Rel $
 *
 *
 * $Log: pe.c,v $
 * Revision 9.0  1992/06/16  12:12:39  isode
 * Release 8.0
 *
 */

/*
 *                                NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include "quipu/util.h"
#include "psap.h"
#include "quipu/attr.h"               /* for defn of READOUT */

extern LLog * log_dsap;

pe_print (ps, pe, format)
PS     ps;
PE     pe;
int     format;
{
	char * ptr, *s;
	int i, j;
	PS sps;
	static char hex[] = "0123456789abcdef";
	char buffer [LINESIZE];

	if ( format == FILEOUT) {
		pe2ps (ps,pe);
		return;
	}

	if ((format == READOUT) && (pe->pe_len >= LINESIZE)) {
		ps_print (ps,"ASN attribute too big to print here!");
		return;
	}

	if ((sps = ps_alloc (str_open)) == NULLPS)
		return;
	if (str_setup (sps,NULLCP,LINESIZE,0) == NOTOK)
		return;

	if (format != READOUT)
		ps_write (ps, (PElementData)"{ASN}", 5);

	pe2ps (sps,pe);

	s = buffer;

	ptr = sps->ps_base;
	for (i=0, j=0; i<sps->ps_byteno; i++) {
		/*
				ps_printf (sps2,fmt,*ptr++ & 255);
		*/
		*s++ = hex [((*ptr & 255)/16) % 16];
		*s++ = hex [(*ptr++ & 255) % 16];
		j += 2;
		if ( j >= EDB_LINEWRAP ) {
			*s++ = '\\';
			*s++ = '\n';
			ps_write (ps, (PElementData)buffer, j);
			s = buffer;
			j = 0;
		}
	}
	ps_write (ps, (PElementData)buffer, j);
	ps_write (ps, (PElementData)"00", 2);
	ps_free (sps);

}

PE asn2pe (str)
char * str;
{
	char * ptr;
	char * pe_ptr;
	int i,j;
	PS sps;
	void StripSpace ();
	PE pe;
	extern char hex2nib[];
#ifdef oldcode
	int val;
#endif

	StripSpace (str);

	j = strlen (str);
	if (j % 2 == 1) {
		LLOG (log_dsap,LLOG_EXCEPTIONS,
			  ("asn2pe: not an even number of bytes `%s'", str));
		j--;
	}
	pe_ptr = (char *) smalloc (j+10);
	ptr = pe_ptr;

	for ( i=0 ; i<j; ) {
		if (!isascii(*str) || !isxdigit(*str)) { /* skip bad chars */
			str ++, i++;
			continue;
		}
		*ptr = hex2nib[*str++ & 0x7f] << 4;
		*ptr |= hex2nib[*str++ & 0x7f];
		*ptr++ &= 0xff;
		i += 2;
#ifdef oldcode
		sscanf (str,"%2x",&val);

		*ptr++ = val & 0xff;
		str++;
		str++;
		i+=2;
#endif
	}

	if ((sps = ps_alloc (str_open)) == NULLPS)
		return(NULLPE);
	if (str_setup (sps,pe_ptr,j+10,0) == NOTOK)
		return(NULLPE);

	pe = ps2pe (sps);
	if (sps->ps_errno != PS_ERR_NONE) {
		LLOG (log_dsap,LLOG_EXCEPTIONS,("%s in ASN attribute ",ps_error(sps->ps_errno)));
		if (pe) {
			pe_free (pe);
			pe = NULLPE;
		}
	}

	free (pe_ptr);
	ps_free (sps);

	return (pe);
}
