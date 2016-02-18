/* template.c - your comments here */

/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/bind.h,v 9.1 1992/08/25 15:50:26 isode Exp $
 *
 *
 * $Log: bind.h,v $
 * Revision 9.1  1992/08/25  15:50:26  isode
 * PARADISE release
 *
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:36  isode
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


#ifndef _xdua_h_
#define _xdua_h_

#include "types.h"

typedef enum {
  EXTERNAL_AUTH,
  STRONG_AUTH,
  PROTECTED_AUTH,
  SIMPLE_AUTH
  } auth_bind_type;

int bind_to_ds();

#endif _xdua_h_
