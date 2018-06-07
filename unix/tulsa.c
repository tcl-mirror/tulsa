/*
 * Copyright (c) 2014,2018 Stuart Cassoff <stwo@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/*
 * Tcl Unix Local Sockets, Eh?
 */


#ifdef __cplusplus
extern "C" {
#endif


#define MIN_TCL_VERSION "8.6"

#include <assert.h>	/* assert */
#include <sys/types.h>	/* socket types */
#include <sys/socket.h>	/* socket */
#include <sys/un.h>	/* unix socket */
#include <errno.h>	/* errvals */
#include <unistd.h>	/* read,write,close */
#include <fcntl.h>	/* fcntl */
#include <string.h>	/* strlen */

#include <tcl.h>

#include "int2ptr_ptr2int.h"
#include "eagain_ewouldblock.h"

#define EZT_NS		"::tulsa"
#define EZT_EN		"tulsa"
#define EZT_CMD		"tulsa"
#define EZT_INIT	Tulsa_Init
#define EZT_SAFE_INIT	Tulsa_SafeInit
#define EZT_PKGINIT	Tulsa_PackageInit

#define EZT_CMD_CLIENTDATA 1
#define EZT_TIMEVAL 1

#include "ezt.h"

TCMD(Tulsa_Tulsa_LICENSE_Cmd);
TCMD(Tulsa_Tulsa_ABOUT_Cmd);

TCMD(Tulsa_Tulsa_open_Cmd);
TCMD(Tulsa_Tulsa_pair_Cmd);
TCMD(Tulsa_Tulsa_accept_Cmd);
TCMD(Tulsa_Tulsa_srdopts_Cmd);
TCMD(Tulsa_Tulsa_swropts_Cmd);
TCMD(Tulsa_Tulsa_crdopts_Cmd);
TCMD(Tulsa_Tulsa_cwropts_Cmd);

static Ezt_Cmd Ezt_Cmds[] = {
	{ "client"  , Tulsa_Tulsa_open_Cmd    , (ClientData) 0 },
	{ "server"  , Tulsa_Tulsa_open_Cmd    , (ClientData) 1 },
	{ "pair"    , Tulsa_Tulsa_pair_Cmd    , NULL           },
	{ "accept"  , Tulsa_Tulsa_accept_Cmd  , NULL           },
	{ "srdopts" , Tulsa_Tulsa_srdopts_Cmd , NULL           },
	{ "swropts" , Tulsa_Tulsa_swropts_Cmd , NULL           },
	{ "crdopts" , Tulsa_Tulsa_crdopts_Cmd , NULL           },
	{ "cwropts" , Tulsa_Tulsa_cwropts_Cmd , NULL           },
	{ NULL, NULL, NULL }
};

/***/

typedef struct Tulsa {
	Tcl_Obj *me;		/* Myself and I */
	Tcl_Channel ch;		/* Tcl handle */
	int fd;			/* OS handle */
	Tcl_Obj *paths;		/* paths to server sockets */
				/* only servers have non-NULL paths */
} Tulsa;

static Tulsa * Tulsa_NewTulsa (void);
static void Tulsa_DeleteTulsa (Tulsa *t);

static Tulsa * Tulsa_Tulsify (int fd);
static int Tulsa_SetupSockaddr (struct sockaddr_un *sap, Tcl_Obj *path);
static Tulsa * Tulsa_OpenSocket (Tcl_Interp *interp, Tcl_Obj *path, int server);

/***/

static Tcl_DriverCloseProc     Tulsa_SocketCloseProc;
static Tcl_DriverInputProc     Tulsa_SocketInputProc;
static Tcl_DriverOutputProc    Tulsa_SocketOutputProc;
static Tcl_DriverSetOptionProc Tulsa_SocketSetOptionProc;
static Tcl_DriverGetOptionProc Tulsa_SocketGetOptionProc;
static Tcl_DriverWatchProc     Tulsa_SocketWatchProc;
static Tcl_DriverGetHandleProc Tulsa_SocketGetHandleProc;
static Tcl_DriverBlockModeProc Tulsa_SocketBlockModeProc;

static Tcl_ChannelType TulsaChannelType = {
	(char *) "tulsa"		, /* Name */
	TCL_CHANNEL_VERSION_2		, /* Version */
	Tulsa_SocketCloseProc		, /* Close */
	Tulsa_SocketInputProc		, /* Input */
	Tulsa_SocketOutputProc		, /* Output */
	NULL				, /* Seek */
	Tulsa_SocketSetOptionProc	, /* Set options */
	Tulsa_SocketGetOptionProc	, /* Get options */
	Tulsa_SocketWatchProc		, /* Watch */
	Tulsa_SocketGetHandleProc	, /* Get handle */
	NULL				, /* Close2 */
	Tulsa_SocketBlockModeProc	, /* Set blocking mode */
	NULL				, /* Flush */
	NULL				, /* Handler */
	NULL				, /* Wide seek */
	NULL				, /* Thread action */
	NULL				  /* Truncate */
};

#ifndef HAVE_GETPEEREID
#  include "bsd-getpeereid.c"
#endif


static int
Tulsa_SocketCloseProc (ClientData instanceData, Tcl_Interp *interp) {
	Tulsa *t = (Tulsa *) instanceData;
	int errCode = 0;

	assert(t);

	if (close(t->fd) < 0) {
		errCode = Tcl_GetErrno();
	}

	t->fd = -2;

	if (t->paths != NULL) {
		Tcl_Obj *srv_path;
		Tcl_ListObjIndex(NULL, t->paths, 1, &srv_path);
		Tcl_FSDeleteFile(srv_path);
	}

	Tulsa_DeleteTulsa(t);

	return errCode;
}


static int
Tulsa_SocketInputProc (ClientData instanceData, char *buf, int qty, int *errorCode) {
	Tulsa *t = (Tulsa *) instanceData;
	int n;

	*errorCode = (n = read(t->fd, buf, (size_t) qty)) == -1 ? Tcl_GetErrno() : 0;

	return n;
}


static int
Tulsa_SocketOutputProc (ClientData instanceData, const char *buf, int qty, int *errorCode) {
	Tulsa *t = (Tulsa *) instanceData;
	int n;

	*errorCode = (n = write(t->fd, buf, (size_t) qty)) == -1 ? Tcl_GetErrno() : 0;

#if 0
/* is this needed? */
	if (Tcl_GetErrno() == ECONNRESET) {
		/*
		 * Turn ECONNRESET into a soft EOF condition.
		 */
		return 0;
	}
#endif

	return n;
}


static void
Tulsa_SocketWatchProc (ClientData instanceData, int mask) {
	Tulsa *t = (Tulsa *) instanceData;

	if (mask == 0) {
		Tcl_DeleteFileHandler(t->fd);
		return;
	}

	Tcl_CreateFileHandler(t->fd, mask, (Tcl_FileProc *) Tcl_NotifyChannel, (ClientData) t->ch);
}


static int
Tulsa_SocketGetHandleProc (ClientData instanceData, int direction, ClientData *handlePtr) {
	*handlePtr = (ClientData) INT2PTR(((Tulsa *) instanceData)->fd);
	return TCL_OK;
}


static int
Tulsa_SocketBlockModeProc (ClientData instanceData, int mode) {
	Tulsa *t = (Tulsa *) instanceData;
	int flags;

	flags = fcntl(t->fd, F_GETFL);

	if (mode == TCL_MODE_BLOCKING) {
		flags &= ~O_NONBLOCK;
	} else {
		flags |= O_NONBLOCK;
	}

	if (fcntl(t->fd, F_SETFL, flags) == -1) {
		return Tcl_GetErrno();
	}

	return 0;
}


static int
Tulsa_SocketSetOptionProc (ClientData instanceData, Tcl_Interp *interp, const char *optionName, const char *value) {
	Tulsa *t = (Tulsa *) instanceData;
	int optionInt;
	const char *optName;

	optName = "-closeonexec"; if (Tcl_StringMatch(optionName, optName)) {
		if (Tcl_GetBoolean(interp, value, &optionInt) != TCL_OK) {
			return TCL_ERROR;
		}
		if (fcntl(t->fd, F_SETFD, optionInt ? FD_CLOEXEC : 0) == -1) {
			return rperr("can't set closeonexec: ");
		}
		return TCL_OK;
	}

	if (t->paths != NULL) {
		return Tcl_BadChannelOption(interp, optionName, "closeonexec");
	}

	optName = "-receivetimeout"; if (Tcl_StringMatch(optionName, optName)) {
		struct timeval tv;
		socklen_t socklen = sizeof tv;
		if (Ezt_StrToTimeval(interp, value, &tv) != TCL_OK) {
			return TCL_ERROR;
		}
		if (setsockopt(t->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, socklen) != 0) {
			return rperr("can't set receive timeout: ");
		}
		return TCL_OK;
	}

	optName = "-sendtimeout"; if (Tcl_StringMatch(optionName, optName)) {
		struct timeval tv;
		socklen_t socklen = sizeof tv;
		if (Ezt_StrToTimeval(interp, value, &tv) != TCL_OK) {
			return TCL_ERROR;
		}
		if (setsockopt(t->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, socklen) != 0) {
			return rperr("can't set send timeout: ");
		}
		return TCL_OK;
	}

	optName = "-sysreceivebuffersize"; if (Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (Tcl_GetInt(interp, value, &optionInt) != TCL_OK) {
			return TCL_ERROR;
		}
		if (setsockopt(t->fd, SOL_SOCKET, SO_RCVBUF, &optionInt, socklen) != 0) {
			return rperr("can't set sys receive buffer size: ");
		}
		return TCL_OK;
	}

	optName = "-syssendbuffersize"; if (Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (Tcl_GetInt(interp, value, &optionInt) != TCL_OK) {
			return TCL_ERROR;
		}
		if (setsockopt(t->fd, SOL_SOCKET, SO_SNDBUF, &optionInt, socklen) != 0) {
			return rperr("can't set sys send buffer size: ");
		}
		return TCL_OK;
	}

#ifdef TULSA_WET
	optName = "-receivelowatermark"; if (Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (Tcl_GetInt(interp, value, &optionInt) != TCL_OK) {
			return TCL_ERROR;
		}
		if (setsockopt(t->fd, SOL_SOCKET, SO_RCVLOWAT, &optionInt, socklen) != 0) {
			return rperr("can't set receive low water mark: ");
		}
		return TCL_OK;
	}

	optName = "-sendlowatermark"; if (Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (Tcl_GetInt(interp, value, &optionInt) != TCL_OK) {
			return TCL_ERROR;
		}
		if (setsockopt(t->fd, SOL_SOCKET, SO_SNDLOWAT, &optionInt, socklen) != 0) {
			return rperr("can't set send low water mark: ");
		}
		return TCL_OK;
	}
#endif

	return Tcl_BadChannelOption(interp, optionName,
		"closeonexec receivetimeout sendtimeout sysreceivebuffersize syssendbuffersize"
#ifdef TULSA_WET
		" receivelowatermark sendlowatermark"
#endif
	);
}


static int
Tulsa_SocketGetOptionProc (ClientData instanceData, Tcl_Interp *interp, const char *optionName, Tcl_DString *dsPtr) {
	Tulsa *t = (Tulsa *) instanceData;
	size_t len;
	const char *optName;
	int optionInt;
	char optionVal[TCL_INTEGER_SPACE];

	len = optionName == NULL ? 0 : strlen(optionName);

	optName = "-error"; if (len > 0 && Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (getsockopt(t->fd, SOL_SOCKET, SO_ERROR, (void *) &optionInt, &socklen) == -1) {
			optionInt = Tcl_GetErrno();
		}
		if (optionInt != 0) {
			Tcl_DStringAppend(dsPtr, Tcl_ErrnoMsg(optionInt), -1);
		}
		return TCL_OK;
	}

	optName = "-closeonexec"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		optionInt = 0;
		optionInt = fcntl(t->fd, F_GETFD, FD_CLOEXEC);
		optionInt &= FD_CLOEXEC;
		snprintf(optionVal, TCL_INTEGER_SPACE, "%d", optionInt);
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		Tcl_DStringAppendElement(dsPtr, optionVal);
		if (len > 0) { return TCL_OK; }
	}

	if (t->paths != NULL) {
		Tcl_Obj *pathetic;

		optName = "-connectto"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
			Tcl_ListObjIndex(NULL, t->paths, 1, &pathetic);
			if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
			Tcl_DStringAppendElement(dsPtr, Tcl_GetString(pathetic));
			if (len > 0) { return TCL_OK; }
		}

		optName = "-openedon"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
			Tcl_ListObjIndex(NULL, t->paths, 0, &pathetic);
			if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
			Tcl_DStringAppendElement(dsPtr, Tcl_GetString(pathetic));
			if (len > 0) { return TCL_OK; }
		}

		if (len > 0) {
			return Tcl_BadChannelOption(interp, optionName, "error connectto openedon closeonexec");
		}

		return TCL_OK;
	}

	optName = "-peerinfo"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		uid_t euid;
		gid_t egid;
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		if (len == 0) { Tcl_DStringStartSublist(dsPtr); }
		if (getpeereid(t->fd, &euid, &egid) == 0) {
			Tcl_DStringAppendElement(dsPtr, "euid");
			snprintf(optionVal, TCL_INTEGER_SPACE, "%u", euid);
			Tcl_DStringAppendElement(dsPtr, optionVal);
			Tcl_DStringAppendElement(dsPtr, "egid");
			snprintf(optionVal, TCL_INTEGER_SPACE, "%u", egid);
			Tcl_DStringAppendElement(dsPtr, optionVal);
		} else if (len > 0) {
			return rperr("can't get peerinfo: ");
		} else {
			Tcl_DStringAppendElement(dsPtr, "euid");
			Tcl_DStringAppendElement(dsPtr, "-1");
			Tcl_DStringAppendElement(dsPtr, "egid");
			Tcl_DStringAppendElement(dsPtr, "-1");
		}
		if (len == 0) { Tcl_DStringEndSublist(dsPtr); }
		if (len > 0) { return TCL_OK; }
	}

	optName = "-receivetimeout"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		Tcl_Obj *o;
		struct timeval tv;
		socklen_t socklen = sizeof tv;
		if (getsockopt(t->fd, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, &socklen) != 0) {
			return rperr("can't get receive timeout: ");
		}
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		o = Ezt_TimevalToDoubleObj(&tv);
		Tcl_IncrRefCount(o);
		Tcl_DStringAppendElement(dsPtr, Tcl_GetString(o));
		Tcl_DecrRefCount(o);
		if (len > 0) { return TCL_OK; }
	}

	optName = "-sendtimeout"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		Tcl_Obj *o;
		struct timeval tv;
		socklen_t socklen = sizeof tv;
		if (getsockopt(t->fd, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, &socklen) != 0) {
			return rperr("can't get send timeout: ");
		}
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		o = Ezt_TimevalToDoubleObj(&tv);
		Tcl_IncrRefCount(o);
		Tcl_DStringAppendElement(dsPtr, Tcl_GetString(o));
		Tcl_DecrRefCount(o);
		if (len > 0) { return TCL_OK; }
	}

	optName = "-sysreceivebuffersize"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (getsockopt(t->fd, SOL_SOCKET, SO_RCVBUF, &optionInt, &socklen) != 0) {
			return rperr("can't get sys receive buffer size: ");
		}
		snprintf(optionVal, TCL_INTEGER_SPACE, "%d", optionInt);
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		Tcl_DStringAppendElement(dsPtr, optionVal);
		if (len > 0) { return TCL_OK; }
	}

	optName = "-syssendbuffersize"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (getsockopt(t->fd, SOL_SOCKET, SO_SNDBUF, &optionInt, &socklen) != 0) {
			return rperr("can't get sys send buffer size: ");
		}
		snprintf(optionVal, TCL_INTEGER_SPACE, "%d", optionInt);
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		Tcl_DStringAppendElement(dsPtr, optionVal);
		if (len > 0) { return TCL_OK; }
	}

