CPPFLAGS=
CC=gcc
CXX=g++

include Makefile.common

CFLAGS += -march=native -ggdb3 -pg -flto=8 -fuse-linker-plugin -O3 -pipe -march=native
CXXFLAGS := $(CFLAGS) -fno-rtti -std=gnu++14
LDFLAGS := -flto=8 -O3 -march=native -fuse-linker-plugin -fno-rtti -std=gnu++14 -L/usr/lib/fltk/ -lfltk_images -lfltk -lpng -ljpeg -lXft -lXext -lXinerama -lX11 -lz -lluajit-5.1 -ldl

EXECUTABLE := RetroGraphicsToolkit.profile

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
