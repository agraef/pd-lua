## Pd library template version 1.0.14
# For instructions on how to use this template, see:
#  http://puredata.info/docs/developer/MakefileTemplate
LIBRARY_NAME = pdlua

# add your .c source files, one object per file, to the SOURCES
# variable, help files will be included automatically, and for GUI
# objects, the matching .tcl file too
SOURCES = pdlua.c

## the source directory: either 'src/' (if it exists) or '.'
PDLUA_SRCPATH=$(firstword $(patsubst src/%,src,$(wildcard src/*)) .)

# list all pd objects (i.e. myobject.pd) files here, and their helpfiles will
# be included automatically
PDOBJECTS = $(PDLUA_SRCPATH)/pd.lua $(PDLUA_SRCPATH)/hello.lua $(PDLUA_SRCPATH)/hello.pd_lua $(PDLUA_SRCPATH)/hello.pd_luax

# example patches and related files, in the 'examples' subfolder
EXAMPLES = $(patsubst examples/%,%,$(wildcard examples/*))

# manuals and related files, in the 'manual' subfolder
MANUAL = $(patsubst doc/%,%,$(wildcard doc/*))


# if you want to include any other files in the source and binary tarballs,
# list them here.  This can be anything from header files, test patches,
# documentation, etc.  README.txt and LICENSE.txt are required and therefore
# automatically included
EXTRA_DIST =

# unit tests and related files here, in the 'unittests' subfolder
UNITTESTS = 

VPATH=$(PDLUA_SRCPATH)
HELPPATCHES= $(PDLUA_SRCPATH)/pdlua-help.pd \
	$(PDLUA_SRCPATH)/pdluax-help.pd     \
	$(PDLUA_SRCPATH)/hello-help.pd

#------------------------------------------------------------------------------#
#
# things you might need to edit if you are using other C libraries
#
#------------------------------------------------------------------------------#

LIBRARY_META=$(PDLUA_SRCPATH)/$(LIBRARY_NAME)-meta.pd

ALL_CFLAGS = -I"$(PD_INCLUDE)"
ALL_LDFLAGS =  
SHARED_LDFLAGS =
ALL_LIBS = 


#------------------------------------------------------------------------------#
#
# you shouldn't need to edit anything below here, if we did it right :)
#
#------------------------------------------------------------------------------#

# Target platform (OSX/macOS only): On Mojave (10.14 with Xcode 10) this needs
# to be at least 10.9, which is the default now. With older Xcode versions you
# can try earlier versions (>= 10.4) if you need to compile for legacy OSX
# versions.
macos_target = 10.9

# these can be set from outside without (usually) breaking the build
CFLAGS = -Wall -W -g
LDFLAGS =
LIBS =

LUA_CFLAGS = -I/usr/include/lua
LUA_LIBS   = -llua

# get library version from meta file
LIBRARY_VERSION = $(shell sed -n 's|^\#X text [0-9][0-9]* [0-9][0-9]* VERSION \(.*\);|\1|p' $(LIBRARY_META))

ALL_CFLAGS += -DPD -DVERSION='"$(LIBRARY_VERSION)"'

PD_INCLUDE = $(PD_PATH)/include/pd
# where to install the library, overridden below depending on platform
prefix = /usr/local
libdir = $(prefix)/lib
pkglibdir = $(libdir)/pd-externals
objectsdir = $(pkglibdir)

INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -p -m 644
INSTALL_DATA = $(INSTALL) -p -m 644
INSTALL_DIR     = $(INSTALL) -p -m 755 -d

ALLSOURCES_ := $(SOURCES) $(SOURCES_android) $(SOURCES_cygwin) $(SOURCES_macosx) \
	         $(SOURCES_iphoneos) $(SOURCES_linux) $(SOURCES_windows)
ALLSOURCES=$(ALLSOURCES_:%=$(PDLUA_SRCPATH)/%)
DISTDIR=$(LIBRARY_NAME)-$(LIBRARY_VERSION)
ORIGDIR=pd-$(LIBRARY_NAME:~=)_$(LIBRARY_VERSION)

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  CPU := $(shell uname -p)
  ifeq ($(CPU),arm) # iPhone/iPod Touch
    SOURCES += $(SOURCES_iphoneos)
    EXTENSION = pd_darwin
    SHARED_EXTENSION = dylib
    OS = iphoneos
    PD_PATH = /Applications/Pd-extended.app/Contents/Resources
    IPHONE_BASE=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin
    CC=$(IPHONE_BASE)/gcc
    CPP=$(IPHONE_BASE)/cpp
    CXX=$(IPHONE_BASE)/g++
    ISYSROOT = -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk
    IPHONE_CFLAGS = -miphoneos-version-min=3.0 $(ISYSROOT) -arch armv6
    OPT_CFLAGS = -fast -funroll-loops -fomit-frame-pointer
    ALL_CFLAGS := $(IPHONE_CFLAGS) $(ALL_CFLAGS)
    ALL_LDFLAGS += -arch armv6 -bundle -undefined dynamic_lookup $(ISYSROOT)
    SHARED_LDFLAGS += -arch armv6 -dynamiclib -undefined dynamic_lookup $(ISYSROOT)
    ALL_LIBS += -lc $(LIBS_iphoneos)
    STRIP = strip -x
    DISTBINDIR=$(DISTDIR)-$(OS)
  else # Mac OS X
    SOURCES += $(SOURCES_macosx)
    EXTENSION = pd_darwin
    SHARED_EXTENSION = dylib
    OS = macosx
    PD_PATH = $(lastword $(wildcard /Applications/Pd-0*.app/Contents/Resources))
    PD_INCLUDE = $(PD_PATH)/src
    OPT_CFLAGS = -ftree-vectorize
# uncomment this to build fat binaries
    #FAT_BINARIES = 1
    ifdef FAT_BINARIES
# build universal 32-bit on 10.4 and 32/64 on newer
    ifeq ($(shell uname -r | sed 's|\([0-9][0-9]*\)\.[0-9][0-9]*\.[0-9][0-9]*|\1|'), 8)
      FAT_FLAGS = -arch ppc -arch i386 -mmacosx-version-min=$(macos_target)
    else
      SOURCES += $(SOURCES_iphoneos)
# Starting with Xcode 4.0, the PowerPC compiler is not installed by default
      ifeq ($(wildcard /usr/llvm-gcc-4.2/libexec/gcc/powerpc*), )
        FAT_FLAGS = -arch i386 -arch x86_64 -mmacosx-version-min=$(macos_target)
      else
        FAT_FLAGS = -arch ppc -arch i386 -arch x86_64 -mmacosx-version-min=$(macos_target)
      endif
    endif
    endif
    ALL_CFLAGS += $(FAT_FLAGS) -fPIC -I/sw/include -I/opt/local/include
    # NOTE: To link statically against Lua using MacPorts:
    #LUA_LIBS = /opt/local/lib/liblua.a
    # if the 'pd' binary exists, check the linking against it to aid with stripping
    BUNDLE_LOADER = $(shell test ! -e $(PD_PATH)/bin/pd || echo -bundle_loader $(PD_PATH)/bin/pd)
    ALL_LDFLAGS += $(FAT_FLAGS) -headerpad_max_install_names -bundle $(BUNDLE_LOADER) \
	-undefined dynamic_lookup -L/sw/lib -L/opt/local/lib
    SHARED_LDFLAGS += $(FAT_FLAGS) -dynamiclib -undefined dynamic_lookup \
	-install_name @loader_path/$(SHARED_LIB) -compatibility_version 1 -current_version 1.0
    ALL_LIBS += -lc $(LIBS_macosx)
    STRIP = strip -x
    DISTBINDIR=$(DISTDIR)-$(OS)
# install into ~/Library/Pd on Mac OS X since /usr/local isn't used much
    pkglibdir=$(HOME)/Library/Pd
  endif
endif
# Tho Android uses Linux, we use this fake uname to provide an easy way to
# setup all this things needed to cross-compile for Android using the NDK
ifeq ($(UNAME),ANDROID)
  CPU := arm
  SOURCES += $(SOURCES_android)
  EXTENSION = so
  SHARED_EXTENSION = so
  OS = android
  PD_PATH = /usr
  NDK_BASE := /opt/android-ndk
  NDK_PLATFORM_LEVEL ?= 5
  NDK_ABI=arm
  NDK_COMPILER_VERSION = 4.6
  NDK_SYSROOT=$(NDK_BASE)/platforms/android-$(NDK_PLATFORM_LEVEL)/arch-$(NDK_ABI)
  NDK_UNAME:=$(shell uname -s | tr '[A-Z]' '[a-z]')
  ifeq ($(NDK_ABI),x86)
    HOST = i686-linux-android
    NDK_TOOLCHAIN = $(NDK_ABI)-$(NDK_COMPILER_VERSION)
  else
    HOST = $(NDK_ABI)-linux-androideabi
    NDK_TOOLCHAIN = $(HOST)-$(NDK_COMPILER_VERSION)
  endif
  NDK_TOOLCHAIN_BASE=$(NDK_BASE)/toolchains/$(NDK_TOOLCHAIN)/prebuilt/$(NDK_UNAME)-$(NDK_PROCESSOR)
  CC := $(NDK_TOOLCHAIN_BASE)/bin/$(HOST)-gcc --sysroot=$(NDK_SYSROOT)
  LD := $(NDK_TOOLCHAIN_BASE)/bin/$(HOST)-ld
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  CFLAGS += 
  LDFLAGS += -rdynamic -shared
  SHARED_LDFLAGS += -Wl,-soname,$(SHARED_LIB) -shared
  LIBS += -lc $(LIBS_android)
  STRIP := $(NDK_TOOLCHAIN_BASE)/bin/$(HOST)-strip) \
	--strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq ($(UNAME),Linux)
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  SHARED_EXTENSION = so
  OS = linux
  PD_PATH = /usr
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -rdynamic -shared -fPIC -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
  SHARED_LDFLAGS += -Wl,-soname,$(SHARED_LIB) -shared
  ALL_LIBS += -lc $(LIBS_linux)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq ($(UNAME),GNU)
  # GNU/Hurd, should work like GNU/Linux for basically all externals
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  SHARED_EXTENSION = so
  OS = linux
  PD_PATH = /usr
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -rdynamic -shared -fPIC -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
  SHARED_LDFLAGS += -shared -Wl,-soname,$(SHARED_LIB)
  ALL_LIBS += -lc $(LIBS_linux)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq ($(UNAME),GNU/kFreeBSD)
  # Debian GNU/kFreeBSD, should work like GNU/Linux for basically all externals
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  SHARED_EXTENSION = so
  OS = linux
  PD_PATH = /usr
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -rdynamic -shared -fPIC -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
  SHARED_LDFLAGS += -shared -Wl,-soname,$(SHARED_LIB)
  ALL_LIBS += -lc $(LIBS_linux)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq (CYGWIN,$(findstring CYGWIN,$(UNAME)))
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_cygwin)
  EXTENSION = dll
  SHARED_EXTENSION = dll
  OS = cygwin
  PD_PATH = $(shell cygpath $$PROGRAMFILES)/pd
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += 
  ALL_LDFLAGS += -rdynamic -shared -L"$(PD_PATH)/src" -L"$(PD_PATH)/bin"
  SHARED_LDFLAGS += -shared -Wl,-soname,$(SHARED_LIB)
  ALL_LIBS += -lc -lpd $(LIBS_cygwin)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_windows)
  EXTENSION = dll
  SHARED_EXTENSION = dll
  OS = windows
  # This assumes an installation of Lua 5.3 from source,
  # cf. https://www.lua.org/download.html
  LUA_CFLAGS = -I/usr/local/include
  LUA_LIBS   = -L/usr/local/lib -llua
  # This assumes that Pd is installed somewhere under "c:\Program*\Pd"
  # (usually "c:\Program Files (x86)\Pd"), the dll is in the "bin" and the
  # include files in the "src" subdir, as it is in the latest incarnations of
  # MSP's installer. (NOTE: You can replace "Pd" with "Purr*Data" if you want
  # to link against jwilke's Pd-l2ork 2.x a.k.a. Purr Data instead. The
  # version compiled against vanilla will also work with Purr Data, though.)
  PD_PATH = $(wildcard /c/Program*/Pd)
  PD_INCLUDE = $(PD_PATH)/src
  PD_LIB = $(PD_PATH)/bin
  # Install into the extra subdir of the program directory. Note that at least
  # on the latest Windows versions you can't install directly there
  # (permissions issue) so you'll have to install into a staging directory
  # first (using DESTDIR) and then manually copy files over.
  pkglibdir = "$(PD_PATH)/extra"
  # MinGW doesn't seem to include cc so force gcc
  CC=gcc
  OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -mms-bitfields
  ALL_LDFLAGS += -s -shared -Wl,--enable-auto-import
  SHARED_LDFLAGS += -shared
  ALL_LIBS += "$(PD_LIB)/pd.dll" \
	-lwsock32 -lkernel32 -luser32 -lgdi32 $(LIBS_windows)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)
endif

# in case somebody manually set the HELPPATCHES above
HELPPATCHES ?= $(SOURCES:.c=-help.pd) $(PDOBJECTS:.pd=-help.pd)

ALL_CFLAGS := $(CPPFLAGS) $(ALL_CFLAGS) $(CFLAGS) $(LUA_CFLAGS) $(OPT_CFLAGS)
ALL_LDFLAGS := $(LDFLAGS) $(LUA_LDFLAGS) $(ALL_LDFLAGS)
ALL_LIBS := $(LIBS) $(LUA_LIBS) $(ALL_LIBS)

SHARED_SOURCE ?= $(wildcard lib$(LIBRARY_NAME).c)
SHARED_HEADER ?= $(shell test ! -e $(LIBRARY_NAME).h || echo $(LIBRARY_NAME).h)
SHARED_LIB ?= $(SHARED_SOURCE:.c=.$(SHARED_EXTENSION))
SHARED_TCL_LIB = $(wildcard lib$(LIBRARY_NAME).tcl)

.PHONY = install libdir_install single_install install-doc install-examples install-manual install-unittests clean distclean dist etags $(LIBRARY_NAME) TAGS_PD TAGS_SOURCES

all: $(SOURCES:.c=.$(EXTENSION)) $(SHARED_LIB)

%.o: %.c
	$(CC) $(ALL_CFLAGS) -o $@ -c $<

%.$(EXTENSION): %.o $(SHARED_LIB)
	$(CC) $(ALL_LDFLAGS) -o $@ $^ $(ALL_LIBS)
	chmod a-x $@

# this links everything into a single binary file
$(LIBRARY_NAME): $(SOURCES:.c=.o) $(LIBRARY_NAME).o $(SHARED_SOURCE:.c=.o)
	$(CC) $(ALL_LDFLAGS) -o $@.$(EXTENSION) $^ $(ALL_LIBS)
	chmod a-x $@.$(EXTENSION)

$(SHARED_LIB): $(SHARED_SOURCE:.c=.o)
	$(CC) $(SHARED_LDFLAGS) -o $@ $^ $(ALL_LIBS)

install: libdir_install

# The meta and help files are explicitly installed to make sure they are
# actually there.  Those files are not optional, then need to be there.
libdir_install: $(SOURCES:.c=.$(EXTENSION)) $(SHARED_LIB) install-doc install-examples install-manual install-unittests
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(LIBRARY_META) \
		$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(SOURCES))" || (\
		$(INSTALL_PROGRAM) $(SOURCES:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME) && \
		$(STRIP) $(addprefix $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/,$(SOURCES:.c=.$(EXTENSION))))
	test -z "$(strip $(SHARED_LIB))" || \
		$(INSTALL_DATA) $(SHARED_LIB) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(wildcard $(SOURCES:.c=.tcl)))" || \
		$(INSTALL_DATA) $(wildcard $(SOURCES:.c=.tcl)) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(PDOBJECTS))" || \
		$(INSTALL_DATA) $(PDOBJECTS) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(SHARED_TCL_LIB))" || \
		$(INSTALL_DATA) $(SHARED_TCL_LIB) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)

