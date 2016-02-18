
#include <signal.h>
#include "manifest.h"
#include "quipu/util.h"
#include <fcntl.h>
#include <sys/stat.h>
#include "quipu/dua.h"
#include "quipu/name.h"
#include "quipu/bind.h"
#include "quipu/dsp.h"
#include "quipu/ds_error.h"
#include "quipu/entry.h"
#include "quipu/read.h"
#include "tailor.h"
/*#include "pepsy.h"*/
#include "extmessages.h"

#include <stdio.h>
#include "bind.h"
#include "quipu/common.h"
#include "quipu/bind.h"
#include "quipu/dap2.h"
#include "logger.h"

extern char *dsa_address;
extern char username[];
extern char password[];
extern char init_user[];
extern char init_pswd[];
extern char addorg[];
extern char yes_string[];
extern char callingDteNumber[];
extern char *new_version();
static char passwd[LINESIZE];

extern int boundToDSA;
extern int deLogLevel;

extern LLog *de_log;

/* this is not the way to do it */
extern int dsap_ad;

struct DAPconnect         dc;
struct DAPindication      di;
struct ds_bind_arg bind_arg;

void de_exit();

static int bindres;
/*
 * bind_to_ds - Bind to directory
 *
 */
int init_bind_to_ds(assoc)
int * assoc;
{
	struct PSAPaddr             *addr;
	int protect_password();
	void exit();

	if ((strlen(username) > 0)) {
		bind_arg.dba_auth_type = DBA_AUTH_SIMPLE;
		bind_arg.dba_dn = NULLDN;
		bind_arg.dba_dn = str2dn(username);
		if (username != NULL && bind_arg.dba_dn == NULLDN) {
			return INV_USERNAME;
		}
		(void) sprintf(passwd, "%s", password);
		(void) strcpy(bind_arg.dba_passwd, passwd);
		bind_arg.dba_passwd_len = strlen(passwd);
	} else {
		if (!(strcmp(addorg, yes_string))) {
			if (strlen(init_user) <= 0) {
				(void) printf("init_user required in tailor file for adding temporary organisations \n");
				return NOTOK;
			} else if(strlen(init_pswd) <= 0) {
				(void) printf("init_pswd required in tailor file for adding temporary organisations\n");
				return NOTOK;
			}
			bind_arg.dba_dn = NULLDN;
			bind_arg.dba_dn = str2dn(init_user);
			if (strlen(init_user) > 0 && (bind_arg.dba_dn == NULLDN)) {
				return INV_USERNAME;
			}
			(void) sprintf(passwd, "%s", init_pswd);
			(void) strcpy(bind_arg.dba_passwd, passwd);
			bind_arg.dba_passwd_len = strlen(passwd);
		} else {
			if (strlen(init_user) <= 0) {
				bind_arg.dba_dn = str2dn("@c=GB@o=University College London@ou=Computer Science@cn=Directory Manager");
				bind_arg.dba_passwd_len = 0;
			} else {
				bind_arg.dba_dn = str2dn(init_user);
				if (strlen(init_pswd) <= 0) {
					bind_arg.dba_passwd_len = 0;
				} else {
					/* Must verify they are correct and set them accordingly */
					(void) sprintf(passwd, "%s", init_pswd);
					(void) strcpy(bind_arg.dba_passwd, passwd);
					bind_arg.dba_passwd_len = strlen(passwd);
				}
			}
		}
		bind_arg.dba_auth_type = DBA_AUTH_SIMPLE;
	}
	bind_arg.dba_time1 = new_version();
	bind_arg.dba_time2 = NULLCP;
	bind_arg.dba_r1.n_bits = 0;
	bind_arg.dba_r1.value = NULLCP;
	bind_arg.dba_r2.n_bits = 0;
	bind_arg.dba_r2.value = NULLCP;
	bind_arg.dba_version = DBA_VERSION_V1988;

	if ((addr = str2paddr (dsa_address)) == NULLPA) {
		(void) fprintf(stderr, "%s", addr_fmt_pbm);
		exit(-1);
	}
	bindres = DapAsynBindRequest(addr, &bind_arg, &dc, &di, ROS_ASYNC);
	if (bindres == NOTOK) {
		(void) fprintf(stderr, "\n\n%s", srvr_unava);
		return NOTOK;
	}

	*assoc = dc.dc_sd;
	dsap_ad = dc.dc_sd;
	return OK;
}

int wait_bind_to_ds(assoc, wantToBlock)
int assoc;
int wantToBlock;
{
	extern int pswd_intrctv;
	extern int username_intrctv;
	struct PSAPindication   pi_s;
	struct PSAPindication   * pi = &(pi_s);
	int     nfds, nevents;
	fd_set  rfds;
	fd_set  wfds;


	while ((bindres == CONNECTING_1) || (bindres == CONNECTING_2)) {

		FD_ZERO (&rfds);
		FD_ZERO (&wfds);

		if ((bindres == CONNECTING_2) && (wantToBlock == FALSE))
			return CONNECTING_2;

		if (bindres == CONNECTING_1) {
			if (PSelectMask (assoc, &wfds, &nfds, pi) == NOTOK) {
				(void) fprintf(stderr, "%s", pSel_wrt_fail);
				return NOTOK;
			}
		}

		if (bindres == CONNECTING_2) {
			if (PSelectMask (assoc, &rfds, &nfds, pi) == NOTOK) {
				(void) fprintf(stderr, "%s", pSel_rd_fail);
				return NOTOK;
			}
		}

		nevents = xselect (nfds, &rfds, &wfds, NULLFD, NOTOK);

		if (nevents == NOTOK) {
			(void) fprintf(stderr, "%s", net_conn_ref);
			return NOTOK;
		} else {
			/*
			 * Hack coming up: nevents is number of events, we only asked for
			 * one event, so unless there are no events we go ahead without
			 * checking those horrid fd things, which would be safer.
			 */
			if (nevents != OK)
				bindres = DapAsynBindRetry(assoc, 0, &dc, &di);
		}

		if (bindres == DONE) {
			/* Check status of bind result */
			if (dc.dc_result != DC_RESULT) {
				/* Error in connection */
				if ((dc.dc_un.dc_bind_err.dbe_type == DBE_TYPE_SECURITY) &&
						dc.dc_un.dc_bind_err.dbe_value == DSE_SC_INVALIDCREDENTIALS) {
					(void) printf(errmsg_pwd);

					if (pswd_intrctv == FALSE) {
						if (username_intrctv == TRUE) {
							(void) printf(usrpsw_nomatch);
							de_exit(-1);
						} else {
							(void) printf(errmsg_byepswd);
							de_exit(-1);
						}
					} else {
						return INV_PSWD;
					}
				} else {
					(void) printf(errmsg_bind);
					boundToDSA = FALSE;
					return NOTOK;
				}
			}
			boundToDSA = TRUE;
			if (deLogLevel)
				(void) ll_log (de_log, LLOG_NOTICE, NULLCP, "Bound: %s", callingDteNumber);
			return OK;
		}
		if (bindres == NOTOK) {
			return NOTOK;
		}
	}
	return NOTOK; /* exit this way if can't talk to access point */
}

de_unbind() {
	if (deLogLevel)
		(void) ll_log (de_log, LLOG_NOTICE, NULLCP, "Unbind:");
	(void) ds_unbind();
	boundToDSA = FALSE;
}
