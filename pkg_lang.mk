#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_peresec.mk
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
LANGUAGE ?= eng

NGTEXTDATS = \
pkg/fxdata/gtext_chi.dat \
pkg/fxdata/gtext_cht.dat \
pkg/fxdata/gtext_cze.dat \
pkg/fxdata/gtext_dut.dat \
pkg/fxdata/gtext_fre.dat \
pkg/fxdata/gtext_ger.dat \
pkg/fxdata/gtext_ita.dat \
pkg/fxdata/gtext_jpn.dat \
pkg/fxdata/gtext_kor.dat \
pkg/fxdata/gtext_lat.dat \
pkg/fxdata/gtext_pol.dat \
pkg/fxdata/gtext_rus.dat \
pkg/fxdata/gtext_spa.dat \
pkg/fxdata/gtext_swe.dat \
pkg/fxdata/gtext_eng.dat

NCTEXTDATS = \
pkg/campgns/ami2019/text_eng.dat \
pkg/campgns/ami2019/text_chi.dat \
pkg/campgns/ancntkpr/text_eng.dat \
pkg/campgns/ancntkpr/text_fre.dat \
pkg/campgns/ancntkpr/text_ger.dat \
pkg/campgns/ancntkpr/text_pol.dat \
pkg/campgns/ancntkpr/text_chi.dat \
pkg/campgns/burdnimp/text_eng.dat \
pkg/campgns/burdnimp/text_pol.dat \
pkg/campgns/cqarctic/text_eng.dat \
pkg/campgns/cqarctic/text_chi.dat \
pkg/campgns/cqarctic/text_pol.dat \
pkg/campgns/dstninja/text_eng.dat \
pkg/campgns/dstninja/text_chi.dat \
pkg/campgns/dstninja/text_pol.dat \
pkg/campgns/evilkeep/text_eng.dat \
pkg/campgns/evilkeep/text_fre.dat \
pkg/campgns/evilkeep/text_ger.dat \
pkg/campgns/evilkeep/text_pol.dat \
pkg/campgns/grkreign/text_eng.dat \
pkg/campgns/grkreign/text_pol.dat \
pkg/campgns/lqizgood/text_eng.dat \
pkg/campgns/lqizgood/text_chi.dat \
pkg/campgns/lqizgood/text_fre.dat \
pkg/campgns/lqizgood/text_pol.dat \
pkg/campgns/ncastles/text_eng.dat \
pkg/campgns/ncastles/text_pol.dat \
pkg/campgns/questfth/text_eng.dat \
pkg/campgns/questfth/text_fre.dat \
pkg/campgns/revlord/text_eng.dat \
pkg/campgns/revlord/text_chi.dat \
pkg/campgns/questfth/text_pol.dat \
pkg/campgns/twinkprs/text_eng.dat \
pkg/campgns/twinkprs/text_pol.dat \
pkg/campgns/twinkprs/text_chi.dat \
pkg/campgns/undedkpr/text_eng.dat \
pkg/campgns/undedkpr/text_chi.dat \
pkg/campgns/undedkpr/text_pol.dat

MPTEXTDATS = \
pkg/levels/classic/text_eng.dat \
pkg/levels/classic/text_chi.dat \
pkg/levels/standard/text_eng.dat \
pkg/levels/standard/text_chi.dat

pkg-languages: lang-before $(NGTEXTDATS) $(NCTEXTDATS) $(MPTEXTDATS) pkg-before

lang-before:
	$(MKDIR) pkg/fxdata

# Creation of Only single language engine language files from PO/POT files (for development)
pkg-lang-single: lang-before pkg/fxdata/gtext_$(LANGUAGE).dat

# Creation of engine language files from PO/POT files
pkg/fxdata/gtext_jpn.dat: lang/gtext_jpn.po tools/po2ngdat/res/char_encoding_tbl_jp.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_rus.dat: lang/gtext_rus.po tools/po2ngdat/res/char_encoding_tbl_ru.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_chi.dat: lang/gtext_chi.po tools/po2ngdat/res/char_encoding_tbl_ch.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_cht.dat: lang/gtext_cht.po tools/po2ngdat/res/char_encoding_tbl_ch.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_kor.dat: lang/gtext_kor.po tools/po2ngdat/res/char_encoding_tbl_kr.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_%.dat: lang/gtext_%.po tools/po2ngdat/res/char_encoding_tbl_eu.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/fxdata/gtext_%.dat: lang/gtext_%.pot tools/po2ngdat/res/char_encoding_tbl_eu.txt $(POTONGDAT)
	-$(ECHO) 'Building language file: $@'
	$(POTONGDAT) -o "$@" -e "$(word 2,$^)" "$<"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# Creation of engine language files for campaigns
define define_campaign_language_rule

pkg/campgns/$(1)/text_ch%.dat: lang/campgns/$(1)/text_ch%.po tools/po2ngdat/res/char_encoding_tbl_ch.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)/%.dat: lang/campgns/$(1)/%.po tools/po2ngdat/res/char_encoding_tbl_eu.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)/%.dat: lang/campgns/$(1)/%.pot tools/po2ngdat/res/char_encoding_tbl_eu.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

endef

# Creation of engine language files for map packs
define define_mappack_language_rule

pkg/levels/$(1)/text_ch%.dat: lang/levels/$(1)/text_ch%.po tools/po2ngdat/res/char_encoding_tbl_ch.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/levels/$(1)/%.dat: lang/levels/$(1)/%.po tools/po2ngdat/res/char_encoding_tbl_eu.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/levels/$(1)/%.dat: lang/levels/$(1)/%.pot tools/po2ngdat/res/char_encoding_tbl_eu.txt $$(POTONGDAT)
	-$$(ECHO) 'Building language file: $$@'
	@$$(MKDIR) $$(@D)
	$$(POTONGDAT) -o "$$@" -e "$$(word 2,$$^)" "$$<"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

endef

$(foreach campaign,$(sort $(CAMPAIGNS)),$(eval $(call define_campaign_language_rule,$(campaign))))
$(foreach mappack,$(sort $(MAPPACKS)),$(eval $(call define_mappack_language_rule,$(mappack))))

#******************************************************************************