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
MKDIR    = mkdir -p
ECHO     = @echo

BIN      = build/keeperfx$(EXEEXT)
RES      = build/keeperfx_private.res

OBJ  = \
build/main.o \
build/frontend.o \
build/bflib_basics.o \
build/bflib_dernc.o \
build/bflib_fileio.o \
build/bflib_keybrd.o \
build/bflib_datetm.o \
build/bflib_memory.o \
build/bflib_pom.o \
build/bflib_mouse.o \
build/bflib_sndlib.o \
build/bflib_sound.o \
build/bflib_video.o \
build/bflib_fmvids.o \
build/bflib_guibtns.o \
$(RES)

LINKOBJ  = $(OBJ)
LIBS =  -mwindows lib/keeperfx.a -lwinmm -g -O0  -march=i386 
INCS = 
CXXINCS = 
CXXFLAGS = $(CXXINCS) -g -O0  -march=i386
CFLAGS = $(INCS) -g -O0  -march=i386

.PHONY: all all-before all-after standard clean clean-custom

all: standard

standard: all-before $(BIN) all-after

all-before:
	mkdir -p build

clean: clean-custom
	$(RM) $(OBJ) $(BIN) lib/keeperfx.a lib/keeperfx.dll lib/keeperfx.def
	make -C tools/ec clean

$(BIN): $(OBJ) lib/keeperfx.a
	@echo "Final link"
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

build/%.o: src/%.cpp
	$(CPP) -c $(CXXFLAGS) -o"$@" "$<"

build/%.o: src/%.c
	$(CPP) -c $(CFLAGS) -o"$@" "$<"

build/keeperfx_private.res: src/keeperfx_private.rc 
	$(WINDRES) -i src/keeperfx_private.rc --input-format=rc -o build/keeperfx_private.res -O coff 

lib/keeperfx.a: lib/keeperfx.dll lib/keeperfx.def
	$(DLLTOOL) --dllname lib/keeperfx.dll --def lib/keeperfx.def --output-lib lib/keeperfx.a

lib/keeperfx.dll lib/keeperfx.def: lib/keeper95_gold.dll lib/keeper95_gold.map tools/ec/keepfx_ec
	cp lib/keeper95_gold.dll lib/keeperfx.dll
	cd lib && ../tools/ec/keepfx_ec

tools/ec/keepfx_ec: tools/ec/keepfx_ec.c
	make -C tools/ec
