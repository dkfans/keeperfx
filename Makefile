#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file Makefile
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Allows invoking "make all" or similar commands to compile all source
#      files and link them into an executable file.
#  @par Comment:
#      Run make from a POSIX-compatible shell; Windows cmd.exe is not supported.
#      A MinGW-w64 toolchain and coreutils are required to build the project.
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
  LINKFLAGS = -static-libgcc -static-libstdc++ -Wl,--enable-auto-import -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
else
  CROSS_EXEEXT =
  CROSS_COMPILE = i686-w64-mingw32-
  LINKFLAGS = -static-libgcc -static-libstdc++ -Wl,--enable-auto-import
endif
EXEEXT = .exe
CPP      = $(CROSS_COMPILE)g++
CC       = $(CROSS_COMPILE)gcc
WINDRES  = $(CROSS_COMPILE)windres
DOXYTOOL = doxygen
BUILD_NUMBER ?= $(VER_BUILD)
PACKAGE_SUFFIX ?= Prototype
BUILD_START := $(shell date +%s.%N)
PNGTOICO = tools/png2ico/png2ico$(CROSS_EXEEXT)
PNGTORAW = tools/pngpal2raw/bin/pngpal2raw$(CROSS_EXEEXT)
PNGTOBSPAL = tools/png2bestpal/bin/png2bestpal$(CROSS_EXEEXT)
POTONGDAT = tools/po2ngdat/bin/po2ngdat$(CROSS_EXEEXT)
WAVTODAT = tools/sndbanker/bin/sndbanker$(CROSS_EXEEXT)
RNC      = tools/rnctools/bin/rnc$(CROSS_EXEEXT)
DERNC    = tools/rnctools/bin/dernc$(CROSS_EXEEXT)
RM       = rm -f
MV       = mv -f
CP       = cp -f
MKDIR    = mkdir -p
ECHO     = @echo
COMPILER_CACHE := $(shell command -v ccache 2> /dev/null || command -v sccache 2> /dev/null)
CV2PDB := $(shell PATH="$(CURDIR):$$PATH" command -v cv2pdb.exe 2> /dev/null)
ifneq (,$(COMPILER_CACHE))
CPP      := $(COMPILER_CACHE) $(CPP)
CC       := $(COMPILER_CACHE) $(CC)
endif
CACHE_KEY := $(shell test -w /var/tmp && echo "$(CURDIR)" | cksum | cut -d' ' -f1)
OBJDIR ?= $(if $(CACHE_KEY),/var/tmp/kfx-$(CACHE_KEY),obj)
BIN      = bin/keeperfx$(EXEEXT)
TEST_BIN = bin/tests$(EXEEXT)
HVLOGBIN = bin/keeperfx_hvlog$(EXEEXT)
GENSRC   = src/ver_defs.h
RES      = $(OBJDIR)/keeperfx_stdres.res
DEPS := \
	obj/centitoml/toml_api.o \
	deps/luajit/lib/libluajit.a
FTEST_DEBUG ?= 0
FTEST_DBGFLAGS := $(if $(filter 1,$(FTEST_DEBUG)),-DFUNCTESTING=1,)
ifneq (,$(filter 1,$(FTEST_DEBUG)))
FTEST_OBJS := \
	obj/ftests/ftest.o \
	obj/ftests/ftest_util.o \
	obj/ftests/ftest_list.o \
	$(patsubst src/ftests/tests/%,obj/ftests/tests/%,$(patsubst %.c,%.o,$(wildcard src/ftests/tests/ftest*.c)))
endif
C_SRCS := $(wildcard \
	src/*.c \
	src/kfx/*.c \
	src/kfx/*/*.c \
	src/kfx/*/*/*.c)
CXX_SRCS := $(filter-out src/kfx/platform/PlatformLinux.cpp src/main.cpp,$(wildcard \
	src/*.cpp \
	src/kfx/*.cpp \
	src/kfx/*/*.cpp \
	src/kfx/*/*/*.cpp))
OBJS_CORE := \
	$(patsubst src/%.c,obj/%.o,$(C_SRCS)) \
	$(patsubst src/%.cpp,obj/%.o,$(CXX_SRCS))
