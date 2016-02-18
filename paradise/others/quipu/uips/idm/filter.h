/* template.c - your comments here */

/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/filter.h,v 8.1 1991/09/13 14:36:41 isode Exp $
 *
 *
 * $Log: filter.h,v $
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:42  isode
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


struct s_filter * andfilter();
struct s_filter * orfilter();
struct s_filter * eqfilter();
struct s_filter * subsfilter();
struct s_filter * presfilter();
