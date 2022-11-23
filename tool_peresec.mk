#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tool_peresec.mk
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

.PHONY: clean-peresec deep-clean-peresec

tools: $(EXETODLL)

clean-tools: clean-peresec

deep-clean-tools: deep-clean-peresec

ifneq (,$(wildcard tools/peresec/src/peresec.c)) 

# If we have source code of this tool, compile it
$(EXETODLL): tools/peresec/src/peresec.c
	$(MAKE) -C tools/peresec

clean-peresec:
	$(MAKE) -C tools/peresec clean

else ifneq (,$(findstring .tar.gz,$(PERESEC_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(EXETODLL): tools/peresec/pkg/$(PERESEC_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/peresec/pkg/$(PERESEC_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PERESEC_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-peresec:
	-$(RM) tools/peresec/bin/*

deep-clean-peresec:
	-$(RM) tools/peresec/pkg/$(PERESEC_PACKAGE)

else ifneq (,$(findstring .zip,$(PERESEC_PACKAGE)))

# If we have zip prebuild, download and extract it
$(EXETODLL): tools/peresec/pkg/$(PERESEC_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/peresec/pkg/$(PERESEC_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PERESEC_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-peresec:
	-$(RM) tools/peresec/bin/*

deep-clean-peresec:
	-$(RM) tools/peresec/pkg/$(PERESEC_PACKAGE)

else

$(error Cannot find peresec tool source nor prebuild. Get package or source manually.)

endif

#******************************************************************************
