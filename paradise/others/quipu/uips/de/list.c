/* template.c - your comments here */

#ifndef	lint
static char *rcsid = "$Header: /xtel/isode/isode/others/quipu/uips/de/RCS/list.c,v 9.1 1992/08/25 15:50:26 isode Exp $";
#endif

/*
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/list.c,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: list.c,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:43  isode
 * Release 7.0
 *
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


#include <stdio.h>
#include "util.h"
#include "namelist.h"
#include "quipu/util.h"

void
addToList(lp, str)
struct namelist ** lp;
char *str;
{
	for (; *lp != NULLLIST; lp = &(*lp)->next) {};
	*lp = list_alloc();
	(*lp)->name = copy_string(str);
	(*lp)->next = NULLLIST;
}

