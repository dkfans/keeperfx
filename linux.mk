include version.mk

BUILD_NUMBER ?= $(VER_BUILD)
VER_SUFFIX ?= Prototype
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(BUILD_NUMBER) $(VER_SUFFIX)

MKDIR ?= mkdir -p
STRIP ?= strip
ECHO ?= echo
MV ?= mv -f

PLATFORM ?= $(shell uname -s)
ARCH ?= $(shell uname -m)

ifeq ($(PLATFORM),Linux)
PLATFORM := linux
endif
ifeq ($(PLATFORM),Darwin)
PLATFORM := mac
endif
ifeq ($(PLATFORM),macos)
PLATFORM := mac
endif

ifeq ($(ARCH),amd64)
ARCH := x86_64
endif
ifeq ($(ARCH),aarch64)
ARCH := arm64
endif

DEPS_ASTRONOMY_TAG ?= 20250418
DEPS_CENTIJSON_TAG ?= 20250418
DEPS_ENET6_TAG ?= 20260213
DEPS_ASTRONOMY_REPO ?= https://github.com/cosinekitty/astronomy.git
DEPS_CENTIJSON_REPO ?= https://github.com/mity/centijson.git
DEPS_ENET6_REPO ?= https://github.com/SirLynix/enet6.git
DEPS_ASTRONOMY_REF ?=
DEPS_CENTIJSON_REF ?=
DEPS_ENET6_REF ?=
DEPS_CLONE_DEPTH ?= 1
DEPS_BUILD_DIR ?= deps/_build
DEPS_SUFFIX ?=
DEPS_DOWNLOAD ?= 1

ifeq ($(PLATFORM),linux)
DEPS_SUFFIX ?= lin64
endif
ifeq ($(PLATFORM),mac)
DEPS_DOWNLOAD ?= 0
endif

ifeq ($(PLATFORM),linux)
	ifeq ($(ARCH),x86_64)
		ARCH_CFLAGS := -march=x86-64
	else ifeq ($(ARCH),arm64)
		ARCH_CFLAGS := -march=armv8-a
	else
		$(error Unsupported ARCH for linux: $(ARCH))
	endif
	ARCH_LDFLAGS :=
else ifeq ($(PLATFORM),mac)
	ifeq ($(ARCH),x86_64)
		ARCH_CFLAGS := -arch x86_64
	else ifeq ($(ARCH),arm64)
		ARCH_CFLAGS := -arch arm64
	else
		$(error Unsupported ARCH for mac: $(ARCH))
	endif
	ARCH_LDFLAGS := $(ARCH_CFLAGS)
else
	$(error Unsupported PLATFORM: $(PLATFORM))
endif

WARN_NO_FORMAT_TRUNCATION :=
ifeq ($(PLATFORM),linux)
WARN_NO_FORMAT_TRUNCATION := -Wno-format-truncation
endif

CXX_STD ?= -std=c++17

