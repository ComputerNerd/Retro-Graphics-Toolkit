-- Sets all tiles to a gradient
local function hueDir(h1,h2)
	if math.abs(h2-h1)>180. then
		if h2>h1 then
			return -(360.-h2+h1)
		else
			return 360.-h1+h2
		end
	else
		return h2-h1
	end
end
if project.have(project.tilesMask+project.mapMask) then
	local ret,L1,c1,h1=fl.color_chooser("First color")
	if ret~=0 then
		local ret,L2,c2,h2=fl.color_chooser("Second color")
		if ret~=0 then
			L1,c1,h1=rgt.rgbToLch(L1,c1,h1)
			L2,c2,h2=rgt.rgbToLch(L2,c2,h2)
			-- Calculate tile
			tile.resize(tilemaps.height[tilemaps.current+1])
			for j=0,tilemaps.height[tilemaps.current+1]-1 do
				for i=0,tilemaps.width[tilemaps.current+1]-1 do
					tilemaps.setTile(tilemaps.current,i,j,j)
				end
			end
			local Ls,cs,hs
			if project.have(project.palMask) then
				local Lt,ct,ht = L1,c1,h1
				Ls,cs,hs = (L2-L1)/palette.maxInRow(0),(c2-c1)/palette.maxInRow(0),hueDir(h1,h2)/palette.maxInRow(0)
				for i=0,palette.maxInRow(0)-1 do
					local r,g,b=rgt.lchToRgb(Lt,ct,ht)
					palette.setRGB(i,r*255.,g*255.,b*255.)
					Lt=Lt+Ls
					ct=ct+cs
					ht=ht+hs
					if(ht>360.) then
						ht=ht-360.
					end
					if(ht<0.) then
						ht=ht+360.
					end
				end
				palette.fixSliders()
			end
			Ls,cs,hs = (L2-L1)/(tilemaps.heightA[tilemaps.current+1]*tile.height[tilemaps.current+1]),(c2-c1)/(tilemap.heightA[tilemaps.current+1]*tile.height[tilemaps.current+1]),hueDir(h1,h2)/(tilemap.heightA[tilemaps.current+1]*tile.height[tilemaps.current+1])
			for t=0,tile.amt-1 do
				local gr={}
				for i=0,tile.height-1 do
					local r,g,b=rgt.lchToRgb(L1,c1,h1)
					for j=0,tile.width*4-1,4 do
						gr[(i*tile.width*4)+j+1]=r*255.
						gr[(i*tile.width*4)+j+2]=g*255.
						gr[(i*tile.width*4)+j+3]=b*255.
						gr[(i*tile.width*4)+j+4]=255.
					end
					L1=L1+Ls
					c1=c1+cs
					h1=h1+hs
					if(h1>360.) then
						h1=h1-360.
					end
					if(h1<0) then
						h1=h1+360.
					end
				end
				tile.setTileRGBA(t,gr)
			end
		end
	end
else
	project.haveMessage(project.tilesMask+project.mapMask)
end
