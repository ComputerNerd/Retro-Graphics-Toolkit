CC=g++
#to disable debug remove -ggdb and replace it wil 02
#gentoo fix needs include directory set to /usr/include/fltk-1 if other distro maybe this does not apply
CFLAGS=-march=native -I/usr/include/fltk-1/ -c -Wall -O2 -ggdb
LDFLAGS=
SOURCES=project.cpp main.cpp callbacks_palette.cpp callback_tiles.cpp class_global.cpp global.cpp quant.cpp tilemap.cpp color_convert.cpp errorMsg.cpp class_palette.cpp dither.cpp \
	class_tiles.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LINKER=-L/usr/lib/fltk-1/ -lfltk_images -lfltk -lpng -ljpeg -lXft -lXext -lXinerama -lX11
EXECUTABLE=RetroGraphicsToolkit

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKER) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm $(OBJECTS) $(EXECUTABLE)
