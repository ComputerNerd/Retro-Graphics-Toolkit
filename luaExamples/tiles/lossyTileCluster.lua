-- Start by removing any duplicate true color tiles.

local progressWindow = fltk.window(400, 45, "Sorting Tiles")
local progress = fltk.progress(25, 7, 350, 30, "Please wait")
progress:color(0x88888800)   -- background color
progress:selection_color(0x4444ff00) -- progress bar color
progress:labelcolor(fltk.WHITE)-- percent text color
progressWindow:done()
progressWindow:show()

local p = projects.current
local cts = p.tiles

local trueColorTiles = {} -- trueColorTiles[rgbaData] = {oldIndices set}
progress:minimum(1)
progress:maximum(#cts)
for i = 1, #cts do
	local rgbaData = cts[i].rgbaData
	if trueColorTiles[rgbaData] == nil then
		trueColorTiles[rgbaData] = {}
	end
	trueColorTiles[rgbaData][i] = true
	progress:value(i)
	fltk.check()
end

-- Now convert it to an array can iterate through it in a predictable manner.

trueColorTileList = {}
for rgbaData, oldIndices in pairs(trueColorTiles) do
	table.insert(trueColorTileList, {rgbaData, oldIndices})
end

function sortByFirstElm(a, b)
	return a[1] < b[1]
end

table.sort(trueColorTileList, sortByFirstElm)

-- Now assign the true color tiles to the tilemap. This simplifies the code because we don't have to look at two indices list later on.
local oldTileIdxToNew = {}
local assignQue = {}
local newIdx = 1
for kunused, tileInfo in pairs(trueColorTileList) do
	local tIdx = nil
	for oldIdx, valUnused in pairs(tileInfo[2]) do
		oldTileIdxToNew[oldIdx] = newIdx
		tIdx = oldIdx
	end
	local ct = cts[tIdx]
	table.insert(assignQue, {ct.rgbaData, ct.data})
	newIdx = newIdx + 1
end

function doAssignQue()
	cts:setAmt(#assignQue)
	print('Set tile amount to', #assignQue)

	for k, v in ipairs(assignQue) do
		local ct = cts[k]
		ct.rgbaData = v[1]
		ct.data = v[2]
	end

	if p:have(project.pjHaveMap) then
		local ctms = p.tilemaps
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
doAssignQue()

-- At this point trueColorTileList and the current tile data match so all we need to do is use the index in trueColorTileList instead of looking at oldIndices.

differenceByTileCombo = {}
progress:minimum(1)
progress:maximum(#trueColorTileList)
progress:label("Comparing tiles")
local rgbaByTileByChannel = {}
for i = 1, #trueColorTileList do
	local t = cts[i]
	rgbaByTileByChannel[i] = {}
	for c = 1, 4 do
		rgbaByTileByChannel[i][c] = t:rgbaGetChannel(c)
	end
end
for i = 1, #trueColorTileList do
	for j = 1, #trueColorTileList do
		if i > j then
			local ssimMin = 1.0
			for c = 1, 4 do
				ssimMin = math.min(ssimMin, iqa.ssim(rgbaByTileByChannel[i][c], rgbaByTileByChannel[j][c], cts.width, cts.height, cts.width, 0, nil))
			end
			-- local dif = iqa.mse(trueColorTileList[i][1], trueColorTileList[j][1], cts.width, cts.height, cts.width)
			if differenceByTileCombo[i] == nil then
				differenceByTileCombo[i] = {}
			end
			differenceByTileCombo[i][j] = ssimMin
		end
	end
	progress:value(i)
	fltk.check()
end
function sortMostSimilarFirst(a, b)
	return a[1] > b[1] -- for SSIM the closer to one the most similar
	-- return a[1] < b[1] -- For MSE the smaller the number the closer it is.
end


local tileClusters = {}

function clusterTiles(threshold)
	progressWindow:show()
	progress:label("Clustering tiles")
	progress:minimum(1)
	progress:maximum(#trueColorTileList)
	tileClusters = {} -- tileClusters[idx] = {list of indices in trueColorTileList}
	local usedTile = {}
	for ti = 1, #trueColorTileList do
		table.insert(usedTile, false)
	end
	for ti = 1, #trueColorTileList do -- differenceByTileCombo[1][2] doesn't exist because we have differenceByTileCombo[2][1].
		if usedTile[ti] == false then
			foundMatches = {}
			table.insert(foundMatches, ti) -- The current tile we are looking at is of course a match against itself.
			usedTile[ti] = true
			for tj = 1, #trueColorTileList do
				if ti ~= tj then
					local mi = math.min(ti, tj)
					local mx = math.max(ti, tj)
					if differenceByTileCombo[mx] ~= nil then
						if differenceByTileCombo[mx][mi] ~= nil then
							if usedTile[tj] == false then
								if differenceByTileCombo[mx][mi] > threshold then
									table.insert(foundMatches, tj)
									usedTile[tj] = true
								end
							end
						end
					end
				end
			end
			table.insert(tileClusters, foundMatches)
			progress:value(ti)
			fltk.check()
		end
	end
	for ti = 1, #trueColorTileList do -- In-case we missed any tiles.
		if usedTile[ti] == false then
			table.insert(tileClusters, {ti})
		end
	end
	function sortByLengthMostFirst(a, b)
		return #a > # b
	end
	table.sort(tileClusters, sortByLengthMostFirst)
	-- local f = io.open('sdump.txt', 'w')
	-- f:write(serpent.block(tileClusters))
	-- f:close()
	progressWindow:hide()
	tileScrollbar:range(1, #tileClusters)
end


local tilesOffsetX = 64
local tilesOffsetY = 48
local tilesZoom = 6
local tileSpacingX = 4
local tileSpacingY = 4

local tileOffsetDisplay = 1


function MyWindow(w, h, label)
	local win = fltk.double_window_sub(w, h, label)
	win:override_draw(draw)
	win:override_handle(handle)
	return win
end

function draw(win)
	win:super_draw()

	local xDraw = tilesOffsetX
	local yDraw = tilesOffsetY
	local tileWidth = cts.width * tilesZoom + tileSpacingX
	local tileHeight = cts.height * tilesZoom + tileSpacingY

	if tileClusters[tileOffsetDisplay] == nil then -- draw might get called while the cluster is being built.
		print('Cannot get', tileOffsetDisplay, 'in tileClusters')
		return
	end

	for kunused, tileIdx in ipairs(tileClusters[tileOffsetDisplay]) do
		cts[tileIdx]:drawTC(xDraw, yDraw, false, false, tilesZoom)
		xDraw = xDraw + tileWidth
		if xDraw >= (mywin:w() - tileWidth) then
			xDraw = tilesOffsetX
			yDraw = yDraw + tileHeight
		end
	end
end

function handle(win, e)
	return win:super_handle(e)
end

function scrollbarUpdate(src, val)
	tileOffsetDisplay = math.floor(src:value())
	mywin:redraw()
end

function clearCallback(src, val)
	tileClusters = {}
	for ti = 1, #trueColorTileList do -- In-case we missed any tiles.
		table.insert(tileClusters, {ti})
	end
	mywin:redraw()
end

function autoCallback(src, val)
	local threshold = tonumber(fltk.input('What SSIM threshold should be used for clustering?'))
	clusterTiles(threshold)
	mywin:redraw()
end

function doneBtnCallback(src, val)
	mywin:hide()
end

mywin = MyWindow(800, 600, "Tile Clusters")

local autoBtn = fltk.button(42, 8, 64, 24, 'Auto')
autoBtn:callback(autoCallback)

local doneBtn = fltk.button(114, 8, 64, 24, 'Done')
doneBtn:callback(doneBtnCallback)

local clearBtn = fltk.button(186, 8, 64, 24, 'Clear')
clearBtn:callback(clearCallback)

tileScrollbar = fltk.scrollbar(8, 8, 28, 580)
tileScrollbar:range(1, #tileClusters)
tileScrollbar:step(1, 10)
tileScrollbar:callback(scrollbarUpdate)

clusterTiles(0.6)

mywin:done()
mywin:show()
while mywin:shown() do
	Fl.wait()
end


oldTileIdxToNew = {}
assignQue = {}
newIdx = 1
print('Number of clusters', #tileClusters) -- The number of clusters should match the number of tiles we end up with.
for kunused, tileList in ipairs(tileClusters) do
	for ku, tileIdx in ipairs(tileList) do
		oldTileIdxToNew[tileIdx] = newIdx
	end
	local ct = cts[tileList[1]]
	table.insert(assignQue, {ct.rgbaData, ct.data})
	newIdx = newIdx + 1
end
doAssignQue()
