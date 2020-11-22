function setOffsetAppendTiles(src, val)
	val[1]:value(#val[2])
end

function okCancelCBTiles(src, val)
	val[1][1] = val[2]
	settingsWindow:hide()
end

function checkTilesFreader(f, prj)

	if f.amt <= 0 then -- The user didn't select any files.
		return false
	end
	local fsRem = f:lens(1) % prj.tiles.tileSize
	if fsRem ~= 0 then
		fltk.alert(string.format('Invalid tile data. The decompressed size or file size if uncompressed should be a multiple of %d but the filesize is %d.', prj.tiles.tileSize, f:lens(1)))
		return false
	end
	return true
end

function loadTilesNoGUI(f, prj, offset, defaultRow, getRowFromTilemap, useAlphaZeroMode)
	-- We have to check these again because this function may be used directly instead of being called with loadTilesWithFileReader. The reason for the precheck in loadTilesWithFileReader is so we don't make the user fill out the GUI only and then get rejected later.
	if not checkTilesFreader(f, prj) then
		return
	end

	local ct = prj.tiles

	local offset = math.floor(offset)
	-- Arrays start with one in Lua so one is the first tile.
	offset = offset + 1
	local defaultRow = math.floor(defaultRow)

	local loadedTileData = f:dat(1)
	local tilesLoaded = #loadedTileData // ct.tileSize
	ct:assignData(prj.tileType, offset, loadedTileData)
	local rowMapping = {}
	if getRowFromTilemap then
		for pl = 1, #prj.tilemaps do
			local tm = prj.tilemaps[pl]
			for y = 1, #tm do
				local tmr = tm[y]
				for x = 1, #tmr do
					local tme = tmr[x]
					rowMapping[tme.tile] = tme.row
				end
			end

		end
	end

	for t = offset, offset + tilesLoaded - 1 do
		local r = rowMapping[t]
		if r == nil then
			r = defaultRow
		end
		ct[t]:convertToRGBA(r, useAlphaZeroMode)
	end
end

function setCBTable(src, val)
	val[1] = src:value()
end

function loadTilesWithFileReader(f, prj)
	if not checkTilesFreader(f, prj) then
		return
	end
	-- Create the GUI to configure the tile loading process.
	settingsWindow = fltk.window(320, 240)
	settingsWindow:set_modal()
	local tileOffsetCounter = fltk.counter(10, 10, 192, 32, "Load at offset (starting at zero)")
	tileOffsetCounter:step(1, 10)
	tileOffsetCounter:minimum(0)

	local appendBtn = fltk.button(10, 64, 192, 32, "Set offset to after last tile.")
	appendBtn:callback(setOffsetAppendTiles, {tileOffsetCounter, prj.tiles})

	local palRowSel = fltk.simple_counter(10, 100, 224, 32, "Default palette row (starting at zero)")
	palRowSel:range(0, prj.palette.rowCnt - 1)
	palRowSel:step(1)

	local shouldGetRowFromTilemap = {false}
	if prj:have(project.mapMask) then
		shouldGetRowFromTilemap[1] = true
		local useTilemapBtn = fltk.check_button(10, 148, 288, 24, "Use tilemap data to choose a palette row?")
		useTilemapBtn:callback(setCBTable, shouldGetRowFromTilemap)
		useTilemapBtn:value(true)
	end

	local shouldSetZeroToAlphaZero = {true}
	local useAlphaZeroBtn = fltk.check_button(10, 172, 288, 24, 'Set color zero to alpha zero?')
	useAlphaZeroBtn:callback(setCBTable, shouldSetZeroToAlphaZero)
	useAlphaZeroBtn:value(true)

	local isOK = {false}
	local okBtn = fltk.button(10, 204, 128, 24, "OK")
	okBtn:callback(okCancelCBTiles, {isOK, true})
	local cancelBtn = fltk.button(142, 204, 128, 24, "Cancel")
	cancelBtn:callback(okCancelCBTiles, {isOK, false})

	settingsWindow:done()
	settingsWindow:show()
	while settingsWindow:shown() do
		Fl.wait()
	end
	if isOK[1] then
		loadTilesNoGUI(f, prj, tileOffsetCounter:value(), palRowSel:value(), shouldGetRowFromTilemap[1], shouldSetZeroToAlphaZero[1])
	end
end

function loadTilesAsk(prj)
	-- For one byte per element we can always use native endian because no transformation is needed in any case.
	local f = filereader(endians.native, 1, "Load Tiles")
	loadTilesWithFileReader(f, prj)
end

function loadTilesCB(unused)
	loadTilesAsk(projects.current)
end
