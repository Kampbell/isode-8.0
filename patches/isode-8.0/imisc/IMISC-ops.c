/* automatically generated by rosy 8.0 #1 (young.cs.ucl.ac.uk), do not edit! */

#include "IMISC-types.h"

#include <stdio.h>
#include "IMISC-ops.h"


/* OPERATIONS */

/* OPERATION utcTime */
static struct RyError *errors_IMISC_utcTime[] = {
	&table_IMISC_Errors[0],
	&table_IMISC_Errors[1]
};


/* OPERATION timeOfDay */
static struct RyError *errors_IMISC_timeOfDay[] = {
	&table_IMISC_Errors[0],
	&table_IMISC_Errors[1]
};


/* OPERATION users */
static struct RyError *errors_IMISC_users[] = {
	&table_IMISC_Errors[0],
	&table_IMISC_Errors[2]
};


/* OPERATION charGen */
static struct RyError *errors_IMISC_charGen[] = {
	&table_IMISC_Errors[0]
};


/* OPERATION qotd */
static struct RyError *errors_IMISC_qotd[] = {
	&table_IMISC_Errors[0],
	&table_IMISC_Errors[3],
	&table_IMISC_Errors[4],
	&table_IMISC_Errors[5],
	&table_IMISC_Errors[6]
};


/* OPERATION finger */
static struct RyError *errors_IMISC_finger[] = {
	&table_IMISC_Errors[0],
	&table_IMISC_Errors[3],
	&table_IMISC_Errors[4],
	&table_IMISC_Errors[5],
	&table_IMISC_Errors[6]
};


/* OPERATION pwdGen */
static struct RyError *errors_IMISC_pwdGen[] = {
	&table_IMISC_Errors[0]
};


/* OPERATION genTime */
static struct RyError *errors_IMISC_genTime[] = {
	&table_IMISC_Errors[0],
	&table_IMISC_Errors[1]
};


/* OPERATION tellUser */
static struct RyError *errors_IMISC_tellUser[] = {
	&table_IMISC_Errors[0],
	&table_IMISC_Errors[2],
	&table_IMISC_Errors[7]
};


/* OPERATION ping */
static struct RyError *errors_IMISC_ping[] = {
	&table_IMISC_Errors[0]
};


/* OPERATION sink */
static struct RyError *errors_IMISC_sink[] = {
	&table_IMISC_Errors[0]
};


/* OPERATION echo */
static struct RyError *errors_IMISC_echo[] = {
	&table_IMISC_Errors[0]
};


struct RyOperation table_IMISC_Operations[] = {
	/* OPERATION utcTime */
	"utcTime", operation_IMISC_utcTime,
	NULL,
	NULL,
	1,
	&_ZIMISC_mod,
	_ZUTCResultIMISC,
	errors_IMISC_utcTime,

	/* OPERATION timeOfDay */
	"timeOfDay", operation_IMISC_timeOfDay,
	NULL,
	NULL,
	1,
	&_ZIMISC_mod,
	_ZTimeResultIMISC,
	errors_IMISC_timeOfDay,

	/* OPERATION users */
	"users", operation_IMISC_users,
	NULL,
	NULL,
	1,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	errors_IMISC_users,

	/* OPERATION charGen */
	"charGen", operation_IMISC_charGen,
	NULL,
	NULL,
	1,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	errors_IMISC_charGen,

	/* OPERATION qotd */
	"qotd", operation_IMISC_qotd,
	NULL,
	NULL,
	1,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	errors_IMISC_qotd,

	/* OPERATION finger */
	"finger", operation_IMISC_finger,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	1,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	errors_IMISC_finger,

	/* OPERATION pwdGen */
	"pwdGen", operation_IMISC_pwdGen,
	NULL,
	NULL,
	1,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	errors_IMISC_pwdGen,

	/* OPERATION genTime */
	"genTime", operation_IMISC_genTime,
	NULL,
	NULL,
	1,
	&_ZIMISC_mod,
	_ZGenResultIMISC,
	errors_IMISC_genTime,

	/* OPERATION tellUser */
	"tellUser", operation_IMISC_tellUser,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	1,
	&_ZIMISC_mod,
	_ZEmptyIMISC,
	errors_IMISC_tellUser,

	/* OPERATION ping */
	"ping", operation_IMISC_ping,
	&_ZIMISC_mod,
	_ZEmptyIMISC,
	1,
	&_ZIMISC_mod,
	_ZEmptyIMISC,
	errors_IMISC_ping,

	/* OPERATION sink */
	"sink", operation_IMISC_sink,
	&_ZIMISC_mod,
	_ZDataIMISC,
	1,
	&_ZIMISC_mod,
	_ZEmptyIMISC,
	errors_IMISC_sink,

	/* OPERATION echo */
	"echo", operation_IMISC_echo,
	&_ZIMISC_mod,
	_ZDataIMISC,
	1,
	&_ZIMISC_mod,
	_ZDataIMISC,
	errors_IMISC_echo,

	NULL
};


/* ERRORS */

struct RyError table_IMISC_Errors[] = {
	/* ERROR congested */
	"congested", error_IMISC_congested,
	NULL,
	NULL,
	/* ERROR unableToDetermineTime */
	"unableToDetermineTime", error_IMISC_unableToDetermineTime,
	NULL,
	NULL,
	/* ERROR unableToOpenFile */
	"unableToOpenFile", error_IMISC_unableToOpenFile,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	/* ERROR unableToAccessFile */
	"unableToAccessFile", error_IMISC_unableToAccessFile,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	/* ERROR unableToPipe */
	"unableToPipe", error_IMISC_unableToPipe,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	/* ERROR unableToFork */
	"unableToFork", error_IMISC_unableToFork,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	/* ERROR errorReading */
	"errorReading", error_IMISC_errorReading,
	&_ZIMISC_mod,
	_ZIA5ListIMISC,
	/* ERROR userNotLoggedIn */
	"userNotLoggedIn", error_IMISC_userNotLoggedIn,
	NULL,
	NULL,
	NULL
};
