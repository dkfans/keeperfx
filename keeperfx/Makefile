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
EXETODLL = peresec/bin/peresec$(CROSS_EXEEXT)
PNGTORAW = tools/pngpal2raw/bin/pngpal2raw$(CROSS_EXEEXT)
PNGTOBSPAL = tools/png2bestpal/bin/png2bestpal$(CROSS_EXEEXT)
POTONGDAT = tools/po2ngdat/bin/po2ngdat$(CROSS_EXEEXT)
RNC   = tools/rnctools/bin/rnc$(CROSS_EXEEXT)
DERNC = tools/rnctools/bin/dernc$(CROSS_EXEEXT)
RM    = rm -f
MV    = mv -f
CP    = cp -f
MKDIR    = mkdir -p
ECHO     = @echo
DOXYTOOL = doxygen
PNG2ICO  = png2ico
BIN      = bin/keeperfx$(EXEEXT)
HVLOGBIN = bin/keeperfx_hvlog$(EXEEXT)
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
obj/gui_msgs.o \
obj/kjm_input.o \
obj/packets.o \
obj/net_sync.o \
obj/net_game.o \
obj/config.o \
obj/front_input.o \
obj/front_network.o \
obj/thing_data.o \
obj/thing_factory.o \
obj/thing_list.o \
obj/lvl_script.o \
obj/dungeon_data.o \
obj/player_data.o \
obj/player_utils.o \
obj/player_states.o \
obj/dungeon_stats.o \
obj/player_instances.o \
obj/player_comptask.o \
obj/player_compprocs.o \
obj/player_compevents.o \
obj/player_compchecks.o \
obj/player_complookup.o \
obj/player_computer.o \
obj/tasks_list.o \
obj/spdigger_stack.o \
obj/lvl_filesdk1.o \
obj/front_simple.o \
obj/game_saves.o \
obj/game_legacy.o \
obj/game_heap.o \
obj/engine_render.o \
obj/engine_camera.o \
obj/engine_lenses.o \
obj/engine_arrays.o \
obj/engine_redraw.o \
obj/front_landview.o \
obj/front_highscore.o \
obj/front_lvlstats.o \
obj/front_lvlstats_data.o \
obj/front_easter.o \
obj/frontmenu_net.o \
obj/frontmenu_options.o \
obj/frontmenu_options_data.o \
obj/frontmenu_saves.o \
obj/frontmenu_saves_data.o \
obj/frontmenu_specials.o \
obj/frontmenu_ingame_tabs.o \
obj/frontmenu_ingame_tabs_data.o \
obj/frontmenu_ingame_evnt.o \
obj/frontmenu_ingame_evnt_data.o \
obj/frontmenu_ingame_opts.o \
obj/frontmenu_ingame_opts_data.o \
obj/frontmenu_ingame_map.o \
obj/gui_parchment.o \
obj/gui_boxmenu.o \
obj/gui_frontmenu.o \
obj/gui_frontbtns.o \
obj/gui_tooltips.o \
obj/gui_topmsg.o \
obj/gui_soundmsgs.o \
obj/thing_creature.o \
obj/thing_creature_data.o \
obj/slab_data.o \
obj/room_data.o \
obj/room_list.o \
obj/room_util.o \
obj/room_workshop.o \
obj/room_library.o \
obj/room_graveyard.o \
obj/room_garden.o \
obj/room_lair.o \
obj/room_scavenge.o \
obj/room_entrance.o \
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
obj/front_torture_data.o \
obj/config_strings.o \
obj/config_settings.o \
obj/config_terrain.o \
obj/config_cubes.o \
obj/config_trapdoor.o \
obj/config_objects.o \
obj/config_rules.o \
obj/config_lenses.o \
obj/config_creature.o \
obj/config_crtrmodel.o \
obj/config_crtrstates.o \
obj/config_compp.o \
obj/lens_mist.o \
obj/lens_flyeye.o \
obj/lens_api.o \
obj/light_data.o \
obj/ariadne_navitree.o \
obj/ariadne_regions.o \
obj/ariadne_tringls.o \
obj/ariadne_naviheap.o \
obj/ariadne_edge.o \
obj/ariadne_points.o \
obj/ariadne_findcache.o \
obj/ariadne.o \
obj/creature_senses.o \
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
obj/creature_states_barck.o \
obj/creature_states.o \
obj/thing_objects.o \
obj/thing_physics.o \
obj/magic.o \
obj/power_specials.o \
obj/power_process.o \
obj/power_hand.o \
obj/map_columns.o \
obj/map_blocks.o \
obj/game_merge.o \
obj/game_lghtshdw.o \
obj/thing_stats.o \
obj/thing_effects.o \
obj/thing_navigate.o \
obj/actionpt.o \
obj/config_magic.o \
obj/config_magic_data.o \
obj/frontend.o \
obj/sounds.o \
obj/vidmode.o \
obj/vidfade.o \
obj/KeeperSpeechImp.o \
$(RES)
CAMPAIGNS  = \
ancntkpr \
burdnimp \
cqarctic \
dstninja \
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
pstunded \
questfth \
twinkprs \
undedkpr
CAMPAIGN_CFGS = $(patsubst %,pkg/campgns/%.cfg,$(CAMPAIGNS))
NGTEXTDATS = \
pkg/fxdata/gtext_chi.dat \
pkg/fxdata/gtext_cht.dat \
pkg/fxdata/gtext_dut.dat \
pkg/fxdata/gtext_fre.dat \
pkg/fxdata/gtext_ger.dat \
pkg/fxdata/gtext_ita.dat \
pkg/fxdata/gtext_jap.dat \
pkg/fxdata/gtext_pol.dat \
pkg/fxdata/gtext_rus.dat \
pkg/fxdata/gtext_spa.dat \
pkg/fxdata/gtext_swe.dat \
pkg/fxdata/gtext_eng.dat
NCTEXTDATS = \
pkg/campgns/ancntkpr/text_eng.dat \
pkg/campgns/ancntkpr/text_fre.dat \
pkg/campgns/burdnimp/text_eng.dat \
pkg/campgns/dstninja/text_eng.dat \
pkg/campgns/evilkeep/text_eng.dat \
pkg/campgns/evilkeep/text_fre.dat \
pkg/campgns/evilkeep/text_ger.dat \
pkg/campgns/grkreign/text_eng.dat \
pkg/campgns/lqizgood/text_eng.dat \
pkg/campgns/ncastles/text_eng.dat \
pkg/campgns/questfth/text_eng.dat \
pkg/campgns/questfth/text_fre.dat
LANDVIEWRAWS = \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/keeporig_lnd/rgmap$(num).raw pkg/campgns/keeporig_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/ancntkpr_lnd/rgmap$(num).raw pkg/campgns/ancntkpr_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09,pkg/campgns/burdnimp_lnd/rgmap$(num).raw pkg/campgns/burdnimp_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/cqarctic_lnd/rgmap$(num).raw pkg/campgns/cqarctic_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10,pkg/campgns/dstninja_lnd/rgmap$(num).raw pkg/campgns/dstninja_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/dzjr06lv_lnd/rgmap$(num).raw pkg/campgns/dzjr06lv_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/dzjr10lv_lnd/rgmap$(num).raw pkg/campgns/dzjr10lv_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/dzjr25lv_lnd/rgmap$(num).raw pkg/campgns/dzjr25lv_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/evilkeep_lnd/rgmap$(num).raw pkg/campgns/evilkeep_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/grkreign_lnd/rgmap$(num).raw pkg/campgns/grkreign_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10,pkg/campgns/jdkmaps8_lnd/rgmap$(num).raw pkg/campgns/jdkmaps8_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18,pkg/campgns/kdklvpck_lnd/rgmap$(num).raw pkg/campgns/kdklvpck_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12,pkg/campgns/lqizgood_lnd/rgmap$(num).raw pkg/campgns/lqizgood_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/lrdvexer_lnd/rgmap$(num).raw pkg/campgns/lrdvexer_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/ncastles_lnd/rgmap$(num).raw pkg/campgns/ncastles_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08,pkg/campgns/postanck_lnd/rgmap$(num).raw pkg/campgns/postanck_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09,pkg/campgns/questfth_lnd/rgmap$(num).raw pkg/campgns/questfth_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/twinkprs_lnd/rgmap$(num).raw pkg/campgns/twinkprs_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/undedkpr_lnd/rgmap$(num).raw pkg/campgns/undedkpr_lnd/viframe$(num).dat)
GUIDATTABS = \
pkg/ldata/dkflag00.dat \
pkg/ldata/netflag.dat
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

