# Project: keeperfx
# Makefile created by Dev-C++, modified manually

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
EXETODLL = tools/peresec/peresec
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
LIBS     = obj/libkeeperfx.a  directx/lib/libddraw.a
OBJS  = \
obj/main.o \
obj/front_simple.o \
obj/game_saves.o \
obj/frontend.o \
obj/packets.o \
obj/config.o \
obj/config_campaigns.o \
obj/config_creature.o \
obj/config_crtrmodel.o \
obj/config_lenses.o \
obj/config_magic.o \
obj/config_rules.o \
obj/config_terrain.o \
obj/front_input.o \
obj/front_credits.o \
obj/front_landview.o \
obj/player_computer.o \
obj/player_instances.o \
obj/creature_control.o \
obj/creature_states.o \
obj/engine_lenses.o \
obj/engine_camera.o \
obj/engine_render.o \
obj/lens_mist.o \
obj/room_data.o \
obj/slab_data.o \
obj/map_data.o \
obj/map_columns.o \
obj/vidmode.o \
obj/scrcapt.o \
obj/kjm_input.o \
obj/gui_draw.o \
obj/gui_tooltips.o \
obj/lvl_script.o \
obj/lvl_filesdk1.o \
obj/game_merge.o \
obj/thing_doors.o \
obj/thing_objects.o \
obj/thing_list.o \
obj/thing_creature.o \
obj/bflib_basics.o \
obj/bflib_dernc.o \
obj/bflib_fileio.o \
obj/bflib_keybrd.o \
obj/bflib_datetm.o \
obj/bflib_math.o \
obj/bflib_memory.o \
obj/bflib_cpu.o \
obj/bflib_pom.o \
obj/bflib_mouse.o \
obj/bflib_mshandler.o \
obj/bflib_mspointer.o \
obj/bflib_heapmgr.o \
obj/bflib_network.o \
obj/bflib_sndlib.o \
obj/bflib_netsp.o \
obj/bflib_netsp_ipx.o \
obj/bflib_semphr.o \
obj/bflib_sound.o \
obj/bflib_video.o \
obj/bflib_fmvids.o \
obj/bflib_filelst.o \
obj/bflib_guibtns.o \
obj/bflib_drawbas.o \
obj/bflib_drawsdk.o \
obj/bflib_vidraw.o \
obj/bflib_sprfnt.o \
obj/bflib_sprite.o \
$(RES)

LINKLIB =  -L"directx/lib" -mwindows obj/libkeeperfx.a -lwinmm -lddraw -g -O0  -march=i386 
INCS =  -I"directx/include"
CXXINCS =  -I"directx/include" 

