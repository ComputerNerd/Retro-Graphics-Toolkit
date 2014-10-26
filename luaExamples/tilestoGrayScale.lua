	local index=0
	for gray=0,255,255/(palette.cnt/palette.rowCnt-1) do
		palette.setRGB(index,gray,gray,gray)
		if palette.haveAlt then
			palette.setRGB(index+palette.cnt,gray,gray,gray)
		end
		index=index+1;
	end
	for i=0,tile.amt-1,1 do
		for y=0,7,1 do
			for x=0,7,1 do
				local r,g,b,a=tile.getPixelRGBA(i,x,y)
				local gray=0.2126*r+0.7152*g+0.0722*b; -- BT.709
				tile.setPixelRGBA(i,x,y,gray,gray,gray,a)
			end
		end
		tile.dither(i)
	end
	palette.fixSliders()
