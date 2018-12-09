local p = projects.current
if p:have(project.mapMask+project.palMask) then
	local tilemap = p.tilemaps.current
	local tiles = p.tiles
	local img = tilemap:toImage(-1, true)

	rgt.ditherImage(img, tilemap.width * tiles.width, tilemap.hAll * tiles.height, 1)
	tilemap:imageToTiles(img, -1, true)

	rgt.redraw()
else
	p:haveMessage(project.mapMask+project.palMask)
end
