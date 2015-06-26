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
#include <FL/fl_ask.H>
#include <string.h>
#include "classlevel.h"
level::level(){
	layeramt=1;
	w.resize(1,1);
	h.resize(1,1);
	dat.resize(1);
	odat.resize(1);
	dat[0]=new std::vector<struct levDat>;
	dat[0]->resize(1);
	odat[0]=new std::vector<struct levobjDat>;
}
void level::addLayer(unsigned at,bool after){
	unsigned base=at;
	if(after)
		++at;
	w.insert(w.begin()+at,w[base]);
	h.insert(h.begin()+at,h[base]);
	std::vector<struct levDat>*vtmp=new std::vector<struct levDat>;
	dat.insert(dat.begin()+at,vtmp);
	dat[at]->resize(w[at]*h[at]);
}
void level::removeLayer(unsigned which){
	w.erase(w.begin()+which);
	h.erase(h.begin()+which);
	delete dat[which];
	dat.erase(dat.begin()+which);
}
void level::setId(unsigned x,unsigned y,unsigned layer,unsigned val){
	(*dat[layer])[(y*w[layer])+x].id=val;
}
void level::setDat(unsigned x,unsigned y,unsigned layer,unsigned val){
	(*dat[layer])[(y*w[layer])+x].dat=val;
}
uint32_t level::getId(unsigned x,unsigned y,unsigned layer){
	return (*dat[layer])[(y*w[layer])+x].id;
}
uint32_t level::getDat(unsigned x,unsigned y,unsigned layer){
	return (*dat[layer])[(y*w[layer])+x].dat;
}
void level::setlayeramt(unsigned amt,bool lastLayerDim){
	if(amt>layeramt){
		if(lastLayerDim){
			w.reserve(amt);
			h.reserve(amt);
		}else{
			w.resize(amt);
			h.resize(amt);
		}
		dat.reserve(amt);
		odat.reserve(amt);
		for(unsigned i=layeramt;i<amt;++i){
			if(lastLayerDim){
				w.push_back(w[layeramt-1]);
				h.push_back(h[layeramt-1]);
			}
			std::vector<struct levDat>*vtmp=new std::vector<struct levDat>;
			if(lastLayerDim)
				vtmp->resize(w[i]*h[i]);
			dat.push_back(vtmp);
			std::vector<struct levobjDat>*votmp=new std::vector<struct levobjDat>;
			odat.push_back(votmp);
		}
	}else if(amt<layeramt){
		for(unsigned i=amt;i<layeramt;++i){
			delete dat[i];
			delete odat[i];
		}
		w.resize(amt);
		h.resize(amt);
		dat.resize(amt);
		odat.resize(amt);
	}else{
		fl_alert("Same amount of layers");
	}
	layeramt=amt;
}
void level::save(FILE*fp){
	/*Format
	 * uint32_t layers amount
	 * For each layer uint32_t width,height
	 * Data see struct levDat
	 * uint32_t objects amount
	 * Data see struct levobjDat*/
	fwrite(&layeramt,1,sizeof(uint32_t),fp);
	for(unsigned i=0;i<layeramt;++i){
		fwrite(&w[i],1,sizeof(uint32_t),fp);
		fwrite(&h[i],1,sizeof(uint32_t),fp);
		fwrite(dat.data(),dat.size(),sizeof(struct levDat),fp);
		uint32_t objamt=odat.size();
		fwrite(&objamt,1,sizeof(uint32_t),fp);
		if(objamt)
			fwrite(odat.data(),odat.size(),sizeof(struct levobjDat),fp);
	}
}
void level::load(FILE*fp){
	uint32_t amtnew;
	fread(&amtnew,1,sizeof(uint32_t),fp);
	setlayeramt(amtnew,false);
	for(unsigned i=0;i<layeramt;++i){
		fread(&w[i],1,sizeof(uint32_t),fp);
		fread(&h[i],1,sizeof(uint32_t),fp);
		dat.resize(w[i]*h[i]);
		fread(dat[i]->data(),w[i]*h[i],sizeof(struct levDat),fp);
		uint32_t objamt;
		fread(&objamt,1,sizeof(uint32_t),fp);
		if(objamt){
			odat.resize(objamt);
			fread(odat.data(),odat.size(),sizeof(struct levobjDat),fp);
		}
	}
}
