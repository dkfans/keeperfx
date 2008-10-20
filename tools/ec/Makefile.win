# Project: keepfx_ec
# Makefile created by Dev-C++ 4.9.9.2

CPP  = g++.exe
CC   = gcc.exe
WINDRES = windres.exe
RES  = 
OBJ  = keepfx_ec.o $(RES)
LINKOBJ  = keepfx_ec.o $(RES)
LIBS =  -L"D:/ProgsNT/Dev-Cpp/lib"  
INCS =  -I"D:/ProgsNT/Dev-Cpp/include" 
CXXINCS =  -I"D:/ProgsNT/Dev-Cpp/lib/gcc/mingw32/3.4.2/include"  -I"D:/ProgsNT/Dev-Cpp/include/c++/3.4.2/backward"  -I"D:/ProgsNT/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"D:/ProgsNT/Dev-Cpp/include/c++/3.4.2"  -I"D:/ProgsNT/Dev-Cpp/include" 
BIN  = keepfx_ec.exe
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS)  
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before keepfx_ec.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o "keepfx_ec.exe" $(LIBS)

keepfx_ec.o: keepfx_ec.c
	$(CC) -c keepfx_ec.c -o keepfx_ec.o $(CFLAGS)
