#pragma once
#include <stdint.h>
#include <stdio.h>
#include <vector>
struct __attribute__ ((__packed__)) levDat{
	uint32_t id,dat;
};
struct __attribute__ ((__packed__)) levobjDat{//For sprite layout
	uint32_t x,y,id,dat;
};
class level{
public:
	uint32_t layeramt;
	std::vector<uint32_t> w;//Allow different sized layers
	std::vector<uint32_t> h;
	std::vector<std::vector<struct levDat>*>dat;
	std::vector<std::vector<struct levobjDat>*>odat;
	level();
	void addLayer(unsigned at,bool after);
	void removeLayer(unsigned which);
	void setId(unsigned x,unsigned y,unsigned layer,unsigned val);
	void setDat(unsigned x,unsigned y,unsigned layer,unsigned val);
	uint32_t getId(unsigned x,unsigned y,unsigned layer);
	uint32_t getDat(unsigned x,unsigned y,unsigned layer);
	void setlayeramt(unsigned amt,bool lastLayerDim);
	void save(FILE*fp);
	void load(FILE*fp);
};
