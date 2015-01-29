#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_pngpal2raw.mk
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

.PHONY: clean-pngpal2raw deep-clean-pngpal2raw

tools: $(PNGTORAW)

clean-tools: clean-pngpal2raw

deep-clean-tools: deep-clean-pngpal2raw

ifneq (,$(wildcard tools/pngpal2raw/src/pngpal2raw.cpp)) 

# If we have source code of this tool, compile it
$(PNGTORAW): tools/pngpal2raw/src/pngpal2raw.cpp
	make -C tools/pngpal2raw

clean-pngpal2raw:
	make -C tools/pngpal2raw clean

else ifneq (,$(findstring .tar.gz,$(PNGTORAW_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(PNGTORAW): tools/pngpal2raw/pkg/$(PNGTORAW_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/pngpal2raw/pkg/$(PNGTORAW_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNGTORAW_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-pngpal2raw:
	-$(RM) tools/pngpal2raw/bin/*

deep-clean-pngpal2raw:
	-$(RM) tools/pngpal2raw/pkg/$(PNGTORAW_PACKAGE)

else ifneq (,$(findstring .zip,$(PNGTORAW_PACKAGE)))

# If we have zip prebuild, download and extract it
$(PNGTORAW): tools/pngpal2raw/pkg/$(PNGTORAW_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/pngpal2raw/pkg/$(PNGTORAW_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNGTORAW_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-pngpal2raw:
	-$(RM) tools/pngpal2raw/bin/*

deep-clean-pngpal2raw:
	-$(RM) tools/pngpal2raw/pkg/$(PNGTORAW_PACKAGE)

else

$(error Cannot find pngpal2raw tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
