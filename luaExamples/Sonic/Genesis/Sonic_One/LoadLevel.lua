function loadPalette(rootDir, fname)
	local p = projects.current
	local paletteFolder = rootDir .. '/palette/'
	local pal = p.palette
	pal:load(paletteFolder .. 'Sonic.bin', rgt.tBinary, 0, compressionType.uncompressed)
	pal:load(paletteFolder .. fname, rgt.tBinary, 16, compressionType.uncompressed)
end

function loadTilemap(rootDir, levelName)
	local map16Fname = rootDir .. '/map16/' .. levelName .. '.bin'
	local f = filereader(endians.big, 2, "", false, 16, true, map16Fname, rgt.tBinary, compressionType.enigma)
	if f.amt > 0 then
		local tm = projects.current.tilemaps.current
		tm:setBlocksEnabled(false) -- Disable blocks and resize the tilemap to contain one block.
		tm:resize(2, 2) -- 16x16 pixel or 2x2 blocks.
		tm.useBlocks = true -- Set the flag for use blocks. This only sets the flag so using this alone is bad.
		tm:setBlocksEnabled(true) -- Ensure that any GUI updates take place. By first setting useBlocks we skip the GUI for asking how big the blocks should be.
		local blockData = f:dat(1)
		local blocksAmt = math.floor(#blockData / 8)
		tm:setBlocksAmt(blocksAmt)
		local tmTab = rgt.stringToTable16(blockData)
		local idx = 1
		for y = 1, blocksAmt * 2 do
			local tr = tm[y]
			for x = 1, 2 do
				local te = tr[x]
				local v = tmTab[idx]
				te.tile = bit32.band(v, 2047) + 1
				v = bit32.rshift(v, 11)
				te.hflip = bit32.band(v, 1) ~= 0
				te.vflip = bit32.band(v, 2) ~= 0
				v = bit32.rshift(v, 2)
				te.row = bit32.band(v, 3) + 1
				te.priority = bit32.band(v, 4) ~= 0

				idx = idx + 1
			end
		end
	end
end

function loadTiles(rootDir, levelName)
	local p = projects.current
	local artnemFolder = rootDir .. '/artnem/'
	local ct = p.tiles

	local tilesFname = artnemFolder .. '8x8 - ' .. levelName .. '.bin'
	if levelName == 'GHZ' then
		splitFname = artnemFolder .. '8x8 - GHZ1.bin'
		local sf = io.open(splitFname)
		if sf ~= nil then
			fltk.alert('GHZ is split. Please fix your dissembly so that it is not. After clicking OK it will be merged but you will still need to edit the code.')
			local tileString = rgt.ucharTableToString(mdcomp.nemesisDecompress(rgt.stringToTable(sf:read("*all"))))
			sf:close()
			local splitFname2 = artnemFolder .. '8x8 - GHZ2.bin'
			sf = io.open(splitFname2)
			ct.data = tileString .. rgt.ucharTableToString(mdcomp.nemesisDecompress(rgt.stringToTable(sf:read("*all"))))
			sf:close()
			os.remove(splitFname)
			os.remove(splitFname2)

			-- Retro Graphics Toolkit internally stores tile data in planar format to make supporting arbitrary bit depths and tile sizes easier.
			ct:toPlanar(tileTypes.linear)
			ct:save(tilesFname, rgt.tBinary, false, compressionType.nemesis)
			return
		end
	end
	local f = filereader(endians.native, 1, "", false, 16, true, tilesFname, rgt.tBinary, compressionType.nemesis)
	if f.amt > 0 then
		ct:setAmt(1)
		loadTilesNoGUI(f, projects.current, 0, 0, true, true)
	end
end

function loadTilesOffset(rootDir, fname, offset, folder)
	local cType = compressionType.nemesis
	if folder == 'artunc' then
		cType = compressionType.uncompressed
	end
	local f = filereader(endians.native, 1, "", false, 16, true, rootDir .. '/' .. folder ..'/' .. fname .. '.bin', rgt.tBinary, cType)
	if f.amt > 0 then
		loadTilesNoGUI(f, projects.current, offset, 0, true, true)
	end
end

function loadChunks(rootDir, levelName)
	local map256Fname = rootDir .. '/map256/' .. levelName .. '.bin'
	local f = filereader(endians.big, 2, "", false, 16, true, map256Fname, rgt.tBinary, compressionType.kosinski)
	if f.amt > 0 then
		local cc = projects.current.chunks
		cc:setWH(16, 16)
		cc.useBlocks = true
		cc.usePlane = projects.current.tilemaps.currentIdx

		local loadedData = f:dat(1)
		local chunkCnt = math.floor(#loadedData / 512)
		cc:setAmt(chunkCnt + 1) -- Chunk zero needs to be blank otherwise the level doesn't load right.
		
		local cTab = rgt.stringToTable16(loadedData)
		local idx = 1
		for c = 2, chunkCnt do
			local ch = cc[c]
			for y = 1, 16 do
				local cy = ch[y]
				for x = 1, 16 do
					local cx = cy[x]
					local v = cTab[idx]
					cx.block = bit32.band(v, 1023) + 1
					cx.flag = bit32.band(bit32.rshift(v, 11), 15) -- The format of the flags match the Sonic 1 format.
					
					idx = idx + 1
				end
			end
		end
	end
end
