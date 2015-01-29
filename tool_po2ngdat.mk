#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_po2ngdat.mk
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

.PHONY: clean-po2ngdat deep-clean-po2ngdat

tools: $(POTONGDAT)

clean-tools: clean-po2ngdat

deep-clean-tools: deep-clean-po2ngdat

ifneq (,$(wildcard tools/po2ngdat/src/po2ngdat.cpp)) 

# If we have source code of this tool, compile it
$(POTONGDAT): tools/po2ngdat/src/po2ngdat.cpp
	make -C tools/po2ngdat

clean-po2ngdat:
	make -C tools/po2ngdat clean

else ifneq (,$(findstring .tar.gz,$(POTONGDAT_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(POTONGDAT): tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<" --exclude="*char_encoding_*.txt"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/po2ngdat/res/%.txt: tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE)
	-$(ECHO) 'Extracting encoding table: $@'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<" --wildcards "*char_encoding_*.txt"
	-$(ECHO) 'Finished extracting: $@'
	-$(ECHO) ' '

tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(POTONGDAT_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-po2ngdat:
	-$(RM) tools/po2ngdat/bin/*

deep-clean-po2ngdat:
	-$(RM) tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE)

else ifneq (,$(findstring .zip,$(POTONGDAT_PACKAGE)))

# If we have zip prebuild, download and extract it
$(POTONGDAT): tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<" -x "char_encoding_*.txt"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/po2ngdat/res/%.txt: tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE)
	-$(ECHO) 'Extracting encoding table: $@'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<" "char_encoding_*.txt"
	-$(ECHO) 'Finished extracting: $@'
	-$(ECHO) ' '

tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(POTONGDAT_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-po2ngdat:
	-$(RM) tools/po2ngdat/bin/*

deep-clean-po2ngdat:
	-$(RM) tools/po2ngdat/pkg/$(POTONGDAT_PACKAGE)

else

$(error Cannot find po2ngdat tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
