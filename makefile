CPPFLAGS=-DLUA_USE_LINUX -DLUA_COMPAT_ALL
CC=gcc
CPP=g++
#gentoo fix needs include directory set to /usr/include/fltk-1 if you are using a different distro then this may not apply to you
CFLAGS=-march=native -flto -fuse-linker-plugin -I/usr/include/fltk-1/ -s -Ikens/ -c -Wall -Wextra -Wdouble-promotion -O3 -pipe -march=native -fomit-frame-pointer
CXXFLAGS= $(CFLAGS) -fno-rtti -std=gnu++11
LDFLAGS=-flto -O3 -march=native -fuse-linker-plugin -s -fno-rtti -std=gnu++11 -L/usr/lib/fltk-1/ -lfltk_images -lfltk -lpng -ljpeg -lXft -lXext -lXinerama -lX11 -lz -s
LUAOBJECTS=lua/lapi.o lua/lcode.o lua/lctype.o lua/ldebug.o lua/ldo.o lua/ldump.o lua/lfunc.o lua/lgc.o lua/llex.o lua/lmem.o lua/lobject.o lua/lopcodes.o lua/lparser.o lua/lstate.o lua/lstring.o lua/ltable.o lua/ltm.o lua/lundump.o lua/lvm.o lua/lzio.o lua/lauxlib.o lua/lbaselib.o lua/lbitlib.o lua/lcorolib.o lua/ldblib.o lua/liolib.o lua/lmathlib.o lua/loslib.o lua/lstrlib.o lua/ltablib.o lua/loadlib.o lua/linit.o
OBJECTS=$(LUAOBJECTS) project.o main.o callbacks_palette.o callback_tiles.o class_global.o global.o quant.o tilemap.o color_convert.o \
	errorMsg.o class_palette.o dither.o \
	class_tiles.o kens/nemesis.o kens/enigma.o kens/kosinski.o spatial_color_quant.o NEUQUANT.o \
	classtilemap.o palette.o zlibwrapper.o color_compare.o windowinit.o tiles_io.o savepng.o \
	callback_project.o callback_tilemap.o callback_gui.o classChunks.o compressionWrapper.o callback_chunk.o gui.o \
	wu.o system.o filemisc.o classSprite.o classSprites.o callbacksprites.o undo.o undocallback.o image.o \
	classlevel.o runlua.o
EXECUTABLE=RetroGraphicsToolkit

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
.c.o:
	$(CC) $(CFLAGS) $< -o $@
.cc.o:
	$(CXX) $(CXXFLAGS) $< -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@
clean:
	rm $(OBJECTS) $(EXECUTABLE)
