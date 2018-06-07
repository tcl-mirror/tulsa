#
# Tcl or Jim extension
#

NAME =		tulsa
PKG_NAME =	tulsa
VER =		1.0
INIT =		
FOR_WHAT =	Tcl

prefix =	/usr/local
srcdir =	..
appdir =	tulsa
pkgdir =	${prefix}/lib/tcl/${appdir}
moddir =	
mandir =	${prefix}/man/mann
docdir =	${prefix}/share/doc/${appdir}
exadir =	${prefix}/share/examples/${appdir}
bindir =	${prefix}/bin

VPATH =		${srcdir}/generic:${srcdir}/unix:${srcdir}/library:${srcdir}/doc:${srcdir}/bin:$(srcdir)/ezt:${srcdir}/doc

CC =		cc
CFLAGS_CC =	-pipe
CFLAGS_WARN =	-Wall
CFLAGS_OPT =	-O2
CFLAGS_SH =	-fpic
CFLAGS_TH = 	-pthread
LIBS_TH = 	
TH_DEFS =	-DTCL_THREADS=1 -DUSE_THREAD_ALLOC=1
OPT_DEFS =	-DPACKAGE_NAME=\"tulsa\" -DPACKAGE_VERSION=\"1.0\"
LDFLAGS =	
LDFLAGS_SH =	-shared
TDEFS =		-DUSE_TCL_STUBS=1
JDEFS =		
DEFS =		$(TDEFS)
SPOTO_DEFS =	-DFOR_TCL=1 -DFOR_WHAT=\"$(FOR_WHAT)\"
EXT_DEFS =	-Wall -Wextra -Wno-unused-parameter -DHAVE_INTPTR_T=1 -DHAVE_UINTPTR_T=1 -DTULSA_WET=1 -DHAVE_GETPEEREID
EXT_INC_SPEC =	-I$(srcdir)/ezt -I$(srcdir)/tcl
EXT_LIB_SPEC =	
TCL_INC_SPEC =	-I/usr/local/include/tcl8.6
JIM_INC_SPEC =	-I/usr/local/include/jim
TCL_LIB_SPEC =	-L/usr/local/lib -ltcl86
JIM_LIB_SPEC =	-L/usr/local/lib -ljim
TCL_STUB_LIB_SPEC = -L/usr/local/lib -ltclstub86

CC_INC_SPEC =	$(TCL_INC_SPEC)
LD_LIB_SPEC =	$(TCL_STUB_LIB_SPEC)
INC_SPEC =	$(EXT_INC_SPEC) $(CC_INC_SPEC)
LIB_SPEC =	$(EXT_LIB_SPEC) $(LD_LIB_SPEC)

CFLAGS_DEBUG =	-DNDEBUG=1

SRC =		$(NAME).c
OBJ =		$(NAME).o
LIB =		libtulsa.so
SHLIB =		libtulsa.so
SCLIB =		
SCLIBSRC =	
DOC =		tulsa.n
DOCSRC =	tulsa.man
DOCBLD =	dtplite
DOCFMT =	nroff
SCBIN =		
SCBINSRC =	
PKGINDEX =	pkgIndex.tcl

EXT_DEP_FILES =	ezt.h ezt_funcs.c ezt_init.c

BJE		= /usr/local/share/examples/jim/build-jim-ext
SHELL		= /bin/sh
TCL_PKG_ENV	= LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" TCLLIBPATH=". $(srcdir)/tcltest" TCL8_5_TM_PATH='.' TCL8_6_TM_PATH='.'
JIM_PKG_ENV	= JIMLIB="."
TCLSH_PROG	= tclsh8.6
TCLSH		= $(TCL_PKG_ENV) $(TCLSH_PROG)
JIMSH_PROG	= jimsh
JIMSH		= $(JIM_PKG_ENV) $(JIMSH_PROG)
SCRIPT		=

TESTENV		= 
TESTFWK		= all.tcl
TESTFLAGS	=

