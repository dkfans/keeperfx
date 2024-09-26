include version.mk

BUILD_NUMBER ?= $(VER_BUILD)
VER_STRING := $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(BUILD_NUMBER) $(PACKAGE_SUFFIX)

ROOTDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

RM       = rm -f
MV       = mv -f
CP       = cp -f
MKDIR    = mkdir -p
ECHO     = echo
CD       = cd
CMAKE    := cmake -DCMAKE_TOOLCHAIN_FILE=$(ROOTDIR)/linux.cmake

SOURCES = \
	src/actionpt.c \
	src/api.c \
	src/ariadne.c \
	src/ariadne_edge.c \
	src/ariadne_findcache.c \
	src/ariadne_naviheap.c \
	src/ariadne_navitree.c \
	src/ariadne_points.c \
	src/ariadne_regions.c \
	src/ariadne_tringls.c \
	src/ariadne_wallhug.c \
	src/bflib_base_tcp.c \
	src/bflib_basics.c \
	src/bflib_bufrw.c \
	src/bflib_coroutine.c \
	src/bflib_client_tcp.c \
	src/bflib_cpu.c \
	src/bflib_datetm.c \
	src/bflib_dernc.c \
	src/bflib_enet.c \
	src/bflib_fileio.c \
	src/bflib_filelst.c \
	src/bflib_guibtns.c \
	src/bflib_inputctrl.c \
	src/bflib_keybrd.c \
	src/bflib_main.c \
	src/bflib_math.c \
	src/bflib_memory.c \
	src/bflib_mouse.c \
	src/bflib_mshandler.c \
	src/bflib_mspointer.c \
	src/bflib_netsession.c \
	src/bflib_netsp.c \
	src/bflib_netsp_ipx.c \
	src/bflib_netsync.c \
	src/bflib_network.c \
	src/bflib_planar.c \
	src/bflib_render.c \
	src/bflib_render_gpoly.c \
	src/bflib_render_trig.c \
	src/bflib_server_tcp.c \
	src/bflib_sndlib.cpp \
	src/bflib_sound.c \
	src/bflib_sprfnt.c \
	src/bflib_sprite.c \
	src/bflib_string.c \
	src/bflib_tcpsp.c \
	src/bflib_threadcond.c \
	src/bflib_video.c \
	src/bflib_vidraw.c \
	src/bflib_vidraw_spr_norm.c \
	src/bflib_vidraw_spr_onec.c \
	src/bflib_vidraw_spr_remp.c \
	src/bflib_vidsurface.c \
	src/config.c \
	src/config_campaigns.c \
	src/config_creature.c \
	src/config_crtrmodel.c \
	src/config_crtrstates.c \
	src/config_lenses.c \
	src/config_magic.c \
	src/config_objects.c \
	src/config_players.c \
	src/config_powerhands.c \
	src/config_rules.c \
	src/config_settings.c \
	src/config_slabsets.c \
	src/config_strings.c \
	src/config_terrain.c \
	src/config_cubes.c \
	src/config_textures.c \
	src/config_trapdoor.c \
	src/config_spritecolors.c \
	src/console_cmd.c \
	src/custom_sprites.c \
	src/creature_battle.c \
	src/creature_control.c \
	src/creature_graphics.c \
	src/creature_groups.c \
	src/creature_instances.c \
	src/creature_jobs.c \
	src/creature_senses.c \
	src/creature_states.c \
	src/creature_states_barck.c \
	src/creature_states_combt.c \
	src/creature_states_gardn.c \
	src/creature_states_guard.c \
	src/creature_states_hero.c \
	src/creature_states_lair.c \
	src/creature_states_mood.c \
	src/creature_states_pray.c \
	src/creature_states_prisn.c \
	src/creature_states_rsrch.c \
	src/creature_states_scavn.c \
	src/creature_states_spdig.c \
	src/creature_states_tortr.c \
	src/creature_states_train.c \
	src/creature_states_tresr.c \
	src/creature_states_wrshp.c \
	src/cursor_tag.c \
	src/dungeon_data.c \
	src/dungeon_stats.c \
	src/engine_arrays.c \
	src/engine_camera.c \
	src/engine_lenses.c \
	src/engine_redraw.c \
	src/engine_render.c \
	src/engine_render_data.c \
	src/engine_textures.c \
	src/front_credits.c \
	src/front_credits_data.c \
	src/front_easter.c \
	src/front_fmvids.c \
	src/front_highscore.c \
	src/front_input.c \
	src/front_landview.c \
	src/front_lvlstats.c \
	src/front_lvlstats_data.c \
	src/front_network.c \
	src/front_simple.c \
	src/front_torture.c \
	src/front_torture_data.c \
	src/frontend.c \
	src/frontmenu_options_data.c \
	src/frontmenu_saves_data.c \
	src/frontmenu_select.c \
	src/frontmenu_select_data.c \
	src/frontmenu_ingame_evnt.c \
	src/frontmenu_ingame_evnt_data.c \
	src/frontmenu_ingame_map.c \
	src/frontmenu_ingame_opts.c \
	src/frontmenu_ingame_opts_data.c \
	src/frontmenu_ingame_tabs.c \
	src/frontmenu_ingame_tabs_data.c \
	src/frontmenu_net.c \
	src/frontmenu_net_data.c \
	src/frontmenu_options.c \
	src/frontmenu_saves.c \
	src/frontmenu_specials.c \
	src/game_heap.c \
	src/game_legacy.c \
	src/game_loop.c \
	src/game_lghtshdw.c \
	src/game_merge.c \
	src/game_saves.c \
	src/gui_boxmenu.c \
	src/gui_draw.c \
	src/gui_frontbtns.c \
	src/gui_frontmenu.c \
	src/gui_msgs.c \
	src/gui_parchment.c \
	src/gui_soundmsgs.c \
	src/gui_tooltips.c \
	src/gui_topmsg.c \
	src/kjm_input.c \
	src/lens_api.c \
	src/config_effects.c \
	src/lens_flyeye.c \
	src/lens_mist.c \
	src/light_data.c \
	src/lvl_filesdk1.c \
	src/lvl_script.c \
	src/lvl_script_commands.c \
	src/lvl_script_commands_old.c \
	src/lvl_script_lib.c \
	src/lvl_script_conditions.c \
	src/lvl_script_value.c \
	src/magic.c \
	src/main.cpp \
	src/main_game.c \
	src/map_blocks.c \
	src/map_columns.c \
	src/map_ceiling.c \
	src/map_data.c \
	src/map_events.c \
	src/map_locations.c \
	src/map_utils.c \
	src/moonphase.c \
	src/music_player.c \
	src/net_game.c \
	src/net_sync.c \
	src/packets.c \
	src/packets_cheats.c \
	src/packets_input.c \
	src/packets_misc.c \
	src/player_compchecks.c \
	src/player_compevents.c \
	src/player_complookup.c \
	src/config_compp.c \
	src/player_compprocs.c \
	src/player_comptask.c \
	src/player_computer.c \
	src/player_computer_data.c \
	src/player_data.c \
	src/player_instances.c \
	src/player_utils.c \
	src/power_hand.c \
	src/power_process.c \
	src/power_specials.c \
	src/room_data.c \
	src/room_entrance.c \
	src/room_garden.c \
	src/room_graveyard.c \
	src/room_jobs.c \
	src/room_lair.c \
	src/room_library.c \
	src/room_list.c \
	src/room_scavenge.c \
	src/room_util.c \
	src/room_workshop.c \
	src/roomspace.c \
	src/roomspace_detection.c \
	src/scrcapt.c \
	src/slab_data.c \
	src/sounds.c \
	src/spdigger_stack.c \
	src/tasks_list.c \
	src/thing_corpses.c \
	src/thing_creature.c \
	src/thing_creature_data.c \
	src/thing_data.c \
	src/thing_doors.c \
	src/thing_effects.c \
	src/thing_factory.c \
	src/thing_list.c \
	src/thing_navigate.c \
	src/thing_objects.c \
	src/thing_physics.c \
	src/thing_shots.c \
	src/thing_stats.c \
	src/thing_traps.c \
	src/value_util.c \
	src/vidfade.c \
	src/vidmode_data.c \
	src/vidmode.c \
	src/KeeperSpeechImp.c \
	src/stubs.cpp \
	src/linux.cpp

