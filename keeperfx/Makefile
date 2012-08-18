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
CPP  = $(CROSS_COMPILE)g++
CC   = $(CROSS_COMPILE)gcc
WINDRES = $(CROSS_COMPILE)windres
DLLTOOL = $(CROSS_COMPILE)dlltool
EXETODLL = peresec/bin/peresec
BMPTORAWLV = tools/landvico/bin/landvico
RM = rm -f
MV = mv -f
CP = cp -f
MKDIR    = mkdir -p
ECHO     = @echo
DOXYTOOL = doxygen
PNG2ICO  = png2ico
BIN      = bin/keeperfx.exe
HVLOGBIN = bin/keeperfx_hvlog.exe
GENSRC   = obj/ver_defs.h
RES      = obj/keeperfx_stdres.res
LIBS     = lib/libkeeperfx.a
OBJS     = \
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
obj/bflib_string.o \
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
obj/engine_redraw.o \
obj/front_landview.o \
obj/front_highscore.o \
obj/front_lvlstats.o \
obj/front_easter.o \
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
obj/room_graveyard.o \
obj/room_jobs.o \
obj/creature_control.o \
obj/creature_instances.o \
obj/creature_graphics.o \
obj/creature_groups.o \
obj/creature_battle.o \
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
obj/config_objects.o \
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
CAMPAIGNS  = \
pkg/campgns/ancntkpr.cfg \
pkg/campgns/burdnimp.cfg \
pkg/campgns/dstninja.cfg \
pkg/campgns/dzjr06lv.cfg \
pkg/campgns/dzjr10lv.cfg \
pkg/campgns/dzjr25lv.cfg \
pkg/campgns/evilkeep.cfg \
pkg/campgns/grkreign.cfg \
pkg/campgns/jdkmaps8.cfg \
pkg/campgns/kdklvpck.cfg \
pkg/campgns/keeporig.cfg \
pkg/campgns/lqizgood.cfg \
pkg/campgns/lrdvexer.cfg \
pkg/campgns/ncastles.cfg \
pkg/campgns/postanck.cfg \
pkg/campgns/questfth.cfg \
pkg/campgns/twinkprs.cfg
STDOBJS   = $(subst obj/,obj/std/,$(OBJS))
HVLOGOBJS = $(subst obj/,obj/hvlog/,$(OBJS))

# load program version
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(VER_BUILD)
# include and library directories
LINKLIB =  -L"directx/lib" -L"lib" -L"sdl/lib" -mwindows -lmingw32 -lkeeperfx -lwinmm -limagehlp -lSDLmain -lSDL -lSDL_net 
INCS =  -I"directx/include" -I"sdl/include"
CXXINCS =  -I"directx/include" -I"sdl/include"

# flags to generate dependency files
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" 
# code optimization and debugging flags
DEBUG ?= 0
ifeq ($(DEBUG), 1)
  OPTFLAGS = -march=pentium -O0
  DBGFLAGS = -g -DDEBUG
else
  OPTFLAGS = -march=pentium -O3
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

# dependencies tracking
-include obj/std/*.d
-include obj/hvlog/*.d

# mark icons as precious, because even tho we can re-create them, it requires having "png2ico" tool
.PRECIOUS: res/%.ico
# name virtual targets
.PHONY: all standard std-before std-after debug hvlog-before hvlog-after docs docsdox clean clean-build clean-package package

all: standard

standard: CXXFLAGS += $(STLOGFLAGS)
standard: CFLAGS += $(STLOGFLAGS)
standard: std-before $(BIN) std-after

heavylog: CXXFLAGS += $(HVLOGFLAGS)
heavylog: CFLAGS += $(HVLOGFLAGS)
heavylog: hvlog-before $(HVLOGBIN) hvlog-after

docs: docsdox

std-before:
	$(MKDIR) obj/std

hvlog-before:
	$(MKDIR) obj/hvlog

docsdox: docs/doxygen.conf
	VERSION=$(VER_STRING) $(DOXYTOOL) docs/doxygen.conf

clean: clean-build clean-package

clean-build:
	-$(RM) $(STDOBJS) $(BIN)
	-$(RM) $(HVLOGOBJS) $(HVLOGBIN)
	-$(RM) $(LIBS) $(GENSRC)
	-$(RM) bin/keeperfx*
	$(MAKE) clean -C peresec
	$(MAKE) clean -C tools/landvico
	-$(ECHO) ' '

clean-package:
	-$(RM) -R pkg/campgns
	-$(RM) -R pkg/creatrs
	-$(RM) -R pkg/fxdata
	-$(RM) -R pkg/ldata
	-$(RM) pkg/keeperfx*
	-$(ECHO) ' '

package: $(CAMPAIGNS)
	-$(RM) "pkg/keeperfx_ccp-$(subst .,_,$(VER_STRING))-patch.7z"
	$(CP) bin/* pkg/
	$(CP) docs/keeperfx_readme.txt pkg/
	$(CP) config/keeperfx.cfg pkg/
	$(MKDIR) pkg/creatrs
	$(CP) config/creatrs/*.cfg pkg/creatrs/
	$(MKDIR) pkg/fxdata
	$(CP) config/fxdata/*.cfg pkg/fxdata/
	$(MKDIR) pkg/ldata
	$(CP) config/ldata/*.txt pkg/ldata/
	cd pkg; \
	7z a "keeperfx_ccp-$(subst .,_,$(VER_STRING))-patch.7z" "*" -x!*/.svn -x!.svn -x!*.7z