DAT_MODE	 = 644
BIN_MODE	 = 755
DESTDIR		 =
INSTALL_OPTIONS	 =
INSTALL		 = $(SHELL) $(srcdir)/tclconfig/install-sh -c $(INSTALL_OPTIONS)
INSTALL_DATA_DIR = $(INSTALL) -d -m 755
INSTALL_PROGRAM	 = $(INSTALL) -m $(BIN_MODE)
INSTALL_DATA	 = $(INSTALL) -m $(DAT_MODE)
INSTALL_SCRIPT	 = $(INSTALL) -m $(BIN_MODE)
INSTALL_LIBRARY	 = $(INSTALL) -m $(DAT_MODE)

PKG_INSTALL_DIR	 = ${pkgdir}
DOC_INSTALL_DIR	 = ${mandir}

INDENT = "    "

DIST_NAME = $(NAME)-$(VER)
DIST_ROOT = /tmp/dist-$(DIST_NAME)
DIST_DIR  = $(DIST_ROOT)/$(DIST_NAME)
DIST_FILE = $(DIST_NAME).tar.gz
DIST_INSTALL_DATA   = CPPROG='cp -p' $(INSTALL) -m 644
DIST_INSTALL_SCRIPT = CPPROG='cp -p' $(INSTALL) -m 755


all: $(SHLIB) $(SCLIB) $(PKGINDEX) $(DOC) $(SCBIN)

$(OBJ): $(SRC) $(EXT_DEP_FILES)
	$(CC) \
	-c \
	$(CFLAGS_CC) \
	$(CFLAGS_WARN) \
	$(CFLAGS_TH) \
	$(CFLAGS_SH) \
	$(CFLAGS_OPT) \
	$(CFLAGS_DEBUG) \
	$(DEFS) \
	$(TH_DEFS) \
	$(OPT_DEFS) \
	$(EXT_DEFS) \
	$(SPOTO_DEFS) \
	$(INC_SPEC) \
	-o $@ $<

$(SHLIB): $(OBJ)
	$(CC) \
	$(CFLAGS_CC) \
	$(CFLAGS_SH) \
	$(CFLAGS_WARN) \
	$(CFLAGS_OPT) \
	$(LDFLAGS) \
	$(LDFLAGS_SH) \
	-o $@ \
	$(OBJ) \
	$(LIBS_TH) \
	$(LIB_SPEC)

bje:
	$(BJE) --keep --notest $(DEFS) $(OPT_DEFS) -o $(PKG_NAME) ${srcdir}/generic/$(SRC)

$(SCLIB): $(SCLIBSRC)
	ln -s $> $^ $@

$(srcdir)/doc/$(DOC): $(DOCSRC)
	$(DOCBLD) -o $@ $(DOCFMT) $> $^

$(DOC): $(srcdir)/doc/$(DOC)
	ln -s $> $^ $@

$(SCBIN): $(SCBINSRC)
	ln -s $> $^ $@

pkgIndex.tcl:
	echo 'package ifneeded $(PKG_NAME) $(VER) [list load [file join $$dir $(LIB)] $(INIT)]' > pkgIndex.tcl

clean-:

clean-pkgIndex.tcl:
	rm -f pkgIndex.tcl

clean-cfg_pkgIndex.tcl:
	rm -f pkgIndex.tcl

clean-doc:
	if test X"$(DOC)" != X"" && test -h "$(DOC)" ; then \
	    rm -f "$(DOC)" ; \
	fi

clean: clean-pkgIndex.tcl clean-doc
	rm -f $(SHLIB) $(OBJ) *.core
	if test X"$(SCLIB)" != X"" && test -h "$(SCLIB)" ; then \
	    rm -f "$(SCLIB)" ; \
	fi
	if test X"$(SCBIN)" != X"" && test -h "$(SCBIN)" ; then \
	    rm -f "$(SCBIN)" ; \
	fi

distclean: clean clean-
	rm -f Makefile

shell-Tcl: all
	$(TCLSH) $(SCRIPT)

shell-Jim: all
	$(JIMSH) $(SCRIPT)

shell: shell-Tcl

test-Tcl: all
	$(TESTENV) $(TCLSH) $(srcdir)/tests/$(TESTFWK) $(TESTFLAGS)

