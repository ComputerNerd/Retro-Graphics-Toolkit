-- The purpose of this script is to import a tile/block sets where each tile/block is a separate image.
-- Constraints for this program:
-- 	1. All images are the same size (in pixels).
-- 	2. The width and height of the image is a multiple of the width and the height of one tile.
-- 	3. All images end in .png and contain an alpha channel if transparency is needed. This code does not support a mask color for transparency.
-- 	4. The project must have tiles and tilemaps enabled.
-- For constraint #1 images that contain a different width and height as the first opened image will be skipped.
-- For constraint #2 an error will be displayed and the program will exit.
-- For constraint #3 only images ending in .png will be loaded by the program. If no images are found an error message is displayed. For mixed directories (Ex: it has both PNGs and JPGs) warnings will be displayed for files that don't end in .png.

-- First make sure that tiles and tilemaps are enabled.
local p = projects.current
local haveMask = project.tilesMask | project.mapMask
if p:have(haveMask) then
	local tiles = p.tiles
	local tilemap = p.tilemaps.current
	local tileWidth = tiles.width
	local tileHeight = tiles.height

	local shouldAppend = fl.ask('Append blocks?')
	if shouldAppend and (not tilemap.useBlocks) then
		fltk.alert('Blocks must be enabled when using append mode.')
		return
	end

	local selectedDirectory = fl.dir_chooser()
	if selectedDirectory ~= nil and selectedDirectory ~= '' then
		local filesIterator = dirent.files(selectedDirectory)
		local startTileQty
		if shouldAppend then
			startTileQty = #tiles
		else
			startTileQty = 0
		end
		local tileQty = startTileQty
		local currentTile = tileQty + 1
		local expectedImageWidth, expectedImageHeight
		local isFirstTile
		local tilesPerBlock
		local tilesX, tilesY
		if shouldAppend then
			tilesX = tilemap.width
			tilesY = tilemap.height -- tilemap.height is the height of one block in tiles.
			tilesPerBlock = tilesX * tilesY
			expectedImageWidth = tilesX * tileWidth
			expectedImageHeight = tilesY * tileHeight
			isFirstTile = false
		else
			isFirstTile = true
		end
		local blocksLoaded = 0
		for fn in filesIterator do
			if fn ~= '.' and fn ~= '..' then
				-- Check the extension and ensure that it ends with .png
				local filenameExt = fltk.filename_ext(fn)
				if filenameExt == '.png' then
					local fullPath = selectedDirectory .. '/' .. fn
					local image = fltk.png_image(fullPath)
					local depth = image:d()
					if depth == 3 or depth == 4 then
						-- Convert the image to tiles.
						local width = image:w()
						local height = image:h()
						if ((width % tileWidth) ~= 0) or ((height % tileHeight) ~= 0) then
							print('The image must be a multiple of the tile width and tile height.')
						else
							local canLoadImage = true
							if isFirstTile then
								expectedImageWidth = width
								expectedImageHeight = height
								tilesX = width // tileWidth
								tilesY = height // tileHeight
								tilesPerBlock = tilesX * tilesY
							else
								if (width ~= expectedImageWidth) or (height ~= expectedImageHeight) then
									canLoadImage = false
								end
							end
							if canLoadImage then
								-- All tests have passed. This means we can process the image.
								-- First allocate memory for the new tiles.
								tileQty = tileQty + tilesPerBlock -- Instead of #tiles which would result in appending tiles. This program overwrites tiles.
								tiles:setAmt(tileQty)

								-- Get the image data
								local data = image:data()
								if #data ~= (width * height * depth) then
									print('Wrong size.', #data, width * height * depth)
								end
								local dataIdx = 1
								for y = 0, height - 1 do
									for x = 0, width -1 do
										local tileIdx = (y // tileHeight * tilesX) + (x // tileWidth) + currentTile
										local tileRGBA = tiles[tileIdx].rgba
										local tileRow = tileRGBA[y % tileHeight + 1]
										local tilePixel = tileRow[x % tileWidth + 1]
										tilePixel.r = data:byte(dataIdx)
										tilePixel.g = data:byte(dataIdx + 1)
										tilePixel.b = data:byte(dataIdx + 2)
										if depth == 4 then
											tilePixel.a = data:byte(dataIdx + 3)
										else
											tilePixel.a = 255
										end
										dataIdx = dataIdx + depth
									end
								end
								currentTile = currentTile + tilesPerBlock
								blocksLoaded = blocksLoaded + 1
							end
						end
									
					else
						print('Invalid depth', depth)
					end
				else
					print('Warning: filename ' .. fn .. ' does not end in .png.')
				end
			end
		end
		-- Build the tilemap.
		-- First setup the tilemap to use blocks.
		local yOffset
		if shouldAppend then
			yOffset = tilemap.hAll
		else
			yOffset = 0
		end
		if shouldAppend then
			local oldBlockCount = tilemap.hAll // tilemap.height
			tilemap:setBlocksAmt(blocksLoaded + oldBlockCount)
		else
			tilemap:setBlocksEnabled(false) -- Disable blocks and resize the tilemap to contain one block.
			tilemap:resize(tilesX, tilesY) -- This sets the size of one block.
			tilemap.useBlocks = true -- Set the flag for use blocks. This only sets the flag so using this alone is bad.
			tilemap:setBlocksEnabled(true) -- Ensure that any GUI updates take place. By first setting useBlocks we skip the GUI for asking how big the blocks should be.
			tilemap:setBlocksAmt(blocksLoaded)
		end
		currentTile = startTileQty
		for y = 1, blocksLoaded * tilesY do
			local tilemapRow = tilemap[y + yOffset]
			for x = 1, tilesX do
				if tilemapRow == nil then
					print('tilemapRow is nil', y)
				else
					tilemapRow[x]:setFull(currentTile) -- Clear all attributes except which tile is selected. This ensures that no tiles are accidently flipped.
				end
				currentTile = currentTile + 1
			end
		end

	end
else
	p:haveMessage(haveMask)
end
