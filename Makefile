#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file Makefile
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Allows to invoke "make all" or similar commands to compile all
#      source code files and link them into executable file.
#  @par Comment:
#      Please note that the make must be run from 'sh'; starting if from
#      Windows 'cmd.exe' won't work.
#      You need mingw32 and coreutils to do the build.
#      To prepare a release package, run:
#        make standard && make heavylog && make package
#  @author   Tomasz Lis
#  @date     25 Jan 2009 - 02 Jul 2011
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************
ifneq (,$(findstring Windows,$(OS)))
  CROSS_EXEEXT = .exe
else
  CROSS_EXEEXT =
endif
EXEEXT = .exe
CPP   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc
WINDRES  = $(CROSS_COMPILE)windres
DLLTOOL  = $(CROSS_COMPILE)dlltool
EXETODLL = tools/peresec/bin/peresec$(CROSS_EXEEXT)
DOXYTOOL = doxygen
PNGTOICO  = tools/png2ico/png2ico$(CROSS_EXEEXT)
RM       = rm -f
MV       = mv -f
CP       = cp -f
MKDIR    = mkdir -p
ECHO     = @echo

# Names of target binary files
BIN      = bin/keeperfx$(EXEEXT)
HVLOGBIN = bin/keeperfx_hvlog$(EXEEXT)
# Names of intermediate build products
GENSRC   = obj/ver_defs.h
RES      = obj/keeperfx_stdres.res
LIBS     = obj/libkeeperfx.a
OBJS = \
obj/actionpt.o \
obj/ariadne.o \
obj/ariadne_edge.o \
obj/ariadne_findcache.o \
obj/ariadne_navitree.o \
obj/ariadne_points.o \
obj/ariadne_regions.o \
obj/ariadne_tringls.o \
obj/bflib_base_tcp.o \
obj/bflib_basics.o \
obj/bflib_bufrw.o \
obj/bflib_client_tcp.o \
obj/bflib_cpu.o \
obj/bflib_crash.o \
obj/bflib_datetm.o \
obj/bflib_dernc.o \
obj/bflib_fileio.o \
obj/bflib_filelst.o \
obj/bflib_fmvids.o \
obj/bflib_guibtns.o \
obj/bflib_heapmgr.o \
obj/bflib_inputctrl.o \
obj/bflib_keybrd.o \
obj/bflib_math.o \
obj/bflib_memory.o \
obj/bflib_mouse.o \
obj/bflib_mshandler.o \
obj/bflib_mspointer.o \
obj/bflib_nethost_udp.o \
obj/bflib_netlisten_udp.o \
obj/bflib_netsp.o \
obj/bflib_netsp_ipx.o \
obj/bflib_netsp_tcp.o \
obj/bflib_netsync.o \
obj/bflib_network.o \
obj/bflib_planar.o \
obj/bflib_pom.o \
obj/bflib_render.o \
obj/bflib_render_gpoly.o \
obj/bflib_render_gtblock.o \
obj/bflib_render_trig.o \
obj/bflib_semphr.o \
obj/bflib_server_tcp.o \
obj/bflib_sndlib.o \
obj/bflib_sound.o \
obj/bflib_sprfnt.o \
obj/bflib_sprite.o \
obj/bflib_string.o \
obj/bflib_tcpsp.o \
obj/bflib_threadcond.o \
obj/bflib_video.o \
obj/bflib_vidraw.o \
obj/bflib_vidsurface.o \
obj/config.o \
obj/config_campaigns.o \
obj/config_creature.o \
obj/config_crtrmodel.o \
obj/config_crtrstates.o \
obj/config_lenses.o \
obj/config_magic.o \
obj/config_objects.o \
obj/config_rules.o \
obj/config_terrain.o \
obj/config_trapdoor.o \
obj/creature_battle.o \
obj/creature_control.o \
obj/creature_graphics.o \
obj/creature_groups.o \
obj/creature_instances.o \
obj/creature_states.o \
obj/creature_states_barck.o \
obj/creature_states_combt.o \
obj/creature_states_gardn.o \
obj/creature_states_guard.o \
obj/creature_states_hero.o \
obj/creature_states_lair.o \
obj/creature_states_mood.o \
obj/creature_states_pray.o \
obj/creature_states_prisn.o \
obj/creature_states_rsrch.o \
obj/creature_states_scavn.o \
obj/creature_states_spdig.o \
obj/creature_states_tortr.o \
obj/creature_states_train.o \
obj/creature_states_tresr.o \
obj/creature_states_wrshp.o \
obj/dungeon_data.o \
obj/engine_arrays.o \
obj/engine_camera.o \
obj/engine_lenses.o \
obj/engine_redraw.o \
obj/engine_render.o \
obj/front_credits.o \
obj/front_easter.o \
obj/front_highscore.o \
obj/front_input.o \
obj/front_landview.o \
obj/front_lvlstats.o \
obj/front_network.o \
obj/front_simple.o \
obj/front_torture.o \
obj/frontend.o \
obj/frontmenu_ingame_evnt.o \
obj/frontmenu_ingame_opts.o \
obj/frontmenu_ingame_tabs.o \
obj/frontmenu_net.o \
obj/frontmenu_options.o \
obj/frontmenu_saves.o \
obj/frontmenu_specials.o \
obj/game_merge.o \
obj/game_saves.o \
obj/gui_boxmenu.o \
obj/gui_draw.o \
obj/gui_frontbtns.o \
obj/gui_frontmenu.o \
obj/gui_parchment.o \
obj/gui_soundmsgs.o \
obj/gui_tooltips.o \
obj/gui_topmsg.o \
obj/kjm_input.o \
obj/lens_api.o \
obj/lens_flyeye.o \
obj/lens_mist.o \
obj/light_data.o \
obj/lvl_filesdk1.o \
obj/lvl_script.o \
obj/magic.o \
obj/map_blocks.o \
obj/map_columns.o \
obj/map_data.o \
obj/map_events.o \
obj/map_utils.o \
obj/net_sync.o \
obj/net_game.o \
obj/packets.o \
obj/player_comptask.o \
obj/player_computer.o \
obj/player_data.o \
obj/player_instances.o \
obj/power_hand.o \
obj/power_specials.o \
obj/room_data.o \
obj/room_graveyard.o \
obj/room_jobs.o \
obj/room_library.o \
obj/room_workshop.o \
obj/scrcapt.o \
obj/sounds.o \
obj/slab_data.o \
obj/spdigger_stack.o \
obj/tasks_list.o \
obj/thing_corpses.o \
obj/thing_creature.o \
obj/thing_data.o \
obj/thing_doors.o \
obj/thing_effects.o \
obj/thing_list.o \
obj/thing_navigate.o \
obj/thing_objects.o \
obj/thing_physics.o \
obj/thing_shots.o \
obj/thing_stats.o \
obj/thing_traps.o \
obj/vidfade.o \
obj/vidmode.o \
obj/KeeperSpeechImp.o \
obj/main.o \
$(RES)

