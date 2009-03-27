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
MV       = mv -f
CP       = cp -f
MKDIR    = mkdir -p
ECHO     = @echo

BIN      = bin/keeperfx$(EXEEXT)
RES      = obj/keeperfx_private.res
GENSRC   = obj/ver_defs.h

OBJ  = \
obj/main.o \
obj/front_simple.o \
obj/game_saves.o \
obj/frontend.o \
obj/packets.o \
obj/config.o \
obj/front_input.o \
obj/player_computer.o \
obj/player_instances.o \
obj/vidmode.o \
obj/scrcapt.o \
obj/kjm_input.o \
obj/gui_draw.o \
obj/lvl_script.o \
obj/lvl_filesdk1.o \
obj/thing_list.o \
obj/bflib_basics.o \
obj/bflib_dernc.o \
obj/bflib_fileio.o \
obj/bflib_keybrd.o \
obj/bflib_datetm.o \
obj/bflib_math.o \
obj/bflib_memory.o \
obj/bflib_pom.o \
obj/bflib_mouse.o \
obj/bflib_sndlib.o \
obj/bflib_sound.o \
obj/bflib_video.o \
obj/bflib_fmvids.o \
obj/bflib_filelst.o \
obj/bflib_guibtns.o \
obj/bflib_vidraw.o \
obj/bflib_sprfnt.o \
obj/bflib_sprite.o \
$(RES)

LINKOBJ  = $(OBJ)
LIBS =  -mwindows obj/keeperfx.a -lwinmm -g -O0  -march=i386 
INCS = 
CXXINCS = 
CXXFLAGS = $(CXXINCS) -g -O0  -march=i386
CFLAGS = $(INCS) -g -O0  -march=i386

# load program version
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(VER_BUILD)

.PHONY: all all-before all-after standard heavylog clean clean-build clean-tools clean-package package pkg-before

all: standard

standard: all-before $(BIN) all-after

heavylog: all-before all-after

all-before:
	$(MKDIR) obj bin

clean: clean-build clean-tools clean-package

clean-build:
	-$(RM) $(OBJ) $(BIN)
	-$(RM) obj/keeperfx.a bin/keeperfx.dll lib/keeperfx.def $(GENSRC)

clean-tools:
	make -C tools/ec clean

clean-package:
	-$(RM) pkg/keeperfx*

$(BIN): $(OBJ) obj/keeperfx.a
	@echo "Final link"
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

obj/%.o: src/%.cpp $(GENSRC)
	$(CPP) -c $(CXXFLAGS) -o"$@" "$<"

obj/%.o: src/%.c $(GENSRC)
	$(CPP) -c $(CFLAGS) -o"$@" "$<"

obj/keeperfx_private.res: src/keeperfx_private.rc $(GENSRC)
	$(WINDRES) -i src/keeperfx_private.rc --input-format=rc -o obj/keeperfx_private.res -O coff 

obj/ver_defs.h: version.mk Makefile
	$(ECHO) \#define VER_MAJOR   $(VER_MAJOR) > "$(@D)/tmp"
	$(ECHO) \#define VER_MINOR   $(VER_MINOR) >> "$(@D)/tmp"
	$(ECHO) \#define VER_RELEASE $(VER_RELEASE) >> "$(@D)/tmp"
	$(ECHO) \#define VER_BUILD   $(VER_BUILD) >> "$(@D)/tmp"
	$(ECHO) \#define VER_STRING  \"$(VER_STRING)\" >> "$(@D)/tmp"
	$(MV) "$(@D)/tmp" "$@"

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

