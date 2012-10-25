#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_png2ico.mk
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

.PHONY: clean-png2ico deep-clean-png2ico

tools: $(PNGTOICO)

clean-tools: clean-png2ico

deep-clean-tools: deep-clean-png2ico

ifneq (,$(wildcard tools/png2ico/png2ico.cpp)) 

# If we have source code of this tool, compile it
$(PNGTOICO): tools/png2ico/png2ico.cpp
	make -C tools/png2ico

clean-png2ico:
	make -C tools/png2ico clean

else ifneq (,$(findstring .tar.gz,$(PNGTOICO_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(PNGTOICO): tools/png2ico/$(PNGTOICO_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/png2ico/$(PNGTOICO_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNGTOICO_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-png2ico:
	-$(RM) $(PNGTOICO) tools/png2ico/README tools/png2ico/VERSION tools/png2ico/LICENSE tools/png2ico/doc/png2ico.txt

deep-clean-png2ico:
	-$(RM) tools/png2ico/$(PNGTOICO_PACKAGE)

else ifneq (,$(findstring .zip,$(PNGTOICO_PACKAGE)))

# If we have zip prebuild, download and extract it
$(PNGTOICO): tools/png2ico/$(PNGTOICO_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/png2ico/$(PNGTOICO_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNGTOICO_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-png2ico:
	-$(RM) $(PNGTOICO) tools/png2ico/README tools/png2ico/VERSION tools/png2ico/LICENSE tools/png2ico/doc/png2ico.txt

deep-clean-png2ico:
	-$(RM) tools/png2ico/$(PNGTOICO_PACKAGE)

else

$(error Cannot find png2ico tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
