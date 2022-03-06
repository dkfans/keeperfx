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
# Executable files extension on host environment
ifneq (,$(findstring Windows,$(OS)))
  CROSS_EXEEXT = .exe
  # linker flags
  LINKFLAGS = -static-libgcc -static-libstdc++ -Wl,--enable-auto-import -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
  # The following flags are only here to prevent a dependency on libwinpthread-1.dll when keeperfx is built with MSYS2:
  # "-Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
else
  CROSS_EXEEXT =
  CROSS_COMPILE = i686-w64-mingw32-
  LINKFLAGS = -static-libgcc -static-libstdc++ -Wl,--enable-auto-import
endif
# Executable files extension on target environment
EXEEXT = .exe
# Names of utility commands
CPP      = $(CROSS_COMPILE)g++
CC       = $(CROSS_COMPILE)gcc
WINDRES  = $(CROSS_COMPILE)windres
DLLTOOL  = $(CROSS_COMPILE)dlltool
EXETODLL = tools/peresec/bin/peresec$(CROSS_EXEEXT)
DOXYTOOL = doxygen
BUILD_NUMBER ?= $(VER_BUILD)
PACKAGE_SUFFIX ?= Prototype
PNGTOICO = tools/png2ico/png2ico$(CROSS_EXEEXT)
PNGTORAW = tools/pngpal2raw/bin/pngpal2raw$(CROSS_EXEEXT)
PNGTOBSPAL = tools/png2bestpal/bin/png2bestpal$(CROSS_EXEEXT)
POTONGDAT = tools/po2ngdat/bin/po2ngdat$(CROSS_EXEEXT)
WAVTODAT = tools/sndbanker/bin/sndbanker$(CROSS_EXEEXT)
RNC      = tools/rnctools/bin/rnc$(CROSS_EXEEXT)
DERNC    = tools/rnctools/bin/dernc$(CROSS_EXEEXT)
DKILLTOLVL = tools/dkillconv/bin/dkillcmpl$(CROSS_EXEEXT)
RM       = rm -f
MV       = mv -f
CP       = cp -f
MKDIR    = mkdir -p
ECHO     = @echo

# Names of target binary files
BIN      = bin/keeperfx$(EXEEXT)
TEST_BIN = bin/tests$(EXEEXT)
HVLOGBIN = bin/keeperfx_hvlog$(EXEEXT)
# Names of intermediate build products
GENSRC   = obj/ver_defs.h
RES      = obj/keeperfx_stdres.res
LIBS     = obj/libkeeperfx.a

DEPS = \
obj/spng.o \
obj/json/json.o \
obj/json/value.o \
obj/json/json-dom.o \
obj/unzip.o \
obj/ioapi.o