test-Jim: all
	for i in $(srcdir)/tests/*.test; do $(TESTENV) $(JIMSH) $$i $(TESTFLAGS); done

test: test-Tcl

install: all install-lib install-doc  install-exa

install-lib:
	@echo "Installing lib files in $(DESTDIR)$(PKG_INSTALL_DIR)/"
	@$(INSTALL_DATA_DIR) $(DESTDIR)$(PKG_INSTALL_DIR)
	@echo -n $(INDENT)
	@echo -n "$(LIB) "
	@$(INSTALL_LIBRARY) $(LIB) $(DESTDIR)$(PKG_INSTALL_DIR)
	@if test X"pkgIndex.tcl" != X"" ; then \
	  echo -n "pkgIndex.tcl"; \
	  $(INSTALL_DATA) pkgIndex.tcl $(DESTDIR)$(PKG_INSTALL_DIR); \
	fi
	@echo ""

install-script-bin:
	@echo "Installing script bin files in $(DESTDIR)${bindir}/"
	@$(INSTALL_DATA_DIR) $(DESTDIR)${bindir}
	@echo -n $(INDENT)
	@list='$(SCBIN)'; \
	for i in $$list; do \
	    echo -n "$$i "; \
	    $(INSTALL_SCRIPT) $$i $(DESTDIR)${bindir}; \
	done
	@echo ""

install-doc:
	@echo "Installing doc files in $(DESTDIR)$(DOC_INSTALL_DIR)/"
	@$(INSTALL_DATA_DIR) $(DESTDIR)$(DOC_INSTALL_DIR)
	@echo -n $(INDENT)
	@list='$(DOC)'; \
	for i in $$list; do \
	    echo -n "$$i "; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(DOC_INSTALL_DIR); \
	done
	@echo ""

install-exa:
	@echo "Installing example files in $(DESTDIR)${exadir}/"
	@$(INSTALL_DATA_DIR) $(DESTDIR)${exadir}
	@echo -n $(INDENT)
	@list='bsrv.tcl cli.tcl csrv.tcl esrv.tcl pair.tcl timeouts.tcl tsrv.tcl'; \
	for i in $$list; do \
	    echo -n "$$i "; \
	    $(INSTALL_SCRIPT) $(srcdir)/examples/$$i $(DESTDIR)${exadir}; \
	done
	@echo ""

dist-clean:
	rm -rf $(DIST_DIR) $(DIST_ROOT)/$(DIST_FILE)

dist: dist-clean
	$(INSTALL_DATA_DIR) $(DIST_DIR)
	list='LICENSE README README.licenses RELEASE Makefile.in spoto.conf'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$p $(DIST_DIR)/; \
	done
	list='configure'; \
	for p in $$list; do \
	    $(DIST_INSTALL_SCRIPT) $(srcdir)/$$p $(DIST_DIR)/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/unix
	DIR=unix; \
	list='$(SRC)'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/doc
	DIR=doc; \
	list='$(NAME).man $(NAME).n $(NAME).html'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/tests
	DIR=tests; \
	list='$(TESTFWK) $(PKG_NAME).test $(PKG_NAME).hts'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/ezt
	DIR=ezt; \
	list='ezt.h ezt_funcs.c ezt_init.c int2ptr_ptr2int.h eagain_ewouldblock.h'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/examples
	DIR=examples; \
	list='bsrv.tcl cli.tcl csrv.tcl esrv.tcl pair.tcl timeouts.tcl tsrv.tcl'; \
	for p in $$list; do \
	    $(DIST_INSTALL_SCRIPT) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/tcltest
	DIR=tcltest; \
	list='pkgIndex.tcl tcltest.tcl patch-doc_tcltest_n'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/openbsd-compat
	DIR=openbsd-compat; \
	list='bsd-getpeereid.c'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	$(INSTALL_DATA_DIR) $(DIST_DIR)/tclconfig
	DIR=tclconfig; \
	list='install-sh'; \
	for p in $$list; do \
	    $(DIST_INSTALL_DATA) $(srcdir)/$$DIR/$$p $(DIST_DIR)/$$DIR/; \
	done
	tar zcvf $(DIST_ROOT)/$(DIST_FILE) -C $(DIST_ROOT) $(DIST_NAME)

.PHONY: all shell test install install-lib install-doc install-exa dist clean distclean
.PHONY: test-Tcl test-Jim

# Spot o' Conf 0.6

# EOF
