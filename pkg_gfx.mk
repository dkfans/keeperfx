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
pkg/data/legal32.raw \
pkg/data/legal64.raw \
pkg/data/startupfx64.raw \
pkg/data/loading32.raw \
pkg/data/loading64.raw \
pkg/data/nocd.raw \
pkg/ldata/front.raw \
pkg/ldata/frontft1.dat \
pkg/ldata/frontft2.dat \
pkg/ldata/frontft3.dat \
pkg/ldata/frontft4.dat \
pkg/ldata/frontbit.dat

ENGINEGFX = \
pkg/data/creature.jty \
pkg/data/frac00.raw \
pkg/data/frac01.raw \
pkg/data/frac02.raw \
pkg/data/frac03.raw \
pkg/data/frac04.raw \
pkg/data/frac05.raw \
pkg/data/frac06.raw \
pkg/data/frac07.raw \
pkg/data/frac08.raw \
pkg/data/gmap.raw \
pkg/data/gmaphi.raw \
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
pkg/data/tmapa000.dat \
pkg/data/tmapa001.dat \
pkg/data/tmapa002.dat \
pkg/data/tmapa003.dat \
pkg/data/tmapa004.dat \
pkg/data/tmapa005.dat \
pkg/data/tmapa006.dat \
pkg/data/tmapa007.dat \
pkg/data/swpe00.dat \
pkg/data/swpe01.dat \
pkg/data/swpe02.dat \
pkg/data/swpe03.dat \
pkg/data/swpe04.dat \
pkg/data/swpe05.dat \
pkg/data/swpe06.dat \
pkg/data/swpe07.dat \
pkg/data/swpe08.dat \
pkg/data/swpe09.dat \
pkg/data/swpe10.dat \
pkg/data/swpe11.dat \
pkg/data/swpe12.dat \
pkg/data/swpe13.dat \
pkg/data/swpe14.dat \
pkg/data/swpe15.dat \
pkg/data/swpe16.dat \
pkg/data/swpe17.dat \
pkg/data/swpe18.dat \
pkg/data/swpe19.dat \
pkg/data/swpe20.dat \
pkg/data/swpe21.dat \
pkg/data/swpe22.dat \
pkg/data/swpe23.dat \
pkg/data/swpe24.dat \
pkg/data/swpe25.dat \
pkg/data/swpe26.dat \
pkg/data/swpe27.dat \
pkg/data/swpe28.dat \
pkg/data/swpe29.dat \
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
pkg/data/legal32.pal: gfx/loading/legal32.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/legal64.pal: gfx/loading/legal64.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/startupfx64.pal: gfx/loading/startupfx64.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/loading32.pal: gfx/loading/loading32.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/loading64.pal: gfx/loading/loading64.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)
pkg/data/nocd.pal: gfx/loading/nocd32.png tools/png2bestpal/res/color_tbl_basic.txt $(PNGTOBSPAL)

pkg/ldata/torture.pal pkg/data/legal32.pal pkg/data/legal64.pal pkg/data/startupfx64.pal pkg/data/loading32.pal pkg/data/loading64.pal pkg/data/nocd.pal:
	-$(ECHO) 'Building palette: $@'
	@$(MKDIR) $(@D)
	$(PNGTOBSPAL) -o "$@" -m "$(filter %.txt,$^)" $(filter %.png,$^)
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/ldata/front.pal: gfx/palettes/front.pal
pkg/data/palette.dat: gfx/palettes/engine.pal

pkg/ldata/front.pal pkg/data/palette.dat:
	-$(ECHO) 'Building palette: $@'
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
pkg/ldata/frontft1.dat: gfx/font_front_hdr_red-64/filelist_frontft1.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft2.dat: gfx/font_front_std_red-64/filelist_frontft2.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft3.dat: gfx/font_front_std_ylw-64/filelist_frontft3.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)
pkg/ldata/frontft4.dat: gfx/font_front_std_dkr-64/filelist_frontft4.txt pkg/ldata/front.pal $(PNGTORAW) $(RNC)

pkg/data/frac00.raw: gfx/textures/frac00.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac01.raw: gfx/textures/frac01.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac02.raw: gfx/textures/frac02.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac03.raw: gfx/textures/frac03.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac04.raw: gfx/textures/frac04.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac05.raw: gfx/textures/frac05.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac06.raw: gfx/textures/frac06.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac07.raw: gfx/textures/frac07.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)
pkg/data/frac08.raw: gfx/textures/frac08.png gfx/textures/fract_bw.pal $(PNGTORAW) $(RNC)

pkg/data/tmapa000.dat: gfx/textures/tmapa000.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa001.dat: gfx/textures/tmapa001.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa002.dat: gfx/textures/tmapa002.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa003.dat: gfx/textures/tmapa003.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa004.dat: gfx/textures/tmapa004.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa005.dat: gfx/textures/tmapa005.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa006.dat: gfx/textures/tmapa006.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/tmapa007.dat: gfx/textures/tmapa007.png pkg/data/palette.dat $(PNGTORAW) $(RNC)