#ifdef TULSA_WET
	optName = "-receivelowatermark"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (getsockopt(t->fd, SOL_SOCKET, SO_RCVLOWAT, &optionInt, &socklen) != 0) {
			return rperr("can't get receive low water mark: ");
		}
		snprintf(optionVal, TCL_INTEGER_SPACE, "%d", optionInt);
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		Tcl_DStringAppendElement(dsPtr, optionVal);
		if (len > 0) { return TCL_OK; }
	}

	optName = "-sendlowatermark"; if (len == 0 || Tcl_StringMatch(optionName, optName)) {
		socklen_t socklen = sizeof optionInt;
		if (getsockopt(t->fd, SOL_SOCKET, SO_SNDLOWAT, &optionInt, &socklen) != 0) {
			return rperr("can't get send low water mark: ");
		}
		snprintf(optionVal, TCL_INTEGER_SPACE, "%d", optionInt);
		if (len == 0) { Tcl_DStringAppendElement(dsPtr, optName); }
		Tcl_DStringAppendElement(dsPtr, optionVal);
		if (len > 0) { return TCL_OK; }
	}
#endif

	if (len > 0) {
		return Tcl_BadChannelOption(interp, optionName,
			"error peerinfo closeonexec receivetimeout sendtimeout sysreceivebuffersize syssendbuffersize"
#ifdef TULSA_WET
			" receivelowatermark sendlowatermark"
#endif
		);
	}

	return TCL_OK;
}


