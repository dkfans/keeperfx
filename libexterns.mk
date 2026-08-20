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
SDL_MAIN_LIBRARY = sdl/lib/libSDL3.dll.a
SDL_EXTENSION_LIBRARIES = \
	sdl/lib/libSDL3_mixer.dll.a \
	sdl/lib/libSDL3_image.dll.a

include prebuilds.mk

.PHONY: clean-libsdl deep-clean-libsdl

.INTERMEDIATE: libsdl libsdlmixer libsdlimage

libexterns: libsdl libsdlmixer libsdlimage
	touch libexterns

clean-libexterns: clean-libsdl
	$(RM) libexterns

deep-clean-libexterns: deep-clean-libsdl

ifneq (,$(findstring .tar.gz,$(SDL_PACKAGE)))

libsdl: $(SDL_MAIN_LIBRARY)

# If we have tar gzip prebuild, download and extract it
$(SDL_MAIN_LIBRARY): sdl/$(SDL_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	# Grep is used to remove bogus error messages, return state of tar is also ignored
	-cd "$(<D)"; \
	tar --strip-components=2 -zxmUf "$(<F)" SDL3-3.4.12/i686-w64-mingw32/bin SDL3-3.4.12/i686-w64-mingw32/include SDL3-3.4.12/i686-w64-mingw32/lib SDL3-3.4.12/i686-w64-mingw32/share 2>&1 | \
	grep -v '^.*: Archive value .* is out of .* range.*$$'
	$(CP) sdl/bin/SDL3.dll sdl/for_final_package/
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

$(SDL_EXTENSION_LIBRARIES): | $(SDL_MAIN_LIBRARY)

ifneq (,$(findstring .tar.gz,$(SDL_MIXER_PACKAGE)))

libsdlmixer: sdl/lib/libSDL3_mixer.dll.a

sdl/lib/libSDL3_mixer.dll.a: sdl/$(SDL_MIXER_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) sdl/lib sdl/include
	cd "$(<D)"; \
	tar -xzf "$(<F)"
	$(CP) -r sdl/SDL3_mixer-*/$(ARCH)/include/* sdl/include/
	$(CP) -r sdl/SDL3_mixer-*/$(ARCH)/lib/* sdl/lib/
	$(CP) sdl/SDL3_mixer-*/$(ARCH)/bin/SDL3_mixer.dll sdl/for_final_package/
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

$(error Cannot handle SDL_mixer library prebuild. You need to prepare the library manually.)

endif

##################

ifneq (,$(findstring .tar.gz,$(SDL_IMAGE_PACKAGE)))

libsdlimage: sdl/lib/libSDL3_image.dll.a

sdl/lib/libSDL3_image.dll.a: sdl/$(SDL_IMAGE_PACKAGE)
	-$(ECHO) 'Extracting package: $<'
	$(MKDIR) sdl/lib sdl/include
	cd "$(<D)"; \
	tar -xzf "$(<F)"
	$(CP) -r sdl/SDL3_image-*/$(ARCH)/include/* sdl/include/
	$(CP) -r sdl/SDL3_image-*/$(ARCH)/lib/* sdl/lib/
	$(CP) sdl/SDL3_image-*/$(ARCH)/bin/SDL3_image.dll sdl/for_final_package/
	-$(ECHO) 'Finished extracting: $<'
	-$(ECHO) ' '

sdl/$(SDL_IMAGE_PACKAGE):
	-$(ECHO) 'Downloading package: $@'
	$(MKDIR) "$(@D)"
	curl -L -o "$@.dl" "$(SDL_IMAGE_DOWNLOAD)"
	tar -tzf "$@.dl"
	$(MV) "$@.dl" "$@"
	-$(ECHO) 'Finished downloading: $@'
	-$(ECHO) ' '

else

$(error Cannot handle SDL_image library prebuild. You need to prepare the library manually.)

endif

clean-libsdl:
	-$(RM) -R sdl/bin sdl/include sdl/lib sdl/share

deep-clean-libsdl:
	-$(RM) -rf sdl
	-$(MKDIR) sdl


#******************************************************************************
