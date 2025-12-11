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
GENSRC   = src/ver_defs.h
RES      = obj/keeperfx_stdres.res

DEPS = \
obj/centitoml/toml_api.o \
deps/luajit/lib/libluajit.a

# functional test debugging flags/objs
FTEST_DEBUG ?= 0
ifeq ($(FTEST_DEBUG), 1)
  FTEST_DBGFLAGS = -DFUNCTESTING=1
  FTEST_OBJS = obj/ftests/ftest.o \
  			   obj/ftests/ftest_util.o \
			   obj/ftests/ftest_list.o
  FTEST_OBJS += $(patsubst src/ftests/tests/%,obj/ftests/tests/%,$(patsubst %.c,%.o,$(wildcard src/ftests/tests/ftest*.c)))
else
  FTEST_DBGFLAGS =
  FTEST_OBJS =
endif

OBJS = \
$(DEPS) \
obj/actionpt.o \
obj/api.o \
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
obj/bflib_coroutine.o \
obj/bflib_client_tcp.o \
obj/bflib_cpu.o \
obj/bflib_crash.o \
obj/bflib_datetm.o \
obj/bflib_dernc.o \
obj/bflib_enet.o \
obj/bflib_fileio.o \
obj/bflib_filelst.o \
obj/bflib_fmvids.o \
obj/bflib_guibtns.o \
obj/bflib_inputctrl.o \
obj/bflib_keybrd.o \
obj/bflib_main.o \
obj/bflib_math.o \
obj/bflib_mouse.o \
obj/bflib_mshandler.o \
obj/bflib_mspointer.o \
obj/bflib_netsession.o \
obj/bflib_netsp.o \
obj/bflib_network.o \
obj/bflib_network_exchange.o \
obj/net_resync.o \
obj/bflib_planar.o \
obj/bflib_render.o \
obj/bflib_render_gpoly.o \
obj/bflib_render_trig.o \
obj/bflib_server_tcp.o \
obj/bflib_sndlib.o \
obj/bflib_sound.o \
obj/bflib_sprfnt.o \
obj/bflib_string.o \
obj/bflib_tcpsp.o \
obj/bflib_video.o \
obj/bflib_vidraw.o \
obj/bflib_vidraw_spr_norm.o \
obj/bflib_vidraw_spr_onec.o \
obj/bflib_vidraw_spr_remp.o \
obj/bflib_vidsurface.o \
obj/cdrom.o \
obj/config.o \
obj/config_campaigns.o \
obj/config_creature.o \
obj/config_crtrmodel.o \
obj/config_crtrstates.o \
obj/config_keeperfx.o \
obj/config_lenses.o \
obj/config_magic.o \
obj/config_objects.o \
obj/config_mods.o \
obj/config_players.o \
obj/config_powerhands.o \
obj/config_rules.o \
obj/config_settings.o \
obj/config_slabsets.o \
obj/config_strings.o \
obj/config_terrain.o \
obj/config_cubes.o \
obj/config_textures.o \
obj/config_trapdoor.o \
obj/config_spritecolors.o \
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
obj/cursor_tag.o \
obj/dungeon_data.o \
obj/dungeon_stats.o \
obj/engine_arrays.o \
obj/engine_camera.o \
obj/local_camera.o \
obj/engine_lenses.o \
obj/engine_redraw.o \
obj/engine_render.o \
obj/engine_render_data.o \
obj/engine_textures.o \
obj/front_credits.o \
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
obj/highscores.o \
obj/kjm_input.o \
obj/lens_api.o \
obj/config_effects.o \
obj/lens_flyeye.o \
obj/lens_mist.o \
obj/light_data.o \
obj/lua_api.o \
obj/lua_api_player.o \
obj/lua_api_room.o \
obj/lua_api_things.o \
obj/lua_api_slabs.o \
obj/lua_base.o \
obj/lua_cfg_funcs.o \
obj/lua_params.o \
obj/lua_triggers.o \
obj/lua_utils.o \
obj/lvl_filesdk1.o \
obj/lvl_script.o \
obj/lvl_script_commands.o \
obj/lvl_script_commands_old.o \
obj/lvl_script_lib.o \
obj/lvl_script_conditions.o \
obj/lvl_script_value.o \
obj/magic_powers.o \
obj/main_game.o \
obj/map_blocks.o \
obj/map_columns.o \
obj/map_ceiling.o \
obj/map_data.o \
obj/map_events.o \
obj/map_locations.o \
obj/map_utils.o \
obj/moonphase.o \
obj/net_game.o \
obj/net_input_lag.o \
obj/net_received_packets.o \
obj/net_redundant_packets.o \
obj/net_checksums.o \
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
obj/room_treasure.o \
obj/room_util.o \
obj/room_workshop.o \
obj/roomspace.o \
obj/roomspace_detection.o \
obj/scrcapt.o \
obj/slab_data.o \
obj/sounds.o \
obj/spdigger_stack.o \
obj/steam_api.o \
obj/tasks_list.o \
obj/thing_corpses.o \
obj/thing_creature.o \
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
obj/value_util.o \
obj/vidfade.o \
obj/vidmode_data.o \
obj/vidmode.o \
obj/KeeperSpeechImp.o \
obj/spritesheet.o \
obj/windows.o \
$(FTEST_OBJS) \
$(RES)