KFX_SOURCES = \
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
src/bflib_base_tcp.cpp \
src/bflib_basics.c \
src/bflib_coroutine.c \
src/bflib_client_tcp.cpp \
src/bflib_cpu.c \
src/bflib_datetm.cpp \
src/bflib_dernc.c \
src/bflib_enet.cpp \
src/net_portforward.cpp \
src/bflib_fileio.c \
src/bflib_filelst.c \
src/bflib_fmvids.cpp \
src/bflib_guibtns.c \
src/bflib_input_joyst.cpp \
src/bflib_inputctrl.cpp \
src/bflib_keybrd.c \
src/bflib_main.cpp \
src/bflib_math.c \
src/bflib_mouse.cpp \
src/bflib_mshandler.cpp \
src/bflib_mspointer.cpp \
src/bflib_netsession.c \
src/bflib_netsp.cpp \
src/bflib_network.cpp \
src/bflib_network_exchange.cpp \
src/bflib_planar.c \
src/bflib_render.c \
src/bflib_render_gpoly.c \
src/bflib_render_trig.c \
src/bflib_server_tcp.cpp \
src/bflib_sndlib.cpp \
src/bflib_sound.c \
src/bflib_sprfnt.c \
src/bflib_string.c \
src/bflib_tcpsp.c \
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
src/config_keeperfx.c \
src/config_lenses.c \
src/config_magic.c \
src/config_mods.c \
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
src/local_camera.c \
src/engine_lenses.c \
src/engine_redraw.c \
src/engine_render.c \
src/engine_render_data.cpp \
src/engine_textures.c \
src/front_credits.c \
src/front_easter.c \
src/front_fmvids.c \
src/front_highscore.c \
src/front_input.c \
src/front_landview.c \
src/front_lvlstats.c \
src/front_lvlstats_data.cpp \
src/front_network.c \
src/front_simple.c \
src/front_torture.c \
src/front_torture_data.cpp \
src/frontend.cpp \
src/frontmenu_select.c \
src/frontmenu_ingame_evnt.c \
src/frontmenu_ingame_evnt_data.cpp \
src/frontmenu_ingame_map.c \
src/frontmenu_ingame_opts.c \
src/frontmenu_ingame_opts_data.cpp \
src/frontmenu_ingame_tabs.c \
src/frontmenu_ingame_tabs_data.cpp \
src/frontmenu_net.c \
src/frontmenu_net_data.cpp \
src/frontmenu_options.c \
src/frontmenu_options_data.cpp \
src/frontmenu_saves.c \
src/frontmenu_saves_data.cpp \
src/frontmenu_select_data.cpp \
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
src/gui_soundmsgs.cpp \
src/gui_tooltips.c \
src/gui_topmsg.c \
src/highscores.c \
src/KeeperSpeechImp.c \
src/kjm_input.c \
src/lens_api.c \
src/config_effects.c \
src/kfx/lense/DisplacementEffect.cpp \
src/kfx/lense/FlyeyeEffect.cpp \
src/kfx/lense/LensEffect.cpp \
src/kfx/lense/LensManager.cpp \
src/kfx/lense/LuaLensEffect.cpp \
src/kfx/lense/MistEffect.cpp \
src/kfx/lense/OverlayEffect.cpp \
src/kfx/lense/PaletteEffect.cpp \
src/light_data.c \
src/linux.cpp \
src/lua_api.c \
src/lua_api_lens.c \
src/lua_api_player.c \
src/lua_api_room.c \
src/lua_api_things.c \
src/lua_api_slabs.c \
src/lua_base.c \
src/lua_cfg_funcs.c \
src/lua_params.c \
src/lua_triggers.c \
src/lua_utils.c \
src/lvl_filesdk1.c \
src/lvl_script.c \
src/lvl_script_commands.c \
src/lvl_script_commands_old.c \
src/lvl_script_lib.c \
src/lvl_script_conditions.c \
src/lvl_script_value.c \
src/magic_powers.c \
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
src/net_checksums.c \
src/net_game.c \
src/net_input_lag.c \
src/net_received_packets.c \
src/net_redundant_packets.c \
src/net_resync.cpp \
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
src/player_computer_data.cpp \
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
src/room_treasure.c \
src/room_util.c \
src/room_workshop.c \
src/roomspace.c \
src/roomspace_detection.c \
src/scrcapt.c \
src/slab_data.c \
src/sounds.c \
src/spdigger_stack.c \
src/spritesheet.cpp \
src/tasks_list.c \
src/thing_corpses.c \
src/thing_creature.c \
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
src/vidmode_data.cpp \
src/vidmode.c

KFX_C_SOURCES = $(filter %.c,$(KFX_SOURCES))
KFX_CXX_SOURCES = $(filter %.cpp,$(KFX_SOURCES))
KFX_C_OBJECTS = $(patsubst src/%.c,obj/%.o,$(KFX_C_SOURCES))
KFX_CXX_OBJECTS = $(patsubst src/%.cpp,obj/%.o,$(KFX_CXX_SOURCES))
KFX_OBJECTS = $(KFX_C_OBJECTS) $(KFX_CXX_OBJECTS)

