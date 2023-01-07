lib.name = pdlua

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

cflags = ${luaflags}

pdlua.class.sources := pdlua.c $(luasrc)
pdlua.class.ldlibs := $(lualibs)

datafiles = pd.lua $(wildcard hello*.* pdlua*-help.pd)
datadirs = examples

include Makefile.pdlibbuilder

install: installtut

installtut:
	mkdir -p "${installpath}/tutorial"
	cp -r ./tutorial/*.pdf ./tutorial/examples "${installpath}/tutorial"