OBJS := $(OBJS_CORE) $(FTEST_OBJS) $(DEPS)
MAIN_OBJ := obj/main.o
TESTS_OBJ := \
	$(OBJDIR)/tests/tst_main.o \
	$(OBJDIR)/tests/tst_fixes.o \
	$(OBJDIR)/tests/001_test.o \
	$(OBJDIR)/tests/tst_enet_server.o \
	$(OBJDIR)/tests/tst_enet_client.o
CU_DIR := deps/CUnit-2.1-3/CUnit
CU_INC := -idirafter"$(CU_DIR)/Headers"
CU_OBJS := \
	$(OBJDIR)/cu/Basic.o \
	$(OBJDIR)/cu/TestDB.o \
	$(OBJDIR)/cu/CUError.o \
	$(OBJDIR)/cu/TestRun.o \
	$(OBJDIR)/cu/Util.o
LINKLIB = -mwindows \
	-L"sdl/lib" -lSDL3 -lSDL3_mixer -lSDL3_image \
	-L"deps/ffmpeg/libavformat" -lavformat \
	-L"deps/ffmpeg/libavcodec" -lavcodec \
	-L"deps/ffmpeg/libswresample" -lswresample \
	-L"deps/ffmpeg/libavutil" -lavutil \
	-L"deps/openal" -lOpenAL32 \
	-L"deps/astronomy" -lastronomy \
	-L"deps/enet6/lib" -lenet6 \
	-L"deps/miniupnpc" -lminiupnpc \
	-L"deps/libnatpmp" -lnatpmp -liphlpapi \
	-L"deps/libcurl/lib" -lcurl -lwldap32 -lcrypt32 -lsecur32 -liphlpapi \
	-L"deps/spng" -lspng \
	-L"deps/centijson" -ljson \
	-L"deps/zlib" -lminizip -lz \
	-lwinmm -lmingw32 -limagehlp -lws2_32 -ldbghelp -lbcrypt -lole32 -luuid
INCS = \
	-I"src" \
	-isystem"deps/zlib/include" \
	-isystem"deps/spng/include" \
	-isystem"sdl/include" \
	-isystem"deps/enet6/include" \
	-isystem"deps/centijson/include" \
	-isystem"deps/centitoml" \
	-isystem"deps/astronomy/include" \
	-isystem"deps/ffmpeg" \
	-isystem"deps/openal/include" \
	-isystem"deps/luajit/include" \
	-isystem"deps/miniupnpc/include" \
	-isystem"deps/libnatpmp/include" \
	-isystem"deps/libcurl/include"
STDOBJS   := $(subst obj/,$(OBJDIR)/std/,$(OBJS))
HVLOGOBJS := $(subst obj/,$(OBJDIR)/hvlog/,$(OBJS))
STD_MAIN_OBJ := $(subst obj/,$(OBJDIR)/std/,$(MAIN_OBJ))
HVLOG_MAIN_OBJ := $(subst obj/,$(OBJDIR)/hvlog/,$(MAIN_OBJ))
NO_PCH_C = bflib_dernc bflib_text net_holepunch net_lan net_matchmaking centitoml/toml_api
NO_PCH_CXX = net_portforward kfx/platform/WindowSystemSDL
STD_CXX_O = \
	$(patsubst src/%.cpp,$(OBJDIR)/std/%.o,$(CXX_SRCS)) \
	$(STD_MAIN_OBJ)
HVLOG_CXX_O = \
	$(patsubst src/%.cpp,$(OBJDIR)/hvlog/%.o,$(CXX_SRCS)) \
	$(HVLOG_MAIN_OBJ)
STD_NO_PCH_C_O = $(addprefix $(OBJDIR)/std/,$(addsuffix .o,$(NO_PCH_C)))
HVLOG_NO_PCH_C_O = $(addprefix $(OBJDIR)/hvlog/,$(addsuffix .o,$(NO_PCH_C)))
STD_NO_PCH_CXX_O = $(addprefix $(OBJDIR)/std/,$(addsuffix .o,$(NO_PCH_CXX)))
HVLOG_NO_PCH_CXX_O = $(addprefix $(OBJDIR)/hvlog/,$(addsuffix .o,$(NO_PCH_CXX)))
$(STD_CXX_O): .EXTRA_PREREQS = $(OBJDIR)/std/.build_config $(OBJDIR)/std/kfx_pch_cxx.h.gch
$(filter-out $(STD_CXX_O),$(filter %.o,$(STDOBJS) $(STD_MAIN_OBJ))): \
	.EXTRA_PREREQS = $(OBJDIR)/std/.build_config $(OBJDIR)/std/kfx_pch_c.h.gch