static Tulsa *
Tulsa_NewTulsa (void) {
	Tulsa *t;
	Tcl_Obj *o;

	o = Tcl_NewByteArrayObj(NULL, 0);
	Tcl_IncrRefCount(o);
	t = (Tulsa *) Tcl_SetByteArrayLength(o, sizeof *t);

	t->me     = o;
	t->ch     = NULL;
	t->fd     = -1;
	t->paths  = NULL;

	return t;
}


static void
Tulsa_DeleteTulsa (Tulsa *t) {
	Tcl_Obj *me;
	assert(t);
	assert(t->me);
	if (t->paths != NULL) {
		Tcl_DecrRefCount(t->paths);
		t->paths = NULL;
	}
	me = t->me;
	t->me = NULL;
	Tcl_DecrRefCount(me);
	me = NULL; /* Doubleplus paranoid. */
}


static Tcl_Channel
Tulsa_CreateTclChannelMode (Tulsa *t, int mode) {
	Tcl_Obj *o;
	o = Tcl_ObjPrintf("tulsa%d", t->fd);
	Tcl_IncrRefCount(o);
	t->ch = Tcl_CreateChannel(&TulsaChannelType, Tcl_GetString(o), (ClientData) t, mode);
	Tcl_DecrRefCount(o);
	return t->ch;
}


