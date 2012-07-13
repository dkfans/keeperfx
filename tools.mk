#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file tools.mk
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Defines make rules for tools needed to build KeeperFX.
#      Most tools can either by compiled from source or downloaded.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     21 Apr 2010 - 25 Apr 2010
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************

.PHONY: clean-peresec deep-clean-peresec
.PHONY: clean-png2ico deep-clean-png2ico

tools: $(EXETODLL) $(PNG2ICO)

clean-tools: clean-peresec clean-png2ico

deep-clean-tools: deep-clean-peresec deep-clean-png2ico

ifneq (,$(wildcard tools/peresec/src/peresec.c)) 

# If we have source code of this tool, compile it
$(EXETODLL): tools/peresec/src/peresec.c
	make -C tools/peresec

clean-peresec:
	make -C tools/peresec clean

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

$(error Cannot find peresec tool source nor prebuild. Use git submodules to get source.)

endif

ifneq (,$(wildcard tools/png2ico/png2ico.cpp)) 

# If we have source code of this tool, compile it
$(PNG2ICO): tools/png2ico/png2ico.cpp
	make -C tools/png2ico

clean-png2ico:
	make -C tools/png2ico clean

else ifneq (,$(findstring .tar.gz,$(PNG2ICO_PACKAGE)))

# If we have tar gzip prebuild, download and extract it
$(PNG2ICO): tools/png2ico/$(PNG2ICO_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	tar -zxmUf "../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/png2ico/$(PNG2ICO_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNG2ICO_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-png2ico:
	-$(RM) $(PNG2ICO) tools/png2ico/README tools/png2ico/VERSION tools/png2ico/LICENSE tools/png2ico/doc/png2ico.txt

deep-clean-png2ico:
	-$(RM) tools/png2ico/$(PNG2ICO_PACKAGE)

else ifneq (,$(findstring .zip,$(PNG2ICO_PACKAGE)))

# If we have zip prebuild, download and extract it
$(PNG2ICO): tools/png2ico/$(PNG2ICO_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) "$(@D)"
	cd "$(@D)"; \
	unzip -DD -qo "../../$<"
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

tools/png2ico/$(PNG2ICO_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(PNG2ICO_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

clean-png2ico:
	-$(RM) $(PNG2ICO) tools/png2ico/README tools/png2ico/VERSION tools/png2ico/LICENSE tools/png2ico/doc/png2ico.txt

deep-clean-png2ico:
	-$(RM) tools/png2ico/$(PNG2ICO_PACKAGE)

else

$(error Cannot find png2ico tool source nor prebuild. Get package or source manually.)

endif