WIN_SOURCES = \
	src/bflib_crash.c \
	src/bflib_fmvids.c \
	src/bflib_sndlib.c \
	src/steam_api.c

INCLUDES = \
	deps/centitoml \
	deps/centijson/src \
	deps/libspng/spng \
	deps/enet/include

DEPS = \
	deps/astronomy/astronomy.o \
	deps/centijson/build/libjson.a \
	deps/centitoml/toml_api.o \
	deps/libspng/build/libspng_static.a \
	deps/zlib/contrib/minizip/unzip.o \
	deps/zlib/contrib/minizip/ioapi.o \
	deps/enet/build/libenet.a

CFLAGS = -c -g -m32 -mno-sse --std=gnu17 -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unknown-pragmas -Wno-format-truncation
CXXFLAGS = -c -g -m32 -mno-sse --std=c++23 -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unknown-pragmas -Wno-format-truncation

all: bin/keeperfx

clean:
	$(RM) -r obj src/ver_defs.h

bin/keeperfx: obj/archive.a $(DEPS) | bin
	$(CXX) -g -m32 -o $@ $^ -lSDL2 -lSDL2_mixer -lSDL2_net -lSDL2_image -lz -lm -lopenal

obj/%.o: src/%.c | obj src/ver_defs.h
	$(CC) $(CFLAGS) -o $@ $(addprefix -I,$(INCLUDES)) $^

