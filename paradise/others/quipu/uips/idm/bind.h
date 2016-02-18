/* template.c - your comments here */

/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/bind.h,v 8.1 1991/09/13 14:36:41 isode Exp $
 *
 *
 * $Log: bind.h,v $
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

#define INV_PSWD -2
#define PSWD_TRY_LIMIT 3
#define USERNAME_TRY_LIMIT 3
#define INV_USERNAME -3
