# Tools and libraries to be used on the host system, not on target
ifneq (,$(findstring Windows,$(OS)))
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-win.zip
  PNGTOICO_DOWNLOAD=https://github.com/dkfans/png2ico/releases/download/2003-01-14/png2ico-win-2003-01-14.zip
  POTONGDAT_DOWNLOAD=no_prebuild_available_for_your_os
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-lin.tar.gz
  PNGTOICO_DOWNLOAD=https://github.com/dkfans/png2ico/releases/download/2003-01-14/png2ico-lin-2003-01-14.tar.gz
  POTONGDAT_DOWNLOAD=no_prebuild_available_for_your_os
else
  PERESEC_DOWNLOAD=no_prebuild_available_for_your_os
  PNGTOICO_DOWNLOAD=no_prebuild_available_for_your_os
  POTONGDAT_DOWNLOAD=no_prebuild_available_for_your_os
endif
endif
PERESEC_PACKAGE=$(notdir $(PERESEC_DOWNLOAD))
PNGTOICO_PACKAGE=$(notdir $(PNGTOICO_DOWNLOAD))
POTONGDAT_PACKAGE=$(notdir $(POTONGDAT_DOWNLOAD))

# Tools and libraries to be used for the target system
# Currently, the target is always windows-mingw32
SDL_DOWNLOAD=https://www.libsdl.org/release/SDL-devel-1.2.14-mingw32.tar.gz
SDL_PACKAGE=$(notdir $(SDL_DOWNLOAD))
SDL_NET_DOWNLOAD=https://www.libsdl.org/projects/SDL_net/release/SDL_net-devel-1.2.7-VC8.zip
SDL_NET_PACKAGE=$(notdir $(SDL_NET_DOWNLOAD))
