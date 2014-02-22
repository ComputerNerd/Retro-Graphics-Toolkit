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
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#include "includes.h"
#include "kens.h"
void * decodeEnigma(const char * filename,uint32_t &fileSize){
	//This data must be free'd that it returns
	std::ifstream file (filename, std::ios::in|std::ios::binary|std::ios::ate);
	fileSize = file.tellg();
	file.seekg (0, std::ios::beg);//return to the beginning of the file
	std::ostringstream outDecomp;
	enigma decomp;
	std::stringstream iss;
	iss << file.rdbuf();
	decomp.decode(iss,outDecomp);
	fileSize=outDecomp.str().length();
	printf("Decompressed to %d bytes\n",fileSize);
	std::string output=outDecomp.str();
	char * Dat=(char *)malloc(fileSize);
	output.copy(Dat, fileSize);
	return (void*)Dat;
}