$(HVLOG_CXX_O): .EXTRA_PREREQS = $(OBJDIR)/hvlog/.build_config $(OBJDIR)/hvlog/kfx_pch_cxx.h.gch
$(filter-out $(HVLOG_CXX_O),$(filter %.o,$(HVLOGOBJS) $(HVLOG_MAIN_OBJ))): \
	.EXTRA_PREREQS = $(OBJDIR)/hvlog/.build_config $(OBJDIR)/hvlog/kfx_pch_c.h.gch
$(TESTS_OBJ) $(CU_OBJS): .EXTRA_PREREQS = $(OBJDIR)/tests/.build_config
DEPFLAGS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -DSPNG_STATIC=1 -DAL_LIBTYPE_STATIC
DEBUG ?= 0
OPTFLAGS = -march=x86-64 -fno-omit-frame-pointer $(if $(filter 1,$(DEBUG)),-O0,-O3)
DBGFLAGS = $(if $(filter 1,$(DEBUG)),-g -DDEBUG,$(if $(CV2PDB),-g,))
std_LOGFLAGS = -DBFDEBUG_LEVEL=0
hvlog_LOGFLAGS = -DBFDEBUG_LEVEL=10
WARNFLAGS = \
	-Wall \
	-W \
	-Wshadow \
	-Werror \
	-Wno-sign-compare \
	-Wno-unused-parameter \
	-Wno-maybe-uninitialized \
	-Wno-strict-aliasing \
	-Wno-unknown-pragmas \
	-Wno-format-truncation
COMMONFLAGS := \
	$(INCS) \
	-c \
	-pipe \
	-fmessage-length=0 \
	-ftrack-macro-expansion=0 \
	$(WARNFLAGS) \
	$(OPTFLAGS) \
	$(DBGFLAGS) \
	$(FTEST_DBGFLAGS) \
	$(if $(filter 1,$(USE_PRE_FILE)),-DUSE_PRE_FILE=1,)
CXXFLAGS = $(COMMONFLAGS) $(DEPFLAGS) -std=gnu++20
CFLAGS = $(COMMONFLAGS) $(DEPFLAGS) -std=gnu11 -Werror=implicit -DCURL_STATICLIB
LDFLAGS = $(LINKLIB) $(DBGFLAGS) $(LINKFLAGS) -Wl,--no-print-map-discarded
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(BUILD_NUMBER) $(PACKAGE_SUFFIX)
ifeq ($(filter -j% --jobserver%,$(MAKEFLAGS)),)
  MAKEFLAGS += -j$(shell nproc)
endif
MAKEFLAGS += -rR
include prebuilds.mk
.DELETE_ON_ERROR:
.PHONY: all standard heavylog package tools tests cppcheck docs docsdox FORCE
.PHONY: clean clean-build deep-clean
.PHONY: clean-package deep-clean-package clean-tools deep-clean-tools clean-libexterns deep-clean-libexterns
STD_DEPFILES = \
	$(filter %.d,$(STDOBJS:%.o=%.d) $(STD_MAIN_OBJ:%.o=%.d)) \
	$(OBJDIR)/std/kfx_pch_c.h.d \
	$(OBJDIR)/std/kfx_pch_cxx.h.d
HVLOG_DEPFILES = \
	$(filter %.d,$(HVLOGOBJS:%.o=%.d) $(HVLOG_MAIN_OBJ:%.o=%.d)) \
	$(OBJDIR)/hvlog/kfx_pch_c.h.d \
	$(OBJDIR)/hvlog/kfx_pch_cxx.h.d
TEST_DEPFILES = $(filter %.d,$(TESTS_OBJ:%.o=%.d) $(CU_OBJS:%.o=%.d))
REQUESTED_GOALS = $(if $(MAKECMDGOALS),$(MAKECMDGOALS),all)
-include $(STD_DEPFILES)
ifneq ($(filter-out all standard clean clean-build,$(REQUESTED_GOALS)),)
-include $(HVLOG_DEPFILES)
-include $(TEST_DEPFILES)
endif
all: standard
	@duration=$$(awk "BEGIN { print $$(date +%s.%N) - $(BUILD_START) }"); printf '\033[97mCompile completed in %0.2fs\033[0m\n' $$duration