OBJS = \
$(DEPS) \
obj/actionpt.o \
obj/ariadne.o \
obj/ariadne_edge.o \
obj/ariadne_findcache.o \
obj/ariadne_naviheap.o \
obj/ariadne_navitree.o \
obj/ariadne_points.o \
obj/ariadne_regions.o \
obj/ariadne_tringls.o \
obj/ariadne_wallhug.o \
obj/bflib_base_tcp.o \
obj/bflib_basics.o \
obj/bflib_bufrw.o \
obj/bflib_coroutine.o \
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
obj/bflib_main.o \
obj/bflib_math.o \
obj/bflib_memory.o \
obj/bflib_mouse.o \
obj/bflib_mshandler.o \
obj/bflib_mspointer.o \
obj/bflib_nethost_udp.o \
obj/bflib_netlisten_udp.o \
obj/bflib_netsession.o \
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
obj/bflib_vidraw_spr_norm.o \
obj/bflib_vidraw_spr_onec.o \
obj/bflib_vidraw_spr_remp.o \
obj/bflib_vidsurface.o \
obj/config.o \
obj/config_campaigns.o \
obj/config_creature.o \
obj/config_crtrmodel.o \
obj/config_crtrstates.o \
obj/config_lenses.o \
obj/config_magic.o \
obj/config_magic_data.o \
obj/config_objects.o \
obj/config_players.o \
obj/config_rules.o \
obj/config_settings.o \
obj/config_strings.o \
obj/config_terrain.o \
obj/config_cubes.o \
obj/config_trapdoor.o \
obj/console_cmd.o \
obj/custom_sprites.o \
obj/creature_battle.o \
obj/creature_control.o \
obj/creature_graphics.o \
obj/creature_groups.o \
obj/creature_instances.o \
obj/creature_jobs.o \
obj/creature_senses.o \
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
obj/dungeon_stats.o \
obj/engine_arrays.o \
obj/engine_camera.o \
obj/engine_lenses.o \
obj/engine_redraw.o \
obj/engine_render.o \
obj/engine_render_data.o \
obj/engine_textures.o \
obj/event_monitoring.o \
obj/front_credits.o \
obj/front_credits_data.o \
obj/front_easter.o \
obj/front_fmvids.o \
obj/front_highscore.o \
obj/front_input.o \
obj/front_landview.o \
obj/front_lvlstats.o \
obj/front_lvlstats_data.o \
obj/front_network.o \
obj/front_simple.o \
obj/front_torture.o \
obj/front_torture_data.o \
obj/frontend.o \
obj/frontmenu_options_data.o \
obj/frontmenu_saves_data.o \
obj/frontmenu_select.o \
obj/frontmenu_select_data.o \
obj/frontmenu_ingame_evnt.o \
obj/frontmenu_ingame_evnt_data.o \
obj/frontmenu_ingame_map.o \
obj/frontmenu_ingame_opts.o \
obj/frontmenu_ingame_opts_data.o \
obj/frontmenu_ingame_tabs.o \
obj/frontmenu_ingame_tabs_data.o \
obj/frontmenu_net.o \
obj/frontmenu_net_data.o \
obj/frontmenu_options.o \
obj/frontmenu_saves.o \
obj/frontmenu_specials.o \
obj/game_heap.o \
obj/game_legacy.o \
obj/game_loop.o \
obj/game_lghtshdw.o \
obj/game_merge.o \
obj/game_saves.o \
obj/gui_boxmenu.o \
obj/gui_draw.o \
obj/gui_frontbtns.o \
obj/gui_frontmenu.o \
obj/gui_msgs.o \
obj/gui_parchment.o \
obj/gui_soundmsgs.o \
obj/gui_tooltips.o \
obj/gui_topmsg.o \
obj/hookfn.o \
obj/kjm_input.o \
obj/lens_api.o \
obj/config_effects.o \
obj/lens_flyeye.o \
obj/lens_mist.o \
obj/light_data.o \
obj/lvl_filesdk1.o \
obj/lvl_script.o \
obj/lvl_script_commands.o \
obj/lvl_script_commands_old.o \
obj/lvl_script_lib.o \
obj/lvl_script_conditions.o \
obj/lvl_script_value.o \
obj/magic.o \
obj/main_game.o \
obj/map_blocks.o \
obj/map_columns.o \
obj/map_data.o \
obj/map_events.o \
obj/map_locations.o \
obj/map_utils.o \
obj/music_player.o \
obj/net_game.o \
obj/net_sync.o \
obj/packets.o \
obj/packets_cheats.o \
obj/packets_input.o \
obj/packets_misc.o \
obj/player_compchecks.o \
obj/player_compevents.o \
obj/player_complookup.o \
obj/config_compp.o \
obj/player_compprocs.o \
obj/player_comptask.o \
obj/player_computer.o \
obj/player_computer_data.o \
obj/player_data.o \
obj/player_instances.o \
obj/player_states.o \
obj/player_utils.o \
obj/power_hand.o \
obj/power_process.o \
obj/power_specials.o \
obj/room_data.o \
obj/room_entrance.o \
obj/room_garden.o \
obj/room_graveyard.o \
obj/room_jobs.o \
obj/room_lair.o \
obj/room_library.o \
obj/room_list.o \
obj/room_scavenge.o \
obj/room_util.o \
obj/room_workshop.o \
obj/roomspace.o \
obj/roomspace_detection.o \
obj/scrcapt.o \
obj/slab_data.o \
obj/sounds.o \
obj/spdigger_stack.o \
obj/tasks_list.o \
obj/thing_corpses.o \
obj/thing_creature.o \
obj/thing_creature_data.o \
obj/thing_data.o \
obj/thing_doors.o \
obj/thing_effects.o \
obj/thing_factory.o \
obj/thing_list.o \
obj/thing_navigate.o \
obj/thing_objects.o \
obj/thing_physics.o \
obj/thing_shots.o \
obj/thing_stats.o \
obj/thing_traps.o \
obj/vidfade.o \
obj/vidmode_data.o \
obj/vidmode.o \
obj/KeeperSpeechImp.o \
$(RES)

