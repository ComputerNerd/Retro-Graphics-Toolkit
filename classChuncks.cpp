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
void ChunckClass::drawChunck(uint32_t id,int xo,int yo,int zoom,int scrollX,int scrollY){
	struct ChunckAttrs * cptr=chuncks;
	for(uint32_t y=scrollY;y<hi;++y){
		cptr=&chuncks[(id*wi*hi)+(y*wi)+scrollX];
		int xoo=xo;
		for(uint32_t x=scrollX;x<wi;++x){
			if(useBlocks){
				uint32_t Ty=cptr->block*currentProject->tileMapC->mapSizeH;
				int yoo=yo;
				if(cptr->flags&2)
					yoo+=(currentProject->tileMapC->mapSizeH-1)*8*zoom;
				int xooo;
				for(uint32_t yb=0;yb<currentProject->tileMapC->mapSizeH;++yb){
					xooo=xoo;
					if(cptr->flags&1)
						xooo+=(currentProject->tileMapC->mapSizeW-1)*8*zoom;
					for(uint32_t xb=0;xb<currentProject->tileMapC->mapSizeW;++xb){
						if((cptr->flags&3)==3)//Both
							currentProject->tileC->draw_tile(xooo,yoo,currentProject->tileMapC->get_tile(xb,Ty),zoom,currentProject->tileMapC->get_palette_map(xb,Ty),currentProject->tileMapC->get_hflip(xb,Ty)^true,currentProject->tileMapC->get_vflip(xb,Ty)^true);
						else if(cptr->flags&2)//Y-flip
							currentProject->tileC->draw_tile(xooo,yoo,currentProject->tileMapC->get_tile(xb,Ty),zoom,currentProject->tileMapC->get_palette_map(xb,Ty),currentProject->tileMapC->get_hflip(xb,Ty),currentProject->tileMapC->get_vflip(xb,Ty)^true);
						else if(cptr->flags&1)//X-flip
							currentProject->tileC->draw_tile(xooo,yoo,currentProject->tileMapC->get_tile(xb,Ty),zoom,currentProject->tileMapC->get_palette_map(xb,Ty),currentProject->tileMapC->get_hflip(xb,Ty)^true,currentProject->tileMapC->get_vflip(xb,Ty));
						else//No flip
							currentProject->tileC->draw_tile(xooo,yoo,currentProject->tileMapC->get_tile(xb,Ty),zoom,currentProject->tileMapC->get_palette_map(xb,Ty),currentProject->tileMapC->get_hflip(xb,Ty),currentProject->tileMapC->get_vflip(xb,Ty));
						if(cptr->flags&1)
							xooo-=8*zoom;
						else
							xooo+=8*zoom;
					}
					if(cptr->flags&2)
						yoo-=8*zoom;
					else
						yoo+=8*zoom;
					++Ty;
				}
				if(cptr->flags&2)
					yoo+=currentProject->tileMapC->mapSizeW*8*zoom;
				xoo+=currentProject->tileMapC->mapSizeH*8*zoom;

			}else{
				currentProject->tileC->draw_tile(xoo,yo,cptr->block,zoom,(cptr->flags>>3)&3,cptr->flags&1,(cptr->flags>>1)&1);
				xoo+=8*zoom;
			}
			cptr++;
			if((xoo)>(window->w()))
				break;
		}
		if(useBlocks)
			yo+=8*zoom*currentProject->tileMapC->mapSizeH;
		else
			yo+=8*zoom;
		if(yo>(window->h()))
			break;
	}
}
void ChunckClass::scrollChuncks(void){
	unsigned oldS=window->chunckX->value();
	int zoom=window->chunck_tile_size->value();
	int off;
	if(useBlocks)
		off=(wi*currentProject->tileMapC->mapSizeW)-((window->w()-ChunckOff[0])/(zoom*8));
	else
		off=wi-((window->w()-ChunckOff[0])/(zoom*8));
	if(oldS>off)
		scrollChunks[0]=oldS=off;
	if(off>0){
		window->chunckX->show();
		window->chunckX->value(oldS,1,0,off+2);
	}else
		window->chunckX->hide();
	oldS=window->chunckY->value();
	if(useBlocks)
		off=(hi*currentProject->tileMapC->mapSizeH)-((window->h()-ChunckOff[1])/(zoom*8));
	else
		off=hi-((window->h()-ChunckOff[1])/(zoom*8));
	if(oldS>off)
		scrollChunks[1]=oldS=off;
	if(off>0){
		window->chunckY->show();
		window->chunckY->value(oldS,1,0,off+2);
	}else
		window->chunckY->hide();
}
void ChunckClass::importSonic1(const char * filename,bool append){
	int compression=fl_choice("Compression?","Uncompressed","Kosinski",0);
	uint16_t* Dat;
	uint32_t fileSize;
	if(compression==1)
		Dat=(uint16_t*)decodeKosinski(filename,fileSize);
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
	struct ChunckAttrs*cptr=chuncks;
	cptr+=off*wi*hi;
	uint16_t * DatC=Dat;
	for(uint32_t l=0;l<(fileSize/512);++l){
		for(uint32_t y=0;y<16;++y){
			for(uint32_t x=0;x<16;++x){
				*DatC=be16toh(*DatC);
				cptr->block=*DatC&1023;
				cptr->flags=(*DatC>>11)&15;
				++cptr;
				++DatC;
			}
		}
	}
	window->chunck_select->maximum(amt-1);
	free(Dat);
}
