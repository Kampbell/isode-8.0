/* template.c - your comments here */

/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/demanifest.h,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: demanifest.h,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:39  isode
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


#ifndef _demanifest_h
#define _demanifest_h

#define INDENTON 1
#define INDENTOFF 0

#define NUMBER_ALLOWED 1
#define NUMBER_NOT_ALLOWED 0

#define PERSON 1
#define ORGUNIT 2
#define ORG 3
#define LOCALITY 4
#define COUNTRY 5
#define MAXTYPE 5

#define SEARCH_OK "ok"
#define SEARCH_ERROR "error"
#define SEARCH_FAIL "failed"

#define LIST_OK "ok"
#define LIST_ERROR "error"

#define READ_OK "ok"
#define READ_FAIL "failed"

#define QUERY_ERROR 1
#define NAME_PRINTED 2
#define LIST_PRINTED 3
#define NO_DEPT_FOUND 4
#define PARENT_PRINTED 5
#define NO_ORG_ENTERED 6
#define START_NEW_QUERY 7
#define NO_LOCALITY 8
#define NO_LOCALITY_SELECTED 9
#define NO_ORG_FOUND 10

#define SF_ABANDONED 1
#define SF_ACCRIGHT 2
#define SF_OPFAIL 3

#endif _demanifest_h
