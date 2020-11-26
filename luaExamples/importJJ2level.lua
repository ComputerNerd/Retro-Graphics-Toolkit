function strti(i)
	return i:byte(1,1)|(i:byte(2,2)<<8)|(i:byte(3,3)<<16)|(i:byte(4,4)<<24)
end
function strts(i)
	return i:byte(1,1)|(i:byte(2,2)<<8)
end
function strtb(i)
	return i:byte(1,1)
end
local p = projects.current
if p:have(project.levelMask | project.chunksMask | project.mapMask | project.tilesMask) then
	local fname=fl.file_chooser("Load j2l",'*.j2l')
	if not (fname == nil or fname == '') then
		local file=assert(io.open(fname,"rb"))
		local str=file:read('*a')
		--[[char Copyright[180]; (1,180)
		char Magic[4] = "LEVL"; (181,184)
		char PasswordHash[3]; // 0xBEBA00 for no password (185,187)
		char HideLevel; (188,188)
		char LevelName[32]; (189,220)
		short Version; (221,222)
		long FileSize; (223,226)
		long CRC32; (227,230)
		long CData1;            // compressed size of Data1 (231,234)
		long UData1;            // uncompressed size of Data1 (235,238)
		long CData2;            // compressed size of Data2 (239,242)
		long UData2;            // uncompressed size of Data2 (243,246)
		long CData3;            // compressed size of Data3 (247,250)
		long UData3;            // uncompressed size of Data3 (251,254)
		long CData4;            // compressed size of Data4 (255,258)
		long UData4;            // uncompressed size of Data4 (259,262)--]]
		io.close(file)
		if str:sub(181,184)~='LEVL' then
			fltk.alert('The magic signature is wrong')
		end
		print('Level name',str:sub(189,220))
		local Version = str:sub(221, 222)
		local Cdat1=strti(str:sub(231,234))
		local Cdat2=strti(str:sub(239,242))
		local Cdat3=strti(str:sub(247,250))
		local Udat3=strti(str:sub(251,254))
		local Cdat4=strti(str:sub(255,258))
		local Cdat4=strti(str:sub(255,258))
		local cd1=zlib.inflate()(str:sub(263,263+Cdat1))
		--[[  short JCSHorizontalOffset; // In pixels (1,2)
		short Security1; // 0xBA00 if passworded, 0x0000 otherwise (3,4)
		short JCSVerticalOffset; // In pixels (5,6)
		short Security2; // 0xBE00 if passworded, 0x0000 otherwise (7,8)
		char SecAndLayer; // Upper 4 bits are set if passworded, zero otherwise. Lower 4 bits represent the layer number as last saved in JCS. (9,9)
		char MinLight; // Multiply by 1.5625 to get value seen in JCS (10,10)
		char StartLight; // Multiply by 1.5625 to get value seen in JCS (11,11)
		short AnimCount; (12,13)
		bool VerticalSplitscreen; (14,14)
		bool IsLevelMultiplayer; (15,15)
		long BufferSize; (16,19)
		char LevelName[32]; (20,51)
		char Tileset[32]; (52,83)
		char BonusLevel[32]; (84,115)
		char NextLevel[32]; (116,147)
		char SecretLevel[32]; (148,179)
		char MusicFile[32]; (180,211)
		char HelpString[16][512]; (212,8403)
		char SoundEffectPointer[48][64]; // only in version 256 (AGA)
		long LayerMiscProperties[8]; // Each property is a bit in the following order: Tile Width, Tile Height, Limit Visible Region, Texture Mode, Parallax Stars. This leaves 27 (32-5) unused bits for each layer? (8404,8435)
		char "Type"[8]; // name from Michiel; function unknown (8436,8443)
		bool DoesLayerHaveAnyTiles[8]; // must always be set to true for layer 4, or JJ2 will crash (8444,8451)
		long LayerWidth[8]; (8452,8483)
		long LayerRealWidth[8]; // for when "Tile Width" is checked. The lowest common multiple of LayerWidth and 4. (8484,8515)
		long LayerHeight[8]; (8516,8547)
		long LayerZAxis[8] = {-300, -200, -100, 0, 100, 200, 300, 400}; // nothing happens when you change these (8548, 8579)
		char "DetailLevel"[8]; // is set to 02 for layer 5 in Battle1 and Battle3, but is 00 the rest of the time, at least for JJ2 levels. No clear effect of altering. Name from Michiel. (8580, 8587)
		int "WaveX"[8]; // name from Michiel; function unknown
		int "WaveY"[8]; // name from Michiel; function unknown
		long LayerXSpeed[8]; // Divide by 65536 to get value seen in JCS
		long LayerYSpeed[8]; // Divide by 65536 to get value seen in JCSvalue
		long LayerAutoXSpeed[8]; // Divide by 65536 to get value seen in JCS
		long LayerAutoYSpeed[8]; // Divide by 65536 to get value seen in JCS
		char LayerTextureMode[8];
		char LayerTextureParams[8][3]; // Red, Green, Blue
		short AnimOffset; // MAX_TILES minus AnimCount, also called StaticTiles
		long TilesetEvents[MAX_TILES]; // same format as in Data2, for tiles
		bool IsEachTileFlipped[MAX_TILES]; // set to 1 if a tile appears flipped anywhere in the level
		char TileTypes[MAX_TILES]; // translucent=1 or caption=4, basically. Doesn't work on animated tiles.
		char "XMask"[MAX_TILES]; // tested to equal all zeroes in almost 4000 different levels, and editing it has no appreciable effect.  // Name from Michiel, who claims it is totally unused.
		char UnknownAGA[32768]; // only in version 256 (AGA)
		Animated_Tile Anim[128]; // or [256] in TSF.
		// only the first [AnimCount] are needed; JCS will save all 128/256, but JJ2 will run your level either way.
		char Padding[512]; //all zeroes; only in levels saved with JCS--]]
		local AnimCount = strts(cd1:sub(12,13))
		print('Level name',cd1:sub(20,51))
		print('Tile set',cd1:sub(52,83))
		local valid,width,height,idx,validcnt,w4,inf,rw,i={},{},{},8404,0,{},{},{}
		for i=1,8 do
			inf[i]=strti(cd1:sub(idx,idx+3))
			print('Info layer:',i,'Tile width',inf[i]&1,'Tile height',(inf[i]>>1)&1,'Limit visible region',(inf[i]>>2)&1,'Texture mode',(inf[i]>>3)&1,'Parallax stars',(inf[i]>>4)&1,'inf',inf[i])
			idx=idx+4
		end
		idx=8444
		for i=1,8 do
			valid[i]=cd1:byte(idx,idx)~=0
			idx=idx+1
			print('Valid layer:',i,valid[i])
			if valid[i] then
				validcnt=validcnt+1
			end
		end
		print('validcnt',validcnt)
		for i=1,8 do
			width[i]=strti(cd1:sub(idx,idx+3))
			idx=idx+4
			print('Width layer:',i,width[i])
		end
		for i=1,8 do
			rw[i]=strti(cd1:sub(idx,idx+3))
			w4[i]=(rw[i]+3)//4
			idx=idx+4
			print('Real width layer:',i,rw[i])
		end
		for i=1,8 do
			height[i]=strti(cd1:sub(idx,idx+3))
			idx=idx+4
			print('Height layer:',i,height[i])
		end

		local currentLevel = p.level
		local levelLayers = currentLevel.layers

		levelLayers:amount(validcnt, false)

		basename = fl.filename_name(fname)

		local cnt = 1
		for i=1,8 do
			if valid[i] then
				local currentLayer = levelLayers[cnt]
				currentLayer.name = string.format("%s layer %d", basename, i)
				cnt = cnt + 1
			end
		end
		cnt = 1
		for i=1,8 do
			print(i)
			if valid[i] then
				local currentLayer = levelLayers[cnt]
				currentLayer:resize(w4[i], height[i])

				local info = currentLayer.info
				info.src = level.CHUNKS

				cnt = cnt + 1
			end
		end

		local chunks = p.chunks
		chunks:setWH(4,1)
		chunks:setAmt(Udat3/8)
		--TODO read data2 in order to work with parameters for sprite features

		-- Read chunks
		cd3=zlib.inflate()(str:sub(263+Cdat1+Cdat2,263+Cdat1+Cdat2+Cdat3))
		idx=1
		local flipBit
		if version == 513 then
			flipBit = 4096
		else
			flipBit = 1024
		end
		print('Maximum number of tiles', flipBit)
		tileMask = flipBit - 1
		for i=1,Udat3/8 do
			for j=1,4 do
				local tile=strts(cd3:sub(idx,idx+1))
				local chunkEntry = chunks[i][1][j]
				local flag
				if tile & flipBit ~= 0 then
					flag = 1
				else
					flag = 0
				end
				chunkEntry.flag = flag
				chunkEntry.block = tile & tileMask
				idx=idx+2
			end
		end
		cd4=zlib.inflate()(str:sub(263+Cdat1+Cdat2+Cdat3,263+Cdat1+Cdat2+Cdat3+Cdat4))
		idx=1

		cnt = 1

		for i=1,8 do
			print(i)
			if valid[i] then
				for y = 1, height[i] do
					for x = 1, w4[i] do
						local currentLayer = levelLayers[cnt]

						currentLayer[y][x].id = strts(cd4:sub(idx,idx+1))
						idx=idx+2
					end
				end
				cnt = cnt + 1
			end
		end
		project.update() -- Sync the level GUI with these changes.
	end
	local fname = fl.file_chooser("Load j2t", '*.j2t')
	if not (fname == nil or fname == '') then
		--[[struct TILE_Header {
		char Copyright[ 180]; (1,180)
		char Magic[ 4] = "TILE"; (181, 184)
		long Signature = 0xAFBEADDE (185, 188)
		char Title[ 32]; (189, 220)
		short Version;  //0x200 for v1.23 and below, 0x201 for v1.24 (221, 222)
		long FileSize; (223, 226)
		long CRC32; (227, 230)
		long CData1;    //compressed size of Data1 (231, 234)
		long UData1;    //uncompressed size of Data1 (235, 238)
		long CData2;    //compressed size of Data2 (239, 242)
		long UData2;    //uncompressed size of Data2 (243, 246)
		long CData3;    //compressed size of Data3 (247, 250)
		long UData3;    //uncompressed size of Data3 (251, 254)
		long CData4;    //compressed size of Data4 (255, 258)
		long UData4;    //uncompressed size of Data4  (259, 262)
		}--]]

		local file=assert(io.open(fname,"rb"))
		local str=file:read('*a')
		io.close(file)
		if str:sub(181,184)~='TILE' then
			fltk.alert('The magic signature is wrong')
		end
		local title = str:sub(189, 220)
		print('Title', title)
		local version = strts(str:sub(221, 222))
		local maxTiles
		if version == 512 then
			maxTiles = 1024
		elseif version == 513 then
			maxTiles = 4096
		else
			fltk.alert(string.format('Unknown version %d', version))
			return
		end
		print('Version', version)
		print('maxTiles', maxTiles)
		local CData1 = strti(str:sub(231, 234))
		local UData1 = strti(str:sub(235, 238))
		print('CData1', CData1, 'UData1', UData1)

		local CData2 = strti(str:sub(239, 242))
		local UData2 = strti(str:sub(243, 246))
		print('CData2', CData2, 'UData2', UData2)

		local CData3 = strti(str:sub(247, 250))
		local UData3 = strti(str:sub(251, 254))
		print('CData3', CData3, 'UData3', UData3)

		local CData4 = strti(str:sub(255, 258))
		local UData4 = strti(str:sub(259, 262))
		print('CData4', CData4, 'UData4', UData4)


		local d = zlib.inflate()(str:sub(263,263+CData1))
		--[[ struct TilesetInfo {
		long PaletteColor[ 256];        //arranged RGBA (1, 1024)
		long TileCount;                 //number of tiles, always a multiple of 10 (1025, 1028)
		char FullyOpaque[MAX_TILES];    //1 if no transparency at all, otherwise 0 Starts on 1029.
		char Unknown1[MAX_TILES];       //appears to be all zeros
		long ImageAddress[MAX_TILES];
		long Unknown2[MAX_TILES];       //appears to be all zeros
		long TMaskAddress[MAX_TILES];   //Transparency masking, for bitblt
		long Unknown3[MAX_TILES];       //appears to be all zeros
		long MaskAddress[MAX_TILES];    //Clipping or tile mask
		long FMaskAddress[MAX_TILES];   //flipped version of the above
		} --]]
		local PaletteColor = {}
		local rgbPalette = {}
		local rgbPalIdx = 1
		for i = 1, 1024, 4 do
			i4 = (i + 3) // 4
			PaletteColor[i4] = {}
			PaletteColor[i4][1] = strtb(d:sub(i, i))
			PaletteColor[i4][2] = strtb(d:sub(i + 1, i + 1))
			PaletteColor[i4][3] = strtb(d:sub(i + 2, i + 2))
			PaletteColor[i4][4] = strtb(d:sub(i + 3, i + 3))
			rgbPalette[rgbPalIdx] = PaletteColor[i4][1]
			rgbPalette[rgbPalIdx + 1] = PaletteColor[i4][2]
			rgbPalette[rgbPalIdx + 2] = PaletteColor[i4][3]
			rgbPalIdx = rgbPalIdx + 3
		end

		local colorCnt = #rgbPalette // 3
		p.palette:importRGB(rgbPalette, colorCnt, 0, colorCnt, 0, -1)

		local TileCount = strti(d:sub(1025, 1028))
		if TileCount > maxTiles then
			fltk.alert('TileCount > maxTiles')
			return
		end
		print('TileCount', TileCount)
		
		local offset = 1029 + maxTiles -- Skip FullyOpaque
		offset = offset + maxTiles -- Skip Unknown1
		local o = offset -- Temporary variable used for loops.
		tileLUT = {}
		for i = 1, TileCount do
			local tileIdx = strti(d:sub(o, o + 3))
			if tileIdx % 1024 ~= 0 then
				fltk.alert('tileIdx must be a multiple of 1024')
				return
			end
			tileLUT[i] = tileIdx // 1024
			o = o + 4
		end

		-- First ensure that we can create the tilemap with this code.
		-- The reason for these checks is because we currently don't handle cases where tiles are bigger than 32 pixels.
		-- This would require somehow merging the tiles. I am not aware of any use cases for this yet.
		local tiles = p.tiles
		if tiles.width > 32 or tiles.height > 32 then
			fltk.alert('Tiles must be 32 pixels or smaller in size')
			return
		end
		if 32 % tiles.width ~= 0 then
			fltk.alert('Not a multiple of 32')
			return
		end
		if 32 % tiles.height ~= 0 then
			fltk.alert('Not a multiple of 32')
			return
		end

		offset = offset + (maxTiles * 4) -- Seek past ImageAddress.
		offset = offset + (maxTiles * 4) -- Skip Unknown2.
		o = offset -- Temporary variable used for loops.
		trLut = {}
		for i = 1, TileCount do
			local tileIdx = strti(d:sub(o, o + 3))
			trLut[i] = tileIdx
			o = o + 4
		end

		-- Build the tilemap.
		-- First we need to setup the tilemap for blocks of 32 / tilewidth, 32 / tileheight size. Then set the number of blocks to TileCount.
		-- Afterwards we enable block mode then set the amount to TileCount.
		local tilemap = p.tilemaps[1]
		tilemap:setBlocksEnabled(false) -- Start with them disabled.
		local tw = 32 // tiles.width
		local th = 32 // tiles.height
		local downscale = math.floor(tonumber(fl.input(string.format('Downscale 1 to %d', tw))))
		if tw % downscale ~= 0 then
			fltk.alert('Invalid downscale.')
			return
		end
		local ds2 = downscale * downscale
		if 32 % downscale ~= 0 then
			fltk.alert('Invalid downscale. 32 % downscale must == 0.')
			return
		end
		tw = tw // downscale
		th = th // downscale
		tilemap:resize(tw, th)
		tilemap.useBlocks = true -- Set the flag for use blocks. This only sets the flag so using this alone is bad.
		tilemap:setBlocksEnabled(true) -- Ensure that any GUI updates take place. By first setting useBlocks we skip the GUI for asking how big the blocks should be.
		tilemap:setBlocksAmt(TileCount)
		p.chunks.useBlocks = true -- Make sure chunks are using blocks because block mode is now enabled.
		-- Now that we have the tilemap setup we can place the tiles.
		-- The tilemap is now 32 // tile.width, 32// tiles.height * TileCount in size. It is accessed per tile.
		for i = 0, TileCount - 1 do
			local ih = i * th -- Offset for height in terms of tiles.
			local it =  i * th * tw -- The tile that will be placed at the top of the tilemap.
			for y = 1, th do
				for x = 1, tw do
					it = it + 1
					tilemap[ih + y][x]:setFull(it)
				end
			end
		end
		

		d = zlib.inflate()(str:sub(263+CData1, 263+CData1 + CData2))
		if #d % 1024 ~= 0 then
			fltk.alert('Not a multiple of 1024')
			return
		end
		local tm = (32 // tiles.width // downscale) * (32 // tiles.height // downscale)
		tiles:setAmt(TileCount * tm)
		for j = 0, TileCount -1 do
			i = tileLUT[j + 1]
			local iof = i * 1024 + 1
			if downscale == 1 then
				for y = 0, 31 do
					for x = 0, 31 do
						local tileIdx = (j * tm) + (x // tiles.width) + (y // tiles.height * tw) + 1
						local palEnt = strtb(d:sub(iof, iof)) + 1
						local tileRGBA = tiles[tileIdx].rgba
						local tileRow = tileRGBA[y % tiles.height + 1]
						local tilePixel = tileRow[x % tiles.width + 1]
						tilePixel.r = PaletteColor[palEnt][1]
						tilePixel.g = PaletteColor[palEnt][2]
						tilePixel.b = PaletteColor[palEnt][3]
						tilePixel.a = 255 -- The alpha values in the palette seem to be invalid.
						iof = iof + 1
					end
				end
			else
				local fullresTile = {}
				for y = 1, 32 do
					fullresTile[y] = {}
					for x = 1, 32 do
						local palEnt = strtb(d:sub(iof, iof)) + 1
						fullresTile[y][x] = {}
						fullresTile[y][x][1] = PaletteColor[palEnt][1]
						fullresTile[y][x][2] = PaletteColor[palEnt][2]
						fullresTile[y][x][3] = PaletteColor[palEnt][3]
						iof = iof + 1
					end
				end
				for y = 0, 31, downscale do
					for x = 0, 31, downscale do
						-- Average the pixel values.
						local r = 0
						local g = 0
						local b = 0
						for yy = 1, downscale do
							for xx = 1, downscale do
								-- Average the pixels.
								r = r + fullresTile[y + yy][x + xx][1]
								g = g + fullresTile[y + yy][x + xx][2]
								b = b + fullresTile[y + yy][x + xx][3]
							end
						end
						local tileIdx = (j * tm) + (x // tiles.width // downscale) + (y // tiles.height // downscale * tw) + 1
						local tileRGBA = tiles[tileIdx].rgba
						local tileRow = tileRGBA[y // downscale % tiles.height + 1]
						local tilePixel = tileRow[x // downscale % tiles.width + 1]
						tilePixel.r = r // ds2
						tilePixel.g = g // ds2
						tilePixel.b = b // ds2
						tilePixel.a = 255
					end
				end
			end
		end

		if false then
			return
		end

		d = zlib.inflate()(str:sub(263 + CData1 + CData2, 263 + CData1 + CData2 + CData3))

		for j = 0, TileCount - 1 do
			local iof = trLut[j + 1]
			if downscale == 1 then
				for y = 0, 31 do
					for x = 0, 31 do
						if (x % 8 == 0) then
							iof = iof + 1
						end
						local tileIdx = (j * tm) + (x // tiles.width) + (y // tiles.height * tw) + 1
						local tileRGBA = tiles[tileIdx].rgba
						local tileRow = tileRGBA[y % tiles.height + 1]
						local tilePixel = tileRow[x % tiles.width + 1]

						local tVal = strtb(d:sub(iof, iof))
						local tMask = 1 << (x % 8)
						local aVal
						if tVal & tMask ~= 0 then
							aVal = 255
						else
							aVal = 0
						end

						tilePixel.a = aVal
					end
				end
			else
				local fullresAlpha = {}
				for y = 1, 32 do
					fullresAlpha[y] = {}
					for x = 0, 31 do
						if (x % 8 == 0) then
							iof = iof + 1
						end

						local tVal = strtb(d:sub(iof, iof))
						local tMask = 1 << (x % 8)
						local aVal
						if tVal & tMask ~= 0 then
							aVal = 255
						else
							aVal = 0
						end

						fullresAlpha[y][x + 1] = aVal
					end
				end
				for y = 0, 31, downscale do
					for x = 0, 31, downscale do
						-- Average the pixel values.
						local a = 0
						for yy = 1, downscale do
							for xx = 1, downscale do
								-- Average the alpha values.
								a = a + fullresAlpha[y + yy][x + xx]
							end
						end
						local tileIdx = (j * tm) + (x // tiles.width // downscale) + (y // tiles.height // downscale * tw) + 1
						local tileRGBA = tiles[tileIdx].rgba
						local tileRow = tileRGBA[y // downscale % tiles.height + 1]
						local tilePixel = tileRow[x // downscale % tiles.width + 1]

						tilePixel.a = a // ds2
					end
				end
			end
		end
	end
else
	p:haveMessage(project.levelMask | project.chunksMask | project.mapMask | project.tilesMask)
end
