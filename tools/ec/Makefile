# Project: keepfx_ec
# Makefile created by Dev-C++, modified by hand

CPP  = g++
CC   = gcc
WINDRES = windres
RES  = 
OBJ  = keepfx_ec.o $(RES)
LINKOBJ  = $(OBJ)
LIBS =
INCS =
CXXINCS =
BIN  = keepfx_ec
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS)  
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before keepfx_ec all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN) $(BIN).exe

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

keepfx_ec.o: keepfx_ec.c
	$(CC) -c keepfx_ec.c -o keepfx_ec.o $(CFLAGS)
