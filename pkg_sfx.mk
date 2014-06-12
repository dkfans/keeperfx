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

NGSPEECHBANKS = \
speech_chi \
speech_cht \
speech_dut \
speech_eng \
speech_fre \
speech_ger \
speech_ita \
speech_jpn \
speech_lat \
speech_pol \
speech_rus \
speech_spa \
speech_swe

NGSOUNDDATS = $(patsubst %,pkg/sound/%.dat,$(NGSPEECHBANKS) sound)

NGSOUNDLISTS = $(patsubst %,sfx/%/filelist.txt,$(NGSPEECHBANKS) sound)

.PHONY: pkg-sfx convert-sfx

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
pkg/sound/speech_lat.dat: sfx/speech_lat/filelist.txt $(WAVTODAT)
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

convert-sfx: $(patsubst %,convert-sfx-%,$(NGSPEECHBANKS))

convert-sfx-%: sfx/%/filelist.txt
	-$(ECHO) 'Converting sound samples in list: $<'
	tail -n +2 "$<" | cut -f1 | xargs -d '\n' -I {} sox "$(<D)/design/{}" -c 1 -b 8 -r 22050 -e unsigned-integer "$(<D)/{}"
	# compand 0.02,0.20 5:-60,-40,-10 -6 -90 0.1
	-$(ECHO) 'Finished converting list: $<'
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
