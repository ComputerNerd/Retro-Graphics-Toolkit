if project.have(project.tilesMask) then
	Lm=tonumber(fl.input("Enter L multiplier","1"))
	am=tonumber(fl.input("Enter a multiplier","1"))
	bm=tonumber(fl.input("Enter b multiplier","1"))
	if Lm~=nil and am~=nil and bm~=nil then
		for i=0,tile.amt-1,1 do
			for y=0,tile.height-1,1 do
				for x=0,tile.width-1,1 do
					local r,g,bl,al=tile.getPixelRGBA(i,x,y)
					local L,a,b=rgt.rgbToLab(r/255,g/255,bl/255)
					r,g,bl=rgt.labToRgb(L*Lm,a*am,b*bm)
					tile.setPixelRGBA(i,x,y,r*255.,g*255.,bl*255.,al)
				end
			end
		end
		rgt.redraw()
	end
else
	project.haveMessage(project.tilesMask)
end
