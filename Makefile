# Project: keeperfx
# Makefile created by Dev-C++, modified manually

CPP  = g++.exe
CC   = gcc.exe
WINDRES = windres.exe
RES  = build/keeperfx_private.res
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
LIBS =  -L"c:/usr/mingw/lib" -mwindows lib/keeperfx.a -lwinmm -g -O0  -march=i386 
INCS =  -I"C:/usr/mingw/include" 
CXXINCS =  -I"C:/usr/mingw/lib/gcc/mingw32/4.3.0/include"  -I"C:/usr/mingw/include/c++/4.3.0/backward"  -I"C:/usr/mingw/include/c++/4.3.0/mingw32"  -I"C:/usr/mingw/include/c++/4.3.0"  -I"C:/usr/mingw/include" 
BIN  = build/keeperfx.exe
CXXFLAGS = $(CXXINCS) -g -O0  -march=i386
CFLAGS = $(INCS) -g -O0  -march=i386
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

all-before:
	mkdir -p build

clean: clean-custom
	${RM} $(OBJ) $(BIN) lib/keeperfx.a lib/keeperfx.dll lib/keeperfx.def
	make -C tools/ec clean

$(BIN): $(OBJ) lib/keeperfx.a
	@echo "Final link"
	$(CPP) $(LINKOBJ) -o "build/keeperfx.exe" $(LIBS)

build/%.o: src/%.cpp
	$(CPP) -c $(CXXFLAGS) -o"$@" "$<"

build/%.o: src/%.c
	$(CPP) -c $(CFLAGS) -o"$@" "$<"

build/keeperfx_private.res: src/keeperfx_private.rc 
	$(WINDRES) -i src/keeperfx_private.rc --input-format=rc -o build/keeperfx_private.res -O coff 

lib/keeperfx.a: lib/keeperfx.dll lib/keeperfx.def
	dlltool --dllname lib/keeperfx.dll --def lib/keeperfx.def --output-lib lib/keeperfx.a

lib/keeperfx.dll: lib/keeper95_gold.dll lib/keeper95_gold.map tools/ec/keepfx_ec.exe
	cp lib/keeper95_gold.dll lib/keeperfx.dll
	cd lib && ../tools/ec/keepfx_ec.exe

tools/ec/keepfx_ec.exe: tools/ec/keepfx_ec.c
	make -C tools/ec
