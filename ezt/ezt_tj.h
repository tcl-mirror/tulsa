/*** ezt_tj.h:begin ***/
#if !defined(FOR_TCL) && !defined(FOR_JIM)
  #error "FOR_TCL or FOR_JIM must be defined"
#endif
#if defined(FOR_TCL) && defined(FOR_JIM)
  #error "FOR_TCL and FOR_JIM cannot both be defined"
#endif

#ifdef FOR_JIM
#include <jim-subcmd.h>
# define EZT_OK JIM_OK
# define EZT_ERROR JIM_ERR
# define Ezt_Alloc Jim_Alloc
# define Ezt_Free Jim_Free
# define Ezt_PosixError() strerror(errno)
# define Ezt_WrongNumArgs Jim_WrongNumArgs
# define Ezt_IncrRefCount Jim_IncrRefCount
# define Ezt_DecrRefCount(O) Jim_DecrRefCount(interp,(O))
# define Ezt_SetResult(O) Jim_SetResult(interp,(O))
# define Ezt_Obj Jim_Obj
# define Ezt_Interp Jim_Interp
# if 0
#  define Ezt_NewInt(I) Ezt_NewIntObj(interp,(I))
# else
#  define Ezt_NewInt(I) Jim_NewWideObj(interp,(I))
# endif
# ifndef EZT_NO_GETINT
#  define Ezt_GetInt(I,P) Ezt_GetIntFromJimObj(interp,(I),(P))
# endif
# define Ezt_NewLong Ezt_NewInt
# define Ezt_GetLong Ezt_GetInt
# define Ezt_NewWide(I) Jim_NewWideObj(interp,(I))
# define Ezt_GetWide Jim_GetWide
# define Ezt_NewString(S,L) Jim_NewStringObj(interp,(S),(L))
# define Ezt_GetString Jim_GetString
# define Ezt_GetStr Jim_String
# define Ezt_GetBytes Jim_GetString
# define Ezt_NewList() Jim_NewListObj(interp, NULL, 0)
# define Ezt_ListAppend(L,O) Jim_ListAppendElement(interp,(L),(O))
# define Ezt_NewDict() Jim_NewDictObj(interp, NULL, 0)
# define Ezt_DictPut(D,KO,VO) Jim_DictAddElement(interp,(D),(KO),(VO))
# define ezt_wide jim_wide
# define EZT_SUBCMD(S) int (S) (Ezt_Interp *interp, int objc, Ezt_Obj *const *objv)
# define EZT_CMD(S) int (S) (Ezt_Interp *interp, int objc, Ezt_Obj *const *objv)
# define ezt_subcmd_type jim_subcmd_type
#else
# include <tcl.h>
# define EZT_OK TCL_OK
# define EZT_ERROR TCL_ERROR
# define Ezt_Alloc ckalloc
# define Ezt_Free ckfree
# define Ezt_PosixError() Tcl_PosixError(interp)
# define Ezt_WrongNumArgs Tcl_WrongNumArgs
# define Ezt_IncrRefCount Tcl_IncrRefCount
# define Ezt_DecrRefCount Tcl_DecrRefCount
# define Ezt_SetResult(O) Tcl_SetObjResult(interp,(O))
# define Ezt_Obj Tcl_Obj
# define Ezt_Interp Tcl_Interp
# define Ezt_NewInt Tcl_NewIntObj
# define Ezt_GetInt(I,P) Tcl_GetIntFromObj(interp,(I),(P))
# define Ezt_NewLong Tcl_NewLongObj
# define Ezt_GetLong(I,P) Tcl_GetLongFromObj(interp,(I),(P))
# define Ezt_NewWide Tcl_NewWideIntObj
# define Ezt_GetWide Tcl_GetWideIntFromObj
# define Ezt_NewString Tcl_NewStringObj
# define Ezt_GetString Tcl_GetStringFromObj
# define Ezt_GetStr(S) Tcl_GetStringFromObj((S),NULL)
# define Ezt_GetBytes Tcl_GetByteArrayFromObj
# define Ezt_NewList() Tcl_NewListObj(0,NULL)
# define Ezt_ListAppend(L,O) Tcl_ListObjAppendElement(interp,(L),(O))
# define Ezt_NewDict() Tcl_NewDictObj()
# define Ezt_DictPut(D,KO,VO) Tcl_DictObjPut(interp,(D),(KO),(VO))
# define ezt_wide TclWideInt
# define EZT_SUBCMD(S) int (S) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
# define EZT_CMD EZT_SUBCMD
typedef struct {
	const char *name;
	const char *args;
	Tcl_ObjCmdProc *proc;
	short minargs;
	short maxargs;
	unsigned short flags;
} ezt_subcmd_type;
#endif /* FOR_JIM / FOR_TCL */

#define Ezt_NewStr(S) Ezt_NewString((S),-1)

#ifdef FOR_JIM
# if 0
static Jim_Obj *
Ezt_NewIntObj (Jim_Interp *interp, int i) {
	return Jim_NewWideObj(interp, (ezt_wide) i);
}
# endif

# ifndef EZT_NO_GETINT
static int
Ezt_GetIntFromJimObj (Jim_Interp *interp, Jim_Obj *objPtr, int *intPtr) {
	jim_wide w;
	if (Jim_GetWide(interp, objPtr, (intPtr != NULL ? &w : NULL)) != JIM_OK) { return JIM_ERR; }
	if (intPtr != NULL) {
		*intPtr = (int) w;
	}
	return JIM_OK;
}
# endif
#endif

#ifdef EZT_ERRORPRINTF
static int
Ezt_ErrorPrintf (Ezt_Interp *interp, const char *format, ...) {
	va_list args;
	char buf[200];
	if (interp == NULL) { return EZT_ERROR; }
	va_start(args, format);
	vsnprintf(buf, sizeof buf, format, args);
	Ezt_SetResult(Ezt_NewStr(buf));
	va_end(args);
	return EZT_ERROR;
}
#define rerrf(fmt,...) Ezt_ErrorPrintf(interp, (fmt), __VA_ARGS__)
#endif
/*** ezt_tj.h:end ***/
