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
WAVTODAT = tools/sndbanker/bin/sndbanker$(CROSS_EXEEXT)
DKILLTOLVL = tools/dkillconv/bin/dkillcmpl$(CROSS_EXEEXT)
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
obj/bflib_vidraw_spr_norm.o \
obj/bflib_vidraw_spr_remp.o \
obj/bflib_vidraw_spr_onec.o \
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
obj/bflib_netsession.o \
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
obj/bflib_main.o \
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
obj/front_fmvids.o \
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
obj/player_computer_data.o \
obj/player_newcompchecks.o \
obj/player_newcompeval.o \
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
obj/engine_textures.o \
obj/engine_redraw.o \
obj/front_landview.o \
obj/front_highscore.o \
obj/front_lvlstats.o \
obj/front_lvlstats_data.o \
obj/front_easter.o \
obj/frontmenu_net.o \
obj/frontmenu_net_data.o \
obj/frontmenu_options.o \
obj/frontmenu_options_data.o \
obj/frontmenu_saves.o \
obj/frontmenu_saves_data.o \
obj/frontmenu_select.o \
obj/frontmenu_select_data.o \
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
obj/creature_jobs.o \
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
obj/config_effects.o \
obj/config_rules.o \
obj/config_players.o \
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
obj/ariadne_wallhug.o \
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
obj/vidmode_data.o \
obj/vidfade.o \
obj/KeeperSpeechImp.o \
obj/music_player.o \
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

LANGS = eng chi cht cze dut fre ger ita jpn lat pol rus spa swe

NGTEXTDATS = \
pkg/fxdata/gtext_chi.dat \
pkg/fxdata/gtext_cht.dat \
pkg/fxdata/gtext_cze.dat \
pkg/fxdata/gtext_dut.dat \
pkg/fxdata/gtext_fre.dat \
pkg/fxdata/gtext_ger.dat \
pkg/fxdata/gtext_ita.dat \
pkg/fxdata/gtext_jpn.dat \
pkg/fxdata/gtext_lat.dat \
pkg/fxdata/gtext_pol.dat \
pkg/fxdata/gtext_rus.dat \
pkg/fxdata/gtext_spa.dat \
pkg/fxdata/gtext_swe.dat \
pkg/fxdata/gtext_eng.dat

NCTEXTDATS = \
pkg/campgns/ancntkpr/text_eng.dat \
pkg/campgns/ancntkpr/text_fre.dat \
pkg/campgns/ancntkpr/text_ger.dat \
pkg/campgns/ancntkpr/text_pol.dat \
pkg/campgns/burdnimp/text_eng.dat \
pkg/campgns/burdnimp/text_pol.dat \
pkg/campgns/cqarctic/text_eng.dat \
pkg/campgns/cqarctic/text_pol.dat \
pkg/campgns/dstninja/text_eng.dat \
pkg/campgns/dstninja/text_pol.dat \
pkg/campgns/evilkeep/text_eng.dat \
pkg/campgns/evilkeep/text_fre.dat \
pkg/campgns/evilkeep/text_ger.dat \
pkg/campgns/evilkeep/text_pol.dat \
pkg/campgns/grkreign/text_eng.dat \
pkg/campgns/grkreign/text_pol.dat \
pkg/campgns/lqizgood/text_eng.dat \
pkg/campgns/lqizgood/text_pol.dat \
pkg/campgns/ncastles/text_eng.dat \
pkg/campgns/ncastles/text_pol.dat \
pkg/campgns/pstunded/text_eng.dat \
pkg/campgns/pstunded/text_pol.dat \
pkg/campgns/questfth/text_eng.dat \
pkg/campgns/questfth/text_fre.dat \
pkg/campgns/questfth/text_pol.dat \
pkg/campgns/twinkprs/text_eng.dat \
pkg/campgns/twinkprs/text_pol.dat \
pkg/campgns/undedkpr/text_eng.dat \
pkg/campgns/undedkpr/text_chi.dat \
pkg/campgns/undedkpr/text_pol.dat

