#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file Makefile
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Allows to invoke "make all" or similar commands to compile all
#      source code files and link them into executable file.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     25 Jan 2009 - 19 Jan 2010
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
DLLTOOL = dlltool
BIN  = ./build/keeperfx.exe
RES  = ./build/keeperfx_stdres.res
LIBS = ./lib/libkeeperfx.a
OBJS = \
./build/main.o \
./build/bflib_dernc.o \
./build/bflib_fileio.o \
./build/bflib_memory.o \
./build/bflib_datetm.o \
./build/bflib_basics.o \
./build/bflib_pom.o \
./build/bflib_keybrd.o \
./build/bflib_sndlib.o \
./build/bflib_sound.o \
./build/bflib_fmvids.o \
./build/bflib_guibtns.o \
./build/bflib_bufrw.o \
./build/scrcapt.o \
./build/bflib_sprite.o \
./build/bflib_vidraw.o \
./build/bflib_sprfnt.o \
./build/gui_draw.o \
./build/kjm_input.o \
./build/packets.o \
./build/config.o \
./build/front_input.o \
./build/thing_list.o \
./build/lvl_script.o \
./build/bflib_filelst.o \
./build/bflib_math.o \
./build/player_instances.o \
./build/lvl_filesdk1.o \
./build/front_simple.o \
./build/game_saves.o \
./build/engine_render.o \
./build/engine_camera.o \
./build/engine_lenses.o \
./build/front_landview.o \
./build/gui_tooltips.o \
./build/thing_creature.o \
./build/slab_data.o \
./build/room_data.o \
./build/creature_control.o \
./build/map_data.o \
./build/thing_doors.o \
./build/config_campaigns.o \
./build/front_credits.o \
./build/bflib_semphr.o \
./build/config_terrain.o \
./build/config_rules.o \
./build/config_lenses.o \
./build/config_creature.o \
./build/config_crtrmodel.o \
./build/bflib_cpu.o \
./build/bflib_heapmgr.o \
./build/bflib_netsp.o \
./build/bflib_netsp_ipx.o \
./build/lens_mist.o \
./build/creature_states.o \
./build/thing_objects.o \
./build/map_columns.o \
./build/bflib_drawbas.o \
./build/bflib_drawsdk.o \
./build/game_merge.o \
./build/bflib_mshandler.o \
./build/bflib_mspointer.o \
./build/thing_effects.o \
./build/bflib_mouse.o \
./build/bflib_network.o \
./build/bflib_video.o \
./build/config_magic.o \
./build/frontend.o \
./build/player_computer.o \
./build/vidmode.o \
$(RES)

LINKOBJ  = $(OBJS)
LINKLIB =  -L"./directx/lib" -L"./lib"  -mwindows -lkeeperfx -lwinmm -lddraw 
INCS =  -I"./directx/include"
CXXINCS =  -I"./directx/include" 
# flags to generate dependency files
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" 
# code optimization flags
OPTFLAGS = -march=pentium -O3
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

./build/%.o: ./src/%.cpp
	@echo 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./build/%.o: ./src/%.c
	@echo 'Building file: $<'
# In order to make the keeperfx.dll work, we must compile .c files
# which use the library with c++ compiler. Not sure why..
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
#	$(CC) $(CFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./lib/libkeeperfx.a: ./lib/keeperfx.dll ./lib/keeperfx.def
	@echo 'Generating gcc library archive for: $<'
	$(DLLTOOL) --dllname "$<" --def "./lib/keeperfx.def" --output-lib "$@"
	@echo 'Finished generating: $@'
	@echo ' '

./lib/keeperfx.dll: ./lib/keeper95_gold.dll ./lib/keeper95_gold.map
	@echo 'Rebuilding DLL export table from: $<'
	./lib/peresec -o"$@" -p"_DK_" "$<"
	@echo 'Finished creating: $@'
	@echo ' '

./build/%.res: ./res/%.rc
	@echo 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 
	@echo 'Finished building: $<'
	@echo ' '
#******************************************************************************
