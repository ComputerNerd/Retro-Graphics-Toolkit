#include "SGlib.h"
#include "tiles.h"
#include "tilemap.h"

void main(void) {
	// Load the tiles and tilemap and then turn on the screen.
	SG_loadTilePatterns(tiles, 0, sizeof(tiles));
	SG_loadTileMap(0, 0, tilemap, sizeof(tilemap));

	SG_displayOn();
	for(;;);
}
