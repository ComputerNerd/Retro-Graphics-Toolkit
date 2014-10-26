	-- Multiplies all pixels in all tiles by m
	m=tonumber(fl.input("Enter a multiplier m>1 brightens m<1 darkens","1"))
	if m~=nil then
		for i=0,tile.amt-1,1 do
			for y=0,7,1 do
				for x=0,7,1 do
					local r,g,b,a=tile.getPixelRGBA(i,x,y)
					tile.setPixelRGBA(i,x,y,r*m,g*m,b*m,a)
				end
			end
		end
		rgt.redraw()
	end
