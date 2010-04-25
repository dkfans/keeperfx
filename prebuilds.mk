ifneq (,$(findstring Windows,$(OS)))
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-win.zip
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  PERESEC_DOWNLOAD=https://github.com/dkfans/peresec/releases/download/1.1.0/peresec-1_1_0_16-devel-lin.tar.gz
else
  PERESEC_DOWNLOAD=no_prebuild_available_for_your_os
endif
endif
PERESEC_PACKAGE=$(notdir $(PERESEC_DOWNLOAD))
