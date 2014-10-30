if project.have(project.mapMask+project.palMask) then
	img=tilemap.toImage(-1,1)
	rgt.ditherImage(img,tilemap.width*tile.width,tilemap.height*tile.height,1)
	tilemap.imageToTiles(img,-1,1)
	rgt.redraw()
else
	project.haveMessage(project.mapMask+project.palMask)
end
