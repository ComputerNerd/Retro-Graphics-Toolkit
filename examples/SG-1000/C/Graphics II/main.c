#include "SGlib.h"
#include "waterfall_tiles.h"
#include "waterfall_attrs.h"

void main(void) {
	// SG_displayOff();
	// Load the tiles and tilemap and then turn on the screen.
	SG_loadTilePatterns(tiles, 0, sizeof(tiles));
	SG_loadTileColours(extAttrsData, 0, sizeof(extAttrsData));

	SG_setNextTileatXY(0, 0);
	uint8_t i,j,t;
	t = 0;
	for (i = 0; i < 3; ++i) {
		j = 255;
		do {
			SG_setTile(t++);
		} while(j--);
	}

	SG_displayOn();
	for(;;);
}