KFX_INCLUDES = \
	-Ideps/centijson/include \
	-Ideps/centitoml \
	-Ideps/astronomy/include \
	-Ideps/enet6/include \
	$(shell pkg-config --cflags-only-I sdl2) \
	$(shell pkg-config --cflags-only-I SDL2_image) \
	$(shell pkg-config --cflags-only-I SDL2_mixer) \
	$(shell pkg-config --cflags-only-I SDL2_net) \
	$(shell pkg-config --cflags-only-I libavformat) \
	$(shell pkg-config --cflags-only-I libavcodec) \
	$(shell pkg-config --cflags-only-I libswresample) \
	$(shell pkg-config --cflags-only-I libavutil) \
	$(shell pkg-config --cflags-only-I openal) \
	$(shell pkg-config --cflags-only-I openal-soft) \
	$(shell pkg-config --cflags-only-I spng) \
	$(shell pkg-config --cflags-only-I minizip) \
	$(shell pkg-config --cflags-only-I zlib) \
	$(shell pkg-config --cflags-only-I luajit)

ifeq ($(PLATFORM),mac)
KFX_INCLUDES += \
	-I/opt/homebrew/opt/openal-soft/include \
	-I/usr/local/opt/openal-soft/include
endif

KFX_CFLAGS += -g -DDEBUG -DBFDEBUG_LEVEL=0 -O3 $(ARCH_CFLAGS) $(KFX_INCLUDES) -Wall -Wextra -Wno-unused-parameter -Wno-absolute-value -Wno-unknown-pragmas $(WARN_NO_FORMAT_TRUNCATION) -Wno-sign-compare -fsigned-char
KFX_CXXFLAGS += -g -DDEBUG -DBFDEBUG_LEVEL=0 -O3 $(CXX_STD) $(ARCH_CFLAGS) $(KFX_INCLUDES) -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas $(WARN_NO_FORMAT_TRUNCATION) -Wno-sign-compare -fsigned-char

KFX_LDFLAGS += \
	-g \
	-Wall -Wextra -Werror \
	$(ARCH_LDFLAGS) \
	-Ldeps/astronomy -lastronomy \
	-Ldeps/centijson -ljson \
	-Ldeps/enet6 -lenet6 \
	$(shell pkg-config --libs-only-l sdl2) \
	$(shell pkg-config --libs-only-l SDL2_mixer) \
	$(shell pkg-config --libs-only-l SDL2_net) \
	$(shell pkg-config --libs-only-l SDL2_image) \
	$(shell pkg-config --libs-only-l libavformat) \
	$(shell pkg-config --libs-only-l libavcodec) \
	$(shell pkg-config --libs-only-l libswresample) \
	$(shell pkg-config --libs-only-l libavutil) \
	$(shell pkg-config --libs-only-l openal) \
	$(shell pkg-config --libs-only-l luajit) \
	$(shell pkg-config --libs-only-l spng) \
	$(shell pkg-config --libs-only-l minizip) \
	$(shell pkg-config --libs-only-l zlib) \
	-lminiupnpc \
	-lnatpmp

TOML_SOURCES = \
	deps/centitoml/toml_api.c

TOML_OBJECTS = $(patsubst deps/centitoml/%.c,obj/centitoml/%.o,$(TOML_SOURCES))

TOML_INCLUDES = \
	-Ideps/centijson/include

TOML_CFLAGS += -O3 $(ARCH_CFLAGS) $(TOML_INCLUDES) -Wall -Wextra -Werror -Wno-unused-parameter

ifeq ($(ENABLE_LTO), 1)
KFX_CFLAGS += -flto
KFX_CXXFLAGS += -flto
KFX_LDFLAGS += -flto=auto
TOML_CFLAGS += -flto
endif

