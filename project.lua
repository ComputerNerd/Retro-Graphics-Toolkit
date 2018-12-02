--[[
	This file is part of Retro Graphics Toolkit

	Retro Graphics Toolkit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or any later version.

	Retro Graphics Toolkit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
	Copyright Sega16 (or whatever you wish to call me) (2012-2016)
--]]
function switchProject()
	rgt.syncProject()

	local p = projects.current

	if is_headless == 0 then
		palTabSel:value(p:getPalType())--Fixes internal pointer
	end

	if p:have(project.palMask) ~= false then
		p.palette:toRgbAll()
	end

	if is_headless == 0 then
		if p:have(project.levelMask) ~= false then
			layerSel:clear()
			for i=1,#level.names do
				layerSel:add(level.names[i])
			end
			layerSel:value(0)
			layerName:value(level.names[1])
			lvlsetlayer(0)
		end
	end
end
