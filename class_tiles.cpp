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
#include <exception>
#include <ctime>
#include "includes.h"
#include "macros.h"
#include "nearestColor.h"
#include "class_tiles.h"
#include "dither.h"
#include "tilemap.h"
#include "errorMsg.h"
#include "undo.h"
#include "classpalettebar.h"
#include "gui.h"
tiles::tiles(struct Project*prj){
	current_tile=0;
	amt=1;
	setDim(8,8,prj->getBitdepthSys());
	this->prj=prj;
}
tiles::tiles(const tiles&other,Project*prj){
	this->prj=prj;
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
	extAttrs.clear();
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
	unsigned bdr=prj->getBitdepthSysraw(),bd;
	bd=bdr+1;
	unsigned maxp=(1<<bd)-1;
	if(val>maxp)
		val=maxp;
	ptr+=y*((sizew+7)/8);
	x=(sizew-1)-x;
	for(unsigned shift=0;shift<bd;++shift){
		*ptr&=~(1<<x);
		*ptr|=(val&1)<<x;
		val>>=1;
		ptr+=((sizew+7)/8)*sizeh;
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
	unsigned bdr=prj->getBitdepthSysraw();
	unsigned val=0;
	x=(sizew-1)-x;
	ptr+=y*((sizew+7)/8);
	for(unsigned shift=0;shift<=bdr;++shift){
		val|=((*ptr>>x)&1)<<shift;
		ptr+=((sizew+7)/8)*sizeh;
	}
	return val;
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
	amt=amtnew;
	resizeAmt();
}
void tiles::resizeAmt(void){
	try{
		tDat.resize(amt*tileSize);
		truetDat.resize(amt*tcSize);
		unsigned tp=prj->extAttrTilesPerByte();
		if(tp)
			extAttrs.resize(amt*prj->szPerExtPalRow()/tp);
	}catch(std::exception&e){
		fl_alert("Error: cannot resize tiles to %u\nAdditional details %s",amt,e.what());
		exit(1);
	}
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
void tiles::truecolor_to_tile_ptr(unsigned palette_row,uint32_t cur_tile,uint8_t * tileinput,bool Usedither,bool isSprite,bool isIndexArray){
	//dithers a truecolor tile to tile
	if(Usedither&&isIndexArray){
		fl_alert("Invalid parameters");
		return;
	}
	uint8_t*truePtr=(uint8_t*)alloca(tcSize);
	if(isIndexArray)
		truePtr=tileinput;
	else
		memcpy(truePtr,tileinput,tcSize);
	if(Usedither){
		ditherImage(truePtr,sizew,sizeh,true,true, true,palette_row);
		ditherImage(truePtr,sizew,sizeh,true,false,true,palette_row);
	}
	//now image needs to be checked for alpha
	uint8_t * tPtr=truePtr;
	for (unsigned y=0;y<sizeh;++y){
		for (unsigned x=0;x<sizew;++x){
			unsigned temp;
			if(isIndexArray)
				setPixel(cur_tile,x,y,(*tPtr++)%prj->pal->perRow);
			else{
				temp=find_near_color_from_row(palette_row,tPtr[0],tPtr[1],tPtr[2],(prj->pal->haveAlt)&&isSprite);
				tPtr+=3;
				if(*tPtr++)
					setPixel(cur_tile,x,y,temp);
				else
					setPixel(cur_tile,x,y,0);
			}
		}
	}
}
void tiles::draw_truecolor(uint32_t tile_draw,unsigned x,unsigned y,bool usehflip,bool usevflip,unsigned zoom){
	static uint8_t DontShow=0;
	if (amt<=tile_draw){
		if (unlikely(!DontShow)){
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
static inline uint_fast32_t cal_offset_zoom_rgb(uint_fast16_t x,uint_fast16_t y,uint_fast16_t zoom,uint8_t channel,struct Project*p){
	return (y*(zoom*p->tileC->sizew*3))+(x*3)+channel;
}
void tiles::draw_tile(int x_off,int y_off,uint32_t tile_draw,unsigned zoom,unsigned pal_row,bool Usehflip,bool Usevflip,bool isSprite,const uint8_t*extAttr,unsigned plane){
	static unsigned DontShow=0;
	if (amt<=tile_draw){
		if (unlikely(DontShow==0)){
			fl_alert("Warning tried to draw tile # %d at X: %d y: %d\nBut there is only %d tiles.\nNote that this message will not be shown again.\n Instead it will be outputted to stdout",tile_draw,x_off,y_off,amt);
			DontShow=1;
		}else
			printf("Warning tried to draw tile # %d at X: %d y: %d\nBut there is only %d tiles.\n",tile_draw,x_off,y_off,amt);
		return;
	}
	static uint8_t * temp_img_ptr;
	static unsigned tempPtrSize;
	if(tempPtrSize<(((sizeh*zoom)*(sizew*zoom))*3)){
		tempPtrSize=((sizeh*zoom)*(sizew*zoom))*3;
		temp_img_ptr=(uint8_t*)realloc(temp_img_ptr,tempPtrSize);
	}
	if(!temp_img_ptr){
		show_realloc_error(((sizeh*zoom)*(sizew*zoom))*3)
		return;
	}
	for(unsigned y=0;y<sizeh;++y){
		for(unsigned x=0;x<sizew;++x){
			unsigned pixOff=getPixel(tile_draw,Usehflip?(sizew-1)-x:x,Usevflip?(sizeh-1)-y:y)*3;
			if(extAttr)
				pixOff=prj->tms->maps[plane].getPalRowExt(extAttr,Usevflip?(sizeh-1)-y:y,pixOff?true:false)*3;
			else if(prj->pal->haveAlt&&isSprite){
				pixOff+=prj->pal->colorCnt*3;
				pixOff+=pal_row*prj->pal->perRowalt*3;
			}else
				pixOff+=pal_row*prj->pal->perRow*3;
			for(unsigned i=0;i<zoom;++i){
				for(unsigned j=0;j<zoom;++j){
					for(unsigned k=0;k<3;++k)
						temp_img_ptr[cal_offset_zoom_rgb((x*zoom)+j,(y*zoom)+i,zoom,k,prj)]=prj->pal->rgbPal[pixOff+k];
				}
			}
		}
	}
	fl_draw_image(temp_img_ptr,x_off,y_off,sizew*zoom,sizeh*zoom,3);
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
	/*vflip_truecolor_ptr needs to be a separate function as the output of hflip may be inputted here to form a vertically and horizontally flipped tile*/
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
				prj->tms->maps[prj->curPlane].sub_tile_map(curT,cur_tile,false,false);
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
				prj->tms->maps[prj->curPlane].sub_tile_map(curT,cur_tile,true,false);
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
				prj->tms->maps[prj->curPlane].sub_tile_map(curT,cur_tile,true,true);
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
				prj->tms->maps[prj->curPlane].sub_tile_map(curT,cur_tile,false,true);
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
	row*=prj->pal->perRow*3;
	for(unsigned y=0;y<sizeh;++y){
		for(unsigned x=0;x<sizew;++x){
			unsigned temp=getPixel(input,x,y);
			*output++=prj->pal->rgbPal[row+(temp*3)];
			*output++=prj->pal->rgbPal[row+(temp*3)+1];
			*output++=prj->pal->rgbPal[row+(temp*3)+2];
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
void tiles::toPlanar(void){
	uint8_t*tmp=(uint8_t*)alloca(tileSize);
	uint8_t*tPtr=tDat.data();
	unsigned bdr=prj->getBitdepthSysraw();
	for(unsigned i=0;i<amt;++i){
		uint8_t*ptr=tmp;
		memcpy(tmp,tPtr,tileSize);
		for(unsigned y=0;y<sizeh;++y){
			for(unsigned x=0;x<sizew;++x){
				unsigned val;
				switch(bdr){
					case 3:
						if(x&1)
							val=(*ptr++)&15;
						else
							val=*ptr>>4;
					break;
					default:
						val=0;
						show_default_error
				}
				setPixel(tPtr,x,y,val);
			}
		}
		tPtr+=tileSize;
	}
}
void*tiles::toLinear(void){
	void*pt=malloc(amt*tileSize);
	uint8_t*ptr=(uint8_t*)ptr;
	unsigned bdr=prj->getBitdepthSysraw();
	uint8_t*tPtr=tDat.data();
	for(unsigned i=0;i<amt;++i){
		for(unsigned y=0;y<sizeh;++y){
			for(unsigned x=0;x<sizew;++x){
				unsigned val=getPixel(tPtr,x,y);
				switch(bdr){
					case 3:
						if(x&1)
							*ptr++|=val;
						else
							*ptr=val<<4;
					break;
				}
			}
		}
		tPtr+=tileSize;
	}
	return pt;
}
void tiles::setDim(unsigned w,unsigned h,unsigned bd){
	sizew=w;
	sizeh=h;
	curBD=bd;
	tcSize=sizew*sizeh*4;
	tileSize=((sizew+7)/8*8)*sizeh*bd/8;
	tDat.resize(tileSize*amt,0);
	truetDat.resize(tcSize*amt,0);
}
void tiles::swap(unsigned first,unsigned second){
	if(first==second)
		return;
	uint8_t*tmp=(uint8_t*)alloca(tileSize);
	uint8_t*tmpT=(uint8_t*)alloca(tcSize);
	memcpy(tmp,tDat.data()+(first*tileSize),tileSize);
	memcpy(tmpT,truetDat.data()+(first*tcSize),tcSize);
	memcpy(tDat.data()+(first*tileSize),tDat.data()+(second*tileSize),tileSize);
	memcpy(tDat.data()+(second*tileSize),tmp,tileSize);
	memcpy(truetDat.data()+(first*tcSize),truetDat.data()+(second*tcSize),tcSize);
	memcpy(truetDat.data()+(second*tcSize),tmp,tcSize);
}
void tiles::changeDim(unsigned w,unsigned h,unsigned bd){
	if(w==sizew&&h==sizeh){
		if(curBD==bd)
			return;
		else{
			setDim(w,h,bd);
			return;
		}
	}
	if(sizew%w){
		show_TODO_error
		return;
	}
	if(sizeh%h){
		show_TODO_error
		return;
	}
	unsigned amto=amt;
	tiles*old=new tiles(*this,prj);
	amt=amt*sizew/w*sizeh/h;
	unsigned sw=sizew,sh=sizeh;
	if(sw>w&&sh>h)
		std::fill(tDat.begin(),tDat.end(),0);
	setDim(w,h,bd);
	//If going to a smaller dimension break up the tiles; discard tile data keep only truecolor data.
	if(sw>w&&sh>h){
		uint8_t  __restrict__ *src=old->truetDat.data(),*dst=truetDat.data();
		for(unsigned i=0;i<amto;++i){
			for(unsigned y=0;y<sh;++y){
				for(unsigned x=0;x<sw/w;++x){
					memcpy(dst+(x*w*4)+(y*sw*4),src+(x*tcSize)+(y%h*w*4),w*4);
				}
			}
		}
	}
	updateTileSelectAmt();
	delete old;
}
