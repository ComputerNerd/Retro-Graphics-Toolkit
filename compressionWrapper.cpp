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
   Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#include "includes.h"
#include "gui.h"
#include "errorMsg.h"
#include "nemesis.h"
#include "kosinski.h"
#include "enigma.h"
static const char*Uncomp="Uncompressed";
static const char*NemTxt="Nemesis";
static const char*KosTxt="Kosinski";
static const char*EngTxt="Enigma";
static const char*const TypeTab[]={Uncomp,NemTxt,KosTxt,EngTxt};
const char*typeToText(int type){
	return TypeTab[type];
}
int compressionAsk(void){
	return MenuPopup("Compression?","Select a compression algorithm or use uncompressed",4,TypeTab[0],TypeTab[1],TypeTab[2],TypeTab[3]);
}
static std::string decodeNemKos(const char * filename,size_t &fileSize,bool kos){
	std::stringstream outDecomp;
	FILE * fi=fopen(filename,"rb");
	fileSize=fseek(fi,0,SEEK_END);
	fileSize=ftell(fi);
	rewind(fi);
	uint8_t * datcmp=(uint8_t *)malloc(fileSize);
	fread(datcmp,1,fileSize,fi);
	fclose(fi);
	std::string input;
	input.assign((const char *)datcmp,fileSize);
	free(datcmp);
	std::istringstream iss(input);
	if (kos){
		kosinski decomp;
		decomp.decode(iss,outDecomp);
	}else{
		nemesis decomp;
		decomp.decode(iss,outDecomp);
	}
	printf("Decompressed to %d bytes\n",fileSize);
	fileSize=outDecomp.str().length();
	return outDecomp.str();
}
static std::string decodeEnigma(const char * filename,size_t &fileSize){
	//This data must be freed that it returns
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
	return outDecomp.str();
}
std::string decodeTypeStr(const char * filename,size_t &filesize,int type){
	switch(type){
		case 1:
			return decodeNemKos(filename,filesize,false);
		break;
		case 2:
			return decodeNemKos(filename,filesize,true);
		break;
		case 3:
			return decodeEnigma(filename,filesize);
		break;
		default:
			show_default_error
	}
}
void*decodeType(const char * filename,size_t &filesize,int type){
	std::string output=decodeTypeStr(filename,filesize,type);
	char * Dat=(char *)malloc(filesize);
	output.copy(Dat, filesize);
	return Dat;
}
static void*encodeNemKos(void*in,size_t n,size_t&outSize,bool kos){
	std::string input;
	input.assign((const char *)in,n);
	std::istringstream iss(input);
	std::ostringstream outfun;
	if(kos){
		kosinski comp;
		comp.encode(iss,outfun);
	}else{
		nemesis comp;
		comp.encode(iss,outfun);
	}
	outSize=outfun.str().length();
	uint8_t*compdat=(uint8_t*)malloc(outSize);
	if(!compdat)
		show_malloc_error(outSize)
	std::string output=outfun.str();
	output.copy((char *)compdat,outSize);
	printf("Compressed to %d from %d\n",outSize,n);
	return (void*)compdat;
}
void*encodeEng(void*in,size_t n,size_t&outSize){
	std::string input,output;
	std::ostringstream outcomp;
	enigma ecomp;
	input.assign((const char*)in,n);
	std::stringstream iss(input);
	ecomp.encode(iss,outcomp);
	output=outcomp.str();
	outSize=outcomp.str().length();
	void*outdat=malloc(outSize);
	output.copy((char*)outdat,outSize);
	printf("Compressed to %d from %d\n",outSize,n);
	return outdat;
}
void*encodeType(void*in,size_t n,size_t&outSize,int type){
	switch(type){
		case 1:
			return encodeNemKos(in,n,outSize,false);
		break;
		case 2:
			return encodeNemKos(in,n,outSize,true);
		break;
		case 3:
			return encodeEng(in,n,outSize);
		break;
		default:
			show_default_error
	}
	return 0;
}
