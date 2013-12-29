#******************************************************************************
#  Game Launcher for KeeperFX - free implementation of Dungeon Keeper.
#******************************************************************************
#   @file Makefile
#      A script used by GNU Make to recompile the launcher.
#  @par Purpose:
#      Allows to invoke "make all" or similar commands to compile all
#      source code files and link them into executable file.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     25 May 2011 - 07 Jun 2011
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************
CPP  = g++
CC   = gcc
WINDRES = windres
RM = rm -f
MV = mv -f
CP = @cp -f
MKDIR = mkdir -p
ECHO  = @echo
BIN    = bin/launcher.exe
RES    = obj/launcher.rc.o
DATA   = bin/launchermn.jpg
LIBS   = 
OBJS   = \
obj/wxImageFrame.o \
obj/wxCheckRadioBox.o \
obj/LauncherApp.o \
obj/GameSettings.o \
obj/CommandOptions.o \
obj/FilelistChecker.o \
obj/LogViewer.o \
obj/md5.o \
$(RES)

# Variables
DEPFLAGS = -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MD -MP 

# linker flags
BUILD = release
WX_RELEASE = 3.0
WX_RELEASE_NODOT = 30
top_srcdir = ./wx
LIBDIRNAME = $(top_srcdir)/lib
WXWIDGETS_CXXFLAGS = -I${top_srcdir}/lib/wx/include/msw-unicode-static-3.0 -I${top_srcdir}/include \
	-DHAVE_W32API_H -D__WXMSW__ -W -Wall -Wno-ctor-dtor-privacy
CXXFLAGS = -c $(WXWIDGETS_CXXFLAGS) $(DEPFLAGS)
CFLAGS = -c $(DEPFLAGS)
LDFLAGS = -static-libgcc -static-libstdc++ -Wl,--enable-auto-import

# include wxWidgets build configuration
include wx/config.gcc

# Conditionally set variables

ifeq ($(UNICODE),1)
WXUNICODEFLAG = u
endif
__WXLIB_CORE_p = \
	-lwx_msw$(WXUNICODEFLAG)_core-$(WX_RELEASE)
__WXLIB_BASE_p = \
	-lwx_base$(WXUNICODEFLAG)-$(WX_RELEASE)
__LIB_TIFF_p = \
	-lwxtiff$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)
__LIB_JPEG_p = \
	-lwxjpeg$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)
__LIB_PNG_p = \
	-lpng
__LIB_ZLIB_p = \
	-lz
___LIB_REGEX_p = \
	-lwxregex$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)
___LIB_EXPAT_p = \
	-lwxexpat$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)

.PHONY: all all-before all-after debug docs docsdox clean clean-custom

# Dependencies tracking
-include obj/*.d

# Targets

all: all-before $(BIN) $(DATA) all-after

clean: clean-custom
	-$(RM) $(OBJS) $(BIN) $(LIBS) $(DATA)

all-before:
	$(MKDIR) obj
	$(MKDIR) bin

$(BIN): $(OBJS)
	-$(ECHO) 'Building target: $@'
	$(CPP) -o $@ $(OBJS)  $(__DEBUGINFO) $(__THREADSFLAG) -L$(LIBDIRNAME) -Wl,--subsystem,windows -mwindows $(____CAIRO_LIBDIR_FILENAMES_p) $(LDFLAGS)  $(__WXLIB_CORE_p)  $(__WXLIB_BASE_p)  $(__WXLIB_MONO_p) $(__LIB_TIFF_p) $(__LIB_JPEG_p) $(__LIB_PNG_p)  $(__LIB_ZLIB_p) $(___LIB_REGEX_p) $(___LIB_EXPAT_p) $(EXTRALIBS_FOR_BASE) $(__UNICOWS_LIB_p) $(__CAIRO_LIB_p) -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lole32 -loleaut32 -luuid -lrpcrt4 -ladvapi32 -lwsock32 -lwininet
	-$(ECHO) 'Finished building target: $@'
	-$(ECHO) ' '

obj/%.o: src/%.cpp
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/%.o: src/%.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/%.rc.o: res/%.rc
	-$(ECHO) 'Building resource: $<'
	$(WINDRES) -i$< -o$@    --define __WX$(TOOLKIT)__ --define WXBUILDING --define WXDLLNAME=$(WXDLLNAMEPREFIXGUI)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)$(WXCOMPILER)$(VENDORTAG)$(WXDLLVERSIONTAG) $(__RCDEFDIR_p) --include-dir $(top_srcdir)/include --include-dir $(top_srcdir)/src/stc/scintilla/include --include-dir $(top_srcdir)/src/stc/scintilla/src --define __WX__ --define SCI_LEXER --define LINK_LEXERS --define wxUSE_BASE=1 --define WXMAKINGDLL
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

bin/%.bmp: res/%.bmp
	-$(ECHO) 'Copying file: $<'
	$(CP) "$<" "$@"

bin/%.jpg: res/%.jpg
	-$(ECHO) 'Copying file: $<'
	$(CP) "$<" "$@"
