#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file pkg_gfx.mk
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

LANDVIEWRAWS = \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/keeporig_lnd/rgmap$(num).raw pkg/campgns/keeporig_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/ancntkpr_lnd/rgmap$(num).raw pkg/campgns/ancntkpr_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09,pkg/campgns/burdnimp_lnd/rgmap$(num).raw pkg/campgns/burdnimp_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/cqarctic_lnd/rgmap$(num).raw pkg/campgns/cqarctic_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10,pkg/campgns/dstninja_lnd/rgmap$(num).raw pkg/campgns/dstninja_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/dzjr06lv_lnd/rgmap$(num).raw pkg/campgns/dzjr06lv_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/dzjr10lv_lnd/rgmap$(num).raw pkg/campgns/dzjr10lv_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/dzjr25lv_lnd/rgmap$(num).raw pkg/campgns/dzjr25lv_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21,pkg/campgns/evilkeep_lnd/rgmap$(num).raw pkg/campgns/evilkeep_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/grkreign_lnd/rgmap$(num).raw pkg/campgns/grkreign_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10,pkg/campgns/jdkmaps8_lnd/rgmap$(num).raw pkg/campgns/jdkmaps8_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18,pkg/campgns/kdklvpck_lnd/rgmap$(num).raw pkg/campgns/kdklvpck_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11 12,pkg/campgns/lqizgood_lnd/rgmap$(num).raw pkg/campgns/lqizgood_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/lrdvexer_lnd/rgmap$(num).raw pkg/campgns/lrdvexer_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09 10 11,pkg/campgns/ncastles_lnd/rgmap$(num).raw pkg/campgns/ncastles_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08,pkg/campgns/postanck_lnd/rgmap$(num).raw pkg/campgns/postanck_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/pstunded_lnd/rgmap$(num).raw pkg/campgns/pstunded_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07 08 09,pkg/campgns/questfth_lnd/rgmap$(num).raw pkg/campgns/questfth_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/twinkprs_lnd/rgmap$(num).raw pkg/campgns/twinkprs_lnd/viframe$(num).dat) \
$(foreach num,00 01 02 03 04 05 06 07,pkg/campgns/undedkpr_lnd/rgmap$(num).raw pkg/campgns/undedkpr_lnd/viframe$(num).dat)

LANDVIEWDATTABS = \
pkg/ldata/dkflag00.dat \
pkg/ldata/netflag.dat \
pkg/ldata/maphand.dat

TOTRUREDATTABS = \
pkg/ldata/door01.dat \
pkg/ldata/door02.dat \
pkg/ldata/door03.dat \
pkg/ldata/door04.dat \
pkg/ldata/door05.dat \
pkg/ldata/door06.dat \
pkg/ldata/door07.dat \
pkg/ldata/door08.dat \
pkg/ldata/door09.dat \
pkg/ldata/fronttor.dat

ENGINEDATTABS = \
pkg/data/gui2-0-1.dat \
pkg/data/gui2-0-0.dat \
pkg/data/guihi.dat \
pkg/data/gui.dat \
pkg/data/gmapbug.dat

GUIDATTABS = $(LANDVIEWDATTABS) $(TOTRUREDATTABS) $(ENGINEDATTABS)

pkg-gfx: pkg-landviews pkg-guidattabs
pkg-landviews: $(LANDVIEWRAWS) pkg-before

pkg-guidattabs: $(GUIDATTABS) pkg-before

# Creation of land view image files for campaigns
define define_campaign_landview_rule
pkg/campgns/$(1)_lnd/rgmap%.pal: gfx/$(1)_lnd/rgmap%.png gfx/$(1)_lnd/viframe.png tools/png2bestpal/res/color_tbl_landview.txt $$(PNGTOBSPAL)
	-$$(ECHO) 'Building land view palette: $$@'
	@$$(MKDIR) $$(@D)
	$$(PNGTOBSPAL) -o "$$@" -m "$$(word 3,$$^)" "$$(word 1,$$^)" "$$(word 2,$$^)"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)_lnd/rgmap%.raw: gfx/$(1)_lnd/rgmap%.png pkg/campgns/$(1)_lnd/rgmap%.pal $$(PNGTORAW) $$(RNC)
	-$$(ECHO) 'Building land view image: $$@'
	$$(PNGTORAW) -o "$$@" -p "$$(word 2,$$^)" -f raw -l 100 "$$<"
	-$$(RNC) "$$@"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