MAIN_OBJ = obj/main.o

TESTS_OBJ = obj/tests/tst_main.o \
obj/tests/tst_fixes.o \
obj/tests/001_test.o

CU_DIR = deps/CUnit-2.1-3/CUnit
CU_INC = -I"$(CU_DIR)/Headers"
CU_OBJS = \
	obj/cu/Basic.o \
	obj/cu/TestDB.o \
	obj/cu/CUError.o \
	obj/cu/TestRun.o \
	obj/cu/Util.o

# include and library directories
LINKLIB =  -L"sdl/lib" -mwindows obj/libkeeperfx.a \
	-lwinmm -lmingw32 -limagehlp -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_net \
	-L"deps/zlib" -lz
INCS =  -I"sdl/include" -I"sdl/include/SDL2"
CXXINCS =  -I"sdl/include" -I"sdl/include/SDL2"

STDOBJS   = $(subst obj/,obj/std/,$(OBJS))
HVLOGOBJS = $(subst obj/,obj/hvlog/,$(OBJS))
STD_MAIN_OBJ = $(subst obj/,obj/std/,$(MAIN_OBJ))
HVLOG_MAIN_OBJ = $(subst obj/,obj/hvlog/,$(MAIN_OBJ))


# allow extracting files from archives, replacing pre-existing ones
ENABLE_EXTRACT ?= 1

# flags to generate dependency files
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -DSPNG_STATIC=1
# other flags to include while compiling
INCFLAGS =
# code optimization and debugging flags
CV2PDB := $(shell PATH=`pwd`:$$PATH command -v cv2pdb.exe 2> /dev/null)
DEBUG ?= 0
ifeq ($(DEBUG), 1)
  OPTFLAGS = -march=i686 -Og -fno-omit-frame-pointer
  DBGFLAGS = -g -DDEBUG
else
  # frame pointer is required for ASM code to work
  OPTFLAGS = -march=i686 -fno-omit-frame-pointer -O3
  # if we can create a separate debug info file then do it
  ifdef CV2PDB
    DBGFLAGS = -g
  else
    DBGFLAGS =
  endif
endif

