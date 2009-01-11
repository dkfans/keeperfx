# Project: keeperfx
# Makefile created by Dev-C++, modified manually

ifneq (,$(findstring Windows,$(OS)))
  CROSS_EXEEXT = .exe
else
  CROSS_EXEEXT =
endif
EXEEXT = .exe
CPP   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc
WINDRES  = $(CROSS_COMPILE)windres
DLLTOOL  = $(CROSS_COMPILE)dlltool
EXETODLL = tools/ec/keepfx_ec
RM       = rm -f
CP       = cp -f
MKDIR    = mkdir -p
ECHO     = @echo

BIN      = bin/keeperfx$(EXEEXT)
RES      = obj/keeperfx_private.res

OBJ  = \
obj/main.o \
obj/frontend.o \
obj/bflib_basics.o \
obj/bflib_dernc.o \
obj/bflib_fileio.o \
obj/bflib_keybrd.o \
obj/bflib_datetm.o \
obj/bflib_memory.o \
obj/bflib_pom.o \
obj/bflib_mouse.o \
obj/bflib_sndlib.o \
obj/bflib_sound.o \
obj/bflib_video.o \
obj/bflib_fmvids.o \
obj/bflib_guibtns.o \
$(RES)

LINKOBJ  = $(OBJ)
LIBS =  -mwindows obj/keeperfx.a -lwinmm -g -O0  -march=i386 
INCS = 
CXXINCS = 
CXXFLAGS = $(CXXINCS) -g -O0  -march=i386
CFLAGS = $(INCS) -g -O0  -march=i386

VER_STRING = 0.2.1.9

.PHONY: all all-before all-after standard heavylog clean clean-build clean-tools clean-package package pkg-before

all: standard

standard: all-before $(BIN) all-after

heavylog: all-before all-after

all-before:
	$(MKDIR) obj bin

clean: clean-build clean-tools clean-package

clean-build:
	$(RM) $(OBJ) $(BIN) obj/keeperfx.a bin/keeperfx.dll lib/keeperfx.def

clean-tools:
	make -C tools/ec clean

clean-package:
	-$(RM) pkg/keeperfx*

$(BIN): $(OBJ) obj/keeperfx.a
	@echo "Final link"
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

obj/%.o: src/%.cpp
	$(CPP) -c $(CXXFLAGS) -o"$@" "$<"

obj/%.o: src/%.c
	$(CPP) -c $(CFLAGS) -o"$@" "$<"

obj/keeperfx_private.res: src/keeperfx_private.rc 
	$(WINDRES) -i src/keeperfx_private.rc --input-format=rc -o obj/keeperfx_private.res -O coff 

obj/keeperfx.a: bin/keeperfx.dll lib/keeperfx.def
	$(DLLTOOL) --dllname bin/keeperfx.dll --def lib/keeperfx.def --output-lib obj/keeperfx.a

bin/keeperfx.dll lib/keeperfx.def: lib/keeper95_gold.dll lib/keeper95_gold.map tools/ec/keepfx_ec
	cp lib/keeper95_gold.dll bin/keeperfx.dll
	$(EXETODLL)

tools/ec/keepfx_ec: tools/ec/keepfx_ec.c
	make -C tools/ec

package: pkg-before
	$(CP) bin/* pkg/
	$(CP) config/* pkg/
	$(CP) docs/keeperfx_readme.txt pkg/
	cd pkg; \
	7z a "keeperfx-$(subst .,_,$(VER_STRING))-patch.7z" "*" -x!*/.svn -x!.svn -x!.git -x!*.7z

pkg-before:
	-$(RM) "pkg/keeperfx-$(subst .,_,$(VER_STRING))-patch.7z"
	$(MKDIR) pkg

