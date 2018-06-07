/*** Ezt.h:begin ***/
#ifndef _EZT_H
#define _EZT_H 1

#ifndef EZT_NO_REPORTERROR
static int Ezt_ReportError (Tcl_Interp *interp, ...);
#define rerr(msg)  Ezt_ReportError(interp, (msg), (char*) NULL)
#define rperr(msg) Ezt_ReportError(interp, (msg), Tcl_PosixError(interp), (char*) NULL)
#endif

typedef struct EztCmd {
	const char *name;
	Tcl_ObjCmdProc *proc;
#ifdef EZT_CMD_CLIENTDATA
	ClientData clientData;
#endif
} Ezt_Cmd;

#ifdef TCMD
#  error "TCMD already #define'd!"
#endif
#define TCMD(S) static int (S) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])


#ifdef EZT_EXT_INIT
	static int EZT_EXT_INIT (Tcl_Interp *interp);
#endif

#ifndef EZT_NO_WRONGNUMARGS
static int Ezt_WrongNumArgs (Tcl_Interp *interp, int objc, Tcl_Obj *const objv[], const char *message);
#endif


#ifdef EZT_TIMEVAL
#  ifndef EZT_FROM_TIMEVAL
#    define EZT_FROM_TIMEVAL 1
#  endif
#  ifndef EZT_TO_TIMEVAL
#    define EZT_TO_TIMEVAL 1
#  endif
#endif
#ifdef EZT_FROM_TIMEVAL
static Tcl_Obj * Ezt_TimevalToDoubleObj (struct timeval *tv);
#endif
#ifdef EZT_TO_TIMEVAL
static int Ezt_StrToTimeval (Tcl_Interp *interp, const char *str, struct timeval *tv);
static int Ezt_DoubleObjToTimeval (Tcl_Interp *interp, Tcl_Obj *obj, struct timeval *tv);
#endif


#ifdef EZT_ASSOCDATA_KEY

typedef struct EztJunk {
	Tcl_Obj *pile;
	unsigned long counter;
} EztJunk;

static EztJunk * Ezt_GetJunk (Tcl_Interp *interp);

static int Ezt_PutIt (Tcl_Interp *interp, Tcl_Obj *what, Tcl_Obj *val);
static void Ezt_RemoveIt (Tcl_Interp *interp, Tcl_Obj *what);
static Tcl_Obj *Ezt_GetIt (Tcl_Interp *interp, Tcl_Obj *what);
static unsigned long Ezt_NextCounter (Tcl_Interp *interp);

#ifdef EZT_FUNC_JUNKPILEASLIST
static Tcl_Obj * Ezt_JunkPileAsList (Tcl_Interp *interp);
#endif

#endif /* EZT_ASSOCDATA_KEY */

#endif /* _EZT_H */
/*** Ezt.h:end ***/