NGSPEECHBANKS = \
speech_chi \
speech_cht \
speech_dut \
speech_eng \
speech_fre \
speech_ger \
speech_ita \
speech_jpn \
speech_lat \
speech_pol \
speech_rus \
speech_spa \
speech_swe

NGSOUNDDATS = $(patsubst %,pkg/sound/%.dat,$(NGSPEECHBANKS) sound)

NGSOUNDLISTS = $(patsubst %,sfx/%/filelist.txt,$(NGSPEECHBANKS) sound)

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
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/pstunded_lnd/rgmap$(num).raw pkg/campgns/pstunded_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09,pkg/campgns/questfth_lnd/rgmap$(num).raw pkg/campgns/questfth_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/twinkprs_lnd/rgmap$(num).raw pkg/campgns/twinkprs_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/undedkpr_lnd/rgmap$(num).raw pkg/campgns/undedkpr_lnd/viframe$(num).dat)

LANDVIEWDATTABS = \
pkg/ldata/lndflag_ens.dat \
pkg/ldata/netflag_ens.dat \
pkg/ldata/lndflag_pin.dat \
pkg/ldata/netflag_pin.dat \
pkg/ldata/maphand.dat \
pkg/ldata/netfont.dat

LANDVIEWSPEECH = \
$(foreach lng,eng,ancntkpr_$(lng)) \
$(foreach lng,eng,burdnimp_$(lng)) \
$(foreach lng,eng,cqarctic_$(lng)) \
$(foreach lng,eng,dstninja_$(lng)) \
$(foreach lng,eng dut,dzjr06lv_$(lng)) \
$(foreach lng,eng,dzjr10lv_$(lng)) \
$(foreach lng,eng,dzjr25lv_$(lng)) \
$(foreach lng,eng fre ger,evilkeep_$(lng)) \
$(foreach lng,eng,grkreign_$(lng)) \
$(foreach lng,eng,jdkmaps8_$(lng)) \
$(foreach lng,eng,kdklvpck_$(lng)) \
$(foreach lng,eng chi cht dut fre ger ita jpn pol rus spa swe,keeporig_$(lng)) \
$(foreach lng,eng dut,lqizgood_$(lng)) \
$(foreach lng,eng,lrdvexer_$(lng)) \
$(foreach lng,eng,ncastles_$(lng)) \
$(foreach lng,eng,postanck_$(lng)) \
$(foreach lng,eng,pstunded_$(lng)) \
$(foreach lng,eng,questfth_$(lng)) \
$(foreach lng,eng dut,twinkprs_$(lng)) \
$(foreach lng,eng,undedkpr_$(lng))

LANDVIEWSPEECHDIRS = $(patsubst %,pkg/campgns/%,$(LANDVIEWSPEECH))

TOTRUREGFX = \
pkg/ldata/torture.raw \
pkg/ldata/door01.dat \
pkg/ldata/door02.dat \
pkg/ldata/door03.dat \
pkg/ldata/door04.dat \
pkg/ldata/door05.dat \
pkg/ldata/door06.dat \
pkg/ldata/door07.dat \
pkg/ldata/door08.dat \
pkg/ldata/door09.dat \
pkg/ldata/fronttor.dat

FRONTENDGFX = \
pkg/data/legal32.raw \
pkg/data/legal64.raw \
pkg/data/startfx32.raw \
pkg/data/startfx64.raw \
pkg/data/loading32.raw \
pkg/data/loading64.raw \
pkg/data/nocd.raw \
pkg/ldata/front.raw \
pkg/ldata/frontft1.dat \
pkg/ldata/frontft2.dat \
pkg/ldata/frontft3.dat \
pkg/ldata/frontft4.dat \
pkg/ldata/frontbit.dat