# dependencies tracking
-include obj/std/*.d
-include obj/hvlog/*.d

# mark icons as precious, because even tho we can re-create them, it requires having "png2ico" tool
.PRECIOUS: res/%.ico
# name virtual targets
.PHONY: all standard std-before std-after debug hvlog-before hvlog-after docs docsdox clean clean-build clean-tools clean-package package pkg-before pkg-copydat pkg-campaigns pkg-languages tools

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

clean: clean-build clean-tools clean-package

clean-build:
	-$(RM) $(STDOBJS) $(BIN)
	-$(RM) $(HVLOGOBJS) $(HVLOGBIN)
	-$(RM) $(LIBS) $(GENSRC)
	-$(RM) bin/keeperfx*
	-$(ECHO) ' '

clean-tools:
	$(MAKE) clean -C peresec
	$(MAKE) clean -C tools/pngpal2raw
	$(MAKE) clean -C tools/png2bestpal
	$(MAKE) clean -C tools/po2ngdat
	$(MAKE) clean -C tools/rnctools

clean-package:
	-$(RM) -R pkg/campgns
	-$(RM) -R pkg/creatrs
	-$(RM) -R pkg/fxdata
	-$(RM) -R pkg/ldata
	-$(RM) pkg/keeperfx*
	-$(ECHO) ' '

package: pkg-before pkg-copydat pkg-campaigns pkg-languages
	$(CP) bin/* pkg/
	$(CP) docs/keeperfx_readme.txt pkg/
	cd pkg; \
	7z a "keeperfx_ccp-$(subst .,_,$(VER_STRING))-patch.7z" "*" -x!*/.svn -x!.svn -x!*.7z