# include and library directories
LINKLIB =  -L"sdl/lib" -mwindows obj/libkeeperfx.a -lwinmm -lmingw32 -limagehlp -lSDLmain -lSDL -lSDL_net 
INCS =  -I"sdl/include"
CXXINCS =  -I"sdl/include"

STDOBJS   = $(subst obj/,obj/std/,$(OBJS))
HVLOGOBJS = $(subst obj/,obj/hvlog/,$(OBJS))

# flags to generate dependency files
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"
# other flags to include while compiling
INCFLAGS =
# code optimization and debugging flags
DEBUG ?= 0
ifeq ($(DEBUG), 1)
  OPTFLAGS = -march=i686 -O0
  DBGFLAGS = -g -DDEBUG
else
  # frame pointer is required for ASM code to work
  OPTFLAGS = -march=i686 -fno-omit-frame-pointer -O3
  DBGFLAGS = 
endif
# linker flags
LINKFLAGS = -static-libgcc -static-libstdc++ -Wl,-Map,"$(@:%.exe=%.map)" -Wl,--enable-auto-import
# logging level flags
STLOGFLAGS = -DBFDEBUG_LEVEL=0 
HVLOGFLAGS = -DBFDEBUG_LEVEL=10
# compiler warning generation flags
WARNFLAGS = -Wall -Wno-sign-compare -Wno-unused-parameter -Wno-strict-aliasing
# disabled warnings: -Wextra -Wtype-limits
CXXFLAGS = $(CXXINCS) -c -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
CFLAGS = $(INCS) -c -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
LDFLAGS = $(LINKLIB) $(OPTFLAGS) $(DBGFLAGS) $(LINKFLAGS)

CAMPAIGNS  = \
ancntkpr \
dstninja \
burdnimp \
dzjr06lv \
dzjr10lv \
dzjr25lv \
evilkeep \
grkreign \
jdkmaps8 \
kdklvpck \
keeporig \
lqizgood \
lrdvexer \
ncastles \
postanck \
questfth \
twinkprs

# load program version
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(VER_BUILD)

# load depenency packages
include prebuilds.mk

# name virtual targets
.PHONY: all docs docsdox clean clean-build deep-clean
.PHONY: standard std-before std-after
.PHONY: heavylog hvlog-before hvlog-after
.PHONY: package clean-package deep-clean-package
.PHONY: tools clean-tools deep-clean-tools
.PHONY: libexterns clean-libexterns deep-clean-libexterns

all: standard

standard: CXXFLAGS += $(STLOGFLAGS)
standard: CFLAGS += $(STLOGFLAGS)
standard: std-before $(BIN) std-after

