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
	$(patsubst lang/%.po,pkg/fxdata/%.dat,$(wildcard lang/gtext_*.po)) \
	$(patsubst lang/%.pot,pkg/fxdata/%.dat,$(wildcard lang/gtext_*.pot))
NCTEXTDATS = \
	$(patsubst lang/%.po,pkg/%.dat,$(foreach campaign,$(CAMPAIGNS),$(wildcard lang/campgns/$(campaign)/*.po))) \
	$(patsubst lang/%.pot,pkg/%.dat,$(foreach campaign,$(CAMPAIGNS),$(wildcard lang/campgns/$(campaign)/*.pot)))
MPTEXTDATS = \
	$(patsubst lang/%.po,pkg/%.dat,$(foreach mappack,$(MAPPACKS),$(wildcard lang/levels/$(mappack)/*.po))) \
	$(patsubst lang/%.pot,pkg/%.dat,$(foreach mappack,$(MAPPACKS),$(wildcard lang/levels/$(mappack)/*.pot)))

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