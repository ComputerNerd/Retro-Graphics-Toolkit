CC=gcc
CPP=g++
#gentoo fix needs include directory set to /usr/include/fltk-1 if you are using a different distro then this may not apply to you
CFLAGS=-march=native -flto -fuse-linker-plugin -I/usr/include/fltk-1/ -s -Ikens/ -c -Wall -Wextra -Wdouble-promotion -O3 -pipe -march=native -fomit-frame-pointer
CXXFLAGS= $(CFLAGS) -fno-rtti -std=gnu++11
LDFLAGS=-flto -O3 -march=native -fuse-linker-plugin -s -fno-rtti -std=gnu++11 -L/usr/lib/fltk-1/ -lfltk_images -lfltk -lpng -ljpeg -lXft -lXext -lXinerama -lX11 -lz -s
OBJECTS=project.o main.o callbacks_palette.o callback_tiles.o class_global.o global.o quant.o tilemap.o color_convert.o \
	errorMsg.o class_palette.o dither.o \
	class_tiles.o kens/nemesis.o kens/enigma.o kens/kosinski.o spatial_color_quant.o NEUQUANT.o \
	classtilemap.o palette.o zlibwrapper.o color_compare.o windowinit.o tiles_io.o savepng.o \
	callback_project.o callback_tilemap.o callback_gui.o classChunks.o compressionWrapper.o callback_chunk.o gui.o \
	wu.o system.o filemisc.o classSprite.o classSprites.o callbacksprites.o undo.o undocallback.o image.o \
	classlevel.o
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
