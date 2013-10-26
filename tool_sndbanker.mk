#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_sndbanker.mk
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

.PHONY: clean-sndbanker deep-clean-sndbanker

tools: $(WAVTODAT)

clean-tools: clean-sndbanker

deep-clean-tools: deep-clean-sndbanker

ifneq (,$(wildcard tools/sndbanker/src/sndbanker.cpp)) 

# If we have source code of this tool, compile it
$(WAVTODAT): tools/sndbanker/src/sndbanker.cpp
	make -C tools/sndbanker

clean-sndbanker:
	make -C tools/sndbanker clean

else ifneq (,$(findstring .tar.gz,$(SNDBANKER_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(WAVTODAT): tools/sndbanker/pkg/$(SNDBANKER_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/sndbanker/pkg/$(SNDBANKER_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SNDBANKER_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-sndbanker:
	-$(RM) tools/sndbanker/bin/*

deep-clean-sndbanker:
	-$(RM) tools/sndbanker/pkg/$(SNDBANKER_PACKAGE)

else ifneq (,$(findstring .zip,$(SNDBANKER_PACKAGE)))

# If we have zip prebuild, download and extract it
$(WAVTODAT): tools/sndbanker/pkg/$(SNDBANKER_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/sndbanker/pkg/$(SNDBANKER_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SNDBANKER_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-sndbanker:
	-$(RM) tools/sndbanker/bin/*

deep-clean-sndbanker:
	-$(RM) tools/sndbanker/pkg/$(SNDBANKER_PACKAGE)

else

$(error Cannot find sndbanker tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
