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
	Copyright Sega16 (or whatever you wish to call me) (2012-2019)
--]]
function switchProject()
	rgt.syncProject()

	local p = projects.current

	if is_headless == 0 then
		updateProjectGUI(p.gameSystem)
		if p.gameSystem == project.segaGenesis then
			palTabSel:value(palTabSelOptions[p:getPalType() + 1]) -- Ensure the right selection is made.
		end
	end

	if p:have(project.palMask) ~= false then
		if p.palette.fixedPalette == false then
			p.palette:toRgbAll()
		end
	end

	if is_headless == 0 then
		if p:have(project.levelMask) ~= false then
			layerSel:clear()
			local layers = p.level.layers
			for i=1, #layers do
				local layer = layers[i]
				print(layer)
				local name = layer.name
				print(i, name)
				layerSel:add(name)
			end
			lvlsetlayer(1)
			local currentLayerName = layers[lvlCurLayer].name
			layerSel:value(currentLayerName)
		end
	end
end