MAIN_OBJ = obj/main.o

TESTS_OBJ = obj/tests/tst_main.o \
obj/tests/tst_fixes.o \
obj/tests/001_test.o \
obj/tests/tst_enet_server.o \
obj/tests/tst_enet_client.o

CU_DIR = deps/CUnit-2.1-3/CUnit
CU_INC = -I"$(CU_DIR)/Headers"
CU_OBJS = \
	obj/cu/Basic.o \
	obj/cu/TestDB.o \
	obj/cu/CUError.o \
	obj/cu/TestRun.o \
	obj/cu/Util.o

# include and library directories
LINKLIB = -mwindows \
	-L"sdl/lib" -lSDL2 -lSDL2_mixer -lSDL2_net -lSDL2_image \
	-L"deps/ffmpeg/libavformat" -lavformat \
	-L"deps/ffmpeg/libavcodec" -lavcodec \
	-L"deps/ffmpeg/libswresample" -lswresample \
	-L"deps/ffmpeg/libavutil" -lavutil \
	-L"deps/openal" -lOpenAL32 \
	-L"deps/astronomy" -lastronomy \
	-L"deps/enet" -lenet \
	-L"deps/spng" -lspng \
	-L"deps/centijson" -ljson \
	-L"deps/zlib" -lminizip -lz \
	deps/luajit/lib/libluajit.a \
	-lwinmm -lmingw32 -limagehlp -lws2_32 -ldbghelp -lbcrypt -lole32 -luuid
INCS = \
	-I"deps/zlib/include" \
	-I"deps/spng/include" \
	-I"sdl/include" \
	-I"sdl/include/SDL2" \
	-I"deps/enet/include" \
	-I"deps/centijson/include" \
	-I"deps/centitoml" \
	-I"deps/astronomy/include" \
	-I"deps/ffmpeg" \
	-I"deps/openal/include" \
	-I"deps/luajit/include"
CXXINCS =  $(INCS)

STDOBJS   = $(subst obj/,obj/std/,$(OBJS))
HVLOGOBJS = $(subst obj/,obj/hvlog/,$(OBJS))
STD_MAIN_OBJ = $(subst obj/,obj/std/,$(MAIN_OBJ))
HVLOG_MAIN_OBJ = $(subst obj/,obj/hvlog/,$(MAIN_OBJ))


# allow extracting files from archives, replacing pre-existing ones
ENABLE_EXTRACT ?= 1

