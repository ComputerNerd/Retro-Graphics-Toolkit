CPPFLAGS=
CC=gcc
CXX=g++

include Makefile.common

CFLAGS += -march=native -flto -fuse-linker-plugin -s -O3 -pipe -march=native -fomit-frame-pointer
CXXFLAGS := $(CFLAGS) -fno-rtti -std=gnu++14
LDFLAGS := -flto -O3 -march=native -fuse-linker-plugin -s -fno-rtti -std=gnu++14 -L/usr/lib/fltk/ -lfltk_images -lfltk -lpng -ljpeg -lXft -lXext -lXinerama -lX11 -lz -s -Llua/src -llua -ldl

EXECUTABLE := RetroGraphicsToolkit

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@
.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