# logging level flags
STLOGFLAGS = -DBFDEBUG_LEVEL=0
HVLOGFLAGS = -DBFDEBUG_LEVEL=10 -DAUTOTESTING=1
# compiler warning generation flags
WARNFLAGS = -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter -Wno-strict-aliasing -Wno-unknown-pragmas
# disabled warnings: -Wextra -Wtype-limits
CXXFLAGS = $(CXXINCS) -c -std=gnu++1y -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
CFLAGS = $(INCS) -c -std=gnu11 -fmessage-length=0 $(WARNFLAGS) -Werror=implicit $(DEPFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
LDFLAGS = $(LINKLIB) $(OPTFLAGS) $(DBGFLAGS) $(LINKFLAGS) -Wl,-Map,"$(@:%.exe=%.map)"

CAMPAIGNS  = \
ami2019 \
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
origplus \
postanck \
pstunded \
questfth \
revlord \
twinkprs \
undedkpr

MAPPACKS  = \
classic \
deepdngn \
legacy \
lostlvls \
standard

LANGS = eng chi cht cze dut fre ger ita jpn kor lat pol rus spa swe

# load program version
include version.mk

VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(BUILD_NUMBER) $(PACKAGE_SUFFIX)

# load depenency packages
include prebuilds.mk

# name virtual targets
.PHONY: all docs docsdox clean clean-build deep-clean
.PHONY: standard std-before std-after
.PHONY: heavylog hvlog-before hvlog-after
.PHONY: package clean-package deep-clean-package
.PHONY: tools clean-tools deep-clean-tools
.PHONY: clean-libexterns deep-clean-libexterns
.PHONY: tests

# dependencies tracking
-include $(filter %.d,$(STDOBJS:%.o=%.d))
-include $(filter %.d,$(HVLOGOBJS:%.o=%.d))

all: standard

standard: CXXFLAGS += $(STLOGFLAGS)
standard: CFLAGS += $(STLOGFLAGS)
standard: std-before $(BIN) std-after

heavylog: CXXFLAGS += $(HVLOGFLAGS)
heavylog: CFLAGS += $(HVLOGFLAGS)
heavylog: hvlog-before $(HVLOGBIN) hvlog-after

# not nice but necessary for make -j to work
$(shell $(MKDIR) bin obj/std obj/hvlog obj/tests obj/cu obj/std/json obj/hvlog/json)
std-before:
hvlog-before:

docs: docsdox

docsdox: docs/doxygen.conf
	VERSION=$(VER_STRING) $(DOXYTOOL) docs/doxygen.conf

deep-clean: deep-clean-tools deep-clean-libexterns deep-clean-package

clean: clean-build clean-tools clean-libexterns clean-package

clean-build:
	-$(RM) $(STDOBJS) $(STD_MAIN_OBJ) $(filter %.d,$(STDOBJS:%.o=%.d)) $(filter %.d,$(STD_MAIN_OBJ:%.o=%.d))
	-$(RM) $(HVLOGOBJS) $(HVLOG_MAIN_OBJ) $(filter %.d,$(HVLOGOBJS:%.o=%.d)) $(filter %.d,$(HVLOG_MAIN_OBJ:%.o=%.d))
	-$(RM) $(BIN) $(BIN:%.exe=%.map)
	-$(RM) $(BIN) $(BIN:%.exe=%.pdb)
	-$(RM) $(HVLOGBIN) $(HVLOGBIN:%.exe=%.map)
	-$(RM) $(HVLOGBIN) $(HVLOGBIN:%.exe=%.pdb)
	-$(RM) bin/keeperfx.dll
	-$(RM) $(LIBS) $(GENSRC)
	-$(RM) res/*.ico
	-$(RM) obj/keeperfx.*

$(BIN): $(GENSRC) $(STDOBJS) $(STD_MAIN_OBJ) $(LIBS) std-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(STDOBJS) $(STD_MAIN_OBJ) $(LDFLAGS)
ifdef CV2PDB
	$(CV2PDB) -C "$@"
endif
	-$(ECHO) ' '

$(HVLOGBIN): $(GENSRC) $(HVLOGOBJS) $(HVLOG_MAIN_OBJ) $(LIBS) hvlog-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(HVLOGOBJS) $(HVLOG_MAIN_OBJ) $(LDFLAGS)
ifdef CV2PDB
	$(CV2PDB) -C "$@"
endif
	-$(ECHO) ' '

$(TEST_BIN): $(GENSRC) $(STDOBJS) $(TESTS_OBJ) $(LIBS) $(CU_OBJS) std-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(TESTS_OBJ) $(STDOBJS) $(CU_OBJS) $(LDFLAGS)
ifdef CV2PDB
	$(CV2PDB) -C "$@"
endif

obj/std/spng.o obj/hvlog/spng.o: deps/libspng/spng/spng.c deps/zlib/libz.a
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -I"deps/zlib" -I"deps/libspng/spng" -o"$@" "$<"
	-$(ECHO) ' '

obj/std/json/%.o obj/hvlog/json/%.o: deps/centijson/src/%.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -I"deps/centijson/src" -o"$@" "$<"
	-$(ECHO) ' '

obj/std/unzip.o obj/hvlog/unzip.o: deps/zlib/contrib/minizip/unzip.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -I"deps/zlib" -o"$@" "$<"
	-$(ECHO) ' '

obj/std/ioapi.o obj/hvlog/ioapi.o: deps/zlib/contrib/minizip/ioapi.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -I"deps/zlib" -o"$@" "$<"
	-$(ECHO) ' '

obj/std/custom_sprites.o obj/hvlog/custom_sprites.o: src/custom_sprites.c deps/zlib/contrib/minizip/unzip.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -I"deps/libspng/spng" -I"deps/centijson/src" -I"deps/zlib" -I"deps/zlib/contrib/minizip" -o"$@" "$<"
	-$(ECHO) ' '

obj/tests/%.o: tests/%.cpp $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -I"src/" $(CU_INC) -o"$@" "$<"
	-$(ECHO) ' '

obj/cu/%.o: $(CU_DIR)/Sources/Framework/%.c
	$(CPP) $(CXXFLAGS) $(CU_INC) -o"$@" "$<"

obj/cu/%.o: $(CU_DIR)/Sources/Basic/%.c
	$(CPP) $(CXXFLAGS) $(CU_INC) -o"$@" "$<"

obj/std/%.o obj/hvlog/%.o: src/%.cpp libexterns $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) ' '

obj/std/%.o obj/hvlog/%.o: src/%.c libexterns $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) ' '

# Windows resources compilation
obj/std/%.res obj/hvlog/%.res: res/%.rc res/keeperfx_icon.ico $(GENSRC)
	-$(ECHO) 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff
	-$(ECHO) ' '

# Creation of Windows icon files from PNG files
res/%.ico: res/%016-08bpp.png res/%032-08bpp.png res/%048-08bpp.png res/%064-08bpp.png res/%128-08bpp.png $(PNGTOICO)
	-$(ECHO) 'Building icon: $@'
	$(PNGTOICO) "$@" --colors 256 $(word 5,$^) $(word 4,$^) $(word 3,$^) --colors 16 $(word 2,$^) $(word 1,$^)
	-$(ECHO) ' '

obj/ver_defs.h: version.mk Makefile
	$(ECHO) \#define VER_MAJOR   $(VER_MAJOR) > "$(@D)/tmp"
	$(ECHO) \#define VER_MINOR   $(VER_MINOR) >> "$(@D)/tmp"
	$(ECHO) \#define VER_RELEASE $(VER_RELEASE) >> "$(@D)/tmp"
	$(ECHO) \#define VER_BUILD   $(BUILD_NUMBER) >> "$(@D)/tmp"
	$(ECHO) \#define VER_STRING  \"$(VER_STRING)\" >> "$(@D)/tmp"
	$(ECHO) \#define PACKAGE_SUFFIX  \"$(PACKAGE_SUFFIX)\" >> "$(@D)/tmp"
	$(ECHO) \#define GIT_REVISION  \"`git describe  --always`\" >> "$(@D)/tmp"
	$(MV) "$(@D)/tmp" "$@"

obj/libkeeperfx.a: bin/keeperfx.dll obj/keeperfx.def
	-$(ECHO) 'Generating gcc library archive for: $<'
	$(DLLTOOL) --dllname "$<" --def "obj/keeperfx.def" --output-lib "$@"
	-$(ECHO) ' '

bin/keeperfx.dll obj/keeperfx.def: lib/keeper95_gold.dll lib/keeper95_gold.map $(EXETODLL)
	-$(ECHO) 'Rebuilding DLL export table from: $<'
	$(EXETODLL) -o"$@" --def "obj/keeperfx.def" -p"_DK_" "$<"
	-$(ECHO) ' '

tests: std-before $(TEST_BIN)

libexterns: libexterns.mk
	$(MAKE) -f libexterns.mk

clean-libexterns: libexterns.mk
	-$(MAKE) -f libexterns.mk clean-libexterns
	-cd deps/zlib && $(MAKE) -f win32/Makefile.gcc clean
	-cd deps/zlib && git checkout Makefile zconf.h
	-$(RM) libexterns

deps/centijson/src/json.c deps/centijson/src/value.c deps/centijson/src/json-dom.c: deps/zlib/configure.log
deps/libspng/spng/spng.c: deps/zlib/configure.log
deps/zlib/contrib/minizip/unzip.c deps/zlib/contrib/minizip/ioapi.c: deps/zlib/configure.log


deps/zlib/configure.log:
	git submodule sync && git submodule update --init && cd deps/zlib && ./configure --static

deps/zlib/libz.a: deps/zlib/configure.log
	cd deps/zlib && $(MAKE) -f win32/Makefile.gcc PREFIX=$(CROSS_COMPILE) libz.a

include tool_peresec.mk
include tool_png2ico.mk
include tool_pngpal2raw.mk
include tool_png2bestpal.mk
include tool_po2ngdat.mk
include tool_sndbanker.mk
include tool_rnctools.mk
#include tool_dkillconv.mk

include package.mk
include pkg_lang.mk
include pkg_gfx.mk
include pkg_sfx.mk

export RM CP MKDIR MV ECHO
#******************************************************************************
