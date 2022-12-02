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
pkg/levels/classic/text_eng.dat \
pkg/levels/classic/text_chi.dat \
pkg/levels/standard/text_eng.dat \
pkg/levels/standard/text_chi.dat

EU_CHAR_ENCODING = tools/po2ngdat/res/char_encoding_tbl_eu.txt
JP_CHAR_ENCODING = tools/po2ngdat/res/char_encoding_tbl_jp.txt
RU_CHAR_ENCODING = tools/po2ngdat/res/char_encoding_tbl_ru.txt
CH_CHAR_ENCODING = tools/po2ngdat/res/char_encoding_tbl_ch.txt
KR_CHAR_ENCODING = tools/po2ngdat/res/char_encoding_tbl_kr.txt

.PRECIOUS: $(EU_CHAR_ENCODING) $(JP_CHAR_ENCODING) $(RU_CHAR_ENCODING) $(CH_CHAR_ENCODING) $(KR_CHAR_ENCODING)

pkg-languages: $(NGTEXTDATS) $(NCTEXTDATS) $(MPTEXTDATS)

# Creation of Only single language engine language files from PO/POT files (for development)
pkg-lang-single: pkg/fxdata/gtext_$(LANGUAGE).dat

$(patsubst %/,%,$(sort $(dir $(NCTEXTDATS)))):
	$(MKDIR) $@

$(patsubst %/,%,$(sort $(dir $(MPTEXTDATS)))):
	$(MKDIR) $@

# Creation of engine language files from PO/POT files
pkg/fxdata/gtext_jpn.dat: lang/gtext_jpn.po $(POTONGDAT) $(JP_CHAR_ENCODING) | pkg/fxdata
	$(POTONGDAT) -o $@ -e $(JP_CHAR_ENCODING) $< >/dev/null

pkg/fxdata/gtext_rus.dat: lang/gtext_rus.po $(POTONGDAT) $(RU_CHAR_ENCODING) | pkg/fxdata
	$(POTONGDAT) -o $@ -e $(RU_CHAR_ENCODING) $< >/dev/null

pkg/fxdata/gtext_chi.dat: lang/gtext_chi.po $(POTONGDAT) $(CH_CHAR_ENCODING) | pkg/fxdata
	$(POTONGDAT) -o $@ -e $(CH_CHAR_ENCODING) $< >/dev/null

pkg/fxdata/gtext_cht.dat: lang/gtext_cht.po $(POTONGDAT) $(CH_CHAR_ENCODING) | pkg/fxdata
	$(POTONGDAT) -o $@ -e $(CH_CHAR_ENCODING) $< >/dev/null

pkg/fxdata/gtext_kor.dat: lang/gtext_kor.po $(POTONGDAT) $(KR_CHAR_ENCODING) | pkg/fxdata
	$(POTONGDAT) -o $@ -e $(KR_CHAR_ENCODING) $< >/dev/null

pkg/fxdata/gtext_%.dat: lang/gtext_%.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/fxdata
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/fxdata/gtext_%.dat: lang/gtext_%.pot $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/fxdata
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_chi.dat : lang/%/text_chi.po $(POTONGDAT) $(CH_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(CH_CHAR_ENCODING) $< >/dev/null

pkg/%/text_fre.dat: lang/%/text_fre.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_ger.dat: lang/%/text_ger.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_pol.dat: lang/%/text_pol.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/landview_pol.dat: lang/%/landview_pol.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/landview_dut.dat: lang/%/landview_dut.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/landview_fre.dat: lang/%/landview_fre.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/landview_kor.dat: lang/%/landview_kor.po $(POTONGDAT) $(KR_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(KR_CHAR_ENCODING) $< >/dev/null

pkg/%/landview_rus.dat: lang/%/landview_rus.po $(POTONGDAT) $(RU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(RU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_eng.dat: lang/%/text_eng.pot $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/landview_eng.dat: lang/%/landview_eng.pot $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null