ENGINEGFX = \
pkg/data/thingspr-32.jty \
pkg/data/thingspr-128.jty \
$(foreach miplv,32 128,$(foreach num,00 01 02 03 04 05 06 07 08,pkg/data/frac$(num)-$(miplv).raw)) \
pkg/data/gui2-64.dat \
pkg/data/gui2-32.dat \
pkg/data/gui1-64.dat \
pkg/data/gui1-32.dat \
pkg/data/pointer64.dat \
pkg/data/pointer32.dat \
pkg/data/points64.dat \
pkg/data/points32.dat \
pkg/data/font1-64.dat \
pkg/data/font1-32.dat \
pkg/data/font2-32.dat \
pkg/data/font2-64.dat \
$(foreach miplv,32 128,$(foreach num,000 001 002 003 004 005 006 007,pkg/data/tmapa$(num)-$(miplv).dat)) \
$(foreach miplv,32 128,$(foreach num,01 02 03 04 05 06,pkg/data/swipe$(num)-$(miplv).dat)) \
pkg/data/gmap64.raw \
pkg/data/gmap32.raw \
pkg/data/gmapbug.dat

STDOBJS   = $(subst obj/,obj/std/,$(OBJS))
HVLOGOBJS = $(subst obj/,obj/hvlog/,$(OBJS))

# load program version
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(VER_BUILD)
# include and library directories
LINKLIB =  -L"directx/lib" -L"lib" -L"sdl/lib" -mwindows -lmingw32 -lkeeperfx -lwinmm -limagehlp -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_net
INCS =  -I"directx/include" -I"sdl/include"
CXXINCS =  -I"directx/include" -I"sdl/include"

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
WARNFLAGS = -Wall -Wno-sign-compare -Wno-unused-parameter -Wno-strict-aliasing -Wno-unknown-pragmas
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
.PHONY: all standard std-before std-after debug hvlog-before hvlog-after docs docsdox clean clean-build clean-tools clean-package package pkg-before pkg-copydat pkg-campaigns pkg-languages tools convert-sfx

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
	-$(RM) $(STDOBJS)
	-$(RM) $(HVLOGOBJS)
	-$(RM) $(BIN) $(BIN:%.exe=%.map)
	-$(RM) $(HVLOGBIN) $(HVLOGBIN:%.exe=%.map)
	-$(RM) bin/keeperfx.dll
	-$(RM) $(LIBS) $(GENSRC)
	-$(ECHO) ' '

clean-tools:
	$(MAKE) clean -C peresec
	$(MAKE) clean -C tools/pngpal2raw
	$(MAKE) clean -C tools/png2bestpal
	$(MAKE) clean -C tools/po2ngdat
	$(MAKE) clean -C tools/rnctools
	$(MAKE) clean -C tools/sndbanker
	$(MAKE) clean -C tools/dkillconv

clean-package:
	-$(RM) -R pkg/campgns
	-$(RM) -R pkg/creatrs
	-$(RM) -R pkg/fxdata
	-$(RM) -R pkg/ldata
	-$(RM) -R pkg/data
	-$(RM) -R pkg/sound
	-$(RM) -R pkg/levels
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

pkg-campaigns: $(CAMPAIGN_CFGS)

pkg-languages: $(NGTEXTDATS) $(NCTEXTDATS)

pkg-gfx: pkg-landviews pkg-menugfx pkg-enginegfx

pkg-sfx: $(NGSOUNDDATS) $(LANDVIEWSPEECHDIRS)

pkg-landviews: $(LANDVIEWRAWS) $(LANDVIEWDATTABS)

pkg-menugfx: $(TOTRUREGFX) $(FRONTENDGFX)

pkg-enginegfx: $(ENGINEGFX)

# Special target for original campaign - placing levels to main folder
pkg/campgns/keeporig.cfg: campgns/keeporig.cfg
	@$(MKDIR) $(@D)
#	 Copy folder with campaign name (w/o extension) to main levels folder
	@$(MKDIR) pkg/levels
	$(if $(wildcard $(<:%.cfg=%)),-$(CP) $(<:%.cfg=%)/map*.* pkg/levels/)
