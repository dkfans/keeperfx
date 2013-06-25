# Tools and libraries to be used on the host system, not on target
ifneq (,$(findstring Windows,$(OS)))
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-win.zip
  PNGTOICO_DOWNLOAD=https://github.com/dkfans/png2ico/releases/download/2003-01-14/png2ico-win-2003-01-14.zip
  PNGTORAW_DOWNLOAD=https://github.com/dkfans/pngpal2raw/releases/download/v1.0.2/pngpal2raw-1_0_2_35-devel-win.zip
  PNGTOBSPAL_DOWNLOAD=https://github.com/dkfans/png2bestpal/releases/download/v1.0.2/png2bestpal-1_0_2_20-devel-win.zip
  POTONGDAT_DOWNLOAD=https://github.com/dkfans/po2ngdat/releases/download/v1.0.2.30/po2ngdat-1_0_2_30-devel-win.zip
  RNCTOOLS_DOWNLOAD=https://github.com/dkfans/rnctools/releases/download/v1.0.2/rnctools-1_0_2_5-devel-win.zip
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-lin.tar.gz
  PNGTOICO_DOWNLOAD=https://github.com/dkfans/png2ico/releases/download/2003-01-14/png2ico-lin-2003-01-14.tar.gz
  PNGTORAW_DOWNLOAD=https://github.com/dkfans/pngpal2raw/releases/download/v1.0.2/pngpal2raw-1_0_2_35-devel-lin.tar.gz
  PNGTOBSPAL_DOWNLOAD=https://github.com/dkfans/png2bestpal/releases/download/v1.0.2/png2bestpal-1_0_2_20-devel-lin.tar.gz
  POTONGDAT_DOWNLOAD=https://github.com/dkfans/po2ngdat/releases/download/v1.0.2.30/po2ngdat-1_0_2_30-devel-lin.tar.gz
  RNCTOOLS_DOWNLOAD=https://github.com/dkfans/rnctools/releases/download/v1.0.2/rnctools-1_0_2_5-devel-lin.tar.gz
else
  PERESEC_DOWNLOAD=no_prebuild_available_for_your_os
  PNGTOICO_DOWNLOAD=no_prebuild_available_for_your_os
  PNGTORAW_DOWNLOAD=no_prebuild_available_for_your_os
  PNGTOBSPAL_DOWNLOAD=no_prebuild_available_for_your_os
  POTONGDAT_DOWNLOAD=no_prebuild_available_for_your_os
  RNCTOOLS_DOWNLOAD=no_prebuild_available_for_your_os
endif
endif
PERESEC_PACKAGE=$(notdir $(PERESEC_DOWNLOAD))
PNGTOICO_PACKAGE=$(notdir $(PNGTOICO_DOWNLOAD))
PNGTORAW_PACKAGE=$(notdir $(PNGTORAW_DOWNLOAD))
PNGTOBSPAL_PACKAGE=$(notdir $(PNGTOBSPAL_DOWNLOAD))
POTONGDAT_PACKAGE=$(notdir $(POTONGDAT_DOWNLOAD))
RNCTOOLS_PACKAGE=$(notdir $(RNCTOOLS_DOWNLOAD))

# Tools and libraries to be used for the target system
# Currently, the target is always windows-mingw32
SDL_DOWNLOAD=https://www.libsdl.org/release/SDL-devel-1.2.14-mingw32.tar.gz
SDL_PACKAGE=$(notdir $(SDL_DOWNLOAD))
SDL_NET_DOWNLOAD=https://www.libsdl.org/projects/SDL_net/release/SDL_net-devel-1.2.7-VC8.zip
SDL_NET_PACKAGE=$(notdir $(SDL_NET_DOWNLOAD))

# Assets which are sources of data files are platform-independent
GFXSRC_DOWNLOAD=http://keeper.lubiki.pl/releases_big/gfx_sources_v5.7z
GFXSRC_PACKAGE=$(notdir $(GFXSRC_DOWNLOAD))
