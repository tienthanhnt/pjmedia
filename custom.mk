CROSS_COMPILE:=3

ARMV7L:=1
LINUX_X86:=2
LINUX_X86_64:=3
MINGW_X86:=4
MACOS_X86_64:=5
ifeq ($(CROSS_COMPILE),$(ARMV7L))
	CROSS_TOOL:=arm-linux-gnueabihf-gcc
	LIBS_DIR:=../libs/linux-armv7l
endif
ifeq ($(CROSS_COMPILE),$(LINUX_X86))
	CROSS_TOOL:=gcc
	LIBS_DIR:=../libs/linux-i686
endif
ifeq ($(CROSS_COMPILE),$(LINUX_X86_64))
	CROSS_TOOL:=gcc
	LIBS_DIR:=../libs/linux-x86_64
endif
ifeq ($(CROSS_COMPILE),$(MINGW_X86))
	CROSS_TOOL:=gcc
	LIBS_DIR:=../libs/mingw32-i586
endif
ifeq ($(CROSS_COMPILE),$(MACOS_X86_64))
	CROSS_TOOL:=gcc
	LIBS_DIR:=../libs/darwin-x86_64
endif
PKG_CFG_PATH=$(PWD)/$(LIBS_DIR)/lib/pkgconfig
LIBS:=$(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --libs libpjproject)
LIBS+=$(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --libs json-c) -lcrypto
#LIBS+=$(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --libs sqlite3)

CFLAGS:=$(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --cflags libpjproject)
CFLAGS+=$(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --cflags json-c)
#CFLAGS+=$(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --cflags sqlite3)
