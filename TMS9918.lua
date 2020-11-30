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
	Copyright Sega16 (or whatever you wish to call me) (2012-2020)
--]]

function tms9918Graphics1RemapTiles(projectIDX, attrsByTile, forceKeepAllUnique)
	local bgColLookFor = bit32.band(projects[projectIDX].palColTMS9918, 15)
	-- attrsByTile starts with a table in the form of attrsByTile[attr] = {list of tiles with this attribute}
	-- Force keep all means we should scan all true color tiles and keep all unique true color tiles.
	
	-- Create a new table in the form of sortedTiles[attr][rgbaData] = {list of old indices}
	local ct = projects[projectIDX].tiles
	local sortedTiles = {}
	-- First start with the forced attributes.
	
	local blankRGBAtile = string.rep('\0', ct.tcSize)

	local tilesAddedToList = {} -- Keep track of tiles added to the list.
	local blankTileSet = {}

	local function processAttr(atr)
		local bgcol = bit32.band(atr, 15)
		local fgcol = bit32.band(bit32.rshift(atr, 4), 15)
		if bgcol == bgColLookFor then
			bgcol = 0
		end
		if fgcol == bgColLookFor then
			fgcol = 0
		end
		local mic = math.min(bgcol, fgcol)
		local mxc = math.max(bgcol, fgcol)
		return bit32.bor(mic, bit32.lshift(mxc, 4))
	end

	for attr, tileIndices in pairs(attrsByTile) do
		attr = processAttr(attr)
		for k, tileIdx in ipairs(tileIndices) do
			local rgbaData = ct[tileIdx].rgbaData
			if rgbaData == blankRGBAtile then
				blankTileSet[tileIdx] = true
			else
				if sortedTiles[attr] == nil then
					sortedTiles[attr] = {}
				end
				if sortedTiles[attr][rgbaData] == nil then
					sortedTiles[attr][rgbaData] = {}
				end
				table.insert(sortedTiles[attr][rgbaData], tileIdx)
			end
			tilesAddedToList[tileIdx] = true
		end
	end
	if forceKeepAllUnique then
		-- Then add any tiles not already in sortedTiles.
		for tileIdx = 1, #ct do
			if tilesAddedToList[tileIdx] == nil then
				local t = ct[tileIdx]
				local attr = processAttr(t.pixels[1]:getExtAttr()) -- The reason it's part of pixels is for Graphics II mode so it can be indexed by a y value. In Graphics I mode getExtAttr will return the same value regardless of the y value.

				local rgbaData = t.rgbaData
				if rgbaData == blankRGBAtile then
					blankTileSet[tileIdx] = true
				else
					if sortedTiles[attr] == nil then
						sortedTiles[attr] = {}
					end
					if sortedTiles[attr][rgbaData] == nil then
						sortedTiles[attr][rgbaData] = {}
					end
					table.insert(sortedTiles[attr][rgbaData], tileIdx)
				end
			end
		end
	end
	-- The last step is to build the final list.
	-- We create two tables, one containing the extended attributes and one containing the true color and regular tile data.
	tilesFinal = {}
	local blankTile = string.rep('\0', ct.tileSize)

	local blankTileList = {}
	for k, v in pairs(blankTileSet) do
		table.insert(blankTileList, k)
	end

	local hasBlankTile = false
	for attr, rgbaDataList in pairs(sortedTiles) do
		local tileCount = 0
		for rgbaData, oldTileIndices in pairs(rgbaDataList) do
			tileCount = tileCount + 1
			table.insert(tilesFinal, {attr, rgbaData, ct[oldTileIndices[1]].data, oldTileIndices})
		end
		local tilesRemaining = tileCount % 8
		if tilesRemaining > 0 then
			local tilesNeeded = 8 - tilesRemaining
			for pi = 1, tilesNeeded do
				hasBlankTile = true
				table.insert(tilesFinal, {attr, blankRGBAtile, blankTile, blankTileList})
			end
		end
	end
	if not hasBlankTile then
		if projects[projectIDX]:have(project.pjHaveMap) then
			-- See if we need a blank tile. If the tilemap is not using a blank tile then we don't need it.
			local ctms = projects[projectIDX].tilemaps
			for tmi = 1, #ctms do
				local ctm = ctms[tmi]
				for y = 1, #ctm do
					local cty = ctm[y]
					for x = 1, #cty do
						local ctx = cty[x]
						if blankTileSet[ctx.tile] ~= nil then
							hasBlankTile = true
						end
					end
				end
			end

			if hasBlankTile then
				for pi = 1, 8 do
					table.insert(tilesFinal, {bgColLookFor, blankRGBAtile, blankTile, blankTileList})
				end
			end
		end
	end
	local function sortTileList(a, b)
		if a[1] == b[1] then
			if a[2] == b[2] then
				return a[3] < b[3]
			else
				return a[2] < b[2]
			end
		else
			return a[1] < b[1]
		end
	end
	table.sort(tilesFinal, sortTileList)

	local oldTileIdxToNew = {}
	ct:setAmt(#tilesFinal)
	for newTileIdx, tileInfo in ipairs(tilesFinal) do
		local t = ct[newTileIdx]
		t.rgbaData = tileInfo[2]
		t.data = tileInfo[3]
		for unused, oldIdx in ipairs(tileInfo[4]) do
			oldTileIdxToNew[oldIdx] = newTileIdx
		end
	end

	-- Attempt to make the blank tiles match the background color.
	for newTileIdx, tileInfo in ipairs(tilesFinal) do
		local bgColFound = bit32.band(tileInfo[1], 15)
		if bgColLookFor == bgColFound then
			if tileInfo[2] == blankRGBAtile then
				for bi = 1, #blankTileList do
					oldTileIdxToNew[blankTileList[bi]] = newTileIdx
				end
				break
			end
		end
	end

	extAttrsFinal = {}
	for ti = 8, #tilesFinal, 8 do
		extAttrsFinal[math.floor(ti / 8)] = string.char(tilesFinal[ti][1])
	end
	ct.extAttrs = table.concat(extAttrsFinal)

	if projects[projectIDX]:have(project.pjHaveMap) then
		local ctms = projects[projectIDX].tilemaps
		for tmi = 1, #ctms do
			local ctm = ctms[tmi]
			for y = 1, #ctm do
				local cty = ctm[y]
				for x = 1, #cty do
					local ctx = cty[x]
					if oldTileIdxToNew[ctx.tile] ~= nil then
						ctx.tile = oldTileIdxToNew[ctx.tile]
					end
				end
			end
		end
	end
end