# install library linked as single binary
single_install: $(LIBRARY_NAME) install-doc install-examples install-manual install-unittests
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_PROGRAM) $(LIBRARY_NAME).$(EXTENSION) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(STRIP) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/$(LIBRARY_NAME).$(EXTENSION)

install-doc:
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(SOURCES) $(PDOBJECTS))" || \
		$(INSTALL_DATA) $(HELPPATCHES) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) README $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/README.txt
	$(INSTALL_DATA) COPYING $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/LICENSE.txt

install-examples:
	test -z "$(strip $(EXAMPLES))" || \
		$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/examples && \
		for file in $(EXAMPLES); do \
			$(INSTALL_DATA) examples/$$file $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/examples; \
		done

install-manual:
	test -z "$(strip $(MANUAL))" || \
		$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/doc && \
		for file in $(MANUAL); do \
			$(INSTALL_DATA) doc/$$file $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/doc; \
		done

install-unittests:
	test -z "$(strip $(UNITTESTS))" || \
		$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/unittests && \
		for file in $(UNITTESTS); do \
			$(INSTALL_DATA) unittests/$$file $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/unittests; \
		done

clean:
	-rm -f -- $(SOURCES:.c=.o) $(SOURCES_LIB:.c=.o) $(SHARED_SOURCE:.c=.o)
	-rm -f -- $(SOURCES:.c=.$(EXTENSION))
	-rm -f -- $(LIBRARY_NAME).o
	-rm -f -- $(LIBRARY_NAME).$(EXTENSION)
	-rm -f -- $(SHARED_LIB)

