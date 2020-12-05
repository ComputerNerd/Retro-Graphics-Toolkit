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
#include <zlib.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
bool decompressFromFile(void * ptr, int size, FILE * fi) {
	/* allocate inflate state */
	uint32_t cSize;
	std::fread(&cSize, 1, sizeof(uint32_t), fi);
	void * cDat = std::malloc(cSize);

	if (!cDat)
		return false;

	std::fread(cDat, 1, cSize, fi);
	std::printf("Compressed size %d uncompressed size %d\n", cSize, size);
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	int ret = inflateInit(&strm);

	if (ret != Z_OK) {
		free(cDat);
		return false;
	}

	strm.avail_in = cSize;
	strm.avail_out = size;
	strm.next_in = (Bytef*)cDat;
	strm.next_out = (Bytef*)ptr;
	ret = inflate(&strm, Z_FINISH);
	bool Retf = ret == Z_STREAM_END;

	if (!Retf)
		printf("ret %d\n", ret);

	free(cDat);
	inflateEnd(&strm);
	return Retf;
}
bool compressToFile(void * ptr, int size, FILE * fo) {
	/* allocate deflate state */
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	int ret = deflateInit(&strm, Z_BEST_COMPRESSION);

	if (ret != Z_OK)
		return false;

	int maxS = deflateBound(&strm, size);
	void * outb = malloc(maxS);

	if (!outb) {
		deflateEnd(&strm);
		return false;
	}

	strm.avail_in = size;
	strm.next_in = (Bytef*)ptr;
	strm.avail_out = maxS;
	strm.next_out = (Bytef*)outb;
	ret = deflate(&strm, Z_FINISH);

	switch (ret) {
		case Z_STREAM_END:
			printf("Compressed to %lu from %d could have maxed out at %d\n", strm.total_out, size, maxS);
			{	uint32_t outS = strm.total_out;
				fwrite(&outS, 1, sizeof(uint32_t), fo);
				fwrite(outb, 1, strm.total_out, fo);
				free(outb);
				deflateEnd(&strm);
			}
			break;

		case Z_OK:
			puts("Should not happen");
			free(outb);
			deflateEnd(&strm);
			return false;
			break;

		default:
			puts("Zlib error");
			free(outb);
			deflateEnd(&strm);
			return false;
	}

	return true;
}
