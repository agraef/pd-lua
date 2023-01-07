lib.name = pdlua

luaflags=-DMAKE_LIB -Ilua
define forDarwin
luaflags += -DLUA_USE_MACOSX
endef
define forLinux
luaflags += -DLUA_USE_LINUX
endef
define forWindows
luaflags += -DLUA_USE_WINDOWS
endef

cflags = ${luaflags}

pdlua.class.sources := pdlua.c lua/onelua.c

datafiles = pd.lua $(wildcard hello*.* pdlua*-help.pd)
datadirs = examples

include Makefile.pdlibbuilder

install: installtut

installtut:
	mkdir -p "${installpath}/tutorial"
	cp -r ./tutorial/*.pdf ./tutorial/examples "${installpath}/tutorial"
