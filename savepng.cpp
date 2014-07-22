/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
*/
#include <stdint.h>
#include <zlib.h>
#include <png.h>
int savePNG(const char * fileName,uint32_t width,uint32_t height,void * ptr){
	//saves a 24bit png with rgb byte order
	png_byte * dat=(png_byte*)ptr;//convert to uint8_t
	FILE * fp=fopen(fileName,"wb");
	if (!fp)
		return 1;
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)0,0,0);
	if (!png_ptr){
		fclose(fp);
		return 1;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		return 1;
	}
	if (setjmp(png_jmpbuf(png_ptr))){
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return 1;
	}
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height,8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);//must be called before other png_set_*() functions
	png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);
	uint32_t y;
	png_set_user_limits(png_ptr, width, height);
	png_write_info(png_ptr, info_ptr);
	for (y=0;y<height;++y)
		png_write_row(png_ptr, &dat[(y*width*3)]);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);//done with file
	return 0;//will return 0 on success non-zero in error
}
