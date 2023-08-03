#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file package.mk
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Defines make rules for package with release of KeeperFX.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     01 Jul 2011 - 01 Jul 2011
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************
empty =
space = $(empty) $(empty)
PKG_NAME = pkg/keeperfx-$(subst $(space),_,$(subst .,_,$(VER_STRING)))-patch.7z
PKG_CAMPAIGN_FILES = \
	$(patsubst %,pkg/campgns/%.cfg,$(CAMPAIGNS)) \
	$(patsubst %,pkg/%,$(foreach campaign,$(CAMPAIGNS),$(wildcard campgns/$(campaign)/*.txt))) \
	$(patsubst %,pkg/%,$(foreach campaign,$(CAMPAIGNS),$(wildcard campgns/$(campaign)_crtr/*.cfg))) \
	$(patsubst %,pkg/%,$(foreach campaign,$(CAMPAIGNS),$(wildcard campgns/$(campaign)_lnd/*.txt)))
PKG_CAMPAIGN_DIRS = $(sort $(dir $(PKG_CAMPAIGN_FILES)))
PKG_CREATURE_FILES = $(patsubst config/creatrs/%,pkg/creatrs/%,$(wildcard config/creatrs/*.cfg))
PKG_FXDATA_FILES = $(patsubst config/fxdata/%,pkg/fxdata/%,$(wildcard config/fxdata/*.cfg))
PKG_MAPPACK_FILES = \
	$(patsubst %,pkg/levels/%.cfg,$(MAPPACKS)) \
	$(patsubst %,pkg/%,$(foreach mappack,$(MAPPACKS),$(wildcard levels/$(mappack)/*.cfg))) \
	$(patsubst %,pkg/%,$(foreach mappack,$(MAPPACKS),$(filter-out %/readme.txt,$(wildcard levels/$(mappack)/*.txt)))) \
	$(patsubst %,pkg/%,$(foreach mappack,$(MAPPACKS),$(wildcard levels/$(mappack)_crtr/*.cfg))) \
	$(patsubst %,pkg/%,$(foreach mappack,$(MAPPACKS),$(wildcard levels/$(mappack)_cfgs/*.cfg)))
PKG_MAPPACK_DIRS = $(sort $(dir $(PKG_MAPPACK_FILES)))
PKG_BIN = pkg/$(notdir $(BIN))
PKG_BIN_PDB = $(PKG_BIN:%.exe=%.pdb)
PKG_HVLOGBIN = pkg/$(notdir $(HVLOGBIN))
PKG_HVLOGBIN_PDB = $(PKG_HVLOGBIN:%.exe=%.pdb)
PKG_DOCS = pkg/keeperfx_readme.txt
PKG_FILES = \
	$(PKG_CAMPAIGN_FILES) \
	$(PKG_CREATURE_FILES) \
	$(PKG_FXDATA_FILES) \
	$(PKG_MAPPACK_FILES) \
	$(NGTEXTDATS) \
	$(NCTEXTDATS) \
	$(MPTEXTDATS) \
	pkg/keeperfx.cfg \
	$(PKG_BIN) \
	$(PKG_BIN_PDB) \
	$(PKG_HVLOGBIN) \
	$(PKG_HVLOGBIN_PDB) \
	$(PKG_DOCS)

.PHONY: package

pkg pkg/creatrs pkg/fxdata pkg/campgns $(PKG_MAPPACK_DIRS) $(PKG_CAMPAIGN_DIRS):
	$(MKDIR) $@

pkg/keeperfx.cfg: config/keeperfx.cfg | pkg
	$(CP) $^ $@

$(PKG_BIN): $(BIN) | pkg
	$(CP) $^ $@

$(PKG_HVLOGBIN): $(HVLOGBIN) | pkg
	$(CP) $^ $@

$(PKG_BIN_PDB): $(BIN) | pkg
	$(CP) $(BIN:%.exe=%.pdb) $@

$(PKG_HVLOGBIN_PDB): $(HVLOGBIN) | pkg
	$(CP) $(HVLOGBIN:%.exe=%.pdb) $@

pkg/%.txt: docs/%.txt | pkg
	$(CP) $^ $@

pkg/campgns/%.cfg: campgns/%.cfg | pkg/campgns
	$(CP) $^ $@

pkg/campgns/%.txt: campgns/%.txt | $(PKG_CAMPAIGN_DIRS)
	$(CP) $^ $@

pkg/creatrs/%.cfg: config/creatrs/%.cfg | pkg/creatrs
	$(CP) $^ $@

pkg/fxdata/%.cfg: config/fxdata/%.cfg | pkg/fxdata
	$(CP) $^ $@

pkg/levels/%.cfg: levels/%.cfg | $(PKG_MAPPACK_DIRS)
	$(CP) $^ $@

pkg/levels/%.txt: levels/%.txt | $(PKG_MAPPACK_DIRS)
	$(CP) $^ $@

$(PKG_NAME): $(PKG_FILES) | pkg
	$(RM) $@ && cd $(dir $(PKG_NAME)) && 7z a $(notdir $(PKG_NAME)) $(patsubst pkg/%,%,$^) >/dev/null

package: $(PKG_NAME)

clean-package:
	$(RM) -r pkg
