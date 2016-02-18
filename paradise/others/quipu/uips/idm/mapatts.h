/* template.c - your comments here */


/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/mapatts.h,v 8.1 1991/09/13 14:36:41 isode Exp $
 *
 *
 * $Log: mapatts.h,v $
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:44  isode
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


struct mapnamelist {
  char *tablename;
  char *nicename;
  struct mapnamelist *next;
};

#define NULLMNLIST (struct mapnamelist *)NULL
#define mapname_alloc()   (struct mapnamelist *)smalloc(sizeof (struct mapnamelist))

void addToAttList();
void addAttNode();
char * mapAttName();
