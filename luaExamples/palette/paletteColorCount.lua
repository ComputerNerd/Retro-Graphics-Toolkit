-- Shows how many colors are in the palette.
local p = projects.current
if p:have(project.palMask) then
	local pl = p.palette
	if pl.haveAlt == true then
		fltk.message(string.format("Main palette colors %u Alternative sprite palette colors %u total %u",pl.cnt,pl.cntAlt,pl.cntTotal))
	else
		fltk.message(string.format("Palette colors %u",#pl))
	end
else
	project.haveMessage(project.palMask)
end
