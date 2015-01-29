#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_dkillconv.mk
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

.PHONY: clean-dkillconv deep-clean-dkillconv

tools: $(DKILLTOLVL)

clean-tools: clean-dkillconv

deep-clean-tools: deep-clean-dkillconv

ifneq (,$(wildcard tools/dkillconv/src/dkillconv.cpp)) 

# If we have source code of this tool, compile it
$(DKILLTOLVL): tools/dkillconv/src/dkillconv.cpp
	make -C tools/dkillconv

clean-dkillconv:
	make -C tools/dkillconv clean

else ifneq (,$(findstring .tar.gz,$(DKILLCONV_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(DKILLTOLVL): tools/dkillconv/pkg/$(DKILLCONV_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/dkillconv/pkg/$(DKILLCONV_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(DKILLCONV_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-dkillconv:
	-$(RM) tools/dkillconv/bin/*

deep-clean-dkillconv:
	-$(RM) tools/dkillconv/pkg/$(DKILLCONV_PACKAGE)

else ifneq (,$(findstring .zip,$(DKILLCONV_PACKAGE)))

# If we have zip prebuild, download and extract it
$(DKILLTOLVL): tools/dkillconv/pkg/$(DKILLCONV_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/dkillconv/pkg/$(DKILLCONV_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(DKILLCONV_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-dkillconv:
	-$(RM) tools/dkillconv/bin/*

deep-clean-dkillconv:
	-$(RM) tools/dkillconv/pkg/$(DKILLCONV_PACKAGE)

else

$(error Cannot find dkillconv tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
