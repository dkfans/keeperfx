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
speech_kor \
speech_lat \
speech_pol \
speech_rus \
speech_spa \
speech_swe

NGSOUNDDATS = $(patsubst %,pkg/sound/%.dat,$(NGSPEECHBANKS) sound)

NGSOUNDLISTS = $(patsubst %,sfx/%/filelist.txt,$(NGSPEECHBANKS) sound)

LANDVIEWSPEECH = \
$(foreach lng,eng,ancntkpr_$(lng)) \
$(foreach lng,eng,burdnimp_$(lng)) \
$(foreach lng,eng,cqarctic_$(lng)) \
$(foreach lng,eng,dstninja_$(lng)) \
$(foreach lng,eng dut,dzjr06lv_$(lng)) \
$(foreach lng,eng,dzjr10lv_$(lng)) \
$(foreach lng,eng,dzjr25lv_$(lng)) \
$(foreach lng,eng fre ger,evilkeep_$(lng)) \
$(foreach lng,eng,grkreign_$(lng)) \
$(foreach lng,eng,jdkmaps8_$(lng)) \
$(foreach lng,eng,kdklvpck_$(lng)) \
$(foreach lng,eng chi cht dut fre ger ita jpn kor pol rus spa swe,keeporig_$(lng)) \
$(foreach lng,eng dut,lqizgood_$(lng)) \
$(foreach lng,eng,lrdvexer_$(lng)) \
$(foreach lng,eng,ncastles_$(lng)) \
$(foreach lng,eng,postanck_$(lng)) \
$(foreach lng,eng,pstunded_$(lng)) \
$(foreach lng,eng,questfth_$(lng)) \
$(foreach lng,eng dut,twinkprs_$(lng)) \
$(foreach lng,eng,undedkpr_$(lng))

LANDVIEWSPEECHDIRS = $(patsubst %,pkg/campgns/%,$(LANDVIEWSPEECH))

.PHONY: pkg-sfx convert-sfx

pkg-sfx: $(NGSOUNDDATS) $(LANDVIEWSPEECHDIRS)

pkg/sound/sound.dat: sfx/sound/filelist.txt $(WAVTODAT)
pkg/sound/speech_chi.dat: sfx/speech_chi/filelist.txt $(WAVTODAT)
pkg/sound/speech_cht.dat: sfx/speech_cht/filelist.txt $(WAVTODAT)
pkg/sound/speech_dut.dat: sfx/speech_dut/filelist.txt $(WAVTODAT)
pkg/sound/speech_eng.dat: sfx/speech_eng/filelist.txt $(WAVTODAT)
pkg/sound/speech_fre.dat: sfx/speech_fre/filelist.txt $(WAVTODAT)
pkg/sound/speech_ger.dat: sfx/speech_ger/filelist.txt $(WAVTODAT)
pkg/sound/speech_ita.dat: sfx/speech_ita/filelist.txt $(WAVTODAT)
pkg/sound/speech_jpn.dat: sfx/speech_jpn/filelist.txt $(WAVTODAT)
pkg/sound/speech_kor.dat: sfx/speech_kor/filelist.txt $(WAVTODAT)
pkg/sound/speech_lat.dat: sfx/speech_lat/filelist.txt $(WAVTODAT)
pkg/sound/speech_pol.dat: sfx/speech_pol/filelist.txt $(WAVTODAT)
pkg/sound/speech_rus.dat: sfx/speech_rus/filelist.txt $(WAVTODAT)
pkg/sound/speech_spa.dat: sfx/speech_spa/filelist.txt $(WAVTODAT)
pkg/sound/speech_swe.dat: sfx/speech_swe/filelist.txt $(WAVTODAT)

pkg/sound/%.dat:
	-$(ECHO) 'Building sound bank: $@'
	@$(MKDIR) "$(@D)"
	$(WAVTODAT) -o "$@" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# Creation of land view speeches for campaigns
define define_campaign_speeches_rule
pkg/campgns/$(1)_$(2): sfx/campgns/$(1)_$(2)/filelist.txt
	-$(ECHO) 'Copying campaign SFX: $$@'
	@$(MKDIR) "$$@"
	tail -n +2 "$$<" | cut -f1 | xargs -d '\n' -I {} $(CP) "$$(<D)/{}" "$$@/"
	-$(ECHO) 'Finished copying: $$@'
	-$(ECHO) ' '

endef

$(foreach campaign,$(sort $(CAMPAIGNS)),$(foreach lng,$(sort $(LANGS)),$(eval $(call define_campaign_speeches_rule,$(campaign),$(lng)))))

convert-sfx: $(patsubst %,convert-speech-sfx-%,$(NGSPEECHBANKS)) $(patsubst %,convert-campaign-sfx-%,$(LANDVIEWSPEECH))

convert-speech-sfx-%: sfx/%/filelist.txt
	-$(ECHO) 'Converting speech samples in list: $<'
	tail -n +2 "$<" | cut -f1 | xargs -d '\n' -I {} sox "$(<D)/design/{}" -c 1 -b 8 -r 22050 -e unsigned-integer "$(<D)/{}" compand 0.02,0.20 5:-40,-40,-35,-20,-10 -6 -90 0.1 gain -n -0.1
#	best would be "compand 0.02,0.20 5:-60,-40,-10 -6 -90 0.1", modification is to skip noise
	-$(ECHO) 'Finished converting list: $<'
	-$(ECHO) ' '

convert-campaign-sfx-%: sfx/campgns/%/filelist.txt
	-$(ECHO) 'Converting campaign speeches in list: $<'
	tail -n +2 "$<" | cut -f1 | xargs -d '\n' -I {} sox "$(<D)/design/{}" -c 1 -r 22050 -e ms-adpcm "$(<D)/{}" compand 0.02,0.20 5:-40,-40,-35,-20,-10 -6 -90 0.1 gain -n -0.1
#	best would be "compand 0.02,0.20 5:-60,-40,-10 -6 -90 0.1", modification is to skip noise
	-$(ECHO) 'Finished converting list: $<'
	-$(ECHO) ' '

ifeq ($(ENABLE_EXTRACT), 1)

sfx/%/filelist.txt sfx/campgns/%/filelist.txt: | sfx/$(SFXSRC_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	7z x -aoa -y -osfx "$|"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

endif

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
