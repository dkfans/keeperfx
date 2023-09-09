VER_MAJOR=0
VER_MINOR=5
VER_RELEASE=0
VER_BUILD=0

# Set version to 0.0.0.0 when compiling with DEBUG=1, to distinguish it from normal releases. DEBUG=1 makes the executable filesize huge when packing it with debug symbols.
ifeq ($(DEBUG), 1)
	VER_MAJOR=0
	VER_MINOR=0
	VER_RELEASE=0
	VER_BUILD=0
endif

PACKAGE_SUFFIX=