pkg/campgns/%.cfg: campgns/%.cfg
	@$(MKDIR) $(@D)
#	 Copy folder with campaign name (w/o extension), if it exists
	$(if $(wildcard $(<:%.cfg=%)),$(MKDIR) $(@:%.cfg=%))
	$(if $(wildcard $(<:%.cfg=%)),-$(CP) $(<:%.cfg=%)/map*.* $(@:%.cfg=%)/)
#	 Copy folder with campaign name and _crtr ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_crtr)),$(MKDIR) $(@:%.cfg=%_crtr))
	$(if $(wildcard $(<:%.cfg=%_crtr)),-$(CP) $(<:%.cfg=%_crtr)/*.cfg $(@:%.cfg=%_crtr)/)
#	 Copy folder with campaign name and _lnd ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_lnd)),$(MKDIR) $(@:%.cfg=%_lnd))
	$(if $(wildcard $(<:%.cfg=%_lnd)),-$(CP) $(<:%.cfg=%_lnd)/*.txt $(@:%.cfg=%_lnd)/)
#	 Copy the actual campaign file
	$(CP) $< $@

$(BIN): $(GENSRC) $(STDOBJS) $(LIBS)
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(STDOBJS) $(LDFLAGS)
	-$(ECHO) 'Finished building target: $@'
	-$(ECHO) ' '

$(HVLOGBIN): $(GENSRC) $(HVLOGOBJS) $(LIBS)
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(HVLOGOBJS) $(LDFLAGS)
	-$(ECHO) 'Finished building target: $@'
	-$(ECHO) ' '

# Some C files have to be compiled in c++ compiler, usually due to c++ initialization
obj/std/front_network.o obj/hvlog/front_network.o: src/front_network.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_net.o obj/hvlog/frontmenu_net.o: src/frontmenu_net.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_options.o obj/hvlog/frontmenu_options.o: src/frontmenu_options.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_saves.o obj/hvlog/frontmenu_saves.o: src/frontmenu_saves.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_ingame_tabs.o obj/hvlog/frontmenu_ingame_tabs.o: src/frontmenu_ingame_tabs.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_ingame_evnt.o obj/hvlog/frontmenu_ingame_evnt.o: src/frontmenu_ingame_evnt.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/frontmenu_ingame_opts.o obj/hvlog/frontmenu_ingame_opts.o: src/frontmenu_ingame_opts.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/front_torture.o obj/hvlog/front_torture.o: src/front_torture.c
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/%.o obj/hvlog/%.o: src/%.cpp
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/%.o obj/hvlog/%.o: src/%.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
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

$(BMPTORAWLV): tools/landvico/src/landvico.cpp tools/landvico/Makefile
	$(MAKE) -C tools/landvico

# Windows resources compilation
# Should depend on res/keeperfx_icon.ico, but currently we've assuming ICO files are alredy made 
obj/std/%.res obj/hvlog/%.res: res/%.rc
	-$(ECHO) 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

# Creation of Windows icon files from PNG files
res/%.ico: res/%016-08bpp.png res/%032-08bpp.png res/%048-08bpp.png res/%064-08bpp.png res/%128-08bpp.png
	-$(ECHO) 'Building icon: $@'
	$(PNG2ICO) "$@" --colors 256 $(word 5,$^) $(word 4,$^) $(word 3,$^) --colors 16 $(word 2,$^) $(word 1,$^)
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '
#******************************************************************************
