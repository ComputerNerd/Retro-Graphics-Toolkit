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
#include <exception>
#include <ctime>
#include "global.h"
#include "class_tiles.h"
#include "dither.h"
#include "tilemap.h"
#include "errorMsg.h"
#include "undo.h"
#include "classpalettebar.h"
tiles::tiles(){
	current_tile=0;
	amt=1;
	sizew=sizeh=8;
	tcSize=sizew*sizeh*4;
	tileSize=sizew*sizeh*getBitdepthcurSys()/8;
	tDat.resize(tileSize,0);
	truetDat.resize(tcSize,0);
}
tiles::tiles(const tiles& other){
	current_tile=other.current_tile;
	amt=other.amt;
	tileSize=other.tileSize;
	sizew=other.sizew;
	sizeh=other.sizeh;
	tcSize=sizew*sizeh*4;
	tDat=other.tDat;
	truetDat=other.truetDat;
}
tiles::~tiles(){
	tDat.clear();
	truetDat.clear();
}
void tiles::insertTile(uint32_t at){
	try{
		if(at>amt)
			resizeAmt(at);
		else
			++amt;
		tDat.insert(tDat.begin()+at*tileSize,tileSize,0);
		truetDat.insert(truetDat.begin()+at*tcSize,tcSize,0);
	}catch(std::exception& e){
		fl_alert("Error inserting tile at %d\nAdditional details: %s",at-1,e.what());
		exit(1);
	}
}
void tiles::setPixel(uint8_t*ptr,uint32_t x,uint32_t y,uint32_t val){
	if(x>=sizew)
		x=sizew-1;
	if(y>=sizeh)
		y=sizeh-1;
	unsigned bdr,bd;
	bdr=getBitdepthcurSysraw();
	bd=bdr+1;
	unsigned maxp=(1<<bd)-1;
	if(val>maxp)
		val=maxp;
	if((currentProject->gameSystem==NES)&&bdr){//NES stores planar tiles
		x=7-x;
		ptr+=y;
		if(val&1)//First plane
			*ptr|=1<<x;
		else
			*ptr&=~(1<<x);
		ptr+=8;
		if(val&2)//Second plane
			*ptr|=1<<x;
		else
			*ptr&=~(1<<x);
	}else{
		switch(bdr){
			case 0:
				x=7-x;
				ptr+=(y*sizew/8)+(x/8);
				if(val)
					*ptr|=1<<x;
				else
					*ptr&=~(1<<x);
			break;
			case 3:
				ptr+=((y*sizew)/2)+(x/2);
				if(x&1){
					*ptr&=~15;
					*ptr|=val;
				}else{
					*ptr&=~(15<<4);
					*ptr|=val<<4;
				}
			break;
			default:
				show_default_error
		}
	}
}
void tiles::setPixel(uint32_t tile,uint32_t x,uint32_t y,uint32_t val){
	uint8_t*ptr=&tDat[(tile*tileSize)];
	setPixel(ptr,x,y,val);
}
uint32_t tiles::getPixel(const uint8_t*ptr,uint32_t x,uint32_t y) const{
	if(x>=sizew)
		x=sizew-1;
	if(y>=sizeh)
		y=sizeh-1;
	unsigned bdr;
	bdr=getBitdepthcurSysraw();
	if((currentProject->gameSystem==NES)&&bdr){//NES stores planar tiles
		x=7-x;
		ptr+=y;
		return ((*ptr)>>x&1)|(((*(ptr+8))>>x&1)<<1);
	}else{
		switch(bdr){
			case 0:
				x=7-x;
				ptr+=y*sizew/8;
				return (*ptr)>>x&1;
			break;
			case 3:
				ptr+=((y*sizew)/2)+(x/2);
				if(x&1)
					return *ptr&15;
				else
					return *ptr>>4;
			break;
			default:
				show_default_error
		}
	}
	return 0;
}
uint32_t tiles::getPixel(uint32_t tile,uint32_t x,uint32_t y) const{
	const uint8_t*ptr=&tDat[(tile*tileSize)];
	return getPixel(ptr,x,y);
}
void tiles::setPixelTc(uint32_t tile,uint32_t x,uint32_t y,uint32_t val){
	uint32_t*tt=(uint32_t*)((uint8_t*)truetDat.data()+(tile*tcSize));
	tt+=y*sizew;
	tt+=x;
	*tt=val;
}
uint32_t tiles::getPixelTc(uint32_t tile,uint32_t x,uint32_t y) const{
	const uint32_t*tt=(const uint32_t*)((const uint8_t*)truetDat.data()+(tile*tcSize));
	tt+=y*sizew;
	tt+=x;
	return*tt;
}
void tiles::resizeAmt(uint32_t amtnew){
	try{
		amt=amtnew;
		tDat.resize(amt*tileSize);
		truetDat.resize(amt*tcSize);
	}catch(std::exception&e){
		fl_alert("Error: cannot resize tiles to %u\nAdditional details %s",amtnew,e.what());
		exit(1);
	}
}
void tiles::resizeAmt(void){
	resizeAmt(amt);
}
void tiles::appendTile(unsigned many){
	resizeAmt(amt+many);
}
void tiles::remove_tile_at(uint32_t tileDel){
	if(tileDel>=amt){
		fl_alert("Cannot delete tile %d as there are only %d tiles",tileDel,amt);
		return;
	}
	if(!amt){
		fl_alert("You already have no tiles");
		return;
	}
	if(amt==1){
		tDat.clear();
		truetDat.clear();
	}else{
		tDat.erase(tDat.begin()+(tileDel*tileSize),tDat.begin()+((tileDel+1)*tileSize));
		truetDat.erase(truetDat.begin()+(tileDel*tcSize),truetDat.begin()+((tileDel+1)*tcSize));
	}
	--amt;
	updateTileSelectAmt(amt);
}
void tiles::truecolor_to_tile(unsigned palette_row,uint32_t cur_tile,bool isSprite){
	truecolor_to_tile_ptr(palette_row,cur_tile,&truetDat[(cur_tile*tcSize)],true,isSprite);
}
void tiles::truecolor_to_tile_ptr(unsigned palette_row,uint32_t cur_tile,uint8_t * tileinput,bool Usedither,bool isSprite){
	//dithers a truecolor tile to tile
	uint8_t*true_color_temp=(uint8_t*)alloca(tcSize);
	memcpy(true_color_temp,tileinput,tcSize);
	if(Usedither){
		ditherImage(true_color_temp,sizew,sizeh,true,true, true,palette_row);
		ditherImage(true_color_temp,sizew,sizeh,true,false,true,palette_row);
	}
	//now image needs to be checked for alpha
	uint8_t * truePtr=true_color_temp;
	for (unsigned y=0;y<sizeh;++y){
		for (unsigned x=0;x<sizew;++x){
			unsigned temp=find_near_color_from_row(palette_row,truePtr[0],truePtr[1],truePtr[2],(currentProject->pal->haveAlt)&&isSprite);
			truePtr+=3;
			if(*truePtr++)
				setPixel(cur_tile,x,y,temp);
		}
	}
}
void tiles::draw_truecolor(uint32_t tile_draw,unsigned x,unsigned y,bool usehflip,bool usevflip,unsigned zoom){
	static uint8_t DontShow=0;
	if (amt<=tile_draw){
		if (unlikely(!DontShow)) {
			fl_alert("Warning tried to draw truecolor tile # %d at X: %d y: %d\nBut there is only %d tiles.\nNote that this message will not be shown again.\n Instead it will be outputted to stdout",tile_draw,x,y,amt);
			DontShow=1;
		}else
			printf("Warning tried to draw truecolor tile # %d at X: %d y: %d\nBut there is only %d tiles.\n",tile_draw,x,y,amt);
		return;
	}
	uint_fast32_t xx,yy,xxx,yyy;
	uint8_t*trueColTemp=(uint8_t*)alloca(tcSize);
	uint8_t*grid=(uint8_t*)alloca(tcSize*3/4);
	uint8_t * grid_ptr=grid;
	uint8_t * truePtr;
	for (xxx=0;xxx<sizew/2;++xxx){
		for (xx=0;xx<sizeh/2;xx++){
			for (yy=0;yy<3;yy++)
				*grid_ptr++=255;
			for (yy=0;yy<3;yy++)
				*grid_ptr++=160;
		}
		for (xx=0;xx<sizew/2;xx++){
			for (yy=0;yy<3;yy++)
				*grid_ptr++=160;
			for (yy=0;yy<3;yy++)
				*grid_ptr++=255;
		}
	}
	if (usehflip == false && usevflip == false)
		std::copy(truetDat.begin()+(tile_draw*tcSize),truetDat.begin()+((tile_draw+1)*tcSize),trueColTemp);
	else if (usehflip == true && usevflip == false)
		hflip_truecolor(tile_draw,(uint32_t *)trueColTemp);
	else if (usehflip == false && usevflip == true)
		vflip_truecolor(tile_draw,trueColTemp);
	else{
		hflip_truecolor(tile_draw,(uint32_t *)trueColTemp);
		vflip_truecolor_ptr(trueColTemp,trueColTemp);
	}
	truePtr=&trueColTemp[3];
	grid_ptr=grid;
	for (xxx=0;xxx<sizew*sizeh;++xxx){
		for (xx=0;xx<3;xx++){
			if (*truePtr){
				double percent=(double)*truePtr/255.0;
				uint8_t grid_nerd=*grid_ptr;
				//*grid_ptr++=((double)trueColTemp[(zz*4)+xx]*percent)+((double)*grid_ptr*(1.0-percent));//this could be undefined
				*grid_ptr++=((double)trueColTemp[(xxx*4)+xx]*percent)+((double)grid_nerd*(1.0-percent));
			}else
				grid_ptr++;
		}
		truePtr+=4;//next alpha value
	}
	if(zoom>1){
		uint8_t*scaled=(uint8_t*)alloca(sizew*sizeh*zoom*zoom*3);
		uint8_t*s=scaled;
		grid_ptr=grid;
		for(yy=0;yy<sizeh;++yy){
			for(xx=0;xx<sizew;++xx){
				for(yyy=0;yyy<zoom;++yyy){
					grid_ptr=grid+(yy*sizew*3)+(xx*3);
					s=scaled+(yy*zoom*zoom*sizew*3)+(xx*3*zoom)+(yyy*sizew*3*zoom);
					for(xxx=0;xxx<zoom;++xxx){
						*s++=grid_ptr[0];
						*s++=grid_ptr[1];
						*s++=grid_ptr[2];
					}
				}
			}
		}
		fl_draw_image(scaled,x,y,sizew*zoom,sizeh*zoom,3);
	}else
		fl_draw_image(grid,x,y,sizew*zoom,sizeh*zoom,3);
}
static inline uint_fast32_t cal_offset_zoom_rgb(uint_fast16_t x,uint_fast16_t y,uint_fast16_t zoom,uint8_t channel){
	return (y*(zoom*currentProject->tileC->sizew*3))+(x*3)+channel;
}
void tiles::draw_tile(int x_off,int y_off,uint32_t tile_draw,int zoom,unsigned pal_row,bool Usehflip,bool Usevflip,bool isSprite){
	static unsigned DontShow=0;
	if (amt<=tile_draw){
		if (unlikely(DontShow==0)){
			fl_alert("Warning tried to draw tile # %d at X: %d y: %d\nBut there is only %d tiles.\nNote that this message will not be shown again.\n Instead it will be outputted to stdout",tile_draw,x_off,y_off,amt);
			DontShow=1;
		}else
			printf("Warning tried to draw tile # %d at X: %d y: %d\nBut there is only %d tiles.\n",tile_draw,x_off,y_off,amt);
		return;
	}
	uint8_t * temp_img_ptr = (uint8_t *)malloc(((sizeh*zoom)*(sizew*zoom))*3);
	if(!temp_img_ptr){
		show_malloc_error(((sizeh*zoom)*(sizew*zoom))*3)
		return;
	}
	for(unsigned y=0;y<sizeh;++y){
		for(unsigned x=0;x<sizew;++x){
			unsigned pixOff=getPixel(tile_draw,Usehflip?(sizew-1)-x:x,Usevflip?(sizeh-1)-y:y)*3;
			if(currentProject->pal->haveAlt&&isSprite){
				pixOff+=currentProject->pal->colorCnt*3;
				pixOff+=pal_row*currentProject->pal->perRowalt*3;
			}else
				pixOff+=pal_row*currentProject->pal->perRow*3;
			for(unsigned i=0;i<zoom;++i){
				for(unsigned j=0;j<zoom;++j){
					for(unsigned k=0;k<3;++k)
						temp_img_ptr[cal_offset_zoom_rgb((x*zoom)+j,(y*zoom)+i,zoom,k)]=currentProject->pal->rgbPal[pixOff+k];
				}
			}
		}
	}
	fl_draw_image(temp_img_ptr,x_off,y_off,sizew*zoom,sizeh*zoom,3);
	free(temp_img_ptr);
}
void tiles::hflip_truecolor(uint32_t id,uint32_t * out){
	//out must contain at least tcSize bytes
	uint32_t * trueColPtr=(uint32_t *)&truetDat[id*tcSize];
	trueColPtr+=sizew-1;
	for(unsigned y=0;y<sizeh;y++){
		for(unsigned x=0;x<sizew;++x)
			*out++=*trueColPtr--;
		trueColPtr+=sizew*2;
	}
}
void tiles::vflip_truecolor_ptr(uint8_t * in,uint8_t * out){
/*!this needs to be a separate function as the output of hflip may be inputted here to form vhfliped tile*/
	uint16_t y;
	uint8_t temp[256];
	memcpy(temp,in,256);
	for (y=0;y<256;y+=32)
		memcpy(&out[224-y],&temp[y],32);
}
void tiles::vflip_truecolor(uint32_t id,uint8_t * out){
	vflip_truecolor_ptr(&truetDat[id*tcSize],out);
}
void tiles::hflip_tile(uint32_t id,uint8_t * out){
	for(unsigned y=0;y<sizeh;++y){
		for(unsigned x=0;x<sizew;++x)
			setPixel(out,(sizew-1)-x,y,getPixel(id,x,y));
	}
}
void tiles::vflip_tile_ptr(const uint8_t*in,uint8_t*out){
	for(unsigned y=0;y<sizeh;++y){
		for(unsigned x=0;x<sizew;++x)
			setPixel(out,x,(sizeh-1)-y,getPixel(in,x,y));
	}
}
void tiles::vflip_tile(uint32_t id,uint8_t * out){
	vflip_tile_ptr(&tDat[id*tileSize],out);
}
void tiles::blank_tile(uint32_t tileUsage){
	if (mode_editor == tile_edit){
		memset(&truetDat[tileUsage*tcSize],0,tcSize);
		truecolor_to_tile(palBar.selRow[1],tileUsage,false);
	}else
		memset(&tDat[tileUsage*tileSize],0,tileSize);
}
#if _WIN32
#define bcmp memcmp
#endif
void tiles::remove_duplicate_tiles(bool tColor){
	pushTilemapAll(false);
	pushTileGroupPrepare(tTypeDelete);
	char bufT[1024];
	Fl_Window *win;
	Fl_Progress *progress;
	mkProgress(&win,&progress);
	uint32_t tile_remove_c=0;
	int32_t cur_tile,curT;
	uint8_t*tileTemp;
	if(tColor)
		tileTemp=(uint8_t *)alloca(tcSize);
	else
		tileTemp=(uint8_t *)alloca(tileSize);
	std::vector<uint32_t> remap(amt);
	time_t lastt=time(NULL);
	for(uint32_t i=0;i<amt;++i)
		remap[i]=i;
	for (cur_tile=0;cur_tile<amt;++cur_tile){
		snprintf(bufT,1024,"Comparing tiles with: %d",cur_tile);
		win->copy_label(bufT);
		for (curT=amt-1;curT>=cur_tile;curT--){
			if (cur_tile == curT)//don't compare with itself
				continue;
			bool rm;
			if(tColor)
				rm=!bcmp(&truetDat[cur_tile*tcSize],&truetDat[curT*tcSize],tcSize);
			else
				rm=!bcmp(&tDat[cur_tile*tileSize],&tDat[curT*tileSize],tileSize);
			if(rm){
				currentProject->tms->maps[currentProject->curPlane].sub_tile_map(curT,cur_tile,false,false);
				addTileGroup(curT,remap[curT]);
				remap.erase(remap.begin()+curT);
				remove_tile_at(curT);
				tile_remove_c++;
				continue;//curT does not exist anymore useless to do more comparisons
			}
			if(tColor){
				hflip_truecolor(curT,(uint32_t*)tileTemp);			
				rm=!bcmp(&truetDat[cur_tile*tcSize],tileTemp,tcSize);
			}else{
				hflip_tile(curT,tileTemp);
				rm=!bcmp(&tDat[cur_tile*tileSize],tileTemp,tileSize);
			}
			if(rm){
				currentProject->tms->maps[currentProject->curPlane].sub_tile_map(curT,cur_tile,true,false);
				addTileGroup(curT,remap[curT]);
				remap.erase(remap.begin()+curT);
				remove_tile_at(curT);
				tile_remove_c++;
				continue;
			}
			if(tColor){
				vflip_truecolor_ptr(tileTemp,tileTemp);
				rm=!bcmp(&truetDat[cur_tile*tcSize],tileTemp,tcSize);
			}else{
				vflip_tile_ptr(tileTemp,tileTemp);
				rm=!bcmp(&tDat[cur_tile*tileSize],tileTemp,tileSize);
			}
			if(rm){
				currentProject->tms->maps[currentProject->curPlane].sub_tile_map(curT,cur_tile,true,true);
				addTileGroup(curT,remap[curT]);
				remap.erase(remap.begin()+curT);
				remove_tile_at(curT);
				tile_remove_c++;
				continue;
			}
			if(tColor){
				vflip_truecolor(curT,tileTemp);
				rm=!bcmp(&truetDat[cur_tile*tcSize],tileTemp,tcSize);
			}else{
				vflip_tile(curT,tileTemp);
				rm=!bcmp(&tDat[cur_tile*tileSize],tileTemp,tileSize);
			}
			if(rm){
				currentProject->tms->maps[currentProject->curPlane].sub_tile_map(curT,cur_tile,false,true);
				addTileGroup(curT,remap[curT]);
				remap.erase(remap.begin()+curT);
				remove_tile_at(curT);
				tile_remove_c++;
				continue;
			}
			if((time(NULL)-lastt)>=1){
				lastt=time(NULL);
				progress->value((float)cur_tile/(float)amt);
				snprintf(bufT,1024,"Removed %d tiles",tile_remove_c);
				progress->label(bufT);
				Fl::check();
			}
		}
		progress->value((float)cur_tile/(float)amt);
		snprintf(bufT,1024,"Removed %d tiles",tile_remove_c);
		progress->label(bufT);
		Fl::check();
	}
	remap.clear();
	printf("Removed %d tiles\n",tile_remove_c);
	win->remove(progress);// remove progress bar from window
	delete(progress);// deallocate it
	//w->draw();
	delete win;
	Fl::check();
}
void tiles::tileToTrueCol(const uint8_t*input,uint8_t*output,unsigned row,bool useAlpha,bool alphaZero){
	row*=currentProject->pal->perRow*3;
	for(unsigned y=0;y<sizeh;++y){
		for(unsigned x=0;x<sizew;++x){
			unsigned temp=getPixel(input,x,y);
			*output++=currentProject->pal->rgbPal[row+(temp*3)];
			*output++=currentProject->pal->rgbPal[row+(temp*3)+1];
			*output++=currentProject->pal->rgbPal[row+(temp*3)+2];
			if(useAlpha){
				if(alphaZero){
					if(temp)
						*output++=255;
					else
						*output++=0;
				}else
					*output++=255;
			}
		}
	}
}
