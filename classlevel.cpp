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
level::level(Project*prj){
	this->prj=prj;
	layeramt=1;
	lvlI.resize(1,{1,1,1,1,0,0,CHUNKS});
	dat.resize(1);
	odat.resize(1);
	dat[0]=new std::vector<struct levDat>;
	dat[0]->resize(1);
	odat[0]=new std::vector<struct levobjDat>;
}
level::level(const level&o,Project*prj){
	this->prj=prj;
	layeramt=o.layeramt;
	lvlI=o.lvlI;
	dat.resize(o.dat.size());
	odat.resize(o.odat.size());
	for(unsigned i=0;i<o.dat.size();++i)
		dat[i]=new std::vector<struct levDat>(*o.dat[i]);
	for(unsigned i=0;i<o.odat.size();++i)
		odat[i]=new std::vector<struct levobjDat>(*o.odat[i]);
}
void level::addLayer(unsigned at,bool after){
	unsigned base=at;
	if(after)
		++at;
	lvlI.insert(lvlI.begin()+at,lvlI[base]);
	std::vector<struct levDat>*vtmp=new std::vector<struct levDat>;
	dat.insert(dat.begin()+at,vtmp);
	dat[at]->resize(lvlI[at].w*lvlI[at].h);
}
void level::removeLayer(unsigned which){
	lvlI.erase(lvlI.begin()+which);
	delete dat[which];
	dat.erase(dat.begin()+which);
	delete odat[which];
	odat.erase(odat.begin()+which);
}
void level::setId(unsigned x,unsigned y,unsigned layer,unsigned val){
	(*dat[layer])[(y*lvlI[layer].w)+x].id=val;
}
void level::setDat(unsigned x,unsigned y,unsigned layer,unsigned val){
	(*dat[layer])[(y*lvlI[layer].w)+x].dat=val;
}
uint32_t level::getId(unsigned x,unsigned y,unsigned layer)const{
	return (*dat[layer])[(y*lvlI[layer].w)+x].id;
}
uint32_t level::getDat(unsigned x,unsigned y,unsigned layer)const{
	return (*dat[layer])[(y*lvlI[layer].w)+x].dat;
}
void level::setlayeramt(unsigned amt,bool lastLayerDim){
	if(amt>layeramt){
		if(lastLayerDim){
			lvlI.reserve(amt);
		}else{
			lvlI.resize(amt,{1,1,1,1,0,0,CHUNKS});
		}
		dat.reserve(amt);
		odat.reserve(amt);
		for(unsigned i=layeramt;i<amt;++i){
			if(lastLayerDim)
				lvlI.push_back(lvlI[layeramt-1]);
			std::vector<struct levDat>*vtmp=new std::vector<struct levDat>;
			if(lastLayerDim)
				vtmp->resize(lvlI[i].w*lvlI[i].h);
			dat.push_back(vtmp);
			std::vector<struct levobjDat>*votmp=new std::vector<struct levobjDat>;
			odat.push_back(votmp);
		}
	}else if(amt<layeramt){
		for(unsigned i=amt;i<layeramt;++i){
			delete dat[i];
			delete odat[i];
		}
		lvlI.resize(amt);
		dat.resize(amt);
		odat.resize(amt);
	}
	layeramt=amt;
}
void level::draw(unsigned x,unsigned y,unsigned zoom,int solo,bool showSprites)const{
	//Painter's algorithm
	uint32_t d=solo>=0?solo+1:layeramt;
	while(d--){
		unsigned xx=x;
	}
}
void level::save(FILE*fp){
	/*Format
	 * uint32_t layers amount
	 * For each layer info struct
	 * Data see struct levDat
	 * uint32_t objects amount
	 * Data see struct levobjDat*/
	fwrite(&layeramt,sizeof(uint32_t),1,fp);
	for(unsigned i=0;i<layeramt;++i){
		fwrite(&lvlI[i],sizeof(struct levelInfo),1,fp);
		fwrite(dat.data(),dat.size(),sizeof(struct levDat),fp);
		uint32_t objamt=odat[i]->size();
		fwrite(&objamt,1,sizeof(uint32_t),fp);
		if(objamt)
			fwrite(odat[i]->data(),odat[i]->size(),sizeof(struct levobjDat),fp);
	}
}
void level::load(FILE*fp,uint32_t version){
	uint32_t amtnew;
	fread(&amtnew,1,sizeof(uint32_t),fp);
	setlayeramt(amtnew,false);
	for(unsigned i=0;i<layeramt;++i){
		uint32_t objamt;
		fread(&lvlI[i],sizeof(struct levelInfo),1,fp);
		dat.resize(lvlI[i].w*lvlI[i].h);
		fread(dat[i]->data(),sizeof(struct levDat),lvlI[i].w*lvlI[i].h,fp);
		fread(&objamt,sizeof(uint32_t),1,fp);
		if(objamt){
			odat[i]->resize(objamt);
			fread(odat[i]->data(),sizeof(struct levobjDat),odat[i]->size(),fp);
		}
	}
}
