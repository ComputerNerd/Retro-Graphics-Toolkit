-- The project is for constants and static functions.
-- The projects table is treated as an array.
local p = projects.current
if p:have(project.tilesMask) then
--[[ Although as of the time of writing this comment Retro Graphics Toolkit uses only tiles that use a palette
     meaning that having tiles implies a palette this could change latter
     that is I add tiles that do not use a palette. This is why the check for having a palette is needed.--]]
	if p:have(project.palMask) then 
		local index=1
		for gray=0,255,255/(p.palette:maxInRow(0)-1) do
			while p.palette[index].pType ~= 0 do
				index = index + 1;
			end
			local l = math.floor(gray)
			paletteEntry = p.palette[index]
			paletteEntry:setRGB(l, l, l)
			index = index + 1;
		end
		if p.palette.haveAlt == true then
			index=p.palette.cnt
			for gray=0,255,255/(p.palette:maxInRow(palette.rowCnt)-1) do
				while p.palette[index].pType ~= 0 do
					index = index + 1;
				end
				local l = math.floor(gray)
				paletteEntry = p.palette[index]
				paletteEntry:setRGB(l, l, l)
				index = index + 1;
			end
		end
	end
	for i=1, #p.tiles do
		local tile = p.tiles[i]
		for y=1, p.tiles.height do
			local row = tile.rgba[y]
			for x=1, p.tiles.width do
				local pixel = row[x]
				local gray = math.floor(0.2126 * pixel.r + 0.7152 * pixel.g + 0.0722 * pixel.b) -- BT.709
				pixel.r = gray
				pixel.g = gray
				pixel.b = gray
			end
		end
	end
	if p:have(project.mapMask) then
		-- Set all tiles to use row zero.
		local tilemap = p.tilemaps.current
		for j = 1, tilemap.hAll do
			for i = 1, tilemap.width do
				tilemap[j][i].row = 0
			end
		end
		tilemap:dither()
	end
	if p:have(project.spritesMask) then
		metasprites:ditherAll()
	end
	if p:have(project.palMask) then
		palette.fixSliders() --calls redraw
	else
		rgt.redraw()
	end
else
	p:haveMessage(project.tilesMask)
end
