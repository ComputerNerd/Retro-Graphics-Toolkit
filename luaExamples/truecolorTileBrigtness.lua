-- Multiplies all pixels in all tiles by m
local p = projects.current
if p:have(project.tilesMask) then
	m=tonumber(fl.input("Enter a multiplier m>1 brightens m<1 darkens","1"))
	if m~=nil then
		for i=1, #p.tiles do
			local tile = p.tiles[i].rgba
			for y=1, p.tiles.height do
				local row = tile[y]
				for x=1, p.tiles.width do
					local pixel = row[x]
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
