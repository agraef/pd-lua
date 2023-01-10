lib.name = pdlua

pdlua_version := $(shell git describe --tags 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g')

luasrc = $(wildcard lua/onelua.c)

ifeq ($(luasrc),)
# compile with installed liblua
$(info ++++ NOTE: using installed lua)
luaflags = $(shell pkg-config --cflags lua)
lualibs = $(shell pkg-config --libs lua)
else
# compile with Lua submodule
$(info ++++ NOTE: using lua submodule)
luaflags = -DMAKE_LIB -Ilua
define forDarwin
luaflags += -DLUA_USE_MACOSX
endef
define forLinux
luaflags += -DLUA_USE_LINUX
endef
define forWindows
luaflags += -DLUA_USE_WINDOWS
endef
endif

cflags = ${luaflags} -DPDLUA_VERSION="$(pdlua_version)"

pdlua.class.sources := pdlua.c $(luasrc)
pdlua.class.ldlibs := $(lualibs)

datafiles = pd.lua $(wildcard pdlua*-help.pd)

include Makefile.pdlibbuilder

install: installplus

installplus:
	cp -r ./pdlua/ "${installpath}"/pdlua
