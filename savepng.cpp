/*
	This file is part of Retro Graphics Toolkit

	Retro Graphics Toolkit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or any later version.

	Retro Graphics Toolkit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
	Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#include <stdlib.h>
#include <zlib.h>
#include <png.h>
#include "savepng.h"
int savePNG(const char * fileName, uint32_t width, uint32_t height, void * ptr, uint8_t*pal, unsigned pn) {
	//saves a 24bit png with rgb byte order
	png_byte * dat = (png_byte*)ptr; //convert to uint8_t
	FILE * fp = fopen(fileName, "wb");

	if (!fp)
		return 1;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)0, 0, 0);

	if (!png_ptr) {
		fclose(fp);
		return 1;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return 1;
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, pal ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT); //must be called before other png_set_*() functions
	png_color_struct*palTmp = 0;

	if (pal) {
		palTmp = (png_color_struct*)malloc(pn * sizeof(png_color_struct));

		for (unsigned i = 0; i < pn; ++i) {
			palTmp[i].red = pal[i * 3];
			palTmp[i].green = pal[i * 3 + 1];
			palTmp[i].blue = pal[i * 3 + 2];
		}

		png_set_PLTE(png_ptr, info_ptr, palTmp, pn);
	}

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
	uint32_t y;
	png_set_user_limits(png_ptr, width, height);
	png_write_info(png_ptr, info_ptr);

	for (y = 0; y < height; ++y)
		png_write_row(png_ptr, &dat[(y * width * (pal ? 1 : 3))]);

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);//done with file
	free(palTmp);
	return 0;//will return 0 on success non-zero in error
}
