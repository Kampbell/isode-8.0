/* template.c - your comments here */

/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/types.h,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: types.h,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:49  isode
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


#ifndef _query_types_h_
#define _query_types_h_

#include <stdio.h>

#define bool unsigned char
#define cardinal unsigned int

typedef enum {
  yes,
  no,
  unknown
  } known;

typedef enum {
  RQ_processing,
  RQ_results_returned,
  RQ_error_returned
  } request_state;

#endif
