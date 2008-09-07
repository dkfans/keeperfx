# Project: keeperfx
# Makefile created by Dev-C++, modified manually

CPP  = g++.exe
CC   = gcc.exe
WINDRES = windres.exe
RES  = build/keeperfx_private.res
OBJ  = build/main.o $(RES)
LINKOBJ  = build/main.o $(RES)
LIBS =  -L"c:/usr/mingw/lib" -mwindows lib/keeperfx.a -lwinmm  -march=i386 
INCS =  -I"C:/usr/mingw/include" 
CXXINCS =  -I"C:/usr/mingw/lib/gcc/mingw32/4.3.0/include"  -I"C:/usr/mingw/include/c++/4.3.0/backward"  -I"C:/usr/mingw/include/c++/4.3.0/mingw32"  -I"C:/usr/mingw/include/c++/4.3.0"  -I"C:/usr/mingw/include" 
BIN  = build/keeperfx.exe
CXXFLAGS = $(CXXINCS)   -march=i386
CFLAGS = $(INCS)   -march=i386
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

all-before:
	mkdir -p build

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ) lib/keeperfx.a
	$(CPP) $(LINKOBJ) -o "build/keeperfx.exe" $(LIBS)

build/main.o: src/main.cpp
	$(CPP) -c src/main.cpp -o build/main.o $(CXXFLAGS)

build/keeperfx_private.res: src/keeperfx_private.rc 
	$(WINDRES) -i src/keeperfx_private.rc --input-format=rc -o build/keeperfx_private.res -O coff 

lib/keeperfx.a: lib/keeperfx.dll lib/keeperfx.def
	dlltool --dllname lib/keeperfx.dll --def lib/keeperfx.def --output-lib lib/keeperfx.a
