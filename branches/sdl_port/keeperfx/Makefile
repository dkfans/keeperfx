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
BIN  = ./bin/keeperfx.exe
RES  = ./obj/keeperfx_stdres.res
LIBS = ./lib/libkeeperfx.a
OBJS = \
./obj/main.o \
./obj/bflib_dernc.o \
./obj/bflib_fileio.o \
./obj/bflib_memory.o \
./obj/bflib_datetm.o \
./obj/bflib_basics.o \
./obj/bflib_pom.o \
./obj/bflib_keybrd.o \
./obj/bflib_sndlib.o \
./obj/bflib_sound.o \
./obj/bflib_fmvids.o \
./obj/bflib_guibtns.o \
./obj/bflib_bufrw.o \
./obj/bflib_sprite.o \
./obj/bflib_vidraw.o \
./obj/bflib_sprfnt.o \
./obj/bflib_filelst.o \
./obj/bflib_math.o \
./obj/bflib_cpu.o \
./obj/bflib_render.o \
./obj/bflib_render_trig.o \
./obj/bflib_render_gpoly.o \
./obj/bflib_heapmgr.o \
./obj/bflib_netsp.o \
./obj/bflib_netsp_ipx.o \
./obj/bflib_netsp_tcp.o \
./obj/bflib_mshandler.o \
./obj/bflib_mspointer.o \
./obj/bflib_mouse.o \
./obj/bflib_network.o \
./obj/bflib_video.o \
./obj/scrcapt.o \
./obj/gui_draw.o \
./obj/kjm_input.o \
./obj/packets.o \
./obj/config.o \
./obj/front_input.o \
./obj/thing_list.o \
./obj/lvl_script.o \
./obj/dungeon_data.o \
./obj/player_data.o \
./obj/player_instances.o \
./obj/player_comptask.o \
./obj/player_computer.o \
./obj/lvl_filesdk1.o \
./obj/front_simple.o \
./obj/game_saves.o \
./obj/engine_render.o \
./obj/engine_camera.o \
./obj/engine_lenses.o \
./obj/front_landview.o \
./obj/gui_tooltips.o \
./obj/thing_creature.o \
./obj/slab_data.o \
./obj/room_data.o \
./obj/creature_control.o \
./obj/map_data.o \
./obj/thing_doors.o \
./obj/config_campaigns.o \
./obj/front_credits.o \
./obj/bflib_semphr.o \
./obj/config_terrain.o \
./obj/config_rules.o \
./obj/config_lenses.o \
./obj/config_creature.o \
./obj/config_crtrmodel.o \
./obj/lens_mist.o \
./obj/ariadne.o \
./obj/creature_states.o \
./obj/thing_objects.o \
./obj/map_columns.o \
./obj/bflib_drawbas.o \
./obj/bflib_drawsdk.o \
./obj/game_merge.o \
./obj/thing_effects.o \
./obj/thing_navigate.o \
./obj/config_magic.o \
./obj/frontend.o \
./obj/vidmode.o \
./obj/sdl_events.o \
$(RES)

LINKOBJ  = $(OBJS)
LINKLIB =  -L"./directx/lib" -L"./lib"  -L"./sdl/lib" -mwindows -lmingw32 -lkeeperfx -lwinmm -lddraw -lSDLmain -lSDL -lSDL_net
INCS =  -I"./directx/include" -I"./sdl/include"
CXXINCS =  -I"./directx/include"  -I"./sdl/include/SDL"
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

./obj/%.o: ./src/%.cpp
	@echo 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./obj/bflib_render_trig.o: ./src/bflib_render_trig.c
	@echo 'Building file: $<'
	$(CC) $(INCS) -c -fmessage-length=0 $(DEPFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./obj/bflib_render_gpoly.o: ./src/bflib_render_gpoly.c
	@echo 'Building file: $<'
	$(CC) $(INCS) -c -fmessage-length=0 $(DEPFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./obj/bflib_render.o: ./src/bflib_render.c
	@echo 'Building file: $<'
	$(CC) $(INCS) -c -fmessage-length=0 $(DEPFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./obj/%.o: ./src/%.c
	@echo 'Building file: $<'
# In order to make the keeperfx.dll work, we must compile .c files
# which use variables from the library with c++ compiler. Not sure why..
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
#	$(CC) $(CFLAGS) -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./lib/libkeeperfx.a: ./bin/keeperfx.dll ./lib/keeperfx.def
	@echo 'Generating gcc library archive for: $<'
	$(DLLTOOL) --dllname "$<" --def "./lib/keeperfx.def" --output-lib "$@"
	@echo 'Finished generating: $@'
	@echo ' '

./bin/keeperfx.dll: ./lib/keeper95_gold.dll ./lib/keeper95_gold.map
	@echo 'Rebuilding DLL export table from: $<'
	peresec -o"$@" --def "./lib/keeperfx.def" -p"_DK_" "$<"
	@echo 'Finished creating: $@'
	@echo ' '

./obj/%.res: ./res/%.rc
	@echo 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 
	@echo 'Finished building: $<'
	@echo ' '
#******************************************************************************