pkg/campgns/$(1)_lnd/viframe%.dat: gfx/$(1)_lnd/viframe.png pkg/campgns/$(1)_lnd/rgmap%.pal $$(PNGTORAW) $$(RNC)
	-$$(ECHO) 'Building land view frame: $$@'
	$$(PNGTORAW) -o "$$@" -p "$$(word 2,$$^)" -f hspr -l 50 "$$<"
	-$$(RNC) "$$@"
	-$$(ECHO) 'Finished building: $$@'
	-$$(ECHO) ' '

# mark palette files precious to make sure they're not auto-removed after dependencies are built
.PRECIOUS: pkg/campgns/$(1)_lnd/rgmap%.pal
endef

$(foreach campaign,$(sort $(CAMPAIGNS)),$(eval $(call define_campaign_landview_rule,$(campaign))))

pkg/ldata/torture.pal: gfx/palettes/torture.pal
	-$(ECHO) 'Building torture screen palette: $@'
	@$(MKDIR) $(@D)
	# Simplified, for now
	$(CP) "$<" "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/data/palette.dat: gfx/palettes/engine.pal
	-$(ECHO) 'Building engine palette: $@'
	@$(MKDIR) $(@D)
	# Simplified, for now
	$(CP) "$<" "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# mark palette files precious to make sure they're not auto-removed after dependencies are built
.PRECIOUS: pkg/ldata/torture.pal pkg/data/palette.dat

pkg/ldata/dkflag00.dat: gfx/landview/filelist_dkflag00.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/netflag.dat: gfx/landview/filelist_netflag.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/maphand.dat: gfx/landview/filelist_maphand.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)

pkg/ldata/fronttor.dat: gfx/torturescr/filelist_fronttor.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door01.dat: gfx/torturescr/filelist_tortr_doora.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door02.dat: gfx/torturescr/filelist_tortr_doorb.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door03.dat: gfx/torturescr/filelist_tortr_doorc.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door04.dat: gfx/torturescr/filelist_tortr_doord.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door05.dat: gfx/torturescr/filelist_tortr_doore.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door06.dat: gfx/torturescr/filelist_tortr_doorf.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door07.dat: gfx/torturescr/filelist_tortr_doorg.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door08.dat: gfx/torturescr/filelist_tortr_doorh.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/door09.dat: gfx/torturescr/filelist_tortr_doori.txt pkg/ldata/torture.pal $(PNGTORAW) $(RNC)
pkg/ldata/torture.raw: gfx/torturescr/tortr_background.png pkg/ldata/torture.pal $(PNGTORAW) $(RNC)

pkg/data/gmapbug.dat: gfx/parchmentbug/filelist-gbug.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui2-0-1.dat: gfx/gui2-64/filelist_gui2-0-1.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui2-0-0.dat: gfx/gui2-32/filelist_gui2-0-0.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/guihi.dat: gfx/gui1-64/filelist_guihi.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui.dat: gfx/gui1-32/filelist_gui.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)

pkg/ldata/%.raw pkg/data/%.raw:
	-$(ECHO) 'Building RAW image: $@'
	$(PNGTORAW) -o "$@" -p "$(word 2,$^)" -f raw -l 100 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/ldata/%.dat pkg/data/%.dat:
	-$(ECHO) 'Building tabulated sprites: $@'
	$(MKDIR) "$(@D)"
	$(PNGTORAW) -b -o "$@" -p "$(word 2,$^)" -f sspr -l 0 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

# The package is extracted only if targets does not exits; the "|" causes file dates to be ignored
# Note that ignoring timestamp means it is possible to have outadated files after a new
# package release, if no targets were modified with the update.
$(foreach campaign,$(sort $(CAMPAIGNS)), gfx/$(campaign)_lnd/%.png) gfx/%/filelist.txt: | gfx/$(GFXSRC_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	7z x -aoa -y -ogfx "$|"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

# Downloading the gfx sources pack
gfx/$(GFXSRC_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(GFXSRC_DOWNLOAD)"
	7z t "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

#******************************************************************************
