local p = projects.current
if p:have(project.tilesMask) then
	local Lm = tonumber(fl.input("Enter L multiplier","1"))
	local am = tonumber(fl.input("Enter a multiplier","1"))
	local bm = tonumber(fl.input("Enter b multiplier","1"))
	if Lm~=nil and am~=nil and bm~=nil then
		for i=1, #p.tiles do
			local tile = p.tiles[i].rgba
			for y=1, p.tiles.height do
				local row = tile[y]
				for x=1, p.tiles.width do
					local pixel = row[x]
					
					local L, a, b = rgt.rgbToLab(pixel.r / 255., pixel.g / 255., pixel.b / 255.)
					r, g, bl = rgt.labToRgb(L * Lm, a * am, b * bm)
					print(r, g, bl)

					pixel.r = math.floor(r * 255.)
					pixel.g = math.floor(g * 255.)
					pixel.b = math.floor(bl * 255.)
				end
			end
		end
		rgt.redraw()
	end
else
	p:haveMessage(project.tilesMask)
end
