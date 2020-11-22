local p = projects.current


if p:have(project.mapMask) then
	local tilemap = p.tilemaps.current
	tilemap:setBlocksEnabled(false)

	if p:have(projects.tileMask) then
		local tiles = p.tiles

		local imgSizePX = 4096 * 4096
		local tileSizePX = tiles.width * tiles.height

		-- Ensure that we can fit the image without padding.
		if 4096 % tiles.width ~= 0 then
			fltk.alert('tile width must be a multiple of 4096')
			return
		end
		if 4096 % tiles.height ~= 0 then
			fltk.alert('tile height must be a multiple of 4096')
			return
		end
		local tileWidth = 4096 // tiles.width
		local tileHeight = 4096 // tiles.height
		tilemap:resize(tileWidth, tileHeight)

		local nTiles = imgSizePX // tileSizePX
		tiles:setAmt(nTiles)

		-- Setup the tilemap.
		for j = 0, tileHeight - 1 do
			local row = tilemap[j + 1]
			for i = 1, tileWidth do
				row[i].tile = j * tileWidth + i
			end
		end
	else
		-- The tilemap is acting as a framebuffer.
		-- This feature is not yet implemented.
		tilemap:resize(4096, 4096)
	end

	-- Generate a 4096x4096 image.
	local img = {}
	local i
	local rgb = 0
	for i = 1, 4096 * 4096 * 3, 3 do
		img[i] = rgb & 255
		img[i + 1] = (rgb >> 8) & 255
		img[i + 2] = (rgb >> 16) & 255
		rgb = rgb + 1
	end
	tilemap:imageToTiles(img, -1, false, true)
else
	p:haveMessage(project.mapMask)
end

