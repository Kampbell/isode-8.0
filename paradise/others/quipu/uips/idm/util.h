/* template.c - your comments here */

/* 
 * $Header: /xtel/isode/isode/others/quipu/uips/de/RCS/util.h,v 8.1 1991/09/13 14:36:41 isode Exp $
 *
 *
 * $Log: util.h,v $
 * Revision 8.1  1991/09/13  14:36:41  isode
 * PARADISE Upgrade release
 *
 * Revision 8.0  91/07/17  13:18:50  isode
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


#ifndef _xdua_util_h_
#define _xdua_util_h_

char *copy_string();
char *lastComponent();
char * removeLastRDN();
char * lastRDN();
char *dn2pstr();

void addToList();
void addToAttList();
void addToCoList();
void addToPhoneList();
void printLastComponent();
void printListCos();
void printListOrgs();
void printListOUs();
void printListPRRs();
void freeCos();
void freeOrgs();
void freeOUs();
void freePRRs();
void initVideo();
void clearProblemFlags();
void setProblemFlags();
void logSearchSuccess();
void logListSuccess();
void initAlarm();
void alarmCleanUp();

#define LEADSUBSTR 1
#define TRAILSUBSTR 2
#define ANYSUBSTR 3
#define LEADANDTRAIL 4

#endif _xdua_util_h_

