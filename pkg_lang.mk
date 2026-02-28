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
pkg/fxdata/gtext_eng.dat \
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
pkg/fxdata/gtext_ukr.dat \
pkg/fxdata/gtext_por.dat \

NCTEXTDATS = \
pkg/campgns/ami2019/text_eng.dat \
pkg/campgns/ami2019/text_chi.dat \
pkg/campgns/ami2019/text_ger.dat \
pkg/campgns/ami2019/text_spa.dat \
pkg/campgns/ancntkpr/text_eng.dat \
pkg/campgns/ancntkpr/text_chi.dat \
pkg/campgns/ancntkpr/text_fre.dat \
pkg/campgns/ancntkpr/text_ger.dat \
pkg/campgns/ancntkpr/text_pol.dat \
pkg/campgns/ancntkpr/text_por.dat \
pkg/campgns/ancntkpr/text_spa.dat \
pkg/campgns/ancntkpr/text_lat.dat \
pkg/campgns/burdnimp/text_eng.dat \
pkg/campgns/burdnimp/text_chi.dat \
pkg/campgns/burdnimp/text_pol.dat \
pkg/campgns/burdnimp/text_por.dat \
pkg/campgns/burdnimp/text_spa.dat \
pkg/campgns/lqizgood/text_eng.dat \
pkg/campgns/lqizgood/text_chi.dat \
pkg/campgns/lqizgood/text_fre.dat \
pkg/campgns/lqizgood/text_pol.dat \
pkg/campgns/lqizgood/text_por.dat \
pkg/campgns/lqizgood/text_ukr.dat \
pkg/campgns/origplus/text_eng.dat \
pkg/campgns/origplus/text_chi.dat \
pkg/campgns/origplus/text_cht.dat \
pkg/campgns/origplus/text_cze.dat \
pkg/campgns/origplus/text_dut.dat \
pkg/campgns/origplus/text_fre.dat \
pkg/campgns/origplus/text_ger.dat \
pkg/campgns/origplus/text_ita.dat \
pkg/campgns/origplus/text_jpn.dat \
pkg/campgns/origplus/text_kor.dat \
pkg/campgns/origplus/text_lat.dat \
pkg/campgns/origplus/text_pol.dat \
pkg/campgns/origplus/text_por.dat \
pkg/campgns/origplus/text_rus.dat \
pkg/campgns/origplus/text_spa.dat \
pkg/campgns/origplus/text_swe.dat \
pkg/campgns/origplus/text_ukr.dat \
pkg/campgns/revlord/text_eng.dat \
pkg/campgns/revlord/text_chi.dat \
pkg/campgns/revlord/text_ger.dat \
pkg/campgns/revlord/text_por.dat \
pkg/campgns/revlord/text_spa.dat \
pkg/campgns/twinkprs/text_eng.dat \
pkg/campgns/twinkprs/text_chi.dat \
pkg/campgns/twinkprs/text_fre.dat \
pkg/campgns/twinkprs/text_jpn.dat \
pkg/campgns/twinkprs/text_pol.dat \
pkg/campgns/twinkprs/text_por.dat \
pkg/campgns/twinkprs/text_spa.dat \
pkg/campgns/twinkprs/text_lat.dat \
pkg/campgns/undedkpr/text_eng.dat \
pkg/campgns/undedkpr/text_chi.dat \
pkg/campgns/undedkpr/text_ger.dat \
pkg/campgns/undedkpr/text_pol.dat \
pkg/campgns/undedkpr/text_por.dat \
pkg/campgns/undedkpr/text_spa.dat

MPTEXTDATS = \
pkg/levels/classic/text_eng.dat \
pkg/levels/classic/text_chi.dat \
pkg/levels/classic/text_fre.dat \
pkg/levels/classic/text_ger.dat \
pkg/levels/classic/text_por.dat \
pkg/levels/classic/text_spa.dat \
pkg/levels/classic/text_rus.dat \
pkg/levels/standard/text_eng.dat \
pkg/levels/standard/text_chi.dat \
pkg/levels/standard/text_fre.dat \
pkg/levels/standard/text_ger.dat \
pkg/levels/standard/text_por.dat \
pkg/levels/standard/text_spa.dat \
pkg/levels/lostlvls/text_eng.dat \
pkg/levels/lostlvls/text_por.dat \
pkg/levels/lostlvls/text_chi.dat \
pkg/levels/lostlvls/text_rus.dat

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

pkg/fxdata/gtext_ukr.dat: lang/gtext_ukr.po $(POTONGDAT) $(RU_CHAR_ENCODING) | pkg/fxdata
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

pkg/%/text_cht.dat : lang/%/text_cht.po $(POTONGDAT) $(CH_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(CH_CHAR_ENCODING) $< >/dev/null

pkg/%/text_cze.dat: lang/%/text_cze.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_dut.dat: lang/%/text_dut.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_fre.dat: lang/%/text_fre.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_ger.dat: lang/%/text_ger.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_ita.dat: lang/%/text_ita.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_kor.dat: lang/%/text_kor.po $(POTONGDAT) $(KR_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(KR_CHAR_ENCODING) $< >/dev/null

pkg/%/text_jpn.dat: lang/%/text_jpn.po $(POTONGDAT) $(JP_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(JP_CHAR_ENCODING) $< >/dev/null

pkg/%/text_lat.dat: lang/%/text_lat.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_pol.dat: lang/%/text_pol.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_por.dat: lang/%/text_por.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_rus.dat: lang/%/text_rus.po $(POTONGDAT) $(RU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(RU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_ukr.dat: lang/%/text_ukr.po $(POTONGDAT) $(RU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(RU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_spa.dat: lang/%/text_spa.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
	$(POTONGDAT) -o $@ -e $(EU_CHAR_ENCODING) $< >/dev/null

pkg/%/text_swe.dat: lang/%/text_swe.po $(POTONGDAT) $(EU_CHAR_ENCODING) | pkg/%
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
