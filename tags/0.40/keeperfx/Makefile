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
EXETODLL = peresec/bin/peresec.exe
RM = rm -f
MV = mv -f
ECHO = @echo
DOXYTOOL = doxygen
BIN    = bin/keeperfx.exe
DBGBIN = bin/keeperfx_dbg.exe
GENSRC = obj/ver_defs.h
RES    = obj/keeperfx_stdres.res
LIBS   = lib/libkeeperfx.a
OBJS   = \
obj/main.o \
obj/bflib_base_tcp.o \
obj/bflib_client_tcp.o \
obj/bflib_dernc.o \
obj/bflib_fileio.o \
obj/bflib_memory.o \
obj/bflib_datetm.o \
obj/bflib_basics.o \
obj/bflib_pom.o \
obj/bflib_keybrd.o \
obj/bflib_inputctrl.o \
obj/bflib_server_tcp.o \
obj/bflib_vidsurface.o \
obj/bflib_sndlib.o \
obj/bflib_sound.o \
obj/bflib_fmvids.o \
obj/bflib_guibtns.o \
obj/bflib_bufrw.o \
obj/bflib_sprite.o \
obj/bflib_vidraw.o \
obj/bflib_sprfnt.o \
obj/bflib_filelst.o \
obj/bflib_math.o \
obj/bflib_planar.o \
obj/bflib_cpu.o \
obj/bflib_crash.o \
obj/bflib_render.o \
obj/bflib_render_trig.o \
obj/bflib_render_gpoly.o \
obj/bflib_render_gtblock.o \
obj/bflib_heapmgr.o \
obj/bflib_nethost_udp.o \
obj/bflib_netlisten_udp.o \
obj/bflib_netsp.o \
obj/bflib_netsp_ipx.o \
obj/bflib_netsp_tcp.o \
obj/bflib_netsync.o \
obj/bflib_mshandler.o \
obj/bflib_mspointer.o \
obj/bflib_mouse.o \
obj/bflib_network.o \
obj/bflib_tcpsp.o \
obj/bflib_threadcond.o \
obj/bflib_video.o \
obj/bflib_semphr.o \
obj/scrcapt.o \
obj/gui_draw.o \
obj/kjm_input.o \
obj/packets.o \
obj/net_sync.o \
obj/net_game.o \
obj/config.o \
obj/front_input.o \
obj/front_network.o \
obj/thing_data.o \
obj/thing_list.o \
obj/lvl_script.o \
obj/dungeon_data.o \
obj/player_data.o \
obj/player_instances.o \
obj/player_comptask.o \
obj/player_computer.o \
obj/tasks_list.o \
obj/spdigger_stack.o \
obj/lvl_filesdk1.o \
obj/front_simple.o \
obj/game_saves.o \
obj/engine_render.o \
obj/engine_camera.o \
obj/engine_lenses.o \
obj/engine_arrays.o \
obj/front_landview.o \
obj/frontmenu_net.o \
obj/frontmenu_options.o \
obj/frontmenu_saves.o \
obj/frontmenu_specials.o \
obj/frontmenu_ingame_tabs.o \
obj/frontmenu_ingame_evnt.o \
obj/frontmenu_ingame_opts.o \
obj/gui_parchment.o \
obj/gui_boxmenu.o \
obj/gui_frontmenu.o \
obj/gui_frontbtns.o \
obj/gui_tooltips.o \
obj/gui_topmsg.o \
obj/gui_soundmsgs.o \
obj/thing_creature.o \
obj/slab_data.o \
obj/room_data.o \
obj/room_workshop.o \
obj/room_library.o \
obj/room_jobs.o \
obj/creature_control.o \
obj/creature_instances.o \
obj/creature_graphics.o \
obj/creature_groups.o \
obj/map_data.o \
obj/map_events.o \
obj/map_utils.o \
obj/thing_doors.o \
obj/thing_traps.o \
obj/thing_shots.o \
obj/thing_corpses.o \
obj/config_campaigns.o \
obj/front_credits.o \
obj/front_torture.o \
obj/config_terrain.o \
obj/config_trapdoor.o \
obj/config_rules.o \
obj/config_lenses.o \
obj/config_creature.o \
obj/config_crtrmodel.o \
obj/config_crtrstates.o \
obj/lens_mist.o \
obj/lens_flyeye.o \
obj/lens_api.o \
obj/light_data.o \
obj/ariadne_navitree.o \
obj/ariadne_regions.o \
obj/ariadne_tringls.o \
obj/ariadne_edge.o \
obj/ariadne_points.o \
obj/ariadne_findcache.o \
obj/ariadne.o \
obj/creature_states_hero.o \
obj/creature_states_mood.o \
obj/creature_states_gardn.o \
obj/creature_states_lair.o \
obj/creature_states_prisn.o \
obj/creature_states_rsrch.o \
obj/creature_states_scavn.o \
obj/creature_states_tortr.o \
obj/creature_states_train.o \
obj/creature_states_wrshp.o \
obj/creature_states_spdig.o \
obj/creature_states_combt.o \
obj/creature_states_guard.o \
obj/creature_states_pray.o \
obj/creature_states_tresr.o \
obj/creature_states.o \
obj/thing_objects.o \
obj/thing_physics.o \
obj/magic.o \
obj/power_specials.o \
obj/power_hand.o \
obj/map_columns.o \
obj/map_blocks.o \
obj/game_merge.o \
obj/thing_stats.o \
obj/thing_effects.o \
obj/thing_navigate.o \
obj/actionpt.o \
obj/config_magic.o \
obj/frontend.o \
obj/sounds.o \
obj/vidmode.o \
obj/vidfade.o \
obj/KeeperSpeechImp.o \
$(RES)

