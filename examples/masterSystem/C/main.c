#include "SMSlib.h"
#include "palette.h"
#include "tiles.h"
#include "tilemap.h"
void main(void){
	SMS_displayOn();
	SMS_loadBGPalette(palDat);
	SMS_loadSpritePalette(palDat+16);
	SMS_loadTiles(tileDat,0,sizeof(tileDat));
	SMS_loadTileMap(0,0,mapDat,sizeof(mapDat));
	for(;;);
}
