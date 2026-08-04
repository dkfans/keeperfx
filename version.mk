VER_MAJOR=1
VER_MINOR=4
VER_RELEASE=0
VER_BUILD=0

# Appended to the version string the game reports and writes into its log.
#
# The default marks a build as not coming from the release workflows: they each
# pass PACKAGE_SUFFIX on the command line, which wins over this, so an official
# package is unaffected while anything built from a fork or a working copy says
# so on its own. That keeps a screenshot or a bug report from being mistaken for
# an official release.
PACKAGE_SUFFIX=Unofficial