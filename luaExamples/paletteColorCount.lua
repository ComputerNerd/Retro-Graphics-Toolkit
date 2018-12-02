-- Shows how many colors are in the palette.
local p = projects.current
if p:have(project.palMask) then
	if p.palette.haveAlt then
		fl.message(string.format("Main palette colors %u Alternative sprite palette colors %u total %u",palette.cnt,palette.cntAlt,palette.cnt+palette.cntAlt))
	else
		fl.message(string.format("Palette colors %u",#palette))
	end
else
	project.haveMessage(project.palMask)
end