distclean: clean
	-rm -f -- $(DISTBINDIR).tar.gz
	-rm -rf -- $(DISTBINDIR)
	-rm -f -- $(DISTDIR).tar.gz
	-rm -rf -- $(DISTDIR)
	-rm -f -- $(ORIGDIR).tar.gz
	-rm -rf -- $(ORIGDIR)


$(DISTBINDIR):
	$(INSTALL_DIR) $(DISTBINDIR)

libdir: all $(DISTBINDIR)
	$(INSTALL_DATA) $(LIBRARY_META)  $(DISTBINDIR)
	$(INSTALL_DATA) $(ALLSOURCES) $(SHARED_SOURCE) $(SHARED_HEADER) $(DISTBINDIR)
	$(INSTALL_DATA) $(HELPPATCHES) $(DISTBINDIR)
	test -z "$(strip $(EXTRA_DIST))" || \
		$(INSTALL_DATA) $(EXTRA_DIST)    $(DISTBINDIR)
#	tar --exclude-vcs -czpf $(DISTBINDIR).tar.gz $(DISTBINDIR)

$(DISTDIR):
	$(INSTALL_DIR) $(DISTDIR)

$(ORIGDIR):
	$(INSTALL_DIR) $(ORIGDIR)

dist: $(DISTDIR)
	$(INSTALL_DATA) Makefile  $(DISTDIR)
	$(INSTALL_DATA) README $(DISTDIR)
	$(INSTALL_DATA) COPYING $(DISTDIR)
	$(INSTALL_DATA) $(LIBRARY_META) $(DISTDIR)
	test -z "$(strip $(ALLSOURCES))" || \
		$(INSTALL_DATA) $(ALLSOURCES)  $(DISTDIR)
	test -z "$(strip $(wildcard $(ALLSOURCES:.c=.tcl)))" || \
		$(INSTALL_DATA) $(wildcard $(ALLSOURCES:.c=.tcl))  $(DISTDIR)
	test -z "$(strip $(wildcard $(LIBRARY_NAME).c))" || \
		$(INSTALL_DATA) $(LIBRARY_NAME).c  $(DISTDIR)
	test -z "$(strip $(SHARED_HEADER))" || \
		$(INSTALL_DATA) $(SHARED_HEADER)  $(DISTDIR)
	test -z "$(strip $(SHARED_SOURCE))" || \
		$(INSTALL_DATA) $(SHARED_SOURCE)  $(DISTDIR)
	test -z "$(strip $(SHARED_TCL_LIB))" || \
		$(INSTALL_DATA) $(SHARED_TCL_LIB)  $(DISTDIR)
	test -z "$(strip $(PDOBJECTS))" || \
		$(INSTALL_DATA) $(PDOBJECTS)  $(DISTDIR)
	test -z "$(strip $(HELPPATCHES))" || \
		$(INSTALL_DATA) $(HELPPATCHES) $(DISTDIR)
	test -z "$(strip $(EXTRA_DIST))" || \
		$(INSTALL_DATA) $(EXTRA_DIST)    $(DISTDIR)
	test -z "$(strip $(EXAMPLES))" || \
		$(INSTALL_DIR) $(DISTDIR)/examples && \
		for file in $(EXAMPLES); do \
			$(INSTALL_DATA) examples/$$file $(DISTDIR)/examples; \
		done
	test -z "$(strip $(MANUAL))" || \
		$(INSTALL_DIR) $(DISTDIR)/doc && \
		for file in $(MANUAL); do \
			$(INSTALL_DATA) doc/$$file $(DISTDIR)/doc; \
		done
	test -z "$(strip $(UNITTESTS))" || \
		$(INSTALL_DIR) $(DISTDIR)/unittests && \
		for file in $(UNITTESTS); do \
			$(INSTALL_DATA) unittests/$$file $(DISTDIR)/unittests; \
		done
	tar --exclude-vcs -czpf $(DISTDIR).tar.gz $(DISTDIR)

