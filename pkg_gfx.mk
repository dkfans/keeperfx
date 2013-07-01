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
pkg/ldata/maphand.dat \
pkg/ldata/netfont.dat

TOTRUREGFX = \
pkg/ldata/torture.raw \
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

FRONTENDGFX = \
pkg/ldata/front.raw \
pkg/ldata/frontft1.dat \
pkg/ldata/frontft2.dat \
pkg/ldata/frontft3.dat \
pkg/ldata/frontft4.dat \
pkg/ldata/frontbit.dat

ENGINEGFX = \
pkg/data/gui2-0-1.dat \
pkg/data/gui2-0-0.dat \
pkg/data/guihi.dat \
pkg/data/gui.dat \
pkg/data/hpointer.dat \
pkg/data/lpointer.dat \
pkg/data/hpoints.dat \
pkg/data/lpoints.dat \
pkg/data/hifont.dat \
pkg/data/lofont.dat \
pkg/data/font2-0.dat \
pkg/data/font2-1.dat \
pkg/data/gmapbug.dat

GUIDATTABS = $(LANDVIEWDATTABS) $(TOTRUREDATTABS) $(ENGINEDATTABS)

pkg-gfx: pkg-landviews pkg-menugfx pkg-enginegfx

pkg-landviews: pkg-before $(LANDVIEWRAWS) $(LANDVIEWDATTABS)

pkg-menugfx: pkg-before $(TOTRUREGFX) $(FRONTENDGFX)

pkg-enginegfx: pkg-before $(ENGINEGFX)

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

pkg/ldata/torture.pal: gfx/torturescr/tortr_background.png gfx/torturescr/tortr_doora_open11.png gfx/torturescr/tortr_doorb_open11.png gfx/torturescr/tortr_doorc_open11.png gfx/torturescr/tortr_doord_open11.png gfx/torturescr/tortr_doore_open11.png gfx/torturescr/tortr_doorf_open11.png gfx/torturescr/tortr_doorg_open11.png gfx/torturescr/tortr_doorh_open11.png gfx/torturescr/tortr_doori_open11.png gfx/torturescr/cursor_horny.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
	-$(ECHO) 'Building torture screen palette: $@'
	@$(MKDIR) $(@D)
	$(PNGTOBSPAL) -o "$@" -m "$(filter %.txt,$^)" $(filter %.png,$^)
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/ldata/front.pal: gfx/palettes/front.pal
	-$(ECHO) 'Building engine palette: $@'
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
.PRECIOUS: pkg/ldata/torture.pal pkg/ldata/front.pal pkg/data/palette.dat

pkg/ldata/dkflag00.dat: gfx/landview/filelist_dkflag00.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/netflag.dat: gfx/landview/filelist_netflag.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/maphand.dat: gfx/landview/filelist_maphand.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)
pkg/ldata/netfont.dat: gfx/font_net/filelist_netfont.txt pkg/campgns/keeporig_lnd/rgmap00.pal $(PNGTORAW) $(RNC)

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

pkg/ldata/front.raw: gfx/frontend/front_background.png pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontbit.dat: gfx/frontend/filelist_frontbit.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft1.dat: gfx/frontft1/filelist_frontft1.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft2.dat: gfx/frontft2/filelist_frontft2.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft3.dat: gfx/frontft3/filelist_frontft3.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft4.dat: gfx/frontft4/filelist_frontft4.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)

pkg/data/gmapbug.dat: gfx/parchmentbug/filelist-gbug.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui2-0-1.dat: gfx/gui2-64/filelist_gui2-0-1.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui2-0-0.dat: gfx/gui2-32/filelist_gui2-0-0.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/guihi.dat: gfx/gui1-64/filelist_guihi.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui.dat: gfx/gui1-32/filelist_gui.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/hpointer.dat: gfx/pointer-64/filelist_hpointer.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/lpointer.dat: gfx/pointer-64/filelist_lpointer.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/hpoints.dat: gfx/pointer-64/filelist_hpoints.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/lpoints.dat: gfx/pointer-64/filelist_lpoints.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/hifont.dat: gfx/font_simp-64/filelist_hifont.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/lofont.dat: gfx/font_simp-32/filelist_lofont.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/font2-0.dat: gfx/font2-32/filelist_font2-0.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/font2-1.dat: gfx/font2-64/filelist_font2-1.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)

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
$(foreach campaign,$(sort $(CAMPAIGNS)), gfx/$(campaign)_lnd/%.png) \
gfx/loading/%.png gfx/palettes/%.pal \
gfx/font2-32/%.txt gfx/font2-64/%.txt gfx/font_net/%.txt gfx/font_simp-32/%.txt gfx/font_simp-64/%.txt \
gfx/frontend/%.txt gfx/frontft1/%.txt gfx/frontft2/%.txt gfx/frontft3/%.txt gfx/frontft4/%.txt \
gfx/gui1-32/%.txt gfx/gui1-64/%.txt gfx/gui2-32/%.txt gfx/gui2-64/%.txt gfx/landview/%.txt \
gfx/swipe_bhandrl/%.txt gfx/swipe_clawsrl/%.txt gfx/swipe_clawsu/%.txt gfx/swipe_scythlr/%.txt \
gfx/swipe_sticklr/%.txt gfx/swipe_stickrl/%.txt gfx/swipe_swordrl/%.txt \
gfx/guimap/%.txt gfx/pointer-64/%.txt gfx/sprites/%.txt gfx/textures/%.txt \
gfx/parchmentbug/%.txt gfx/torturescr/%.txt gfx/creatrportrait/%.txt: | gfx/$(GFXSRC_PACKAGE)
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
