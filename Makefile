#ifeq ($(OS), Windows_NT)
#  CC=cl
#else
#  EXE=graphics_test
#  LIB=graphics
#  CC=clang
#  ifeq ($(UNAME_S), Darwin)
#    OSX
#  else
#    Linux
#  endif
#endif

OUTDIR = build
DOCDIR = docs
LIBDIR = graphics

EXENAME = graphics_test
EXEOBJ := $(OUTDIR)/$(EXENAME).o
EXEFILE = test.c
EXEEXT =
EXE := $(OUTDIR)/$(EXENAME)$(EXEEXT)

LIBNAME = graphics
LIBOBJ := $(OUTDIR)/$(LIBNAME).o
LIBFILE := $(LIBDIR)/graphics.c
LIBEXT = .dylib
LIB := $(OUTDIR)/lib$(LIBNAME)$(LIBEXT)

CC = clang
OPTS = -x objective-c -fno-objc-arc
DEPS = -framework Cocoa -framework AppKit

all: $(EXE) docs

lib: $(LIB)

docs: $(DOCDIR)/index.html

$(DOCDIR)/index.html:
	headerdoc2html -udpb $(LIBDIR)/graphics.h -o $(DOCDIR)
	gatherheaderdoc $(DOCDIR)
	mv $(DOCDIR)/masterTOC.html $(DOCDIR)/index.html

$(EXE): $(EXEOBJ) $(LIB)
	$(CC) $^ -o $@

$(EXEOBJ): $(EXEFILE)
	$(CC) -c $< -o $@

$(LIB): $(LIBOBJ)
	$(CC) -shared -fpic $(DEPS) -o $@ $^

$(LIBOBJ): $(LIBFILE)
	$(CC) -c $(OPTS) -o $@ $<

clean:
	rm -f $(LIBOBJ) $(EXEOBJ) $(LIB) $(EXE)