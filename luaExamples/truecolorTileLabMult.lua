local p = projects.current
if p:have(project.tilesMask) then
	local Lm = tonumber(fl.input("Enter L multiplier","1"))
	local am = tonumber(fl.input("Enter a multiplier","1"))
	local bm = tonumber(fl.input("Enter b multiplier","1"))
	if Lm~=nil and am~=nil and bm~=nil then
		for i=1, #p.tiles do
			local tile = p.tiles[i]
			for y=1, tile.height do
				local row = tile[y]
				for x=1, tile.width do
					local pixel = row[x].rgba
					
					local L ,a ,b = rgt.rgbToLab(pixel.r / 255., pixel.g / 255., bl / 255.)
					r, g, bl = rgt.labToRgb(L * Lm, a * am, b * bm)

					pixel.r = math.floor(r)
					pixel.g = math.floor(g)
					pixel.b = math.floor(bl)
				end
			end
		end
		rgt.redraw()
	end
else
	p:haveMessage(project.tilesMask)
end
