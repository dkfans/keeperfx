#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_png2bestpal.mk
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

.PHONY: clean-png2bestpal deep-clean-png2bestpal

tools: $(PNGTOBSPAL)

clean-tools: clean-png2bestpal

deep-clean-tools: deep-clean-png2bestpal

ifneq (,$(wildcard tools/png2bestpal/src/png2bestpal.cpp)) 

# If we have source code of this tool, compile it
$(PNGTOBSPAL): tools/png2bestpal/src/png2bestpal.cpp
	make -C tools/png2bestpal

clean-png2bestpal:
	make -C tools/png2bestpal clean

else ifneq (,$(findstring .tar.gz,$(PNGTOBSPAL_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(PNGTOBSPAL): tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<" --exclude="*color_tbl_*.txt"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/png2bestpal/res/%.txt: tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE)
	-$(ECHO) 'Extracting encoding table: $@'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<" --wildcards "*color_tbl_*.txt"
	-$(ECHO) 'Finished extracting: $@'
	-$(ECHO) ' '

tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNGTOBSPAL_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-png2bestpal:
	-$(RM) tools/png2bestpal/bin/*

deep-clean-png2bestpal:
	-$(RM) tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE)

else ifneq (,$(findstring .zip,$(PNGTOBSPAL_PACKAGE)))

# If we have zip prebuild, download and extract it
$(PNGTOBSPAL): tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<" -x "color_tbl_*.txt"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/png2bestpal/res/%.txt: tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE)
	-$(ECHO) 'Extracting encoding table: $@'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<" "color_tbl_*.txt"
	-$(ECHO) 'Finished extracting: $@'
	-$(ECHO) ' '

tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNGTOBSPAL_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-png2bestpal:
	-$(RM) tools/png2bestpal/bin/*

deep-clean-png2bestpal:
	-$(RM) tools/png2bestpal/pkg/$(PNGTOBSPAL_PACKAGE)

else

$(error Cannot find png2bestpal tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
