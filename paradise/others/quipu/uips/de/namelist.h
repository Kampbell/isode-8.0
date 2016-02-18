/* template.c - your comments here */


/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/namelist.h,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: namelist.h,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:46  isode
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
#include "quipu/attrvalue.h"
#include "util.h"

struct namelist {
  char *name;
  struct namelist *next;
  Attr_Sequence ats;
};

#define NULLLIST (struct namelist *)NULL
#define list_alloc()   (struct namelist *)smalloc(sizeof (struct namelist))