pkg-before:
	-$(RM) "pkg/keeperfx_ccp-$(subst .,_,$(VER_STRING))-patch.7z"
	$(MKDIR) pkg/creatrs
	$(MKDIR) pkg/fxdata
	$(MKDIR) pkg/ldata

pkg-copydat:
	$(CP) config/keeperfx.cfg pkg/
	$(CP) config/creatrs/*.cfg pkg/creatrs/
	$(CP) config/fxdata/*.cfg pkg/fxdata/
	$(CP) config/ldata/*.txt pkg/ldata/

pkg-campaigns: $(CAMPAIGN_CFGS)

pkg-languages: $(NGTEXTDATS) $(NCTEXTDATS)

pkg-gfx: pkg-landviews pkg-guidattabs

pkg-landviews: $(LANDVIEWRAWS)

pkg-guidattabs: $(GUIDATTABS)

pkg/campgns/%.cfg: campgns/%.cfg
	@$(MKDIR) $(@D)
#	 Copy folder with campaign name (w/o extension), if it exists
	$(if $(wildcard $(<:%.cfg=%)),$(MKDIR) $(@:%.cfg=%))
	$(if $(wildcard $(<:%.cfg=%)),-$(CP) $(<:%.cfg=%)/map*.* $(@:%.cfg=%)/)
#	 Copy folder with campaign name and _cfgs ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_cfgs)),$(MKDIR) $(@:%.cfg=%_cfgs))
	$(if $(wildcard $(<:%.cfg=%_cfgs)),-$(CP) $(<:%.cfg=%_cfgs)/*.cfg $(@:%.cfg=%_cfgs)/)
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

tools: $(EXETODLL) $(PNGTORAW) $(PNGTOBSPAL) $(POTONGDAT) $(RNC) $(DERNC)

$(EXETODLL): peresec/src/peresec.c peresec/Makefile
	$(MAKE) -C peresec

$(PNGTORAW): tools/pngpal2raw/src/pngpal2raw.cpp tools/pngpal2raw/Makefile
	$(MAKE) -C tools/pngpal2raw

$(PNGTOBSPAL): tools/png2bestpal/src/png2bestpal.cpp tools/png2bestpal/Makefile
	$(MAKE) -C tools/png2bestpal

$(POTONGDAT): tools/po2ngdat/src/po2ngdat.cpp tools/po2ngdat/Makefile
	$(MAKE) -C tools/po2ngdat

$(RNC) $(DERNC): tools/rnctools/src/rnc.c tools/rnctools/src/dernc.c tools/rnctools/Makefile
	$(MAKE) -C tools/rnctools

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

# Creation of engine language files from PO/POT files
pkg/fxdata/gtext_jap.dat: lang/gtext_jap.po $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "tools/po2ngdat/res/char_encoding_tbl_jp.txt" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# Creation of engine language files from PO/POT files
pkg/fxdata/gtext_rus.dat: lang/gtext_rus.po $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "tools/po2ngdat/res/char_encoding_tbl_ru.txt" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_chi.dat: lang/gtext_chi.po $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "tools/po2ngdat/res/char_encoding_tbl_ch.txt" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_cht.dat: lang/gtext_cht.po $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "tools/po2ngdat/res/char_encoding_tbl_ch.txt" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# Creation of engine language files from PO files
pkg/fxdata/%.dat: lang/%.po $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "tools/po2ngdat/res/char_encoding_tbl_eu.txt" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# Creation of engine language files from POT files
pkg/fxdata/%.dat: lang/%.pot $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "tools/po2ngdat/res/char_encoding_tbl_eu.txt" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# Creation of engine language files for campaigns
define define_campaign_language_rule
pkg/campgns/$(1)/%.dat: lang/$(1)/%.po $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "tools/po2ngdat/res/char_encoding_tbl_eu.txt" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)/%.dat: lang/$(1)/%.pot $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "tools/po2ngdat/res/char_encoding_tbl_eu.txt" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '
endef

$(foreach campaign,$(sort $(CAMPAIGNS)),$(eval $(call define_campaign_language_rule,$(campaign))))

# Creation of land view image files for campaigns
define define_campaign_landview_rule
pkg/campgns/$(1)_lnd/rgmap%.pal: gfx/$(1)_lnd/rgmap%.png gfx/$(1)_lnd/viframe.png tools/png2bestpal/res/color_tbl_landview.txt $$(PNGTOBSPAL)
	-$$(ECHO) 'Building land view palette: $$@'
	@$$(MKDIR) $$(@D)
	$$(PNGTOBSPAL) -o "$$@" -m "$$(word 3,$$^)" "$$(word 1,$$^)" "$$(word 2,$$^)"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)_lnd/rgmap%.raw: gfx/$(1)_lnd/rgmap%.png pkg/campgns/$(1)_lnd/rgmap%.pal $$(PNGTORAW) $$(RNC)
	-$$(ECHO) 'Building land view image: $$@'
	$$(PNGTORAW) -o "$$@" -p "$$(word 2,$$^)" -f raw -l 100 "$$<"
	-$$(RNC) "$$@"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)_lnd/viframe%.dat: gfx/$(1)_lnd/viframe.png pkg/campgns/$(1)_lnd/rgmap%.pal $$(PNGTORAW) $$(RNC)
	-$$(ECHO) 'Building land view frame: $$@'
	$$(PNGTORAW) -o "$$@" -p "$$(word 2,$$^)" -f hspr -l 50 "$$<"
	-$$(RNC) "$$@"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

# mark palette files precious to make sure they're not auto-removed after raw is built
.PRECIOUS: pkg/campgns/$(1)_lnd/rgmap%.pal
endef

$(foreach campaign,$(sort $(CAMPAIGNS)),$(eval $(call define_campaign_landview_rule,$(campaign))))

pkg/ldata/dkflag00.dat: gfx/dkflag00/filelist.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
	-$(ECHO) 'Building tabulated sprites: $@'
	@$(MKDIR) $(@D)
	$(PNGTORAW) -b -o "$@" -p "$(word 2,$^)" -f sspr -l 0 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/ldata/netflag.dat: gfx/netflag/filelist.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
	-$(ECHO) 'Building tabulated sprites: $@'
	@$(MKDIR) $(@D)
	$(PNGTORAW) -b -o "$@" -p "$(word 2,$^)" -f sspr -l 0 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

#******************************************************************************