static Tcl_Channel
Tulsa_CreateTclChannel (Tulsa *t) {
	return Tulsa_CreateTclChannelMode(t, TCL_READABLE | TCL_WRITABLE);
}


/*
 * Put an OS handle in a new Tulsa.
 */
static Tulsa *
Tulsa_Tulsify (int fd) {
	Tulsa *t;

	t = Tulsa_NewTulsa();

	t->fd = fd;

	return t;
}


static int
Tulsa_SetupSockaddr (struct sockaddr_un *sap, Tcl_Obj *path) {
	Tcl_DString ds;
	size_t size;

	memset((void *) sap, 0, sizeof *sap);

	sap->sun_family = AF_UNIX;

	Tcl_DStringInit(&ds);
	Tcl_UtfToExternalDString(NULL, Tcl_GetString(path), -1, &ds);

	/* Even if negative (impossible?) and is huge after conversion to unsigned the test below will still catch it. */
	size = Tcl_DStringLength(&ds);
	if (size > sizeof sap->sun_path) {
		Tcl_DStringFree(&ds);
		Tcl_SetErrno(ENAMETOOLONG);
		return -1;
	}

	memcpy((void *) sap->sun_path, (const void *) Tcl_DStringValue(&ds), size);

	Tcl_DStringFree(&ds);

	return (sizeof *sap - sizeof sap->sun_path + size);
}