all: bin/keeperfx

clean:
	rm -rf obj bin src/ver_defs.h deps/astronomy deps/centijson deps/enet6

.PHONY: all clean

bin/keeperfx: $(KFX_OBJECTS) $(TOML_OBJECTS) | bin
	$(CXX) -o $@ $(KFX_OBJECTS) $(TOML_OBJECTS) $(KFX_LDFLAGS)

$(KFX_C_OBJECTS): obj/%.o: src/%.c src/ver_defs.h | obj
	$(MKDIR) $(dir $@)
	$(CC) $(KFX_CFLAGS) -c $< -o $@

$(KFX_CXX_OBJECTS): obj/%.o: src/%.cpp src/ver_defs.h | obj
	$(MKDIR) $(dir $@)
	$(CXX) $(KFX_CXXFLAGS) -c $< -o $@

$(TOML_OBJECTS): obj/centitoml/%.o: deps/centitoml/%.c | obj/centitoml
	$(CC) $(TOML_CFLAGS) -c $< -o $@

bin obj deps/astronomy deps/centijson deps/enet6 obj/centitoml:
	$(MKDIR) $@

src/actionpt.c: deps/centijson/include/json.h
src/api.c: deps/centijson/include/json.h
src/bflib_enet.cpp: deps/enet6/include/enet6/enet.h
src/moonphase.c: deps/astronomy/include/astronomy.h
deps/centitoml/toml_api.c: deps/centijson/include/json.h
deps/centitoml/toml_conv.c: deps/centijson/include/json.h

deps/astronomy/include/astronomy.h: | deps/astronomy
	@set -eux; \
	if [ -f "$@" ]; then exit 0; fi; \
	if [ "$(DEPS_DOWNLOAD)" = "1" ] && [ -n "$(DEPS_SUFFIX)" ]; then \
		if [ ! -f "deps/astronomy-$(DEPS_SUFFIX).tar.gz" ]; then \
			curl -Lfso "deps/astronomy-$(DEPS_SUFFIX).tar.gz" \
				"https://github.com/dkfans/kfx-deps/releases/download/$(DEPS_ASTRONOMY_TAG)/astronomy-$(DEPS_SUFFIX).tar.gz"; \
		fi; \
		if [ -f "deps/astronomy-$(DEPS_SUFFIX).tar.gz" ]; then \
			tar xzmf "deps/astronomy-$(DEPS_SUFFIX).tar.gz" -C deps/astronomy; \
			exit 0; \
		fi; \
	fi; \
	rm -rf "$(DEPS_BUILD_DIR)/astronomy"; \
	$(MKDIR) "$(DEPS_BUILD_DIR)"; \
	git clone --depth "$(DEPS_CLONE_DEPTH)" "$(DEPS_ASTRONOMY_REPO)" "$(DEPS_BUILD_DIR)/astronomy"; \
	if [ -n "$(DEPS_ASTRONOMY_REF)" ]; then git -C "$(DEPS_BUILD_DIR)/astronomy" checkout "$(DEPS_ASTRONOMY_REF)"; fi; \
	if [ -f "$(DEPS_BUILD_DIR)/astronomy/CMakeLists.txt" ]; then \
		cmake -S "$(DEPS_BUILD_DIR)/astronomy" -B "$(DEPS_BUILD_DIR)/astronomy/build" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release; \
		cmake --build "$(DEPS_BUILD_DIR)/astronomy/build"; \
	else \
		astro_src=$$(find "$(DEPS_BUILD_DIR)/astronomy" -path "*/source/c/astronomy.c" | head -n 1); \
		if [ -z "$$astro_src" ]; then astro_src=$$(find "$(DEPS_BUILD_DIR)/astronomy" -path "*/source/astronomy.c" | head -n 1); fi; \
		if [ -z "$$astro_src" ]; then astro_src=$$(find "$(DEPS_BUILD_DIR)/astronomy" -path "*/src/astronomy.c" | head -n 1); fi; \
		if [ -z "$$astro_src" ]; then echo "astronomy.c not found"; exit 1; fi; \
		astro_inc=$$(find "$(DEPS_BUILD_DIR)/astronomy" -path "*/source/c/astronomy.h" -exec dirname {} \; | head -n 1); \
		if [ -z "$$astro_inc" ]; then astro_inc=$$(find "$(DEPS_BUILD_DIR)/astronomy" -path "*/source/astronomy.h" -exec dirname {} \; | head -n 1); fi; \
		if [ -z "$$astro_inc" ]; then astro_inc=$$(find "$(DEPS_BUILD_DIR)/astronomy" -path "*/src/astronomy.h" -exec dirname {} \; | head -n 1); fi; \
		if [ -z "$$astro_inc" ]; then echo "astronomy.h not found"; exit 1; fi; \
		$(MKDIR) "$(DEPS_BUILD_DIR)/astronomy/build"; \
		$(CC) -O3 -I"$$astro_inc" -c "$$astro_src" -o "$(DEPS_BUILD_DIR)/astronomy/build/astronomy.o"; \
		ar rcs "$(DEPS_BUILD_DIR)/astronomy/build/libastronomy.a" "$(DEPS_BUILD_DIR)/astronomy/build/astronomy.o"; \
	fi; \
	$(MKDIR) deps/astronomy/include; \
	header_path=$$(find "$(DEPS_BUILD_DIR)/astronomy" -name astronomy.h | head -n 1); \
	lib_path=$$(find "$(DEPS_BUILD_DIR)/astronomy" -name libastronomy.a | head -n 1); \
	if [ -z "$$header_path" ] || [ -z "$$lib_path" ]; then echo "astronomy build output not found"; exit 1; fi; \
	cp "$$header_path" deps/astronomy/include/; \
	cp "$$lib_path" deps/astronomy/libastronomy.a

