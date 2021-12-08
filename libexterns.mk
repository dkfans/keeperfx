#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file libexterns.mk
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Defines make rules for libraries which source is external to KeeperFX.
#      Most libraries can be downloaded from official prebuilds.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     08 Jun 2010 - 09 Jun 2010
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************

ARCH = i686-w64-mingw32

include prebuilds.mk

.PHONY: clean-libsdl deep-clean-libsdl

.INTERMEDIATE: libsdl libsdlnet libsdlmixer

libexterns: libsdl libsdlnet libsdlmixer
	touch libexterns

clean-libexterns: clean-libsdl
	$(RM) libexterns

deep-clean-libexterns: deep-clean-libsdl

ifneq (,$(findstring .tar.gz,$(SDL_PACKAGE)))

libsdl: sdl/lib/libSDL2main.a

# If we have tar gzip prebuild, download and extract it
sdl/lib/libSDL2main.a: sdl/$(SDL_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	# Grep is used to remove bogus error messages, return state of tar is also ignored
	-cd "$(<D)"; \
	tar --strip-components=2 -zxmUf "$(<F)" SDL2-2.0.12/i686-w64-mingw32/bin SDL2-2.0.12/i686-w64-mingw32/include SDL2-2.0.12/i686-w64-mingw32/lib SDL2-2.0.12/i686-w64-mingw32/share 2>&1 | \
	grep -v '^.*: Archive value .* is out of .* range.*$$'
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

sdl/$(SDL_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SDL_DOWNLOAD)"
	tar -tzf "$@.dl" >/dev/null
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

else

$(error Cannot handle SDL library prebuild. You need to prepare the library manually.)

endif

##################

ifneq (,$(findstring .tar.gz,$(SDL_NET_PACKAGE)))

libsdlnet: sdl/lib/libSDL2_net.lib

sdl/lib/libSDL2_net.lib: sdl/$(SDL_NET_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) sdl/lib sdl/include/SDL2
	cd "$(<D)"; \
	tar -xzf "$(<F)"
	$(MV) sdl/SDL2_net-*/$(ARCH)/include/SDL2/* sdl/include/SDL2/
	$(CP) -r sdl/SDL2_net-*/$(ARCH)/lib/* sdl/lib/
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

sdl/$(SDL_NET_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SDL_NET_DOWNLOAD)"
	tar -tzf "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

else

libsdlnet: sdl/lib/SDL2_net.lib

sdl/lib/SDL2_net.lib: sdl/$(SDL_NET_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) sdl/lib sdl/include/SDL2
	cd "$(<D)"; \
	unzip -DD -qo "$(<F)"
	$(MV) sdl/SDL2_net-*/include/* sdl/include/SDL2/
	$(MV) sdl/SDL2_net-*/lib/x86/* sdl/lib/
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

sdl/$(SDL_NET_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SDL_NET_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

endif

##################

ifneq (,$(findstring .tar.gz,$(SDL_MIXER_PACKAGE)))

libsdlmixer: sdl/lib/libSDL2_mixer.lib

sdl/lib/libSDL2_mixer.lib: sdl/$(SDL_MIXER_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) sdl/lib sdl/include/SDL2
	cd "$(<D)"; \
	tar -xzf "$(<F)"
	$(MV) sdl/SDL2_mixer-*/$(ARCH)/include/SDL2/* sdl/include/SDL2/
	$(CP) -r sdl/SDL2_mixer-*/$(ARCH)/lib/* sdl/lib/
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

sdl/$(SDL_MIXER_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SDL_MIXER_DOWNLOAD)"
	tar -tzf "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

else

libsdlmixer: sdl/lib/SDL2_mixer.lib

sdl/lib/SDL2_mixer.lib: sdl/$(SDL_MIXER_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) sdl/lib sdl/include/SDL2
	cd "$(<D)"; \
	unzip -DD -qo "$(<F)"
	$(MV) sdl/SDL2_mixer-*/include/* sdl/include/SDL2/
	$(MV) sdl/SDL2_mixer-*/lib/x86/* sdl/lib/
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

sdl/$(SDL_MIXER_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SDL_MIXER_DOWNLOAD)"
	unzip -qt "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

endif

clean-libsdl:
	-$(RM) -R sdl/bin sdl/include sdl/lib sdl/share

deep-clean-libsdl:
	-$(RM) sdl/$(SDL_PACKAGE)
	-$(RM) sdl/$(SDL_NET_PACKAGE)
	-$(RM) sdl/$(SDL_MIXER_PACKAGE)

#******************************************************************************
