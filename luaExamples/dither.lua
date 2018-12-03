local p = projects.current
if p:have(project.mapMask+project.palMask) then
	local tilemap = p.tilemaps.current
	local img = tilemap:toImage(-1, true)

	rgt.ditherImage(img,tilemap.width * tile.width,tilemap.height * tile.height, 1)
	tilemap:imageToTiles(img, -1, true)

	rgt.redraw()
else
	p:haveMessage(project.mapMask+project.palMask)
end
