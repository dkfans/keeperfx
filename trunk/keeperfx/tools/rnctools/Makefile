#******************************************************************************
#  ProPak/RNC Compressor/Decompressor
#******************************************************************************
#   @file Makefile
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Allows to invoke "make all" or similar commands to compile all
#      source code files and link them into executable file.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     14 Feb 2008 - 01 Nov 2011
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************
OS = $(shell uname -s)
ifneq (,$(findstring MINGW,$(OS)))
RES  = obj/rnc_stdres.res
EXEEXT = .exe
else
RES  = 
EXEEXT =
endif
CPP  = g++
CC   = gcc
WINDRES = windres
DLLTOOL = dlltool
ENCBIN  = bin/rnc$(EXEEXT)
DECBIN  = bin/dernc$(EXEEXT)
ENCOBJS = obj/rnc.o obj/lbrncpack.o
DECOBJS = obj/dernc.o
LIBS =
OBJS = \
obj/lbfileio.o \
obj/lblogging.o \
obj/lbrncbase.o \
obj/lbpathutil.o \
$(RES)

LINKLIB = 
INCS = 
CXXINCS = 
# flags to generate dependency files
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" 
# code optimization flags
OPTFLAGS = -O3
# compiler warning generation flags
WARNFLAGS = -Wall -Wno-sign-compare -Wno-unused-parameter
# disabled warnings: -Wextra -Wtype-limits
CXXFLAGS = $(CXXINCS) -c -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS)
CFLAGS = $(INCS) -c -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS)
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(ENCBIN) $(DECBIN) all-after

clean: clean-custom
	-${RM} $(OBJS) $(ENCOBJS) $(DECOBJS) $(ENCBIN) $(DECBIN)
	-@echo ' '

$(ENCBIN): $(ENCOBJS) $(OBJS) $(LIBS)
	@echo 'Building target: $@'
	$(CPP) $(ENCOBJS) $(OBJS) -o "$@" $(LINKLIB) $(OPTFLAGS)
	@echo 'Finished building target: $@'
	@echo ' '

$(DECBIN): $(DECOBJS) $(OBJS) $(LIBS)
	@echo 'Building target: $@'
	$(CPP) $(DECOBJS) $(OBJS) -o "$@" $(LINKLIB) $(OPTFLAGS)
	@echo 'Finished building target: $@'
	@echo ' '

obj/%.o: src/%.cpp
	@echo 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj/%.o: src/%.c
	@echo 'Building file: $<'
	$(CC) $(CFLAGS) -DCOMPRESSOR -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj/%.res: res/%.rc
	@echo 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 
	@echo 'Finished building: $<'
	@echo ' '
#******************************************************************************
