-- This is setup for the Github version.
dofile "LoadLevel.lua"
local dir = fl.dir_chooser()
if not (dir == nil or dir == '') then
	loadPalette(dir, 'Green Hill Zone.bin')
	loadTilemap(dir, 'GHZ')

	loadTiles(dir, 'GHZ')

	loadTilesOffset(dir, 'GHZ Flower Large', 0x6B80 // 32, 'artunc')
	loadTilesOffset(dir, 'GHZ Flower Small', 0x6D80 // 32, 'artunc')
	loadTilesOffset(dir, 'GHZ Waterfall', 0x6F00 // 32, 'artunc')


	loadTilesOffset(dir, 'GHZ Flower Stalk', 0x6B00 // 32, 'artnem')
	loadTilesOffset(dir, 'GHZ Purple Rock', 0x7A00 // 32, 'artnem')

	loadChunks(dir, 'GHZ')

	loadS1layoutFname(dir .. '/levels/' .. 'ghzbg.bin')
	layerNameInput:value('Green Hill Zone background')
	setLayerName(nil)

	lvlappend(nil)
	lvlsetlayer(2)
	loadS1layoutFname(dir .. '/levels/' .. 'ghz1.bin')
	layerNameInput:value('Green Hill Zone Act 1 foreground')
	setLayerName(nil)
end
