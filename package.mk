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

CAMPAIGN_CFGS = $(patsubst %,pkg/campgns/%.cfg,$(CAMPAIGNS))

.PHONY: pkg-before pkg-copydat pkg-campaigns pkg-languages

package: pkg/keeperfx-$(subst $(space),_,$(subst .,_,$(VER_STRING)))-$(PACKAGE_SUFFIX)-patch.7z

clean-package:
	-$(RM) -R pkg/campgns
	-$(RM) -R pkg/creatrs
	-$(RM) -R pkg/fxdata
	-$(RM) -R pkg/ldata
	-$(RM) -R pkg/data
	-$(RM) -R pkg/sound
	-$(RM) -R pkg/levels
	-$(RM) pkg/keeperfx*

deep-clean-package:

pkg/%.7z: pkg-before pkg-copybin pkg-copydat pkg-campaigns pkg-languages
	$(ECHO) 'Creating package: $@'
	cd $(@D); \
	7z a "$(@F)" "*" -x!*/.svn -x!.svn -x!.git -x!*.7z
	$(ECHO) 'Finished creating: $@'
	$(ECHO) ' '

pkg-before:
	-$(RM) "pkg/keeperfx-$(subst $(space),_,$(subst .,_,$(VER_STRING)))-*-patch.7z"
	$(MKDIR) pkg/creatrs
	$(MKDIR) pkg/data
	$(MKDIR) pkg/ldata
	$(MKDIR) pkg/fxdata
	$(MKDIR) pkg/campgns

pkg-copybin: pkg-before
	$(CP) bin/* pkg/
	$(CP) docs/keeperfx_readme.txt pkg/

pkg-copydat: pkg-before
	$(CP) config/keeperfx.cfg pkg/
	$(CP) config/creatrs/*.cfg pkg/creatrs/
	$(CP) config/fxdata/*.cfg pkg/fxdata/

pkg-campaigns: pkg-before $(CAMPAIGN_CFGS)

# special block for Original Campaign - it is copied elswhere
pkg/campgns/keeporig.cfg: campgns/keeporig.cfg
	@$(MKDIR) $(@D)
#	 Copy folder with campaign name (w/o extension), if it exists
	$(if $(wildcard $(<:%.cfg=%)),$(MKDIR) pkg/levels)
	$(if $(wildcard $(<:%.cfg=%)),-$(CP) $(<:%.cfg=%)/map*.* pkg/levels/)
#	 Copy folder with campaign name and _cfgs ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_cfgs)),$(MKDIR) $(@:%.cfg=%_cfgs))
	$(if $(wildcard $(<:%.cfg=%_cfgs)),-$(CP) $(<:%.cfg=%_cfgs)/*.cfg $(@:%.cfg=%_cfgs)/)
#	 Copy folder with campaign name and _crtr ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_crtr)),$(MKDIR) $(@:%.cfg=%_crtr))
	$(if $(wildcard $(<:%.cfg=%_crtr)),-$(CP) $(<:%.cfg=%_crtr)/*.cfg $(@:%.cfg=%_crtr)/)
#	 Copy folder with campaign name and _lnd ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_lnd)),$(MKDIR) pkg/ldata)
	$(if $(wildcard $(<:%.cfg=%_lnd)),-$(CP) $(<:%.cfg=%_lnd)/*.txt pkg/ldata/)
#	 Copy the actual campaign file
	$(CP) $< $@

pkg/campgns/%.cfg: campgns/%.cfg
	@$(MKDIR) $(@D)
#	 Copy folder with campaign name (w/o extension), if it exists
	$(if $(wildcard $(<:%.cfg=%)),$(MKDIR) $(@:%.cfg=%))
	$(if $(wildcard $(<:%.cfg=%)),-$(CP) $(<:%.cfg=%)/map*.* $(@:%.cfg=%)/)
#	 Copy folder with campaign name and _crtr ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_crtr)),$(MKDIR) $(@:%.cfg=%_crtr))
	$(if $(wildcard $(<:%.cfg=%_crtr)),-$(CP) $(<:%.cfg=%_crtr)/*.cfg $(@:%.cfg=%_crtr)/)
#	 Copy folder with campaign name and _lnd ending, if it exists
	$(if $(wildcard $(<:%.cfg=%_lnd)),$(MKDIR) $(@:%.cfg=%_lnd))
	$(if $(wildcard $(<:%.cfg=%_lnd)),-$(CP) $(<:%.cfg=%_lnd)/*.txt $(@:%.cfg=%_lnd)/)
#	 Copy the actual campaign file
	$(CP) $< $@

#******************************************************************************