/*
 * Setup sockaddr and connect (client) or bind/listen (server).
 * If ok, wrap in new Tulsa struct.
 */
static Tulsa *
Tulsa_OpenSocket (Tcl_Interp *interp, Tcl_Obj *path, int server) {
	int sock = -1;
	struct sockaddr_un sa;
	socklen_t socklen;

	if ((socklen = Tulsa_SetupSockaddr(&sa, path)) ==  0) goto sockProb;
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0))   == -1) goto sockProb;
	if (fcntl(sock, F_SETFD, FD_CLOEXEC)           == -1) goto sockProb;

	if (server) {
		if (bind    (sock, (struct sockaddr *) &sa, socklen) == -1) goto sockProb;
		if (listen  (sock, SOMAXCONN)                        == -1) goto sockProb;
	} else {
		if (connect (sock, (struct sockaddr *) &sa, socklen) == -1) goto sockProb;
	}

	return Tulsa_Tulsify(sock);

sockProb:
	if (sock != -1) { close(sock); }
	rperr("couldn't open socket: ");
	return NULL;
}


TCMD(Tulsa_Tulsa_open_Cmd) {
	Tulsa *t;

	if (objc != 2) {
		return Ezt_WrongNumArgs(interp, 1, objv, "path");
	}

	if (PTR2INT(clientData)) { /* server */
		Tcl_Obj *apaths[2] = { objv[1], NULL };
		if ((apaths[1] = Tcl_FSGetNormalizedPath(interp, objv[1])) == NULL) { return TCL_ERROR; }
		if ((t = Tulsa_OpenSocket(interp, objv[1], 1))             == NULL) { return TCL_ERROR; }
		t->paths = Tcl_NewListObj(2, apaths);
		Tcl_IncrRefCount(t->paths);
	} else {
		if ((t = Tulsa_OpenSocket(interp, objv[1], 0))             == NULL) { return TCL_ERROR; }
	}

	Tcl_RegisterChannel(interp, Tulsa_CreateTclChannel(t));

	Tcl_SetObjResult(interp, Tcl_NewStringObj(Tcl_GetChannelName(t->ch), -1));

	return TCL_OK;
}


