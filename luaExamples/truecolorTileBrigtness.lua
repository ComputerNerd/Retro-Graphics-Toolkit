-- Multiplies all pixels in all tiles by m
local p = projects.current
if p:have(project.tilesMask) then
	m=tonumber(fl.input("Enter a multiplier m>1 brightens m<1 darkens","1"))
	if m~=nil then
		for i=1, tile.amt do
			local tile = p.tiles[i]
			for y=1, tile.height do
				local row = tile[y]
				for x=1, tile.width do
					local pixel = row[x].rgba
					pixel.r = math.floor(pixel.r * m)
					pixel.g = math.floor(pixel.g * m)
					pixel.b = math.floor(pixel.b * m)
				end
			end
		end
		rgt.redraw()
	end
else
	p:haveMessage(project.tilesMask)
end