LINKOBJ  = $(OBJS)
# load program version
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(VER_BUILD)
# include and library directories
LINKLIB =  -L"directx/lib" -L"lib" -L"sdl/lib" -mwindows -lmingw32 -lkeeperfx -lwinmm -limagehlp -lSDLmain -lSDL -lSDL_net 
INCS =  -I"directx/include" -I"sdl/include"
CXXINCS =  -I"directx/include" -I"sdl/include"

# flags to generate dependency files
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" 
# code optimization flags
OPTFLAGS = -march=pentium -O3
# linker flags
LDFLAGS = -static-libgcc -static-libstdc++ -Wl,-Map,"$(@:%.exe=%.map)" -Wl,--enable-auto-import
# debugging flags
DBGFLAGS = -DBFDEBUG_LEVEL=0 
# compiler warning generation flags
WARNFLAGS = -Wall -Wno-sign-compare -Wno-unused-parameter -Wno-strict-aliasing
# disabled warnings: -Wextra -Wtype-limits
CXXFLAGS = $(CXXINCS) -c -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
CFLAGS = $(INCS) -c -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)

# dependencies tracking
-include obj/*.d

.PHONY: all all-before all-after debug docs docsdox clean clean-custom

all: all-before $(BIN) all-after

debug: DBGFLAGS = -DBFDEBUG_LEVEL=10
debug: all-before $(DBGBIN) all-after

docs: docsdox

docsdox: docs/doxygen.conf
	VERSION=$(VER_STRING) $(DOXYTOOL) docs/doxygen.conf

clean: clean-custom
	-$(RM) $(OBJS) $(BIN) $(DBGBIN) $(LIBS) $(GENSRC)
	$(MAKE) clean -C peresec
	-$(ECHO) ' '

$(BIN) $(DBGBIN): $(GENSRC) $(OBJS) $(LIBS)
	-$(ECHO) 'Building target: $@'
	$(CPP) $(LINKOBJ) -o "$@" $(LINKLIB) $(OPTFLAGS) $(DBGFLAGS) $(LDFLAGS)
	-$(ECHO) 'Finished building target: $@'
	-$(ECHO) ' '

obj/%.o: src/%.cpp
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/bflib_render_trig.o: src/bflib_render_trig.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(INCS) -c -fmessage-length=0 $(DEPFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/bflib_render_gpoly.o: src/bflib_render_gpoly.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(INCS) -c -fmessage-length=0 $(DEPFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/bflib_render_gtblock.o: src/bflib_render_gtblock.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(INCS) -c -fmessage-length=0 $(DEPFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/bflib_render.o: src/bflib_render.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(INCS) -c -fmessage-length=0 $(DEPFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/%.o: src/%.c
	-$(ECHO) 'Building file: $<'
# In order to make the keeperfx.dll work, we must compile .c files
# which use variables from the library with c++ compiler. Not sure why..
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
#	$(CC) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/ver_defs.h: version.mk Makefile
	-$(ECHO) 'Generating: $@'
	$(ECHO) \#define VER_MAJOR   $(VER_MAJOR) > "$(@D)/tmp"
	$(ECHO) \#define VER_MINOR   $(VER_MINOR) >> "$(@D)/tmp"
	$(ECHO) \#define VER_RELEASE $(VER_RELEASE) >> "$(@D)/tmp"
	$(ECHO) \#define VER_BUILD   $(VER_BUILD) >> "$(@D)/tmp"
	$(ECHO) \#define VER_STRING  \"$(VER_STRING)\" >> "$(@D)/tmp"
	$(MV) "$(@D)/tmp" "$@"
	-$(ECHO) ' '

lib/libkeeperfx.a: bin/keeperfx.dll lib/keeperfx.def
	-$(ECHO) 'Generating gcc library archive for: $<'
	$(DLLTOOL) --dllname "$<" --def "lib/keeperfx.def" --output-lib "$@"
	-$(ECHO) 'Finished generating: $@'
	-$(ECHO) ' '

bin/keeperfx.dll lib/keeperfx.def: lib/keeper95_gold.dll lib/keeper95_gold.map $(EXETODLL)
	-$(ECHO) 'Rebuilding DLL export table from: $<'
	$(EXETODLL) -o"$@" --def "lib/keeperfx.def" -p"_DK_" "$<"
	-$(ECHO) 'Finished creating: $@'
	-$(ECHO) ' '

$(EXETODLL): peresec/src/peresec.c peresec/Makefile
	$(MAKE) -C peresec

obj/%.res: res/%.rc
	-$(ECHO) 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '
#******************************************************************************