TCMD(Tulsa_Tulsa_pair_Cmd) {
	int sv[2];
	Tulsa *t;
	Tcl_Obj *l;
	int i;

	if (objc != 1) {
		return Ezt_WrongNumArgs(interp, 1, objv, NULL);
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
		return rperr("couldn't create pair: ");
	}

	fcntl(sv[0], F_SETFD, FD_CLOEXEC);
	fcntl(sv[1], F_SETFD, FD_CLOEXEC);

	l = Tcl_NewListObj(0, NULL);
	Tcl_IncrRefCount(l);

	for (i = 0; i < 2; i++) {
		t = Tulsa_Tulsify(sv[i]);
		Tcl_RegisterChannel(interp, Tulsa_CreateTclChannel(t));
		Tcl_ListObjAppendElement(NULL, l, Tcl_NewStringObj(Tcl_GetChannelName(t->ch), -1));
	}

	Tcl_SetObjResult(interp, l);
	Tcl_DecrRefCount(l);

	return TCL_OK;
}


#ifdef BEM
#  error "BEM already #define'd!"
#endif
#define BEM "couldn't accept connection: "

TCMD(Tulsa_Tulsa_accept_Cmd) {
	Tulsa *t;
	Tulsa *ct;
	int sock;
	struct sockaddr_un sa;
	socklen_t socklen = sizeof sa;
	Tcl_Channel ch;
	const Tcl_ChannelType *cht;
	int mode;

	if (objc != 2) {
		return Ezt_WrongNumArgs(interp, 1, objv, "tulsa");
	}
	if ((ch = Tcl_GetChannel(interp, Tcl_GetString(objv[1]), &mode)) == NULL) {
		return TCL_ERROR;
	}
	if ((cht = Tcl_GetChannelType(ch)) == NULL) {
		return rerr(BEM "unable to obtain channel type");
	}
	if (!Tcl_StringMatch(Tcl_ChannelName(cht), "tulsa")) {
		return rerr(BEM "channel is not a Tulsa channel");
	}
	if ((t = (Tulsa *) Tcl_GetChannelInstanceData(ch)) == NULL) {
		return rerr(BEM "no instance data");
	}
	if (t->paths == NULL) {
		return rerr(BEM "Tulsa channel is not a server");
	}
	if ((sock = accept(t->fd, (struct sockaddr *) &sa, &socklen)) == -1) {
		if (Tcl_GetErrno() == EAGAIN || Tcl_GetErrno() == EWOULDBLOCK) {
			return TCL_OK;
		}
		return rperr(BEM);
	}

	fcntl(sock, F_SETFD, FD_CLOEXEC);

	ct = Tulsa_Tulsify(sock);

	Tcl_RegisterChannel(interp, Tulsa_CreateTclChannel(ct));

	Tcl_SetObjResult(interp, Tcl_NewStringObj(Tcl_GetChannelName(ct->ch), -1));

	return TCL_OK;
}

#undef BEM


TCMD(Tulsa_Tulsa_srdopts_Cmd) {
	if (objc != 1) { return Ezt_WrongNumArgs(interp, 1, objv, NULL); }
	rerr("-error -connectto -openedon -closeonexec");
	return TCL_OK;
}