obj/%.o: src/%.cpp | obj src/ver_defs.h
	$(CXX) $(CXXFLAGS) -o $@ $(addprefix -I,$(INCLUDES)) $^

obj/archive.a: $(patsubst src/%.c,obj/%.o,$(patsubst src/%.cpp,obj/%.o,$(SOURCES))) | obj
	$(AR) r $@ $^

bin obj:
	$(MKDIR) -p $@

src/ver_defs.h: version.mk
	$(ECHO) \#define VER_MAJOR   $(VER_MAJOR) > "$@.swp"
	$(ECHO) \#define VER_MINOR   $(VER_MINOR) >> "$@.swp"
	$(ECHO) \#define VER_RELEASE $(VER_RELEASE) >> "$@.swp"
	$(ECHO) \#define VER_BUILD   $(BUILD_NUMBER) >> "$@.swp"
	$(ECHO) \#define VER_STRING  \"$(VER_STRING)\" >> "$@.swp"
	$(ECHO) \#define PACKAGE_SUFFIX  \"$(PACKAGE_SUFFIX)\" >> "$@.swp"
	$(ECHO) \#define GIT_REVISION  \"`git describe  --always`\" >> "$@.swp"
	$(MV) "$@.swp" "$@"

deps/astronomy/astronomy.o: deps/astronomy/astronomy.c
	$(CC) $(CFLAGS) -o $@ $^

deps/centijson/build/libjson.a: deps/centijson/build/Makefile
	$(CD) deps/centijson/build && $(MAKE)

deps/centijson/build/Makefile:
	$(MKDIR) -p deps/centijson/build
	$(CD) deps/centijson/build && $(CMAKE) ..

deps/centitoml/toml_api.o: deps/centitoml/toml_api.c
	$(CC) $(CFLAGS) -o $@ $^ -Ideps/centijson/src

deps/zlib/contrib/minizip/unzip.o: deps/zlib/contrib/minizip/unzip.c
	$(CC) $(CFLAGS) -o $@ $^

deps/zlib/contrib/minizip/ioapi.o: deps/zlib/contrib/minizip/ioapi.c
	$(CC) $(CFLAGS) -o $@ $^

deps/libspng/build/libspng_static.a: deps/libspng/build/Makefile
	$(CD) deps/libspng/build && $(MAKE)

deps/libspng/build/Makefile:
	$(MKDIR) -p deps/libspng/build
	$(CD) deps/libspng/build && $(CMAKE) ..

deps/enet/build/libenet.a: deps/enet/build/Makefile
	$(CD) deps/enet/build && $(MAKE)

deps/enet/build/Makefile:
	$(MKDIR) -p deps/enet/build
	$(CD) deps/enet/build && $(CMAKE) ..