#	 Copy folder with campaign name and _lnd ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_lnd)),$(MKDIR) $(@:%.cfg=%_lnd))
	$(if $(wildcard $(<:%.cfg=%_lnd)),-$(CP) $(<:%.cfg=%_lnd)/*.txt $(@:%.cfg=%_lnd)/)
#	 Copy the actual campaign file
	$(CP) $< $@

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

tools: $(EXETODLL) $(PNGTORAW) $(PNGTOBSPAL) $(POTONGDAT) $(RNC) $(DERNC) $(WAVTODAT) $(DKILLTOLVL)

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

$(WAVTODAT): tools/sndbanker/src/sndbanker.cpp tools/sndbanker/Makefile
	$(MAKE) -C tools/sndbanker

$(DKILLTOLVL): tools/dkillconv/src/dkillconv.cpp tools/dkillconv/Makefile
	$(MAKE) -C tools/dkillconv

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
pkg/fxdata/gtext_jpn.dat: lang/gtext_jpn.po tools/po2ngdat/res/char_encoding_tbl_jp.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_rus.dat: lang/gtext_rus.po tools/po2ngdat/res/char_encoding_tbl_ru.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_chi.dat: lang/gtext_chi.po tools/po2ngdat/res/char_encoding_tbl_ch.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_cht.dat: lang/gtext_cht.po tools/po2ngdat/res/char_encoding_tbl_ch.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_%.dat: lang/gtext_%.po tools/po2ngdat/res/char_encoding_tbl_eu.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_%.dat: lang/gtext_%.pot tools/po2ngdat/res/char_encoding_tbl_eu.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# Creation of engine language files for campaigns
define define_campaign_language_rule
pkg/campgns/$(1)/%.dat: lang/$(1)/%.po tools/po2ngdat/res/char_encoding_tbl_eu.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)/%.dat: lang/$(1)/%.pot tools/po2ngdat/res/char_encoding_tbl_eu.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
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

# mark palette files precious to make sure they're not auto-removed after dependencies are built
.PRECIOUS: pkg/campgns/$(1)_lnd/rgmap%.pal
endef

$(foreach campaign,$(sort $(CAMPAIGNS)),$(eval $(call define_campaign_landview_rule,$(campaign))))

# Creation of land view speeches for campaigns
define define_campaign_speeches_rule
pkg/campgns/$(1)_$(2): sfx/campgns/$(1)_$(2)/filelist.txt
	-$(ECHO) 'Copying campaign SFX: $$@'
	@$(MKDIR) "$$@"
	tail -n +2 "$$<" | cut -f1 | xargs -d '\n' -I {} $(CP) "$$(<D)/{}" "$$@/"
	-$(ECHO) 'Finished copying: $$@'
	-$(ECHO) ' '

endef

$(foreach campaign,$(sort $(CAMPAIGNS)),$(foreach lng,$(sort $(LANGS)),$(eval $(call define_campaign_speeches_rule,$(campaign),$(lng)))))

pkg/ldata/torture.pal: gfx/torturescr/tortr_background.png gfx/torturescr/tortr_doora_open11.png gfx/torturescr/tortr_doorb_open11.png gfx/torturescr/tortr_doorc_open11.png gfx/torturescr/tortr_doord_open11.png gfx/torturescr/tortr_doore_open11.png gfx/torturescr/tortr_doorf_open11.png gfx/torturescr/tortr_doorg_open11.png gfx/torturescr/tortr_doorh_open11.png gfx/torturescr/tortr_doori_open11.png gfx/torturescr/cursor_horny.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/legal32.pal: gfx/loading/legal32.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/legal64.pal: gfx/loading/legal64.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/startfx32.pal: gfx/loading/startupfx32.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/startfx64.pal: gfx/loading/startupfx64.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/loading32.pal: gfx/loading/loading32.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/loading64.pal: gfx/loading/loading64.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/nocd.pal: gfx/loading/nocd32.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)

pkg/ldata/torture.pal pkg/data/legal32.pal pkg/data/legal64.pal pkg/data/startfx32.pal pkg/data/startfx64.pal pkg/data/loading32.pal pkg/data/loading64.pal pkg/data/nocd.pal:
	-$(ECHO) 'Building palette: $@'
	@$(MKDIR) $(@D)
	$(PNGTOBSPAL) -o "$@" -m "$(filter %.txt,$^)" $(filter %.png,$^)
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/ldata/front.pal: gfx/palettes/front.pal
pkg/data/palette.dat: gfx/palettes/engine.pal

pkg/ldata/front.pal pkg/data/palette.dat:
	-$(ECHO) 'Building palette: $@'
	@$(MKDIR) $(@D)
	# Simplified, for now
	$(CP) "$<" "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# mark palette files precious to make sure they're not auto-removed after dependencies are built
.PRECIOUS: pkg/ldata/torture.pal pkg/ldata/front.pal pkg/data/palette.dat

pkg/ldata/lndflag_ens.dat: gfx/landview_ensign/filelist_lndflag.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/netflag_ens.dat: gfx/landview_ensign/filelist_netflag.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/lndflag_pin.dat: gfx/landview_pinpnt/filelist_lndflag.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/netflag_pin.dat: gfx/landview_pinpnt/filelist_netflag.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/maphand.dat: gfx/landview_hand/filelist_maphand.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/netfont.dat: gfx/font_net/filelist_netfont.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)

pkg/ldata/fronttor.dat: gfx/torturescr/filelist_fronttor.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door01.dat: gfx/torturescr/filelist_tortr_doora.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door02.dat: gfx/torturescr/filelist_tortr_doorb.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door03.dat: gfx/torturescr/filelist_tortr_doorc.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door04.dat: gfx/torturescr/filelist_tortr_doord.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door05.dat: gfx/torturescr/filelist_tortr_doore.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door06.dat: gfx/torturescr/filelist_tortr_doorf.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door07.dat: gfx/torturescr/filelist_tortr_doorg.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door08.dat: gfx/torturescr/filelist_tortr_doorh.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door09.dat: gfx/torturescr/filelist_tortr_doori.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/torture.raw: gfx/torturescr/tortr_background.png pkg/ldata/torture.pal $(PNGTORAW) $(RNC)

pkg/ldata/front.raw: gfx/frontend-64/front_background.png pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontbit.dat: gfx/frontend-64/filelist_frontbit.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft1.dat: gfx/font_front_hdr_red-64/filelist_frontft1.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft2.dat: gfx/font_front_std_red-64/filelist_frontft2.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft3.dat: gfx/font_front_std_ylw-64/filelist_frontft3.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft4.dat: gfx/font_front_std_dkr-64/filelist_frontft4.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)

define define_gfx_texture_map_rule
pkg/data/frac00-$(1).raw: gfx/textures-$(1)/frac00.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac01-$(1).raw: gfx/textures-$(1)/frac01.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac02-$(1).raw: gfx/textures-$(1)/frac02.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac03-$(1).raw: gfx/textures-$(1)/frac03.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac04-$(1).raw: gfx/textures-$(1)/frac04.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac05-$(1).raw: gfx/textures-$(1)/frac05.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac06-$(1).raw: gfx/textures-$(1)/frac06.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac07-$(1).raw: gfx/textures-$(1)/frac07.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac08-$(1).raw: gfx/textures-$(1)/frac08.png gfx/textures-$(1)/fract_bw.pal $(PNGTORAW) $(RNC)

pkg/data/tmapa000-$(1).dat: gfx/textures-$(1)/filelist_tmapa000.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa001-$(1).dat: gfx/textures-$(1)/filelist_tmapa001.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa002-$(1).dat: gfx/textures-$(1)/filelist_tmapa002.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa003-$(1).dat: gfx/textures-$(1)/filelist_tmapa003.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa004-$(1).dat: gfx/textures-$(1)/filelist_tmapa004.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa005-$(1).dat: gfx/textures-$(1)/filelist_tmapa005.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa006-$(1).dat: gfx/textures-$(1)/filelist_tmapa006.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa007-$(1).dat: gfx/textures-$(1)/filelist_tmapa007.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
endef

$(foreach miplv,32 128,$(eval $(call define_gfx_texture_map_rule,$(miplv))))

pkg/data/gmap32.raw: gfx/guimap/gmap32.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gmap64.raw: gfx/guimap/gmap64.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gmapbug.dat: gfx/parchmentbug/filelist-gbug.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/legal32.raw: gfx/loading/legal32.png pkg/data/legal32.pal $(PNGTORAW) $(RNC)
pkg/data/legal64.raw: gfx/loading/legal64.png pkg/data/legal64.pal $(PNGTORAW) $(RNC)
pkg/data/startfx32.raw: gfx/loading/startupfx32.png pkg/data/startfx32.pal $(PNGTORAW) $(RNC)
pkg/data/startfx64.raw: gfx/loading/startupfx64.png pkg/data/startfx64.pal $(PNGTORAW) $(RNC)
pkg/data/loading32.raw: gfx/loading/loading32.png pkg/data/loading32.pal $(PNGTORAW) $(RNC)
pkg/data/loading64.raw: gfx/loading/loading64.png pkg/data/loading64.pal $(PNGTORAW) $(RNC)
pkg/data/nocd.raw: gfx/loading/nocd32.png pkg/data/nocd.pal $(PNGTORAW) $(RNC)
pkg/data/pointer64.dat: gfx/pointer-64/filelist_pointer.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/pointer32.dat: gfx/pointer-64/filelist_lpointer.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/points64.dat: gfx/pointer-64/filelist_points.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/points32.dat: gfx/pointer-64/filelist_lpoints.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)

pkg/data/thingspr-32.jty: gfx/sprites-32/animlist.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/thingspr-128.jty: gfx/sprites-128/animlist.txt pkg/data/palette.dat $(PNGTORAW)

define define_gfx_font_n_pointers_rule
pkg/data/gui2-$(1).dat: gfx/gui2-$(1)/filelist_gui2.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui1-$(1).dat: gfx/gui1-$(1)/filelist_gui1.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/font1-$(1).dat: gfx/font_simp-$(1)/filelist_font1.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/font2-$(1).dat: gfx/font2-$(1)/filelist_font2.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
endef

$(foreach miplv,32 64,$(eval $(call define_gfx_font_n_pointers_rule,$(miplv))))

define define_gfx_swipe_rule
pkg/data/swipe01-$(1).dat: gfx/swipes-$(1)/filelist_bhandrl.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swipe02-$(1).dat: gfx/swipes-$(1)/filelist_swordrl.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swipe03-$(1).dat: gfx/swipes-$(1)/filelist_scythlr.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swipe04-$(1).dat: gfx/swipes-$(1)/filelist_sticklr.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swipe05-$(1).dat: gfx/swipes-$(1)/filelist_stickrl.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swipe06-$(1).dat: gfx/swipes-$(1)/filelist_clawsrl.txt pkg/data/palette.dat $(PNGTORAW)
endef

$(foreach miplv,32 128,$(eval $(call define_gfx_swipe_rule,$(miplv))))

pkg/sound/sound.dat: sfx/sound/filelist.txt $(WAVTODAT)
pkg/sound/speech_chi.dat: sfx/speech_chi/filelist.txt $(WAVTODAT)
pkg/sound/speech_cht.dat: sfx/speech_cht/filelist.txt $(WAVTODAT)
pkg/sound/speech_dut.dat: sfx/speech_dut/filelist.txt $(WAVTODAT)
pkg/sound/speech_eng.dat: sfx/speech_eng/filelist.txt $(WAVTODAT)
pkg/sound/speech_fre.dat: sfx/speech_fre/filelist.txt $(WAVTODAT)
pkg/sound/speech_ger.dat: sfx/speech_ger/filelist.txt $(WAVTODAT)
pkg/sound/speech_ita.dat: sfx/speech_ita/filelist.txt $(WAVTODAT)
pkg/sound/speech_jpn.dat: sfx/speech_jpn/filelist.txt $(WAVTODAT)
pkg/sound/speech_lat.dat: sfx/speech_lat/filelist.txt $(WAVTODAT)
pkg/sound/speech_pol.dat: sfx/speech_pol/filelist.txt $(WAVTODAT)
pkg/sound/speech_rus.dat: sfx/speech_rus/filelist.txt $(WAVTODAT)
pkg/sound/speech_spa.dat: sfx/speech_spa/filelist.txt $(WAVTODAT)
pkg/sound/speech_swe.dat: sfx/speech_swe/filelist.txt $(WAVTODAT)

pkg/data/frac%.raw:
	-$(ECHO) 'Building RAW texture: $@'
	$(PNGTORAW) -o "$@" -p "$(word 2,$^)" -r 255 -f raw -l 100 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/data/tmapa%.dat:
	-$(ECHO) 'Building RAW texture: $@'
	$(PNGTORAW) -b -o "$@" -p "$(word 2,$^)" -f raw -l 0 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/ldata/%.raw pkg/data/%.raw:
	-$(ECHO) 'Building RAW image: $@'
	$(PNGTORAW) -o "$@" -p "$(word 2,$^)" -f raw -l 100 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/ldata/%.dat pkg/data/%.dat:
	-$(ECHO) 'Building tabulated sprites: $@'
	@$(MKDIR) "$(@D)"
	$(PNGTORAW) -b -o "$@" -p "$(word 2,$^)" -f sspr2 -l 0 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/creatrs/%.jty pkg/data/%.jty:
	-$(ECHO) 'Building jonty sprites: $@'
	@$(MKDIR) "$(@D)"
	$(PNGTORAW) -m -o "$@" -p "$(word 2,$^)" -f jspr2 -l 0 "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

convert-sfx: $(patsubst %,convert-speech-sfx-%,$(NGSPEECHBANKS)) $(patsubst %,convert-campaign-sfx-%,$(LANDVIEWSPEECH))

convert-speech-sfx-%: sfx/%/filelist.txt
	-$(ECHO) 'Converting speech samples in list: $<'
	tail -n +2 "$<" | cut -f1 | xargs -d '\n' -I {} sox "$(<D)/design/{}" -c 1 -b 8 -r 22050 -e unsigned-integer "$(<D)/{}" compand 0.02,0.20 5:-40,-40,-35,-20,-10 -6 -90 0.1 gain -n -0.1
#	best would be "compand 0.02,0.20 5:-60,-40,-10 -6 -90 0.1", modification is to skip noise
	-$(ECHO) 'Finished converting list: $<'
	-$(ECHO) ' '

convert-campaign-sfx-%: sfx/campgns/%/filelist.txt
	-$(ECHO) 'Converting campaign speeches in list: $<'
	tail -n +2 "$<" | cut -f1 | xargs -d '\n' -I {} sox "$(<D)/design/{}" -c 1 -r 22050 -e ms-adpcm "$(<D)/{}" compand 0.02,0.20 5:-40,-40,-35,-20,-10 -6 -90 0.1 gain -n -0.1
#	best would be "compand 0.02,0.20 5:-60,-40,-10 -6 -90 0.1", modification is to skip noise
	-$(ECHO) 'Finished converting list: $<'
	-$(ECHO) ' '

pkg/sound/%.dat:
	-$(ECHO) 'Building sound bank: $@'
	@$(MKDIR) $(@D)
	$(WAVTODAT) -o "$@" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

#******************************************************************************