deps/centijson/include/json.h: | deps/centijson
	@set -eux; \
	if [ -f "$@" ]; then exit 0; fi; \
	if [ "$(DEPS_DOWNLOAD)" = "1" ] && [ -n "$(DEPS_SUFFIX)" ]; then \
		if [ ! -f "deps/centijson-$(DEPS_SUFFIX).tar.gz" ]; then \
			curl -Lfso "deps/centijson-$(DEPS_SUFFIX).tar.gz" \
				"https://github.com/dkfans/kfx-deps/releases/download/$(DEPS_CENTIJSON_TAG)/centijson-$(DEPS_SUFFIX).tar.gz"; \
		fi; \
		if [ -f "deps/centijson-$(DEPS_SUFFIX).tar.gz" ]; then \
			tar xzmf "deps/centijson-$(DEPS_SUFFIX).tar.gz" -C deps/centijson; \
			exit 0; \
		fi; \
	fi; \
	rm -rf "$(DEPS_BUILD_DIR)/centijson"; \
	$(MKDIR) "$(DEPS_BUILD_DIR)"; \
	git clone --depth "$(DEPS_CLONE_DEPTH)" "$(DEPS_CENTIJSON_REPO)" "$(DEPS_BUILD_DIR)/centijson"; \
	if [ -n "$(DEPS_CENTIJSON_REF)" ]; then git -C "$(DEPS_BUILD_DIR)/centijson" checkout "$(DEPS_CENTIJSON_REF)"; fi; \
	if [ "$(PLATFORM)" = "mac" ]; then \
		find "$(DEPS_BUILD_DIR)/centijson" -name "*.c" -print0 | xargs -0 perl -pi -e 's@#include <malloc.h>@#include <stdlib.h>@'; \
	fi; \
	cmake -S "$(DEPS_BUILD_DIR)/centijson" -B "$(DEPS_BUILD_DIR)/centijson/build" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release; \
	cmake --build "$(DEPS_BUILD_DIR)/centijson/build"; \
	$(MKDIR) deps/centijson/include; \
	header_path=$$(find "$(DEPS_BUILD_DIR)/centijson" \( -name value.h -o -name json.h \) | head -n 1); \
	header_dir=$$(dirname "$$header_path"); \
	lib_path=$$(find "$(DEPS_BUILD_DIR)/centijson" \( -name libjson.a -o -name libcentijson.a \) | head -n 1); \
	if [ -z "$$header_path" ] || [ -z "$$lib_path" ]; then echo "centijson build output not found"; exit 1; fi; \
	cp "$$header_dir"/*.h deps/centijson/include/; \
	cp "$$lib_path" deps/centijson/libjson.a

deps/enet6/include/enet6/enet.h: | deps/enet6
	@set -eux; \
	if [ -f "$@" ]; then exit 0; fi; \
	if [ "$(DEPS_DOWNLOAD)" = "1" ] && [ -n "$(DEPS_SUFFIX)" ]; then \
		if [ ! -f "deps/enet6-$(DEPS_SUFFIX).tar.gz" ]; then \
			curl -Lfso "deps/enet6-$(DEPS_SUFFIX).tar.gz" \
				"https://github.com/dkfans/kfx-deps/releases/download/$(DEPS_ENET6_TAG)/enet6-$(DEPS_SUFFIX).tar.gz"; \
		fi; \
		if [ -f "deps/enet6-$(DEPS_SUFFIX).tar.gz" ]; then \
			tar xzmf "deps/enet6-$(DEPS_SUFFIX).tar.gz" -C deps/enet6; \
			exit 0; \
		fi; \
	fi; \
	rm -rf "$(DEPS_BUILD_DIR)/enet6"; \
	$(MKDIR) "$(DEPS_BUILD_DIR)"; \
	git clone --depth "$(DEPS_CLONE_DEPTH)" "$(DEPS_ENET6_REPO)" "$(DEPS_BUILD_DIR)/enet6"; \
	if [ -n "$(DEPS_ENET6_REF)" ]; then git -C "$(DEPS_BUILD_DIR)/enet6" checkout "$(DEPS_ENET6_REF)"; fi; \
	cmake -S "$(DEPS_BUILD_DIR)/enet6" -B "$(DEPS_BUILD_DIR)/enet6/build" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release; \
	cmake --build "$(DEPS_BUILD_DIR)/enet6/build"; \
	$(MKDIR) deps/enet6/include/enet6; \
	header_path=$$(find "$(DEPS_BUILD_DIR)/enet6" -path "*/enet6/enet.h" | head -n 1); \
	lib_path=$$(find "$(DEPS_BUILD_DIR)/enet6" \( -name libenet6.a -o -name libenet.a \) | head -n 1); \
	if [ -z "$$header_path" ] || [ -z "$$lib_path" ]; then echo "enet6 build output not found"; exit 1; fi; \
	header_dir=$$(dirname "$$header_path"); \
	cp "$$header_dir"/*.h deps/enet6/include/enet6/; \
	cp "$$lib_path" deps/enet6/libenet6.a

src/ver_defs.h: version.mk
	$(ECHO) "#define VER_MAJOR   $(VER_MAJOR)" > $@.swp
	$(ECHO) "#define VER_MINOR   $(VER_MINOR)" >> $@.swp
	$(ECHO) "#define VER_RELEASE $(VER_RELEASE)" >> $@.swp
	$(ECHO) "#define VER_BUILD   $(BUILD_NUMBER)" >> $@.swp
	$(ECHO) "#define VER_STRING  \"$(VER_STRING)\"" >> $@.swp
	$(ECHO) "#define PACKAGE_SUFFIX  \"$(VER_SUFFIX)\"" >> $@.swp
	$(ECHO) "#define GIT_REVISION  \"$(shell git describe  --always)\"" >> $@.swp
	$(MV) $@.swp $@