STDOBJS   = $(subst obj/,obj/std/,$(OBJS))
HVLOGOBJS = $(subst obj/,obj/hvlog/,$(OBJS))

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
CXXFLAGS = $(CXXINCS) -c -fmessage-length=0 $(WARNFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
CFLAGS = $(INCS) -c -fmessage-length=0 $(WARNFLAGS) $(OPTFLAGS) $(DBGFLAGS) $(INCFLAGS)
LDFLAGS = $(LINKLIB) $(OPTFLAGS) $(DBGFLAGS) $(LINKFLAGS)

CAMPAIGNS  = \
ancntkpr \
keeporig \
questfth

CAMPAIGN_CFGS = $(patsubst %,pkg/campgns/%.cfg,$(CAMPAIGNS))

# load program version
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(VER_BUILD)

# mark icons as precious, because even though we can re-create them, it requires having "png2ico" tool
.PRECIOUS: res/%.ico
# name virtual targets
.PHONY: all standard std-before std-after debug hvlog-before hvlog-after docs docsdox clean clean-build clean-tools clean-package package pkg-before pkg-copydat pkg-campaigns tools

all: standard

standard: CXXFLAGS += $(STLOGFLAGS)
standard: CFLAGS += $(STLOGFLAGS)
standard: std-before $(BIN) std-after

heavylog: CXXFLAGS += $(HVLOGFLAGS)
heavylog: CFLAGS += $(HVLOGFLAGS)
heavylog: hvlog-before $(HVLOGBIN) hvlog-after

std-before:
	$(MKDIR) obj/std bin

hvlog-before:
	$(MKDIR) obj/hvlog bin

clean: clean-build clean-tools clean-package

clean-build:
	-$(RM) $(STDOBJS)
	-$(RM) $(HVLOGOBJS)
	-$(RM) $(BIN) $(BIN:%.exe=%.map)
	-$(RM) $(HVLOGBIN) $(HVLOGBIN:%.exe=%.map)
	-$(RM) bin/keeperfx.dll
	-$(RM) $(LIBS) $(GENSRC)
	-$(RM) obj/keeperfx.*
	-$(RM) directx/lib/lib*.a

clean-tools:
	make -C tools/peresec clean

clean-package:
	-$(RM) -R pkg/campgns
	-$(RM) -R pkg/creatrs
	-$(RM) -R pkg/fxdata
	-$(RM) -R pkg/ldata
	-$(RM) -R pkg/data
	-$(RM) -R pkg/sound
	-$(RM) -R pkg/levels
	-$(RM) pkg/keeperfx*

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

obj/std/%.o obj/hvlog/%.o: src/%.cpp $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/%.o obj/hvlog/%.o: src/%.c $(GENSRC)
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/std/%.res obj/hvlog/%.res: res/%.rc $(GENSRC)
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 

obj/ver_defs.h: version.mk Makefile
	$(ECHO) \#define VER_MAJOR   $(VER_MAJOR) > "$(@D)/tmp"
	$(ECHO) \#define VER_MINOR   $(VER_MINOR) >> "$(@D)/tmp"
	$(ECHO) \#define VER_RELEASE $(VER_RELEASE) >> "$(@D)/tmp"
	$(ECHO) \#define VER_BUILD   $(VER_BUILD) >> "$(@D)/tmp"
	$(ECHO) \#define VER_STRING  \"$(VER_STRING)\" >> "$(@D)/tmp"
	$(MV) "$(@D)/tmp" "$@"

obj/libkeeperfx.a: bin/keeperfx.dll obj/keeperfx.def
	-$(ECHO) 'Generating gcc library archive for: $<'
	$(DLLTOOL) --dllname "$<" --def "obj/keeperfx.def" --output-lib "$@"
	-$(ECHO) 'Finished generating: $@'
	-$(ECHO) ' '

bin/keeperfx.dll obj/keeperfx.def: lib/keeper95_gold.dll lib/keeper95_gold.map tools/peresec/peresec
	$(CP) lib/keeper95_gold.dll obj/keeperfx.dll
	$(CP) lib/keeper95_gold.map obj/keeperfx.map
	cd obj; \
	../$(EXETODLL) keeperfx "_DK_"
	$(MV) obj/keeperfx.dll bin/keeperfx.dll

tools/peresec/peresec: tools/peresec/peresec.c
	make -C tools/peresec

directx/lib/lib%.a: directx/lib/%.def
	make -C "$(@D)" "$(@F)" CC=$(CC) DLLTOOL=$(DLLTOOL) CFLAGS="-I\"../include\" -Wall"

package: pkg-before pkg-copydat pkg-campaigns
	$(CP) bin/* pkg/
	$(CP) docs/keeperfx_readme.txt pkg/
	cd pkg; \
	7z a "keeperfx-$(subst .,_,$(VER_STRING))-patch.7z" "*" -x!*/.svn -x!.svn -x!.git -x!*.7z

pkg-before:
	-$(RM) "pkg/keeperfx-$(subst .,_,$(VER_STRING))-patch.7z"
	$(MKDIR) pkg/creatrs
	$(MKDIR) pkg/fxdata
	$(MKDIR) pkg/campgns

pkg-copydat: pkg-before
	$(CP) config/keeperfx.cfg pkg/
	$(CP) config/creatrs/*.cfg pkg/creatrs/
	$(CP) config/fxdata/*.cfg pkg/fxdata/

pkg-campaigns: pkg-before $(CAMPAIGN_CFGS)

pkg/campgns/%.cfg: campgns/%.cfg
	@$(MKDIR) $(@D)
#	 Copy folder with campaign name (w/o extension), if it exists
	$(if $(wildcard $(<:%.cfg=%)),$(MKDIR) $(@:%.cfg=%))
	$(if $(wildcard $(<:%.cfg=%)),-$(CP) $(<:%.cfg=%)/map*.* $(@:%.cfg=%)/)
#	 Copy the actual campaign file
	$(CP) $< $@

