/*** Ezt Functions:begin ***/
#ifndef EZT_NO_REPORTERROR
static int
Ezt_ReportError (Tcl_Interp *interp, ...) {
	va_list argList;
	Tcl_Obj *result;
	if (interp == NULL) { return TCL_ERROR; }
	result = Tcl_NewObj();
	Tcl_IncrRefCount(result);
	va_start(argList, interp);
	Tcl_AppendStringsToObjVA(result, argList);
	va_end(argList);
	Tcl_SetObjResult(interp, result);
	Tcl_DecrRefCount(result);
	return TCL_ERROR;
}
#endif

#ifndef EZT_NO_WRONGNUMARGS
static int
Ezt_WrongNumArgs (Tcl_Interp *interp, int objc, Tcl_Obj *const objv[], const char *message) {
	Tcl_WrongNumArgs(interp, objc, objv, message);
	return TCL_ERROR;
}
#endif


#ifdef EZT_TIMEVAL2DBL
static Tcl_Obj *
Ezt_TimevalToDoubleObj (struct timeval *tv) {
	return Tcl_NewDoubleObj(tv->tv_sec + tv->tv_usec / 1000000.0);
}
#endif

#ifdef EZT_STR2TIMEVAL
static int
Ezt_StrToTimeval (Tcl_Interp *interp, const char *str, struct timeval *tv) {
	Tcl_Obj *o;
	int res;
	o = Tcl_NewStringObj(str, -1);
	Tcl_IncrRefCount(o);
	res = Ezt_DoubleObjToTimeval(interp, o, tv);
	Tcl_DecrRefCount(o);
	return res;
}
#endif

#ifdef EZT_DBL2TIMEVAL
static int
Ezt_DoubleObjToTimeval (Tcl_Interp *interp, Tcl_Obj *obj, struct timeval *tv) {
	double d;

	if (Tcl_GetDoubleFromObj(interp, obj, &d) != TCL_OK) { return TCL_ERROR; }

	tv->tv_sec = (time_t) d;	/* truncate to get secs */
	d -= tv->tv_sec;		/* remove secs */
	d *= 1000000;			/* scale up */
	tv->tv_usec = (suseconds_t) d;	/* cast/truncate */

	return TCL_OK;
}
#endif


#ifdef EZT_ASSOCDATA_KEY

static void
Ezt_GoodbyeCruelInterp (ClientData clientData, Tcl_Interp *interp) {
	Tcl_Obj *o;
	EztJunk *j;

	o = (Tcl_Obj *) clientData;
	j = (EztJunk *) Tcl_GetByteArrayFromObj(o, NULL);

#ifdef EZT_BYE
	EZT_BYE(j->pile);
#endif

	Tcl_DecrRefCount(j->pile);
	Tcl_DecrRefCount(o);

	/* Prevent being called recursively. Happens when called from Unload. */
	Tcl_SetAssocData(interp, EZT_ASSOCDATA_KEY, NULL, (ClientData) o);
	Tcl_DeleteAssocData(interp, EZT_ASSOCDATA_KEY);
}

static EztJunk *
Ezt_GetJunk (Tcl_Interp *interp) {
	Tcl_Obj *o;
	if ((o = (Tcl_Obj *) Tcl_GetAssocData(interp, EZT_ASSOCDATA_KEY, NULL)) == NULL) {
		EztJunk j;
		j.counter = 0;
		j.pile = Tcl_NewDictObj();
		Tcl_IncrRefCount(j.pile);
		o = Tcl_NewByteArrayObj((const unsigned char *) &j, sizeof(j));
		Tcl_IncrRefCount(o);
		Tcl_SetAssocData(interp, EZT_ASSOCDATA_KEY, Ezt_GoodbyeCruelInterp, (ClientData) o);
	}
	return (EztJunk *) Tcl_GetByteArrayFromObj(o, NULL);
}

static int
Ezt_PutIt (Tcl_Interp *interp, Tcl_Obj *what, Tcl_Obj *val) {
	EztJunk *j;
	j = Ezt_GetJunk(interp);
	Tcl_DictObjPut(NULL, j->pile, what, val);
	return 1;
}

static void
Ezt_RemoveIt (Tcl_Interp *interp, Tcl_Obj *what) {
	EztJunk *j;
	j = Ezt_GetJunk(interp);
	Tcl_DictObjRemove(NULL, j->pile, what);
}

static Tcl_Obj *
Ezt_GetIt (Tcl_Interp *interp, Tcl_Obj *what) {
	EztJunk *j;
	Tcl_Obj *o;
	j = Ezt_GetJunk(interp);
	Tcl_DictObjGet(NULL, j->pile, what, &o);
	return o;
}

static unsigned long
Ezt_NextCounter (Tcl_Interp *interp) {
	EztJunk *j;
	j = Ezt_GetJunk(interp);
	return j->counter++;
}

# ifdef EZT_FUNC_JUNKPILEASLIST
/*
 * The returned object has a refcount of 1.
 */
static Tcl_Obj *
Ezt_JunkPileAsList (Tcl_Interp *interp) {
	EztJunk *j;
	Tcl_Obj *o;
	Tcl_DictSearch s;
	Tcl_Obj *k = NULL;
	int d = 0;

	j = Ezt_GetJunk(interp);
	o = Tcl_NewListObj(0, NULL);
	Tcl_IncrRefCount(o);
	Tcl_DictObjFirst(NULL, j->pile, &s, &k, NULL, &d);
	while (!d) {
		Tcl_ListObjAppendElement(NULL, o, k);
		Tcl_DictObjNext(&s, &k, NULL, &d);
        }
	Tcl_DictObjDone(&s);
	return o;
}
# endif /* EZT_FUNC_JUNKPILEASLIST */

#endif /* EZT_ASSOCDATA_KEY */
/*** Ezt Functions:end ***/
