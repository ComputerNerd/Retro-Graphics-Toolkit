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
   Copyright Sega16 (or whatever you wish to call me) (2012-2018)
*/

#include <fstream>

#include <mdcomp/nemesis.hh>
#include <mdcomp/kosinski.hh>
#include <mdcomp/enigma.hh>
#include <mdcomp/comper.hh>
#include <mdcomp/saxman.hh>

#include "gui.h"
#include "errorMsg.h"
static const char*const TypeTab[] = {"Uncompressed", "Nemesis", "Kosinski", "Enigma", "Saxman", "Comper"};
const char*typeToText(int type) {
	return TypeTab[type];
}
int compressionAsk(void) {
	return MenuPopup("Compression?", "Select a compression algorithm or use uncompressed", 6, 0, TypeTab[0], TypeTab[1], TypeTab[2], TypeTab[3], TypeTab[4], TypeTab[5]);
}
std::string decodeTypeStr(const char * filename, size_t &filesize, int type) {
	std::stringstream outDecomp;
	std::ifstream ifs (filename, std::ifstream::in | std::ifstream::binary);

	switch (type) {
		case 1:
		{	nemesis decomp;
			decomp.decode(ifs, outDecomp);
		}
		break;

		case 2:
		{	kosinski decomp;
			decomp.decode(ifs, outDecomp);
		}
		break;

		case 3:
		{	enigma decomp;
			decomp.decode(ifs, outDecomp);
		}
		break;

		case 4:
		{	saxman decomp;
			decomp.decode(ifs, outDecomp);
		}
		break;

		case 5:
		{	comper decomp;
			decomp.decode(ifs, outDecomp);
		}
		break;

		default:
			show_default_error
	}

	filesize = outDecomp.str().length();
	printf("Decompressed to %d bytes\n", filesize);
	return outDecomp.str();
}
void*decodeType(const char * filename, size_t &filesize, int type) {
	std::string output = decodeTypeStr(filename, filesize, type);
	char * Dat = (char *)malloc(filesize);
	output.copy(Dat, filesize);
	return Dat;
}
void*decodeTypeRam(const uint8_t*dat, size_t inputSize, size_t &filesize, int type) {
	std::stringstream ss, outDecomp;

	for (size_t i = 0; i < inputSize; ++i)
		ss << dat[i];

	switch (type) {
		case 1:
		{	nemesis decomp;
			decomp.decode(ss, outDecomp);
		}
		break;

		case 2:
		{	kosinski decomp;
			decomp.decode(ss, outDecomp);
		}
		break;

		case 3:
		{	enigma decomp;
			decomp.decode(ss, outDecomp);
		}
		break;

		case 4:
		{	saxman decomp;
			decomp.decode(ss, outDecomp);
		}
		break;

		case 5:
		{	comper decomp;
			decomp.decode(ss, outDecomp);
		}
		break;

		default:
			show_default_error
	}

	filesize = outDecomp.str().length();
	printf("Decompressed to %d bytes\n", filesize);
	std::string out = outDecomp.str();
	void* dst = malloc(filesize);
	out.copy((char*)dst, filesize);
	return dst;
}
void*encodeType(void*in, size_t n, size_t&outSize, int type) {
	std::string input;
	input.assign((const char *)in, n);
	std::istringstream iss(input);
	std::ostringstream outfun;

	switch (type) {
		case 1:
		{	nemesis comp;
			comp.encode(iss, outfun);
		}
		break;

		case 2:
		{	kosinski comp;
			comp.encode(iss, outfun);
		}
		break;

		case 3:
		{	enigma comp;
			comp.encode(iss, outfun);
		}
		break;

		case 4:
		{	basic_saxman comp;
			comp.encode(iss, outfun, true);
			//is >> iss;
		}
		break;

		case 5:
		{	comper comp;
			comp.encode(iss, outfun);
		}
		break;
	}

	outSize = outfun.str().length();
	uint8_t*compdat = (uint8_t*)malloc(outSize);

	if (!compdat)
		show_malloc_error(outSize)
		std::string output = outfun.str();

	output.copy((char *)compdat, outSize);
	printf("Compressed to %d from %d\n", outSize, n);
	return (void*)compdat;
}