TCMD(Tulsa_Tulsa_swropts_Cmd) {
	if (objc != 1) { return Ezt_WrongNumArgs(interp, 1, objv, NULL); }
	rerr("-closeonexec");
	return TCL_OK;
}

TCMD(Tulsa_Tulsa_crdopts_Cmd) {
	if (objc != 1) { return Ezt_WrongNumArgs(interp, 1, objv, NULL); }
	rerr("-error -peerinfo -closeonexec -receivetimeout -sendtimeout -sysreceivebuffersize -syssendbuffersize"
#ifdef TULSA_WET
	     " -receivelowatermark -sendlowatermark"
#endif
	);
	return TCL_OK;
}

TCMD(Tulsa_Tulsa_cwropts_Cmd) {
	if (objc != 1) { return Ezt_WrongNumArgs(interp, 1, objv, NULL); }
	rerr("-closeonexec -receivetimeout -sendtimeout -sysreceivebuffersize -syssendbuffersize"
#ifdef TULSA_WET
	     " -receivelowatermark -sendlowatermark"
#endif
	);
	return TCL_OK;
}


TCMD(Tulsa_Tulsa_LICENSE_Cmd) {
	Tcl_Obj *o;
	if (objc != 1) { return Ezt_WrongNumArgs(interp, 1, objv, NULL); }
	o = Tcl_NewDictObj();
	Tcl_IncrRefCount(o);
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("type", -1), Tcl_NewStringObj("ISC", -1));
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("text", -1), Tcl_NewStringObj(
		"Copyright (c) 2014,2018 Stuart Cassoff <stwo@users.sourceforge.net>\n"
		"\n"
		"Permission to use, copy, modify, and distribute this software for any\n"
		"purpose with or without fee is hereby granted, provided that the above\n"
		"copyright notice and this permission notice appear in all copies.\n"
		"\n"
		"THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES\n"
		"WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF\n"
		"MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR\n"
		"ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES\n"
		"WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN\n"
		"ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF\n"
		"OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE."
	, -1));
	Tcl_SetObjResult(interp, o);
	Tcl_DecrRefCount(o);
	return TCL_OK;
}

TCMD(Tulsa_Tulsa_ABOUT_Cmd) {
	Tcl_Obj *o;
	if (objc != 1) { return Ezt_WrongNumArgs(interp, 1, objv, NULL); }
	o = Tcl_NewDictObj();
	Tcl_IncrRefCount(o);
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("name",    -1), Tcl_NewStringObj("tulsa", -1));
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("title",   -1), Tcl_NewStringObj("Tulsa", -1));
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("descr",   -1), Tcl_NewStringObj("Tcl Unix Local Sockets, Eh?", -1));
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("author",  -1), Tcl_NewStringObj("Stuart Cassoff", -1));
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("when",    -1), Tcl_NewStringObj("Spring 2018", -1));
	Tcl_DictObjPut(NULL, o, Tcl_NewStringObj("version", -1), Tcl_NewStringObj(PACKAGE_VERSION, -1));
	Tcl_SetObjResult(interp, o);
	Tcl_DecrRefCount(o);
	return TCL_OK;
}


int
Tulsa_PackageInit (Tcl_Interp *interp, const char *nsn) {
	Tcl_Obj *o;
	o = Tcl_ObjPrintf("%s::%s", nsn, "LICENSE");
	Tcl_IncrRefCount(o);
	if (Tcl_CreateObjCommand(interp, Tcl_GetString(o), Tulsa_Tulsa_LICENSE_Cmd, NULL, NULL) == NULL) { goto bummer; }
	Tcl_DecrRefCount(o);
	o = Tcl_ObjPrintf("%s::%s", nsn, "ABOUT");
	Tcl_IncrRefCount(o);
	if (Tcl_CreateObjCommand(interp, Tcl_GetString(o), Tulsa_Tulsa_ABOUT_Cmd, NULL, NULL) == NULL) { goto bummer; }
	Tcl_DecrRefCount(o);
	return TCL_OK;
bummer:
	Tcl_DecrRefCount(o);
	return TCL_ERROR;
}


#include "ezt_funcs.c"

#include "ezt_init.c"


#ifdef __cplusplus
}
#endif


/* EOF */