standard: $(BIN) $(BIN:%.exe=%.map)
heavylog: $(HVLOGBIN) $(HVLOGBIN:%.exe=%.map)
$(OBJDIR)/std/%: COMMONFLAGS += $(std_LOGFLAGS)
$(OBJDIR)/hvlog/%: COMMONFLAGS += $(hvlog_LOGFLAGS)
PCH_SRC = src/kfx_pch.h
PCH_HDRS = \
	$(OBJDIR)/std/kfx_pch_c.h \
	$(OBJDIR)/std/kfx_pch_cxx.h \
	$(OBJDIR)/hvlog/kfx_pch_c.h \
	$(OBJDIR)/hvlog/kfx_pch_cxx.h
PCH_C_FLAGS = -include "$(PCHDIR)/kfx_pch_c.h"
PCH_CXX_FLAGS = -include "$(PCHDIR)/kfx_pch_cxx.h"
$(OBJDIR)/std/%.o: PCHDIR = $(OBJDIR)/std
$(OBJDIR)/hvlog/%.o: PCHDIR = $(OBJDIR)/hvlog
$(STD_NO_PCH_C_O) $(HVLOG_NO_PCH_C_O): PCH_C_FLAGS =
$(STD_NO_PCH_CXX_O) $(HVLOG_NO_PCH_CXX_O): PCH_CXX_FLAGS =
$(STD_NO_PCH_C_O) $(STD_NO_PCH_CXX_O): .EXTRA_PREREQS = $(OBJDIR)/std/.build_config
$(HVLOG_NO_PCH_C_O) $(HVLOG_NO_PCH_CXX_O): .EXTRA_PREREQS = $(OBJDIR)/hvlog/.build_config
$(PCH_HDRS): $(PCH_SRC)
	@$(CP) "$<" "$@"
$(OBJDIR)/%/kfx_pch_c.h.gch: $(OBJDIR)/%/kfx_pch_c.h $(OBJDIR)/%/.build_config | libexterns $(GENSRC)
	$(CC) $(filter-out -M%,$(CFLAGS)) \
		-MMD -MP -MF"$(@:%.h.gch=%.h.d)" -MT"$@" \
		-x c-header -o"$@" "$<"
$(OBJDIR)/%/kfx_pch_cxx.h.gch: $(OBJDIR)/%/kfx_pch_cxx.h $(OBJDIR)/%/.build_config | libexterns $(GENSRC)
	$(CPP) $(filter-out -M%,$(CXXFLAGS)) \
		-MMD -MP -MF"$(@:%.h.gch=%.h.d)" -MT"$@" \
		-x c++-header -o"$@" "$<"
FOLDERS = \
	bin \
	$(sort $(dir $(STDOBJS) $(HVLOGOBJS) $(STD_MAIN_OBJ) $(HVLOG_MAIN_OBJ) $(TESTS_OBJ) $(CU_OBJS))) \
	sdl/for_final_package
$(shell $(MKDIR) $(FOLDERS))
define UPDATE_BUILD_CONFIG
$(shell \
	printf '%s\n' '$(CPP) $(CXXFLAGS) $($(1)_LOGFLAGS) $(CC) $(CFLAGS) $($(1)_LOGFLAGS)' > "$(OBJDIR)/$(1)/.build_config.tmp"; \
	cmp -s "$(OBJDIR)/$(1)/.build_config.tmp" "$(OBJDIR)/$(1)/.build_config" || $(MV) "$(OBJDIR)/$(1)/.build_config.tmp" "$(OBJDIR)/$(1)/.build_config"; \
	$(RM) "$(OBJDIR)/$(1)/.build_config.tmp")
endef
$(foreach variant,std hvlog tests,$(call UPDATE_BUILD_CONFIG,$(variant)))
docs: docsdox
docsdox: docs/doxygen.conf
	VERSION=$(VER_STRING) $(DOXYTOOL) docs/doxygen.conf
deep-clean: deep-clean-tools deep-clean-package
	$(MAKE) -f libexterns.mk deep-clean-libexterns
