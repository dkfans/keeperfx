# Tools and libraries to be used on the host system, not on target
ifneq (,$(findstring Windows,$(OS)))
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-win.zip
  PNGTOICO_DOWNLOAD=https://github.com/dkfans/png2ico/releases/download/2024-10-28/png2ico-win-2024-10-28.zip
  PNGTORAW_DOWNLOAD=https://github.com/dkfans/pngpal2raw/releases/download/v1.0.2/pngpal2raw-1_0_2_35-devel-win.zip
  PNGTOBSPAL_DOWNLOAD=https://github.com/dkfans/png2bestpal/releases/download/v1.0.3/png2bestpal-1_0_3_21-devel-win.zip
  POTONGDAT_DOWNLOAD=https://github.com/dkfans/po2ngdat/releases/download/v1.0.2.31-c/po2ngdat-1_0_2_31-devel-win.zip
  SNDBANKER_DOWNLOAD=https://github.com/dkfans/sndbanker/releases/download/v1.0.1/sndbanker-1_0_1_13-devel-win.zip
  RNCTOOLS_DOWNLOAD=https://github.com/dkfans/rnctools/releases/download/v1.0.2/rnctools-1_0_2_5-devel-win.zip
  DKILLCONV_DOWNLOAD=no_prebuild_available_for_your_os
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-lin.tar.gz
  PNGTOICO_DOWNLOAD=https://github.com/dkfans/png2ico/releases/download/2024-10-28/png2ico-lin-2024-10-28.tar.gz
  PNGTORAW_DOWNLOAD=https://github.com/dkfans/pngpal2raw/releases/download/v1.0.2/pngpal2raw-1_0_2_35-devel-lin.tar.gz
  PNGTOBSPAL_DOWNLOAD=https://github.com/dkfans/png2bestpal/releases/download/v1.0.3/png2bestpal-1_0_3_21-devel-lin.tar.gz
  POTONGDAT_DOWNLOAD=https://github.com/dkfans/po2ngdat/releases/download/v1.0.2.31-c/po2ngdat-1_0_2_31-devel-lin.tar.gz
  SNDBANKER_DOWNLOAD=https://github.com/dkfans/sndbanker/releases/download/v1.0.1/sndbanker-1_0_1_13-devel-lin.tar.gz
  RNCTOOLS_DOWNLOAD=https://github.com/dkfans/rnctools/releases/download/v1.0.2/rnctools-1_0_2_5-devel-lin.tar.gz
  DKILLCONV_DOWNLOAD=no_prebuild_available_for_your_os
else
  PERESEC_DOWNLOAD=no_prebuild_available_for_your_os
  PNGTOICO_DOWNLOAD=no_prebuild_available_for_your_os
  PNGTORAW_DOWNLOAD=no_prebuild_available_for_your_os
  PNGTOBSPAL_DOWNLOAD=no_prebuild_available_for_your_os
  POTONGDAT_DOWNLOAD=no_prebuild_available_for_your_os
  SNDBANKER_DOWNLOAD=no_prebuild_available_for_your_os
  RNCTOOLS_DOWNLOAD=no_prebuild_available_for_your_os
  DKILLCONV_DOWNLOAD=no_prebuild_available_for_your_os
endif
endif
PERESEC_PACKAGE=$(notdir $(PERESEC_DOWNLOAD))
PNGTOICO_PACKAGE=$(notdir $(PNGTOICO_DOWNLOAD))
PNGTORAW_PACKAGE=$(notdir $(PNGTORAW_DOWNLOAD))
PNGTOBSPAL_PACKAGE=$(notdir $(PNGTOBSPAL_DOWNLOAD))
POTONGDAT_PACKAGE=$(notdir $(POTONGDAT_DOWNLOAD))
SNDBANKER_PACKAGE=$(notdir $(SNDBANKER_DOWNLOAD))
RNCTOOLS_PACKAGE=$(notdir $(RNCTOOLS_DOWNLOAD))
DKILLCONV_PACKAGE=$(notdir $(DKILLCONV_DOWNLOAD))

# Tools and libraries to be used for the target system
# Currently, the target is always windows-mingw32
SDL_DOWNLOAD=https://github.com/libsdl-org/SDL/releases/download/release-2.30.7/SDL2-devel-2.30.7-mingw.tar.gz
#SDL_NET_DOWNLOAD=https://github.com/libsdl-org/SDL_net/releases/download/release-2.2.0/SDL2_net-devel-2.2.0-VC.zip
SDL_NET_DOWNLOAD=https://github.com/libsdl-org/SDL_net/releases/download/release-2.2.0/SDL2_net-devel-2.2.0-mingw.tar.gz
#SDL_MIXER_DOWNLOAD=https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-VC.zip
SDL_MIXER_DOWNLOAD=https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-mingw.tar.gz
#SDL_IMAGE_DOWNLOAD=https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-VC.zip
SDL_IMAGE_DOWNLOAD=https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-mingw.tar.gz
SDL_PACKAGE=$(notdir $(SDL_DOWNLOAD))
SDL_NET_PACKAGE=$(notdir $(SDL_NET_DOWNLOAD))
SDL_MIXER_PACKAGE=$(notdir $(SDL_MIXER_DOWNLOAD))
SDL_IMAGE_PACKAGE=$(notdir $(SDL_IMAGE_DOWNLOAD))
