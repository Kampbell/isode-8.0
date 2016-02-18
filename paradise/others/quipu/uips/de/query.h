/* query.h - main information structure */

/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/query.h,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: query.h,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
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


#ifndef _query_h
#define _query_h

#include "quipu/util.h"

struct query {
	char defvalue [LINESIZE];
	char entered [LINESIZE];
	char displayed [LINESIZE];
	struct namelist * lp;
	int listlen;
};

#endif _query_h
