-- Swaps one block with another.
-- Use case example: The first block is not blank there is a blank block somewhere in the middle.

local p = projects.current
if p:have(project.mapMask) then
	local firstBlock = fl.input('Counting from zero enter the first block.')
	if firstBlock == nil then
		return
	end
	local secondBlock = fl.input('Counting from zero enter the second block.')
	if secondBlock == nil then
		return
	end

	firstBlock = tonumber(firstBlock)
	secondBlock = tonumber(secondBlock)

	local tilemap = p.tilemaps.current
	local maxBlock = tilemap.hAll // tilemap.height

	if firstBlock < 0 or secondBlock < 0 then
		fl.alert('Must be greater than or equal to zero.')
		return
	end

	if firstBlock >= maxBlock or secondBlock >= maxBlock then
		fl.alert('Must be less than the number of blocks.')
		return
	end

	firstBlock = firstBlock * tilemap.width
	secondBlock = secondBlock * tilemap.width
	for y = 1, tilemap.height do
		for x = 1, tilemap.width do
			local tmp = tilemap[y + secondBlock][x].raw -- Use raw to ensure everything is copied.
			tilemap[y + secondBlock][x].raw = tilemap[y + firstBlock][x].raw
			tilemap[y + firstBlock][x].raw = tmp
		end
	end
	rgt.damage()
else
	p:haveMessage(project.mapMask)
end
