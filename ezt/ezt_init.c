/*** Ezt Init:begin ***/


/* #define the following:

EZT_NS		Main namespace where lives the extension. Example: "::myns"
EZT_EN		Name of ensemble command that lives in EZT_NS. Example: "mycmd"
EZT_CMD		Name of ensemble subcommand. Example: "mysubcmd"

Optional:
EZT_ENCMD	Name of ensemble subcommand if ensemble subcommand is itself an ensemble. Example: "mysubcommand"

Init procs:
EZT_INIT	Extension's init proc. Example: Myext_Init
EZT_SAFE_INIT	Extension's safe init proc. Example: Myext_SafeInit

*/


static int
Ezt_PackageInit (Tcl_Interp *interp) {
	Tcl_Namespace *ns;
#ifndef EZT_CMDPROC
	ClientData clientData = NULL;
	const Ezt_Cmd *cmd;
	Tcl_Obj *nameObj;
#endif

	/* Main namespace */
	if ((ns = Tcl_FindNamespace(interp, EZT_NS, NULL, 0)) == NULL) {
		if ((ns = Tcl_CreateNamespace(interp, EZT_NS, NULL, NULL)) == NULL) { return TCL_ERROR; }
	}

/*
	if ((ns = Tcl_FindNamespace (interp, EZT_NS, NULL, TCL_LEAVE_ERR_MSG)) == NULL) { return TCL_ERROR; }
	if ((ns = Tcl_CreateNamespace (interp, EZT_NS, NULL, NULL)) == NULL) { return TCL_ERROR; }
*/
	if (Tcl_Export (interp, ns, EZT_EN, 0) != TCL_OK) { return TCL_ERROR; }

	/* Ensemble or command namespace */
#if 0
	if ((ns = Tcl_CreateNamespace (interp, EZT_NS "::" EZT_EN, NULL, NULL)) == NULL) { return TCL_ERROR; }
#else
	if ((ns = Tcl_FindNamespace (interp, EZT_NS "::" EZT_EN, NULL, 0)) == NULL) {
		if ((ns = Tcl_CreateNamespace (interp, EZT_NS "::" EZT_EN, NULL, NULL)) == NULL) { return TCL_ERROR; }
	}
#endif

#ifdef EZT_CMDPROC
	/* Only a command that lives in the ensemble */
	if (Tcl_CreateObjCommand(interp, EZT_NS "::" EZT_EN "::" EZT_CMD, EZT_CMDPROC, NULL, NULL) == NULL) { return TCL_ERROR; }
#else

	/* Main ensemble subcommand is itself an ensemble */
#  ifdef EZT_ENCMD
#    define EZT_CMDFMT EZT_NS "::" EZT_EN "::" EZT_ENCMD
	if ((ns = Tcl_CreateNamespace (interp, EZT_CMDFMT, NULL, NULL)) == NULL) { return TCL_ERROR; }
#  else
#    define EZT_CMDFMT EZT_NS "::" EZT_EN
#  endif

	/* All subcommands for ensemble */
	for (cmd = &Ezt_Cmds[0]; cmd->name != NULL; cmd++) {
#  ifdef EZT_CMD_CLIENTDATA
		clientData = cmd->clientData;
#  endif
		nameObj = Tcl_ObjPrintf(EZT_CMDFMT "::" "%s", cmd->name);
		Tcl_IncrRefCount(nameObj);
		if (Tcl_CreateObjCommand(interp, Tcl_GetString(nameObj), cmd->proc, clientData, NULL) == NULL) { Tcl_DecrRefCount(nameObj); return TCL_ERROR; }
		Tcl_DecrRefCount(nameObj);
		if (Tcl_Export(interp, ns, cmd->name, 0) != TCL_OK) { return TCL_ERROR; }
	}

#  ifdef EZT_ENCMD
	if (Tcl_CreateEnsemble (interp, EZT_CMDFMT, ns, 0) == NULL) { return TCL_ERROR; }
#  endif
#endif

	if ((ns = Tcl_FindNamespace (interp, EZT_NS "::" EZT_EN, NULL, TCL_LEAVE_ERR_MSG)) == NULL) { return TCL_ERROR; }
	if (Tcl_CreateEnsemble (interp, EZT_NS "::" EZT_EN, ns, 0) == NULL) { return TCL_ERROR; }
	if (Tcl_Export (interp, ns, EZT_CMD, 0) != TCL_OK) { return TCL_ERROR; }

#ifdef EZT_PKGINIT
	if (EZT_PKGINIT (interp, EZT_NS) != TCL_OK) { return TCL_ERROR; }
#endif

	return TCL_OK;
}

static int
Ezt_PackageProvide (Tcl_Interp *interp) {
	return Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION);
}

static int
Ezt_CommonInit (Tcl_Interp *interp) {
	if (Tcl_InitStubs       (interp,        EZT_MIN_TCL_VER, 0) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgRequire      (interp, "Tcl", EZT_MIN_TCL_VER, 0) == NULL) { return TCL_ERROR; }
/*
	if (Tcl_FindNamespace   (interp, EZT_NS, NULL, 0)           != NULL) { return TCL_OK;    }
	if (Tcl_CreateNamespace (interp, EZT_NS, NULL, NULL)        == NULL) { return TCL_ERROR; }
*/
	return TCL_OK;
}

static int
Ezt_Init (Tcl_Interp *interp) {
	if (Ezt_CommonInit     (interp) != TCL_OK) { return TCL_ERROR; }
#ifdef EZT_EXT_INIT
	if (EZT_EXT_INIT       (interp) != TCL_OK) { return TCL_ERROR; }
#endif
	if (Ezt_PackageInit    (interp) != TCL_OK) { return TCL_ERROR; }
	if (Ezt_PackageProvide (interp) != TCL_OK) { return TCL_ERROR; }
	return TCL_OK;
}

/***/

EXTERN int
EZT_INIT (Tcl_Interp *interp) {
	return Ezt_Init(interp);
}

EXTERN int
EZT_SAFE_INIT (Tcl_Interp *interp) {
	return EZT_INIT(interp);
}

/*** Ezt Init:end ***/