# make a Debian source package
dpkg-source:
	debclean
	make distclean dist
	mv $(DISTDIR) $(ORIGDIR)
	tar --exclude-vcs -czpf ../$(ORIGDIR).orig.tar.gz $(ORIGDIR)
	rm -f -- $(DISTDIR).tar.gz
	rm -rf -- $(DISTDIR) $(ORIGDIR)
	cd .. && dpkg-source -b $(LIBRARY_NAME)

etags: TAGS

TAGS_PD: $(wildcard $(PD_INCLUDE)/*.h)
	etags $^
TAGS_SOURCES: $(SOURCES) $(SHARED_SOURCE) $(SHARED_HEADER)
	etags -a $^

TAGS: TAGS_PD TAGS_SOURCES
	etags -a *.h
	etags -a --language=none --regex="/proc[ \t]+\([^ \t]+\)/\1/" *.tcl

showsetup:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIBS: $(LIBS)"
	@echo "ALL_CFLAGS: $(ALL_CFLAGS)"
	@echo "ALL_LDFLAGS: $(ALL_LDFLAGS)"
	@echo "ALL_LIBS: $(ALL_LIBS)"
	@echo "PD_INCLUDE: $(PD_INCLUDE)"
	@echo "PD_PATH: $(PD_PATH)"
	@echo "objectsdir: $(objectsdir)"
	@echo "LIBRARY_NAME: $(LIBRARY_NAME)"
	@echo "LIBRARY_VERSION: $(LIBRARY_VERSION)"
	@echo "SOURCES: $(SOURCES)"
	@echo "SHARED_HEADER: $(SHARED_HEADER)"
	@echo "SHARED_SOURCE: $(SHARED_SOURCE)"
	@echo "SHARED_LIB: $(SHARED_LIB)"
	@echo "SHARED_TCL_LIB: $(SHARED_TCL_LIB)"
	@echo "PDOBJECTS: $(PDOBJECTS)"
	@echo "ALLSOURCES: $(ALLSOURCES)"
	@echo "ALLSOURCES TCL: $(wildcard $(ALLSOURCES:.c=.tcl))"
	@echo "UNAME: $(UNAME)"
	@echo "CPU: $(CPU)"
	@echo "OS: $(OS)"
	@echo "pkglibdir: $(pkglibdir)"
	@echo "DISTDIR: $(DISTDIR)"
	@echo "ORIGDIR: $(ORIGDIR)"
	@echo "NDK_TOOLCHAIN: $(NDK_TOOLCHAIN)"
	@echo "NDK_BASE: $(NDK_BASE)"
	@echo "NDK_SYSROOT: $(NDK_SYSROOT)"
	@echo ""
	@echo "EXAMPLES: $(EXAMPLES)"
	@echo "MANUAL: $(MANUAL)"
	@echo "EXTRA: $(EXTRA_DIST)"
	@echo "foo: $(PDLUA_SRCPATH)"
