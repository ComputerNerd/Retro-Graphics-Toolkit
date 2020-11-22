-- Sets all tiles to a gradient
local p = projects.current
if p:have(project.tilesMask+project.mapMask) then
	local tilemap = p.tilemaps.current
	local ret,r1,g1,b1=fl.color_chooser("First color")
	if ret~=0 then
		local ret,r2,g2,b2=fl.color_chooser("Second color")
		if ret~=0 then
			-- We need the same number of tiles as the tilemap height.
			p.tiles:setAmt(tilemap.height)
			
			-- Fill the tilemap.
			for j = 1, tilemap.height do
				for i = 1, tilemap.width do
					tilemap[j][i].tile = j -- CAUTION: in Lua arrays start with one.
				end
			end

			local rs,gs,bs
			if p:have(project.palMask) then
				local rt, gt, bt = r1, g1, b1
				local maxInRow = p.palette:maxInRow(0)
				rs = (r2 - r1) / maxInRow
				gs = (g2 - g1) / maxInRow
				bs = (b2 - b1) / maxInRow
				for i=1, maxInRow do

					local paletteEntry = p.palette[i]
					paletteEntry.r = math.floor(rt*255.) -- Must be an integer.
					paletteEntry.g = math.floor(gt*255.)
					paletteEntry.b = math.floor(bt*255.)
					paletteEntry:convertFromRGB() -- This must be called after setting .r, .g, .b

					rt = rt + rs
					gt = gt + gs
					bt = bt + bs

				end
				palette.fixSliders()
			end

			local tilemapHeightPixels = tilemap.hAll * p.tiles.height
			rs = (r2 - r1) / tilemapHeightPixels
			gs = (g2 - g1) / tilemapHeightPixels
			bs = (b2 - b1) / tilemapHeightPixels

			for t=1, #p.tiles do
				local tile = p.tiles[t]
				for i=1, p.tiles.height do
					local row = tile.rgba[i]
					for j=1, p.tiles.width do
						local pixel = row[j] -- We can access each value like an array or like a struct.
						pixel.r = math.floor(r1 * 255.)
						pixel.g = math.floor(g1 * 255.)
						pixel.b = math.floor(b1 * 255.)
						pixel.a = 255
					end
					r1 = r1 + rs
					g1 = g1 + gs
					b1 = b1 + bs
					
				end
			end
		end
	end
else
	project.haveMessage(project.tilesMask+project.mapMask)
end