heavylog: CXXFLAGS += $(HVLOGFLAGS)
heavylog: CFLAGS += $(HVLOGFLAGS)
heavylog: hvlog-before $(HVLOGBIN) hvlog-after

std-before: libexterns
	$(MKDIR) obj/std bin

hvlog-before: libexterns
	$(MKDIR) obj/hvlog bin

docs: docsdox

docsdox: docs/doxygen.conf
	VERSION=$(VER_STRING) $(DOXYTOOL) docs/doxygen.conf

deep-clean: deep-clean-tools deep-clean-libexterns deep-clean-package

clean: clean-build clean-tools clean-libexterns clean-package

clean-build:
	-$(RM) $(STDOBJS) $(STDOBJS:%.o=%.d)
	-$(RM) $(HVLOGOBJS) $(HVLOGOBJS:%.o=%.d)
	-$(RM) $(BIN) $(BIN:%.exe=%.map)
	-$(RM) $(HVLOGBIN) $(HVLOGBIN:%.exe=%.map)
	-$(RM) bin/keeperfx.dll
	-$(RM) $(LIBS) $(GENSRC)
	-$(RM) res/*.ico
	-$(RM) obj/keeperfx.*

$(BIN): $(GENSRC) $(STDOBJS) $(LIBS) std-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(STDOBJS) $(LDFLAGS)
	-$(ECHO) 'Finished building target: $@'
	-$(ECHO) ' '

$(HVLOGBIN): $(GENSRC) $(HVLOGOBJS) $(LIBS) hvlog-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(HVLOGOBJS) $(LDFLAGS)
	-$(ECHO) 'Finished building target: $@'
	-$(ECHO) ' '

# Some C files have to be compiled as C++, because their language is not clear

obj/std/front_network.o obj/hvlog/front_network.o: src/front_network.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/front_torture.o obj/hvlog/front_torture.o: src/front_torture.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_ingame_evnt.o obj/hvlog/frontmenu_ingame_evnt.o: src/frontmenu_ingame_evnt.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_ingame_opts.o obj/hvlog/frontmenu_ingame_opts.o: src/frontmenu_ingame_opts.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_ingame_tabs.o obj/hvlog/frontmenu_ingame_tabs.o: src/frontmenu_ingame_tabs.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_net.o obj/hvlog/frontmenu_net.o: src/frontmenu_net.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_options.o obj/hvlog/frontmenu_options.o: src/frontmenu_options.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_saves.o obj/hvlog/frontmenu_saves.o: src/frontmenu_saves.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/%.o obj/hvlog/%.o: src/%.cpp $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/%.o obj/hvlog/%.o: src/%.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

# Windows resources compilation
obj/std/%.res obj/hvlog/%.res: res/%.rc res/keeperfx_icon.ico $(GENSRC)
	-$(ECHO) 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

# Creation of Windows icon files from PNG files
res/%.ico: res/%016-08bpp.png res/%032-08bpp.png res/%048-08bpp.png res/%064-08bpp.png res/%128-08bpp.png $(PNGTOICO)
	-$(ECHO) 'Building icon: $@'
	$(PNGTOICO) "$@" --colors 256 $(word 5,$^) $(word 4,$^) $(word 3,$^) --colors 16 $(word 2,$^) $(word 1,$^)
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

obj/ver_defs.h: version.mk Makefile
	$(ECHO) \#define VER_MAJOR   $(VER_MAJOR) > "$(@D)/tmp"
	$(ECHO) \#define VER_MINOR   $(VER_MINOR) >> "$(@D)/tmp"
	$(ECHO) \#define VER_RELEASE $(VER_RELEASE) >> "$(@D)/tmp"
	$(ECHO) \#define VER_BUILD   $(VER_BUILD) >> "$(@D)/tmp"
	$(ECHO) \#define VER_STRING  \"$(VER_STRING)\" >> "$(@D)/tmp"
	$(ECHO) \#define PACKAGE_SUFFIX  \"$(PACKAGE_SUFFIX)\" >> "$(@D)/tmp"
	$(MV) "$(@D)/tmp" "$@"

obj/libkeeperfx.a: bin/keeperfx.dll obj/keeperfx.def
	-$(ECHO) 'Generating gcc library archive for: $<'
	$(DLLTOOL) --dllname "$<" --def "obj/keeperfx.def" --output-lib "$@"
	-$(ECHO) 'Finished generating: $@'
	-$(ECHO) ' '

bin/keeperfx.dll obj/keeperfx.def: lib/keeper95_gold.dll lib/keeper95_gold.map $(EXETODLL)
	-$(ECHO) 'Rebuilding DLL export table from: $<'
	$(EXETODLL) -o"$@" --def "obj/keeperfx.def" -p"_DK_" "$<"
	-$(ECHO) 'Finished creating: $@'
	-$(ECHO) ' '

include libexterns.mk

include tool_peresec.mk
include tool_png2ico.mk

include package.mk
