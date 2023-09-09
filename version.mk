VER_MAJOR=111
VER_MINOR=222
VER_RELEASE=333
VER_BUILD=444

# Set version to 0.0.0.0 when compiling with DEBUG=1, to distinguish it from normal releases. DEBUG=1 makes the executable filesize huge when packing it with debug symbols.
ifeq ($(DEBUG), 0)
	VER_MAJOR=555
	VER_MINOR=666
	VER_RELEASE=777
	VER_BUILD=888
endif

PACKAGE_SUFFIX=