clean: submodule clean-build clean-tools clean-libexterns clean-package
submodule:
	-git submodule init && git submodule update
clean-build:
	-$(RM) \
		$(filter $(OBJDIR)/%,$(STDOBJS) $(STD_MAIN_OBJ) $(HVLOGOBJS) $(HVLOG_MAIN_OBJ)) \
		$(filter %.d,$(STDOBJS:%.o=%.d) $(STD_MAIN_OBJ:%.o=%.d) $(HVLOGOBJS:%.o=%.d) $(HVLOG_MAIN_OBJ:%.o=%.d))
	-$(RM) \
		$(BIN) $(BIN:%.exe=%.map) $(BIN:%.exe=%.pdb) \
		$(HVLOGBIN) $(HVLOGBIN:%.exe=%.map) $(HVLOGBIN:%.exe=%.pdb)
	-$(RM) bin/keeperfx.dll
	-$(RM) $(GENSRC) $(OBJDIR)/keeperfx.* $(OBJDIR)/keeperfx_hvlog.* $(OBJDIR)/tests.*
define LINK_RULE
$1 $2 &: $(GENSRC) $3 | libexterns
	$(CPP) -o"$(OBJDIR)/$4" $3 $(LDFLAGS) -Wl,-Map,"$(OBJDIR)/$5"
	@$(CP) "$(OBJDIR)/$4" "$1" && $(CP) "$(OBJDIR)/$5" "$2" && $(RM) "$(OBJDIR)/$4" "$(OBJDIR)/$5"
	$(if $(CV2PDB),$(CV2PDB) -C "$1")
endef
$(eval $(call LINK_RULE,$(BIN),$(BIN:%.exe=%.map),$(STDOBJS) $(STD_MAIN_OBJ) $(RES),keeperfx.exe,keeperfx.map))
$(eval $(call LINK_RULE,$(HVLOGBIN),$(HVLOGBIN:%.exe=%.map),$(HVLOGOBJS) $(HVLOG_MAIN_OBJ) $(RES),keeperfx_hvlog.exe,keeperfx_hvlog.map))
$(eval $(call LINK_RULE,$(TEST_BIN),$(TEST_BIN:%.exe=%.map),$(TESTS_OBJ) $(STDOBJS) $(CU_OBJS) $(RES),tests.exe,tests.map))
$(OBJDIR)/std/centitoml/toml_api.o $(OBJDIR)/hvlog/centitoml/toml_api.o: deps/centitoml/toml_api.c
	$(CC) $(CFLAGS) -o"$@" "$<"
$(OBJDIR)/tests/%.o: src/tests/%.cpp | $(GENSRC)
	$(CPP) $(CXXFLAGS) -I"src/" $(CU_INC) -o"$@" "$<"
$(OBJDIR)/cu/%.o: $(CU_DIR)/Sources/Framework/%.c
	$(CPP) $(CXXFLAGS) $(CU_INC) -o"$@" "$<"
$(OBJDIR)/cu/%.o: $(CU_DIR)/Sources/Basic/%.c
	$(CPP) $(CXXFLAGS) $(CU_INC) -o"$@" "$<"
define COMPILE_RULES
$(patsubst src/%.cpp,$(OBJDIR)/$1/%.o,$(CXX_SRCS) src/main.cpp): \
	$(OBJDIR)/$1/%.o: src/%.cpp | libexterns $(GENSRC)
	$$(CPP) $$(CXXFLAGS) $$(PCH_CXX_FLAGS) -o"$$@" "$$<"
$(patsubst src/%.c,$(OBJDIR)/$1/%.o,$(C_SRCS)) $(subst obj/,$(OBJDIR)/$1/,$(FTEST_OBJS)): \
	$(OBJDIR)/$1/%.o: src/%.c | libexterns $(GENSRC)
	$$(CC) $$(CFLAGS) $$(PCH_C_FLAGS) -o"$$@" "$$<"
