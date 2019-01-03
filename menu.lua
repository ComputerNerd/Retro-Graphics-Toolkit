--[[
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
	Copyright Sega16 (or whatever you wish to call me) (2012-2018)
--]]
function userGuide(userData)--callback functions **must** have the user data (name does not matter) parameter and only that.
	fl.alert("The user's guide can be found on the wiki https://github.com/ComputerNerd/Retro-Graphics-Toolkit/wiki or locally in the Manual folder");
end
function allMetaDither(unused)
	local p = projects.current
	print(p.metasprites)

	if p:have(project.spritesMask)==true then
		p.metasprites:dither()
	else
		project.haveMessage(project.spritesMask)
	end
end
function removeDuplicateBlocks(unused)
	rgt.syncProject()
	local p = projects.current
	if p:have(project.mapMask) then
		local tilemaps = p.tilemaps
		for i=1, #tilemaps do
			local tilemap = tilemaps[i]
			if tilemap.useBlocks then
				local a=0
				while a<tilemap.hAll do
					local b = tilemap.hAll - tilemap.height
					while b>=0 do
						if a~=b then
							local equ=true
							for j=1, tilemap.height do
								local tr1 = tilemap[a + j]
								local tr2 = tilemap[b + j]
								for k=1, tilemap.width do
									local r1 = tr1[k].raw
									local r2 = tr2[k].raw
									if r1 ~= r2 then
										equ = false
										break
									end
								end
								if not equ then
									break
								end
							end
							if equ then
								local aa,bb=a//tilemap.height, b//tilemap.height
								print('Found match', aa, bb)
								tilemap:removeBlock(bb)
								if p:have(project.chunksMask) and i == p.chunks.usePlane then
									p.chunks:subBlock(bb,aa)
								end
								if p:have(project.levelMask) then
									p.level:subType(bb, aa, level.BLOCKS, i)
								end
							end
						end
						b = b - tilemap.height
					end
					a = a + tilemap.height
				end
			else
				print(string.format('Skipping plane %d -- it does not use blocks', i))
			end
		end
	else
		p:haveMessage(project.mapMast)
	end
