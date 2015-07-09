if project.have(project.mapMask+project.palMask) then
	img=tilemaps.toImage(tilemaps.current,-1,1)
	rgt.ditherImage(img,tilemap.width*tile.width,tilemap.height*tile.height,1)
	tilemaps.imageToTiles(tilemaps.current,img,-1,1)
	rgt.redraw()
else
	project.haveMessage(project.mapMask+project.palMask)
end
