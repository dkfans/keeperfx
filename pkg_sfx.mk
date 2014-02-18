#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file pkg_sfx.mk
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Defines make rules for tools needed to build KeeperFX.
#      Most tools can either by compiled from source or downloaded.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     25 Jan 2009 - 02 Jul 2011
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************

NGSOUNDDATS = \
pkg/sound/speech_chi.dat \
pkg/sound/speech_cht.dat \
pkg/sound/speech_dut.dat \
pkg/sound/speech_eng.dat \
pkg/sound/speech_fre.dat \
pkg/sound/speech_ger.dat \
pkg/sound/speech_ita.dat \
pkg/sound/speech_jpn.dat \
pkg/sound/speech_pol.dat \
pkg/sound/speech_rus.dat \
pkg/sound/speech_spa.dat \
pkg/sound/speech_swe.dat \
pkg/sound/sound.dat

pkg-sfx: $(NGSOUNDDATS)

pkg/sound/sound.dat: sfx/sound/filelist.txt $(WAVTODAT)
pkg/sound/speech_chi.dat: sfx/speech_chi/filelist.txt $(WAVTODAT)
pkg/sound/speech_cht.dat: sfx/speech_cht/filelist.txt $(WAVTODAT)
pkg/sound/speech_dut.dat: sfx/speech_dut/filelist.txt $(WAVTODAT)
pkg/sound/speech_eng.dat: sfx/speech_eng/filelist.txt $(WAVTODAT)
pkg/sound/speech_fre.dat: sfx/speech_fre/filelist.txt $(WAVTODAT)
pkg/sound/speech_ger.dat: sfx/speech_ger/filelist.txt $(WAVTODAT)
pkg/sound/speech_ita.dat: sfx/speech_ita/filelist.txt $(WAVTODAT)
pkg/sound/speech_jpn.dat: sfx/speech_jpn/filelist.txt $(WAVTODAT)
pkg/sound/speech_pol.dat: sfx/speech_pol/filelist.txt $(WAVTODAT)
pkg/sound/speech_rus.dat: sfx/speech_rus/filelist.txt $(WAVTODAT)
pkg/sound/speech_spa.dat: sfx/speech_spa/filelist.txt $(WAVTODAT)
pkg/sound/speech_swe.dat: sfx/speech_swe/filelist.txt $(WAVTODAT)

pkg/sound/%.dat:
	-$(ECHO) 'Building sound bank: $@'
	@$(MKDIR) $(@D)
	$(WAVTODAT) -o "$@" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

sfx/%/filelist.txt: | sfx/$(SFXSRC_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	7z x -aoa -y -osfx "$|"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

# Downloading the sfx sources pack
sfx/$(SFXSRC_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SFXSRC_DOWNLOAD)"
	7z t "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

#******************************************************************************