endef
$(foreach variant,std hvlog,$(eval $(call COMPILE_RULES,$(variant))))
$(OBJDIR)/%.res: res/%.rc res/keeperfx_icon.ico src/version.h $(GENSRC)
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff -I"$(OBJDIR)/"
.SECONDARY: res/keeperfx_icon.ico
res/%.ico: \
	res/%016-08bpp.png \
	res/%032-08bpp.png \
	res/%048-08bpp.png \
	res/%064-08bpp.png \
	res/%128-08bpp.png \
	res/%128-24bpp.png \
	res/%256-24bpp.png \
	res/%512-24bpp.png \
	$(PNGTOICO)
	-$(ECHO) 'Building icon: $@'
	$(PNGTOICO) "$@" \
		$(word 8,$^) $(word 7,$^) $(word 6,$^) \
		--colors 256 $(word 5,$^) $(word 4,$^) $(word 3,$^) \
		--colors 16 $(word 2,$^) $(word 1,$^)
src/ver_defs.h: FORCE version.mk Makefile
	@printf '%s\n' \
		'#define VER_MAJOR $(VER_MAJOR)' \
		'#define VER_MINOR $(VER_MINOR)' \
		'#define VER_RELEASE $(VER_RELEASE)' \
		'#define VER_BUILD $(BUILD_NUMBER)' \
		'#define VER_STRING "$(VER_STRING)"' \
		'#define PACKAGE_SUFFIX "$(PACKAGE_SUFFIX)"' \
		"#define GIT_REVISION \"$$(git describe --always)\"" \
		> "$@.tmp"
	@cmp -s "$@.tmp" "$@" || $(MV) "$@.tmp" "$@"; $(RM) "$@.tmp"
tests: $(TEST_BIN) $(TEST_BIN:%.exe=%.map)
libexterns: libexterns.mk
	$(MAKE) -f libexterns.mk
clean-libexterns: libexterns.mk
	-$(MAKE) -f libexterns.mk clean-libexterns
	-$(RM) -rf \
		deps/enet6 deps/zlib deps/spng deps/astronomy deps/centijson \
		deps/luajit deps/miniupnpc deps/libnatpmp deps/libcurl
	-$(RM) libexterns
deps/enet6 deps/zlib deps/spng deps/astronomy deps/centijson deps/ffmpeg \
deps/openal deps/luajit deps/miniupnpc deps/libnatpmp deps/libcurl:
	$(MKDIR) $@
src/api.c: deps/centijson/include/json.h
src/bflib_enet.cpp: deps/enet6/include/enet6/enet.h
src/custom_sprites.c: deps/zlib/include/zlib.h deps/spng/include/spng.h deps/centijson/include/json.h
src/custom_zip.c: deps/zlib/include/zlib.h deps/centijson/include/json.h
src/moonphase.c: deps/astronomy/include/astronomy.h
deps/centitoml/toml_api.c: deps/centijson/include/json.h
deps/centitoml/toml_conv.c: deps/centijson/include/json.h
src/bflib_fmvids.cpp: deps/ffmpeg/libavformat/avformat.h
src/bflib_sndlib.cpp: deps/openal/include/AL/al.h
src/net_exchange_gameplay.c: deps/zlib/include/zlib.h
src/net_resync.cpp: deps/zlib/include/zlib.h
src/console_cmd.c: deps/luajit/include/lua.h
src/net_portforward.cpp: deps/miniupnpc/include/miniupnpc/miniupnpc.h deps/libnatpmp/include/natpmp/natpmp.h
src/net_matchmaking.c: deps/libcurl/include/curl/curl.h
KFX_DEPS_URL = https://github.com/dkfans/kfx-deps/releases/download
define DEP_PKG
deps/$1-mingw32.tar.gz:
	$(MKDIR) "$$(@D)"
	curl -Lso "$$@" "$(KFX_DEPS_URL)/$2/$1-mingw32.tar.gz" && tar -tzf "$$@" >/dev/null
$3: deps/$1-mingw32.tar.gz | deps/$1
	tar xzmf "$$<" -C deps/$1
