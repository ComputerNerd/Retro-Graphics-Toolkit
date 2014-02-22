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
#include "global.h"
#include "compressionWrapper.h"
ChunckClass::ChunckClass(void){
	chuncks=(struct ChunckAttrs*)calloc(sizeof(struct ChunckAttrs),256);
	amt=1;
	wi=hi=16;//16*16=256
	useBlocks=false;
}
ChunckClass::~ChunckClass(void){
	free(chuncks);
}
void ChunckClass::setBlock(uint32_t id,uint32_t x,uint32_t y,uint32_t block){
	chuncks[(id*wi*hi)+(y*wi)+x].block=block;
	/*This contains which block/tile to use*/
}
void ChunckClass::setFlag(uint32_t id,uint32_t x,uint32_t y,uint32_t flag){
	chuncks[(id*wi*hi)+(y*wi)+x].flags=flag;
	/*!If not using blocks flags will contain the following
	bit 0 hflip
	bit 1 vflip
	bit 2 prioity
	bit 3,4 palette row
	All other bits are unsued and can be used for video game usage
	If using blocks flags will simply contain video game settings
	Here are video game settings used If using tiles instead of blocks add 3 to bit count and ignore x and y flip
	bit 0 x-flip
	bit 1 y-flip
	bit 2,3 solidity 00 means not solid, 01 means top solid, 10 means left/right/bottom solid, and 11 means all solid.
	*/
}
void ChunckClass::drawChunck(uint32_t id,int xo,int yo,int zoom){
	struct ChunckAttrs * cptr=chuncks;
	cptr+=id*wi*hi;
	int xoo=xo;
	for(uint32_t y=0;y<hi;++y){
		for(uint32_t x=0;x<wi;++x){
			if(useBlocks){
				uint32_t Ty;
				Ty=cptr->block*currentProject->tileMapC->mapSizeH;
				for(uint32_t yb=0;yb<currentProject->tileMapC->mapSizeH;++yb){
					for(uint32_t xb=0;xb<currentProject->tileMapC->mapSizeW;++xb)
						currentProject->tileC->draw_tile(xoo,yo,currentProject->tileMapC->get_tile(xb,Ty),zoom,currentProject->tileMapC->get_palette_map(xb,Ty),currentProject->tileMapC->get_hflip(xb,Ty),currentProject->tileMapC->get_vflip(xb,Ty));
					++Ty;
				}
			}else
				currentProject->tileC->draw_tile(xoo,yo,cptr->block,zoom,(cptr->flags>>3)&3,cptr->flags&1,(cptr->flags>>1)&1);
			cptr++;
			xoo+=8*zoom;
		}
		xoo=xo;
		yo+=8*zoom;
	}
}
void ChunckClass::importSonic1(const char * filename,bool append){
	int compression=fl_choice("Compression?","Uncompressed","Enigma",0);
	uint16_t* Dat;
	uint32_t fileSize;
	if(compression==1)
		Dat=(uint16_t*)decodeEnigma(filename,fileSize);
	else{
		FILE * fi=fopen(filename,"rb");
		fseek(fi,0,SEEK_END);
		fileSize=ftell(fi);
		rewind(fi);
		fread(Dat,1,fileSize,fi);
		fclose(fi);
	}
	uint32_t off;
	if(append)
		off=amt;
	else
		off=0;
	wi=hi=16;
	amt=(fileSize/512)+off;
	chuncks=(struct ChunckAttrs*)realloc(chuncks,amt*sizeof(struct ChunckAttrs)*wi*hi);
	struct ChunckAttrs*cptr;
	cptr+=off*wi*hi;
	uint16_t * DatC=Dat;
	for(uint32_t l=0;l<(fileSize/512);++l){
		for(uint32_t y=0;y<16;++y){
			for(uint32_t x=0;x<16;++x){
				cptr->block=*DatC&1023;
				cptr->flags=(*DatC>>10)&15;
				++cptr;
				++DatC;
			}
		}
	}
	free(Dat);
}