# flags to generate dependency files
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -DSPNG_STATIC=1 -DAL_LIBTYPE_STATIC
# other flags to include while compiling
INCFLAGS =
# code optimization and debugging flags
CV2PDB := $(shell PATH=`pwd`:$$PATH command -v cv2pdb.exe 2> /dev/null)
DEBUG ?= 0
ifeq ($(DEBUG), 1)
  OPTFLAGS = -march=x86-64 -fno-omit-frame-pointer -O0
  DBGFLAGS = -g -DDEBUG
else
  # frame pointer is required for ASM code to work
  OPTFLAGS = -march=x86-64 -fno-omit-frame-pointer -O3
  # if we can create a separate debug info file then do it
  ifdef CV2PDB
    DBGFLAGS = -g
  else
    DBGFLAGS =
  endif
endif

# logging level flags
STLOGFLAGS = -DBFDEBUG_LEVEL=0
HVLOGFLAGS = -DBFDEBUG_LEVEL=10
# compiler warning generation flags
WARNFLAGS = -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter -Wno-maybe-uninitialized -Wno-sign-compare -Wno-strict-aliasing -Wno-unknown-pragmas -Werror
# disabled warnings: -Wextra -Wtype-limits
CXXFLAGS = $(CXXINCS) -c -std=gnu++1y -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(FTEST_DBGFLAGS) $(INCFLAGS)
CFLAGS = $(INCS) -c -std=gnu11 -fmessage-length=0 $(WARNFLAGS) -Werror=implicit $(DEPFLAGS) $(FTEST_DBGFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
LDFLAGS = $(LINKLIB) $(OPTFLAGS) $(DBGFLAGS) $(FTEST_DBGFLAGS) $(LINKFLAGS) -Wl,-Map,"$(@:%.exe=%.map)"

ifeq ($(USE_PRE_FILE), 1)
CXXFLAGS += -DUSE_PRE_FILE=1
CFLAGS += -DUSE_PRE_FILE=1
endif

CAMPAIGNS = $(patsubst campgns/%.cfg,%,$(wildcard campgns/*.cfg))
MAPPACKS = $(patsubst levels/%.cfg,%,$(filter-out %/personal.cfg,$(wildcard levels/*.cfg)))
LANGS = eng chi cht cze dut fre ger ita jpn kor lat pol rus spa swe

# load program version
include version.mk

VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(BUILD_NUMBER) $(PACKAGE_SUFFIX)

# Enable parallel compilation by default. Users can still override with: make -j8, make -j1, etc.
ifndef MAKEFLAGS
  MAKEFLAGS = -j$(shell nproc)
endif

# load depenency packages
include prebuilds.mk

# name virtual targets
.PHONY: all docs docsdox clean clean-build deep-clean build-before
.PHONY: standard std-before std-after
.PHONY: heavylog hvlog-before hvlog-after
.PHONY: package clean-package deep-clean-package
.PHONY: tools clean-tools deep-clean-tools
.PHONY: clean-libexterns deep-clean-libexterns
.PHONY: tests cppcheck

# dependencies tracking
-include $(filter %.d,$(STDOBJS:%.o=%.d))
-include $(filter %.d,$(HVLOGOBJS:%.o=%.d))


# 'make all' calculates the current checksum of all .h and .hpp files, storing the checksum in a file. Then it decides whether to run 'make clean' or 'make standard' based on whether any .h and .hpp files have been altered
HEADER_CHECKSUM_FILE=.header_checksum

all:
	@start_time=$$(date +%s.%N); \
	get_header_cksum=$$(find ./src/ -type f \( -name "*.h" -o -name "*.hpp" \) -print0 | sort -z | xargs -0 cksum | cksum | awk '{print $$1}'); \
	current_checksum=$$(echo $$get_header_cksum $(DEBUG) | cksum | awk '{print $$1}'); \
	if [ ! -f $(HEADER_CHECKSUM_FILE) ] || [ "$$(cat $(HEADER_CHECKSUM_FILE))" != "$$current_checksum" ]; then \
		$(MAKE) clean; \
	fi; \
	$(MAKE) standard || exit 1; \
	echo "$$current_checksum" > $(HEADER_CHECKSUM_FILE); \
	end_time=$$(date +%s.%N); \
	duration=$$(awk "BEGIN {print $$end_time - $$start_time}"); \
	printf "\033[97mCompiled in: %0.2f seconds\033[0m\n" $$duration;

standard: CXXFLAGS += $(STLOGFLAGS)
standard: CFLAGS += $(STLOGFLAGS)
standard: std-before $(BIN) std-after

heavylog: CXXFLAGS += $(HVLOGFLAGS)
heavylog: CFLAGS += $(HVLOGFLAGS)
heavylog: hvlog-before $(HVLOGBIN) hvlog-after

# not nice but necessary for make -j to work
FOLDERS = bin obj/std obj/hvlog \
obj/std/ftests \
obj/std/ftests/tests \
obj/tests obj/cu \
obj/std/centitoml obj/hvlog/centitoml \
sdl/for_final_package

$(shell $(MKDIR) $(FOLDERS))

build-before: libexterns

std-before: build-before
hvlog-before: build-before

docs: docsdox

docsdox: docs/doxygen.conf
	VERSION=$(VER_STRING) $(DOXYTOOL) docs/doxygen.conf

deep-clean: deep-clean-tools deep-clean-package
	$(MAKE) -f libexterns.mk deep-clean-libexterns

clean: submodule clean-build clean-tools clean-libexterns clean-package

submodule:
	-git submodule init && git submodule update

clean-build:
	-$(RM) $(STDOBJS) $(STD_MAIN_OBJ) $(filter %.d,$(STDOBJS:%.o=%.d)) $(filter %.d,$(STD_MAIN_OBJ:%.o=%.d))
	-$(RM) $(HVLOGOBJS) $(HVLOG_MAIN_OBJ) $(filter %.d,$(HVLOGOBJS:%.o=%.d)) $(filter %.d,$(HVLOG_MAIN_OBJ:%.o=%.d))
	-$(RM) $(BIN) $(BIN:%.exe=%.map)
	-$(RM) $(BIN) $(BIN:%.exe=%.pdb)
	-$(RM) $(HVLOGBIN) $(HVLOGBIN:%.exe=%.map)
	-$(RM) $(HVLOGBIN) $(HVLOGBIN:%.exe=%.pdb)
	-$(RM) bin/keeperfx.dll
	-$(RM) $(GENSRC)
	-$(RM) res/*.ico
	-$(RM) obj/keeperfx.*

$(BIN): $(GENSRC) $(STDOBJS) $(STD_MAIN_OBJ) std-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(STDOBJS) $(STD_MAIN_OBJ) $(LDFLAGS)
ifdef CV2PDB
	$(CV2PDB) -C "$@"
endif
	-$(ECHO) ' '

$(HVLOGBIN): $(GENSRC) $(HVLOGOBJS) $(HVLOG_MAIN_OBJ) hvlog-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(HVLOGOBJS) $(HVLOG_MAIN_OBJ) $(LDFLAGS)
ifdef CV2PDB
	$(CV2PDB) -C "$@"
endif
	-$(ECHO) ' '

$(TEST_BIN): $(GENSRC) $(STDOBJS) $(TESTS_OBJ) $(CU_OBJS) std-before
	-$(ECHO) 'Building target: $@'
	$(CPP) -o "$@" $(TESTS_OBJ) $(STDOBJS) $(CU_OBJS) $(LDFLAGS)
ifdef CV2PDB
	$(CV2PDB) -C "$@"
endif

obj/std/centitoml/toml_api.o obj/hvlog/centitoml/toml_api.o: deps/centitoml/toml_api.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) ' '

obj/tests/%.o: tests/%.cpp $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -I"src/" $(CU_INC) -o"$@" "$<"
	-$(ECHO) ' '

obj/cu/%.o: $(CU_DIR)/Sources/Framework/%.c
	$(CPP) $(CXXFLAGS) $(CU_INC) -o"$@" "$<"

obj/cu/%.o: $(CU_DIR)/Sources/Basic/%.c
	$(CPP) $(CXXFLAGS) $(CU_INC) -o"$@" "$<"


define BUILD_CPP_FILES_CMD
	-$(ECHO) 'Building cpp file: $<'
	@grep -E "#include \"(\.\./)?(\.\./)?pre_inc.h\"" "$<" >/dev/null || echo "\n\nAll files should have #include \"pre_inc.h\" as first include\n\n" >&2 | false
	@grep -E "#include \"(\.\./)?(\.\./)?post_inc.h\"" "$<" >/dev/null || echo "\n\nAll files should have #include \"post_inc.h\" as last include\n\n" >&2 | false
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
endef

obj/std/%.o: src/%.cpp libexterns $(GENSRC)
	$(BUILD_CPP_FILES_CMD)

obj/hvlog/%.o: src/%.cpp libexterns $(GENSRC)
	$(BUILD_CPP_FILES_CMD)


define BUILD_CC_FILES_CMD
	-$(ECHO) 'Building cc file: $<'
	@grep -E "#include \"(\.\./)?(\.\./)?pre_inc.h\"" "$<" >/dev/null || echo "\n\nAll files should have #include \"pre_inc.h\" as first include\n\n" >&2 | false
	@grep -E "#include \"(\.\./)?(\.\./)?post_inc.h\"" "$<" >/dev/null || echo "\n\nAll files should have #include \"post_inc.h\" as last include\n\n" >&2 | false
	$(CC) $(CFLAGS) -o"$@" "$<"
endef

obj/std/%.o: src/%.c libexterns $(GENSRC)
	$(BUILD_CC_FILES_CMD)

obj/hvlog/%.o: src/%.c libexterns $(GENSRC)
	$(BUILD_CC_FILES_CMD)


# Windows resources compilation

define BUILD_RESOURCE_CMD
	-$(ECHO) 'Building resource: $<'
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff -I"obj/"
	-$(ECHO) ' '
endef

obj/std/%.res: res/%.rc res/keeperfx_icon.ico $(GENSRC)
	$(BUILD_RESOURCE_CMD)

obj/hvlog/%.res: res/%.rc res/keeperfx_icon.ico $(GENSRC)
	$(BUILD_RESOURCE_CMD)


# Creation of Windows icon files from PNG files
res/%.ico: res/%016-08bpp.png res/%032-08bpp.png res/%048-08bpp.png res/%064-08bpp.png res/%128-08bpp.png res/%128-24bpp.png res/%256-24bpp.png res/%512-24bpp.png $(PNGTOICO)
	-$(ECHO) 'Building icon: $@'
	$(PNGTOICO) "$@" $(word 8,$^) $(word 7,$^) $(word 6,$^) --colors 256 $(word 5,$^) $(word 4,$^) $(word 3,$^) --colors 16 $(word 2,$^) $(word 1,$^)
	-$(ECHO) ' '

src/ver_defs.h: version.mk Makefile
	$(ECHO) \#define VER_MAJOR   $(VER_MAJOR) > "$(@D)/tmp"
	$(ECHO) \#define VER_MINOR   $(VER_MINOR) >> "$(@D)/tmp"
	$(ECHO) \#define VER_RELEASE $(VER_RELEASE) >> "$(@D)/tmp"
	$(ECHO) \#define VER_BUILD   $(BUILD_NUMBER) >> "$(@D)/tmp"
	$(ECHO) \#define VER_STRING  \"$(VER_STRING)\" >> "$(@D)/tmp"
	$(ECHO) \#define PACKAGE_SUFFIX  \"$(PACKAGE_SUFFIX)\" >> "$(@D)/tmp"
	$(ECHO) \#define GIT_REVISION  \"`git describe  --always`\" >> "$(@D)/tmp"
	$(MV) "$(@D)/tmp" "$@"

tests: std-before $(TEST_BIN)

libexterns: libexterns.mk
	$(MAKE) -f libexterns.mk

clean-libexterns: libexterns.mk
	-$(MAKE) -f libexterns.mk clean-libexterns
	-$(RM) -rf deps/enet deps/zlib deps/spng deps/astronomy deps/centijson deps/luajit
	-$(RM) libexterns

deps/enet deps/zlib deps/spng deps/astronomy deps/centijson deps/ffmpeg deps/openal deps/luajit:
	$(MKDIR) $@

src/api.c: deps/centijson/include/json.h
src/bflib_enet.cpp: deps/enet/include/enet/enet.h
src/custom_sprites.c: deps/zlib/include/zlib.h deps/spng/include/spng.h deps/centijson/include/json.h
src/moonphase.c: deps/astronomy/include/astronomy.h
deps/centitoml/toml_api.c: deps/centijson/include/json.h
deps/centitoml/toml_conv.c: deps/centijson/include/json.h
src/bflib_fmvids.cpp: deps/ffmpeg/libavformat/avformat.h
src/bflib_sndlib.cpp: deps/openal/include/AL/al.h
src/net_resync.cpp: deps/zlib/include/zlib.h
src/console_cmd.c: deps/luajit/include/lua.h

deps/enet-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/initial/enet-mingw32.tar.gz"

deps/enet/include/enet/enet.h: deps/enet-mingw32.tar.gz | deps/enet
	tar xzmf $< -C deps/enet

deps/zlib-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/initial/zlib-mingw32.tar.gz"

deps/zlib/include/zlib.h: deps/zlib-mingw32.tar.gz | deps/zlib
	tar xzmf $< -C deps/zlib

deps/spng-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/initial/spng-mingw32.tar.gz"

deps/spng/include/spng.h: deps/spng-mingw32.tar.gz | deps/spng
	tar xzmf $< -C deps/spng

deps/astronomy-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/astronomy_fix/astronomy-mingw32.tar.gz"

deps/astronomy/include/astronomy.h: deps/astronomy-mingw32.tar.gz | deps/astronomy
	tar xzmf $< -C deps/astronomy

deps/centijson-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/initial/centijson-mingw32.tar.gz"

deps/centijson/include/json.h: deps/centijson-mingw32.tar.gz | deps/centijson
	tar xzmf $< -C deps/centijson

deps/ffmpeg-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/initial/ffmpeg-mingw32.tar.gz"

deps/ffmpeg/libavformat/avformat.h: deps/ffmpeg-mingw32.tar.gz | deps/ffmpeg
	tar xzmf $< -C deps/ffmpeg

deps/openal-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/2024-11-14/openal-mingw32.tar.gz"

deps/openal/include/AL/al.h: deps/openal-mingw32.tar.gz | deps/openal
	tar xzmf $< -C deps/openal

deps/luajit-mingw32.tar.gz:
	curl -Lso $@ "https://github.com/dkfans/kfx-deps/releases/download/20250418/luajit-mingw32.tar.gz"

deps/luajit/lib/libluajit.a: | deps/luajit/include/lua.h

deps/luajit/include/lua.h: deps/luajit-mingw32.tar.gz | deps/luajit
	tar xzmf $< -C deps/luajit

cppcheck: | src/ver_defs.h
cppcheck: | deps/zlib/include/zlib.h
cppcheck: | deps/spng/include/spng.h
cppcheck: | deps/astronomy/include/astronomy.h
cppcheck: | deps/centijson/include/json.h
cppcheck: | deps/enet/include/enet/enet.h
cppcheck: | deps/luajit/include/lua.h
cppcheck: | deps/openal/include/AL/al.h
cppcheck: | deps/ffmpeg/libavformat/avformat.h

cppcheck:
	$(MKDIR) cppcheck.cache
	cppcheck \
		--cppcheck-build-dir=cppcheck.cache \
		--check-level=exhaustive \
		--enable=all \
		--platform=win32A \
		--std=c++14 \
		--inconclusive \
		-j $(shell nproc) \
		-q \
		-I deps/zlib/include \
		-I deps/spng/include \
		-I sdl/include \
		-I sdl/include/SDL2 \
		-I deps/enet/include \
		-I deps/centijson/include \
		-I deps/centitoml \
		-I deps/astronomy/include \
		-I deps/ffmpeg \
		-I deps/openal/include \
		-I deps/luajit/include \
		-I obj \
		-D__WIN32__ \
		-DBFDEBUG_LEVEL=99 \
		-DSPNG_STATIC=1 \
		-DAL_LIBTYPE_STATIC \
		-DDEBUG_NETWORK_PACKETS=1 \
		--suppress=missingIncludeSystem \
		--suppress=constParameterPointer \
		--suppress=constVariablePointer \
		--suppress=functionConst \
		--suppress=unreadVariable \
		--suppress=uninitvar \
		--suppress=variableScope \
		--suppress=unusedStructMember \
		--suppress=funcArgNamesDifferent \
		--suppress=funcArgOrderDifferent \
		--suppress=cstyleCast \
		--suppress=functionStatic \
		--suppress=unsignedLessThanZero \
		--suppress=constParameterCallback \
		--suppress=constParameter \
		--suppress=knownConditionTrueFalse \
		--suppress=negativeIndex \
		--suppress=nullPointerRedundantCheck \
		--suppress=nullPointerArithmeticRedundantCheck \
		--suppress=invalidscanf \
		--suppress=invalidScanfArgType_int \
		--suppress=invalidPrintfArgType_uint \
		--suppress=invalidPrintfArgType_sint \
		--suppress=redundantAssignment \
		--suppress=preprocessorErrorDirective \
		--suppress=uninitMemberVar \
		--suppress=truncLongCastAssignment \
		--suppress=shiftNegativeLHS \
		--suppress=bitwiseOnBoolean \
		--suppress=shiftTooManyBits \
		--suppress=shiftTooManyBitsSigned \
		--suppress=identicalConditionAfterEarlyExit \
		--suppress=useInitializationList \
		--suppress=operatorEqVarError \
		--suppress=noExplicitConstructor \
		--suppress=useStlAlgorithm \
		--suppress=duplicateExpression \
		--suppress=duplicateBranch \
		--suppress=duplicateConditionalAssign \
		--suppress=duplicateAssignExpression \
		--suppress=nullPointer \
		--suppress=compareValueOutOfTypeRangeError \
		--suppress=redundantInitialization \
		--suppress=multiCondition \
		--suppress=internalAstError \
		--suppress=clarifyCondition \
		--suppress=memsetClassFloat \
		--suppress=comparePointers \
		--suppress=identicalInnerCondition \
		--suppress=uselessAssignmentPtrArg \
		--suppress=unassignedVariable \
		--suppress=shiftNegative \
		--suppress=duplicateCondition \
		--suppress=badBitmaskCheck \
		--suppress=shadowFunction \
		--suppress=shadowVariable \
		--suppress=uninitStructMember \
		--suppress=CastIntegerToAddressAtReturn \
		src 2>cppcheck.log

include tool_png2ico.mk
include tool_pngpal2raw.mk
include tool_png2bestpal.mk
include tool_po2ngdat.mk
include tool_sndbanker.mk
include tool_rnctools.mk
#include tool_dkillconv.mk

include pkg_lang.mk
include pkg_gfx.mk
include pkg_sfx.mk
include package.mk

export RM CP MKDIR MV ECHO
#******************************************************************************
