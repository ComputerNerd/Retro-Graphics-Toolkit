#include "SMSlib.h"
#include "palette.h"
#include "tiles.h"
#include "tilemap.h"
void main(void){
	SMS_displayOn();
	GG_loadBGPalette(palDat);
	GG_loadSpritePalette(palDat+32);
	SMS_loadTiles(tileDat,0,sizeof(tileDat));
	SMS_loadTileMapArea((32-20)/2,(24-18)/2,mapDat,20,18);
	for(;;);
}
