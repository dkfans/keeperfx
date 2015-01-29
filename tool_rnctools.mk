#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_rnctools.mk
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

.PHONY: clean-rnctools deep-clean-rnctools

tools: $(RNC) $(DERNC)

clean-tools: clean-rnctools

deep-clean-tools: deep-clean-rnctools

ifneq (,$(wildcard tools/rnctools/src/dernc.c)) 

# If we have source code of this tool, compile it
$(RNC) $(DERNC): tools/rnctools/src/rnc.c tools/rnctools/src/dernc.c
	make -C tools/rnctools

clean-rnctools:
	make -C tools/rnctools clean

else ifneq (,$(findstring .tar.gz,$(RNCTOOLS_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(RNC) $(DERNC): tools/rnctools/pkg/$(RNCTOOLS_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/rnctools/pkg/$(RNCTOOLS_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(RNCTOOLS_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-rnctools:
	-$(RM) tools/rnctools/bin/*

deep-clean-rnctools:
	-$(RM) tools/rnctools/pkg/$(RNCTOOLS_PACKAGE)

else ifneq (,$(findstring .zip,$(RNCTOOLS_PACKAGE)))

# If we have zip prebuild, download and extract it
$(RNC) $(DERNC): tools/rnctools/pkg/$(RNCTOOLS_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/rnctools/pkg/$(RNCTOOLS_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(RNCTOOLS_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-rnctools:
	-$(RM) tools/rnctools/bin/*

deep-clean-rnctools:
	-$(RM) tools/rnctools/pkg/$(RNCTOOLS_PACKAGE)

else

$(error Cannot find rnctools tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
