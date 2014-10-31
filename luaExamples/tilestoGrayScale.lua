if project.have(project.tilesMask) then
-- Although as of writing this comment Retro Graphics Toolkit uses only tiles that use a palette
-- meaning that having tiles implies a palette this could change latter
-- that is I add tiles that do not use a palette.
	if project.have(project.palMask) then 
		local index=0
		for gray=0,255,255/(palette.maxInRow(0)-1) do
			while palette.getType(index)~=0 do
				index=index+1;
			end
			palette.setRGB(index,gray,gray,gray)
			index=index+1;
		end
		if palette.haveAlt then
			index=palette.cnt
			for gray=0,255,255/(palette.maxInRow(palette.rowCnt)-1) do
				while palette.getType(index)~=0 do
					index=index+1;
				end
				palette.setRGB(index,gray,gray,gray)
				index=index+1;
			end
		end
	end
	for i=0,tile.amt-1,1 do
		for y=0,7,1 do
			for x=0,7,1 do
				local r,g,b,a=tile.getPixelRGBA(i,x,y)
				local gray=0.2126*r+0.7152*g+0.0722*b; -- BT.709
				tile.setPixelRGBA(i,x,y,gray,gray,gray,a)
			end
		end
	end
	if project.have(project.mapMask) then
		tilemap.dither()
	end
	if project.have(project.spritesMask) then
		sprite.ditherAll()
	end
	if project.have(project.palMask) then
		palette.fixSliders()
	end
else
	project.haveMessage(project.tilesMask)
end
