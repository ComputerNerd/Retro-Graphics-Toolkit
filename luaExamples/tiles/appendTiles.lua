local p = projects.current
if p:have(project.tilesMask) then
	fltk.alert("Started with "..#p.tiles.." tile.")
	p.tiles:append(10) -- The parameter is optional and defaults to one
	fltk.alert("Now there are "..#p.tiles.." tiles.")
else
	project.haveMessage(project.tilesMask)
end
