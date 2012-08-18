#******************************************************************************
#  Land View files converter for KeeperFX
#******************************************************************************
#   @file Makefile
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Allows to invoke "make all" or similar commands to compile all
#      source code files and link them into executable file.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     25 Jul 2012 - 18 Aug 2012
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************
OS = $(shell uname -s)
ifneq (,$(findstring MINGW,$(OS)))
RES  = obj/landvico_stdres.res
else
RES  = 
endif
CPP  = g++
CC   = gcc
WINDRES = windres
DLLTOOL = dlltool
BIN  = bin/landvico
LIBS =
OBJS = \
obj/landvico.o \
$(RES)

LINKOBJ  = $(OBJS)
LINKLIB = -lpng -lz -lm
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

all: all-before $(BIN) all-after

clean: clean-custom
	-${RM} $(OBJS) $(BIN) $(LIBS)
	-@echo ' '

$(BIN): $(OBJS) $(LIBS)
	@echo 'Building target: $@'
	$(CPP) $(LINKOBJ) -o "$@" $(LINKLIB) $(OPTFLAGS)
	@echo 'Finished building target: $@'
	@echo ' '

obj/%.o: src/%.cpp
	@echo 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj/%.o: src/%.c
	@echo 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj/%.res: res/%.rc
	@echo 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 
	@echo 'Finished building: $<'
	@echo ' '
#******************************************************************************
