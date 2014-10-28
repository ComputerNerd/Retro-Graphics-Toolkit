-- Sets all tiles to a gradient
if project.have(project.tilesMask) then
	local ret,r1,g1,b1=fl.color_chooser("First color")
	if ret~=0 then
		local ret,r2,g2,b2=fl.color_chooser("Second color")
		if ret~=0 then
			-- Calculate tile
			gr={}
			local rs,gs,bs = (r2-r1)/tile.height,(g2-g1)/tile.height,(b2-b1)/tile.height
			for i=0,tile.height-1 do
				for j=0,tile.width*4-1,4 do
					gr[(i*tile.width*4)+j+1]=r1*255
					gr[(i*tile.width*4)+j+2]=g1*255
					gr[(i*tile.width*4)+j+3]=b1*255
					gr[(i*tile.width*4)+j+4]=255
				end
				r1=r1+rs
				g1=g1+gs
				b1=b1+bs
			end
			for i=0,tile.amt-1 do
				tile.setTileRGBA(i,gr)
			end
		end
	end
else
	project.haveMessage(project.tilesMask)
end
