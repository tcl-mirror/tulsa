/*** Ezt.h:begin ***/
#ifndef _EZT_H
#define _EZT_H 1

#define _EZT_VER 1.0


/* Always */

#ifndef EZT_MIN_TCL_VER
# define EZT_MIN_TCL_VER "8.6"
#endif

#define COMBIEN(Z) ((int) (sizeof(Z) / sizeof(Z[0])))

#define TCMD(S) static int (S) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])

typedef struct EztCmd {
	const char *name;
	Tcl_ObjCmdProc *proc;
#ifdef EZT_CMD_CLIENTDATA
	ClientData clientData;
#endif
} Ezt_Cmd;


/* On by default */

#ifndef EZT_NO_REPORTERROR
static int Ezt_ReportError (Tcl_Interp *interp, ...);
#define rerr(msg)  Ezt_ReportError(interp, (msg), (char*) NULL)
#define rperr(msg) Ezt_ReportError(interp, (msg), Tcl_PosixError(interp), (char*) NULL)
#define NAGFO(str) Ezt_ReportError(interp, "no argument given for ", (str), " option", (char*) NULL)
#endif

#ifndef EZT_NO_WRONGNUMARGS
static int Ezt_WrongNumArgs (Tcl_Interp *interp, int objc, Tcl_Obj *const objv[], const char *message);
#endif


/* Off by default */

#ifdef EZT_EXT_INIT
static int EZT_EXT_INIT (Tcl_Interp *interp);
#endif

#ifdef EZT_TIMEVAL2DBL
static Tcl_Obj * Ezt_TimevalToDoubleObj (struct timeval *tv);
#endif

#ifdef EZT_STR2TIMEVAL
static int Ezt_StrToTimeval (Tcl_Interp *interp, const char *str, struct timeval *tv); /* Needs dbl2timeval */
# ifndef EZT_DBL2TIMEVAL
#  define EZT_DBL2TIMEVAL 1
# endif
#endif

#ifdef EZT_DBL2TIMEVAL
static int Ezt_DoubleObjToTimeval (Tcl_Interp *interp, Tcl_Obj *obj, struct timeval *tv);
#endif


/* Junkpile */

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

# ifdef EZT_FUNC_JUNKPILEASLIST
static Tcl_Obj * Ezt_JunkPileAsList (Tcl_Interp *interp);
# endif

#endif /* EZT_ASSOCDATA_KEY */

#endif /* _EZT_H */
/*** Ezt.h:end ***/