pkg/data/gmap.raw: gfx/guimap/gmap32.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gmaphi.raw: gfx/guimap/gmap64.png pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gmapbug.dat: gfx/parchmentbug/filelist-gbug.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/legal32.raw: gfx/loading/legal32.png pkg/data/legal32.pal $(PNGTORAW) $(RNC)
pkg/data/legal64.raw: gfx/loading/legal64.png pkg/data/legal64.pal $(PNGTORAW) $(RNC)
pkg/data/startupfx64.raw: gfx/loading/startupfx64.png pkg/data/startupfx64.pal $(PNGTORAW) $(RNC)
pkg/data/loading32.raw: gfx/loading/loading32.png pkg/data/loading32.pal $(PNGTORAW) $(RNC)
pkg/data/loading64.raw: gfx/loading/loading64.png pkg/data/loading64.pal $(PNGTORAW) $(RNC)
pkg/data/nocd.raw: gfx/loading/nocd32.png pkg/data/nocd.pal $(PNGTORAW) $(RNC)
pkg/data/gui2-0-1.dat: gfx/gui2-64/filelist_gui2-64.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui2-0-0.dat: gfx/gui2-32/filelist_gui2-32.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/guihi.dat: gfx/gui1-64/filelist_gui1-64.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/gui.dat: gfx/gui1-32/filelist_gui1-32.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/hpointer.dat: gfx/pointer-64/filelist_pointer.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/lpointer.dat: gfx/pointer-64/filelist_lpointer.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/hpoints.dat: gfx/pointer-64/filelist_points.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/lpoints.dat: gfx/pointer-64/filelist_lpoints.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/hifont.dat: gfx/font_simp-64/filelist_font1.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/lofont.dat: gfx/font_simp-32/filelist_font1.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/font2-0.dat: gfx/font2-32/filelist_font2.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/font2-1.dat: gfx/font2-64/filelist_font2.txt pkg/data/palette.dat $(PNGTORAW) $(RNC)
pkg/data/creature.jty: gfx/sprites/animlist.txt pkg/data/palette.dat $(PNGTORAW)

pkg/data/swpe00.dat: gfx/swipes/filelist_bhandrl_frame1.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe01.dat: gfx/swipes/filelist_bhandrl_frame2.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe02.dat: gfx/swipes/filelist_bhandrl_frame3.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe03.dat: gfx/swipes/filelist_bhandrl_frame4.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe04.dat: gfx/swipes/filelist_bhandrl_frame5.txt pkg/data/palette.dat $(PNGTORAW)

pkg/data/swpe05.dat: gfx/swipes/filelist_swordrl_frame1.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe06.dat: gfx/swipes/filelist_swordrl_frame2.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe07.dat: gfx/swipes/filelist_swordrl_frame3.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe08.dat: gfx/swipes/filelist_swordrl_frame4.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe09.dat: gfx/swipes/filelist_swordrl_frame5.txt pkg/data/palette.dat $(PNGTORAW)

pkg/data/swpe10.dat: gfx/swipes/filelist_scythlr_frame1.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe11.dat: gfx/swipes/filelist_scythlr_frame2.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe12.dat: gfx/swipes/filelist_scythlr_frame3.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe13.dat: gfx/swipes/filelist_scythlr_frame4.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe14.dat: gfx/swipes/filelist_scythlr_frame5.txt pkg/data/palette.dat $(PNGTORAW)

pkg/data/swpe15.dat: gfx/swipes/filelist_sticklr_frame1.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe16.dat: gfx/swipes/filelist_sticklr_frame2.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe17.dat: gfx/swipes/filelist_sticklr_frame3.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe18.dat: gfx/swipes/filelist_sticklr_frame4.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe19.dat: gfx/swipes/filelist_sticklr_frame5.txt pkg/data/palette.dat $(PNGTORAW)

pkg/data/swpe20.dat: gfx/swipes/filelist_stickrl_frame1.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe21.dat: gfx/swipes/filelist_stickrl_frame2.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe22.dat: gfx/swipes/filelist_stickrl_frame3.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe23.dat: gfx/swipes/filelist_stickrl_frame4.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe24.dat: gfx/swipes/filelist_stickrl_frame5.txt pkg/data/palette.dat $(PNGTORAW)

pkg/data/swpe25.dat: gfx/swipes/filelist_clawsrl_frame1.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe26.dat: gfx/swipes/filelist_clawsrl_frame2.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe27.dat: gfx/swipes/filelist_clawsrl_frame3.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe28.dat: gfx/swipes/filelist_clawsrl_frame4.txt pkg/data/palette.dat $(PNGTORAW)
pkg/data/swpe29.dat: gfx/swipes/filelist_clawsrl_frame5.txt pkg/data/palette.dat $(PNGTORAW)

pkg/data/frac%.raw:
	-$(ECHO) 'Building RAW texture: $@'
	$(PNGTORAW) -o "$@" -p "$(word 2,$^)" -r 255 -f raw -l 100 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

pkg/data/tmapa%.dat:
	-$(ECHO) 'Building RAW texture: $@'
	$(PNGTORAW) -o "$@" -p "$(word 2,$^)" -f raw -l 0 "$<"
	-$(RNC) "$@"
	-$(ECHO) 'Finished building: $@'
	-$(ECHO) ' '

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

pkg/creatrs/%.jty pkg/data/%.jty:
	-$(ECHO) 'Building jonty sprites: $@'
	@$(MKDIR) $(@D)
	$(PNGTORAW) -m -o "$@" -p "$(word 2,$^)" -f jspr -l 0 "$<"
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
gfx/guimap/%.txt gfx/pointer-64/%.txt gfx/sprites/%.txt gfx/textures/%.png gfx/textures/%.txt \
gfx/torturescr/%.png gfx/torturescr/%.txt gfx/parchmentbug/%.txt gfx/creatrportrait/%.txt: | gfx/$(GFXSRC_PACKAGE)
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