endef
$(eval $(call DEP_PKG,enet6,20260212,deps/enet6/include/enet6/enet.h))
$(eval $(call DEP_PKG,zlib,initial,deps/zlib/include/zlib.h))
$(eval $(call DEP_PKG,spng,initial,deps/spng/include/spng.h))
$(eval $(call DEP_PKG,astronomy,astronomy_fix,deps/astronomy/include/astronomy.h))
$(eval $(call DEP_PKG,centijson,initial,deps/centijson/include/json.h))
$(eval $(call DEP_PKG,ffmpeg,initial,deps/ffmpeg/libavformat/avformat.h))
$(eval $(call DEP_PKG,openal,2024-11-14,deps/openal/include/AL/al.h))
$(eval $(call DEP_PKG,luajit,20250418,deps/luajit/include/lua.h))
$(eval $(call DEP_PKG,miniupnpc,20260102,deps/miniupnpc/include/miniupnpc/miniupnpc.h))
$(eval $(call DEP_PKG,libnatpmp,20260102,deps/libnatpmp/include/natpmp/natpmp.h))
$(eval $(call DEP_PKG,libcurl,20260310,deps/libcurl/include/curl/curl.h))
deps/luajit/lib/libluajit.a: | deps/luajit/include/lua.h
CPSUPPRESS = \
	missingIncludeSystem constParameterPointer constVariablePointer functionConst \
	unreadVariable uninitvar variableScope unusedStructMember \
	funcArgNamesDifferent funcArgOrderDifferent cstyleCast functionStatic \
	unsignedLessThanZero constParameterCallback constParameter knownConditionTrueFalse \
	negativeIndex nullPointerRedundantCheck nullPointerArithmeticRedundantCheck \
	invalidscanf invalidScanfArgType_int invalidPrintfArgType_uint invalidPrintfArgType_sint \
	redundantAssignment preprocessorErrorDirective uninitMemberVar truncLongCastAssignment \
	shiftNegativeLHS bitwiseOnBoolean shiftTooManyBits shiftTooManyBitsSigned \
	identicalConditionAfterEarlyExit useInitializationList operatorEqVarError noExplicitConstructor \
	useStlAlgorithm duplicateExpression duplicateBranch duplicateConditionalAssign \
	duplicateAssignExpression nullPointer compareValueOutOfTypeRangeError redundantInitialization \
	multiCondition internalAstError clarifyCondition memsetClassFloat comparePointers \
	identicalInnerCondition uselessAssignmentPtrArg unassignedVariable shiftNegative \
	duplicateCondition badBitmaskCheck shadowFunction shadowVariable \
	uninitStructMember CastIntegerToAddressAtReturn
cppcheck: | \
	src/ver_defs.h \
	deps/zlib/include/zlib.h \
	deps/spng/include/spng.h \
	deps/astronomy/include/astronomy.h \
	deps/centijson/include/json.h \
	deps/enet6/include/enet6/enet.h \
	deps/luajit/include/lua.h \
	deps/openal/include/AL/al.h \
	deps/ffmpeg/libavformat/avformat.h
cppcheck:
	$(MKDIR) cppcheck.cache
	cppcheck \
		--cppcheck-build-dir=cppcheck.cache \
		--check-level=exhaustive \
		--enable=all \
		--platform=win32A \
		--std=c++20 \
		--inconclusive \
		-j $(shell nproc) \
		-q \
		-I deps/zlib/include \
		-I deps/spng/include \
		-I sdl/include \
		-I deps/enet6/include \
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
		$(addprefix --suppress=,$(CPSUPPRESS)) \
		src 2>cppcheck.log
include tool_png2ico.mk
include tool_pngpal2raw.mk
include tool_png2bestpal.mk
include tool_po2ngdat.mk
include tool_sndbanker.mk
include tool_rnctools.mk
PKG_GOALS = \
	package \
	pkg-assemble \
	pkg-gfx \
	pkg-landviews \
	pkg-menugfx \
	pkg-enginegfx \
	pkg-sfx \
	convert-sfx \
	clean \
	deep-clean \
	clean-package \
	deep-clean-package
ifneq ($(filter $(PKG_GOALS) pkg/% tools/% sfx/%,$(MAKECMDGOALS)),)
CAMPAIGNS = $(patsubst campgns/%.cfg,%,$(wildcard campgns/*.cfg))
MAPPACKS = $(patsubst levels/%.cfg,%,$(filter-out %/personal.cfg,$(wildcard levels/*.cfg)))
LANGS = eng chi cht cze dut fre ger ita jpn kor lat pol rus spa swe
include pkg_lang.mk
include pkg_gfx.mk
include pkg_sfx.mk
include package.mk
endif
export RM CP MKDIR MV ECHO
