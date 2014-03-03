CC=gcc
CPP=g++
#to disable debug remove -ggdb and replace it wil 03
#gentoo fix needs include directory set to /usr/include/fltk-1 if you are using a different distro then this may not apply to you
CFLAGS=-march=native -flto -fuse-linker-plugin -I/usr/include/fltk-1/ -s -Ikens/ -c -Wall -Wextra -Wdouble-promotion -O3 -pipe -march=native -fomit-frame-pointer
LDFLAGS=-flto -O3 -march=native -fuse-linker-plugin -s
OBJECTS=project.o main.o callbacks_palette.o callback_tiles.o class_global.o global.o quant.o tilemap.o color_convert.o \
	errorMsg.o class_palette.o dither.o \
	class_tiles.o kens/nemesis.o kens/enigma.o kens/kosinski.o spatial_color_quant.o NEUQUANT.o \
	classtilemap.o palette.o zlibwrapper.o color_compare.o windowinit.o tiles_io.o savepng.o \
	callback_project.o callback_tilemap.o callback_gui.o classChuncks.o compressionWrapper.o callback_chunck.o gui.o \
	wu.o
LINKER=-L/usr/lib/fltk-1/ -lfltk_images -lfltk -lpng -ljpeg -lXft -lXext -lXinerama -lX11 -lz -s
EXECUTABLE=RetroGraphicsToolkit

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CPP) $(LDFLAGS) $(OBJECTS) $(LINKER) -o $@
.c.o:
	$(CC) $(CFLAGS) $< -o $@
.cpp.o:
	$(CPP) $(CFLAGS) $< -o $@
clean:
	rm $(OBJECTS) $(EXECUTABLE)