end
function generateMenu()
--[[ This function shall return a table containing menu items that will be added to the main menu.
--   The table shall be a two dimensional array. The first dimension specifies the amount of menu items.
--   Per menu item the length of the array is variable with up to five indices that follow the form of:
     {text,shortcut,callback (string for a Lua function or an integer greater than zero for a C function) an integer with a value of zero signifies no callback,callback data (integer),flags}
     If the last n indices are not filled in they will be replaced with zero.
--]]
	return {
	{"File",0,0,0,FL.SUBMENU},
		{"Tiles",0,0,0,FL.SUBMENU},
			{"Open tiles",0,1--[[load_tiles--]]},
			{"Open tiles (append)",0,1--[[load_tiles--]],1},
			{"Open tiles starting at",0,1--[[load_tiles--]],2},
			{"Open truecolor tiles",0,2--[[load_truecolor_tiles--]]},
			{"Save tiles",0,3--[[save_tiles--]]},
			{"Save truecolor tiles",0,4--[[save_tiles_truecolor--]]},
			{"Import image to tiles",0,5--[[load_image_to_tilemap--]],2},
			{},-- This signifies the end of the submenu
		{"Palette",0,0,0,FL.SUBMENU},
			{"Open palette",0,6--[[loadPalette--]]},
			{"Save palette",0,7--[[save_palette--]]},
			{},
		{"Tile map",0,0,0,FL.SUBMENU},
			{"Import tile map or blocks and if NES attributes",0,8--[[load_tile_map--]],0},
			{"Import image to tile map",0,5--[[load_image_to_tilemap--]],0},
			{"Import image over current tile map",0,5--[[load_image_to_tilemap--]],1},
			{"Export tile map as image",0,9--[[save_tilemap_as_image--]],0},
			{"Export tile map as with system color space",0,10--[[save_tilemap_as_colspace--]],0},
			{"Export tile map and if NES attributes",0,11--[[save_map--]],0},
			{},
		{"Projects",0,0,0,FL.SUBMENU},
			{"Load project group",FL.CTRL+string.byte('o'),14--[[loadAllProjectsCB--]],0},
			{"Save project group",FL.CTRL+string.byte('s'),15--[[saveAllProjectsCB--]],0},
			{"Load project",FL.CTRL+FL.SHIFT+string.byte('o'),12--[[loadProjectCB--]],0},
			{"Save project",FL.CTRL+FL.SHIFT+string.byte('s'),13--[[saveProjectCB--]],0},
			{},
		{"Chunks",0,0,0,FL.SUBMENU},
			{"Import sonic 1 chunks",0,16--[[ImportS1CBChunks--]],0},
			{"Import sonic 1 chunks (append)",0,16--[[ImportS1CBChunks--]],1},
			{"Export chunks as sonic 1 format",0,17--[[saveChunkS1CB--]]},
			{},
		{"Sprites",0,0,0,FL.SUBMENU},
			{"Import sprite from image",0,18--[[SpriteimportCB--]],0},
			{"Import sprite from image (append)",0,18--[[SpriteimportCB--]],1},
			{"Import sprite sheet",0,19--[[spriteSheetimportCB--]],1},
			{"Import mapping",0,0,0,FL.SUBMENU},
				{"Sonic 1",0,20--[[importSonicMappingCB--]],0},
				{"Sonic 2",0,20--[[importSonicMappingCB--]],1},
				{"Sonic 3",0,20--[[importSonicMappingCB--]],2},
				{},
			{"Import DPLC",0,0,0,FL.SUBMENU},
				{"Sonic 1",0,21--[[importSonicDPLCCB--]],0},
				{"Sonic 2 (or sonic 3 character)",0,21--[[importSonicDPLCCB--]],1},
				{"Sonic 3",0,21--[[importSonicDPLCCB--]],2},
				{},
			{"Export mapping",0,0,0,FL.SUBMENU},
				{"Sonic 1",0,22--[[exportSonicMappingCB--]],0},
				{"Sonic 2",0,22--[[exportSonicMappingCB--]],1},
				{"Sonic 3",0,22--[[exportSonicMappingCB--]],2},
				{},
			{"Export DPLC",0,0,0,FL.SUBMENU},
				{"Sonic 1",0,23--[[exportSonicDPLCCB--]],0},
				{"Sonic 2 (or sonic 3 character)",0,23--[[exportSonicDPLCCB--]],1},
				{"Sonic 3",0,23--[[exportSonicDPLCCB--]],2},
				{},
			{},
		{"Levels",0,0,0,FL.SUBMENU},
			{'Import Sonic One level chunk layout',0,'loadS1layout'},
			{'Export Sonic One level chunk layout',0,'saveS1layout'},
			{},
		{"Scripts",0,0,0,FL.SUBMENU},
			{"Run Lua script",FL.CTRL+string.byte('r'),24--[[runLuaCB--]],0},
			{},
		{},
	{"Palette actions",0,0,0,FL.SUBMENU},
		{"Clear entire Palette",0,26--[[clearPalette--]],0},
		{"Pick nearest color algorithm",0,27--[[pickNearAlg--]],0},
		{"RGB color to entry",0,28--[[rgb_pal_to_entry--]],0},
		{"Entry to RGB color",0,29--[[entryToRgb--]],0},
		{"Sort each row by",0,30--[[sortRowbyCB--]],0},
		{},
	{"Tile actions",0,0,0,FL.SUBMENU},
		{"Append blank tile to end of buffer",0,31--[[new_tile--]],0},
		{"Fill tile with selected color",0,32--[[fill_tile--]],0},
		{"Fill tile with color 0",0,33--[[blank_tile--]],0},
		{"Remove duplicate truecolor tiles",0,34--[[remove_duplicate_truecolor--]],0},
		{"Remove duplicate tiles",0,35--[[remove_duplicate_tiles--]],0},
		{"Update dither all tiles",0,36--[[update_all_tiles--]],0},
		{"Delete currently selected tile",0,37--[[delete_tile_at_location--]],0},
		{"Create new tiles for flipped tiles",0,38--[[tilesnewfilppedCB--]]},
		{},
	{"Tilemap actions",0,0,0,FL.SUBMENU},
		{"Generate optimal palette with x amount of colors using the tilemap",0,25--[[generate_optimal_palette--]],0},
		{"Fix tile delete on tile map",0,39--[[tilemap_remove_callback--]],0},
		{"Toggle true color Viewing (defaults to off)",0,40--[[trueColTileToggle--]],0},
		{"Pick tile row based on color delta",0,41--[[tileDPicker--]],0},
		{"Auto determine if use shadow highlight",0,42--[[shadow_highligh_findout--]],0},
		{"Dither tile map as image",0,43--[[dither_tilemap_as_imageCB--]],0},
		{"Fill tile map with selection including attributes",0,44--[[fill_tile_map_with_tile--]],0},
		{"Fix out of range tiles (replace with current attributes in plane editor)",0,45--[[FixOutOfRangeCB--]],0},
		{"Pick extended attributes",0,46--[[pickExtAttrsCB--]],0},
		{'Remove duplicate blocks',0,'removeDuplicateBlocks'},
		{},
	{"Sprite actions",0,0,0,FL.SUBMENU},
		{"Generate optimal palette for selected sprite",0,25--[[generate_optimal_palette--]],1},
		{"Dither sprite group as image",0,47--[[ditherSpriteAsImageCB--]],0},
		{"Dither meta-sprite as image",0,48--[[ditherSpriteAsImageAllCB--]],0},
		{"Dither all meta-sprites as image",0,"allMetaDither",0},
		{"Remove blank and duplicate tiles without affect sprite amount",0,49--[[optimizeSpritesCB--]],0},
		{"Set all sprites with same start tile to currently selected palette row",0,50--[[palRowstCB--]],0},
		{},
	{"Undo/Redo",0,0,0,FL.SUBMENU},
		{"Undo",FL.CTRL+string.byte('z'),51--[[undoCB--]]},
		{"Redo",FL.CTRL+string.byte('y'),52--[[redoCB--]]},
		{"Show history window",FL.CTRL+string.byte('h'),53--[[historyWindow--]]},
		{"Clear undo buffer",0,54--[[clearUndoCB--]]},
		{},
	{"Help",0,0,0,FL.SUBMENU},
		{"About",0,55--[[showAbout--]]},
		{"User's guide",0,'userGuide'},
		{},
	{},
}
 
end
--print('The code here will be ran BEFORE generateMenu')
