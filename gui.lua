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
function setPalTypeCB(menuItm)
	local vStr = menuItm:value()
	local v = palTabSelLUT[vStr]

	local p = projects.current
	p:setPalType(v)
	p.palette:toRgbAll()
	rgt.damage()
end

function setSpriteSizeCB(unused)
	local p = projects.current
	p:setSpriteSizeID(spriteSizeSel:get_index())
	rgt.redraw()
end

function generateLutFromList(choiceList)
	local tmp = {}
	for i = 1, #choiceList do
		tmp[choiceList[i]] = i - 1
	end
	return tmp
end

function addItemsToChoice(menu, choiceList)
	for i = 1, #choiceList do
		local menuItemEscaped = choiceList[i]:gsub('/', '\\/')
		menu:add(menuItemEscaped)
	end
end

function tabConfig(tab)
	if tab == rgt.paletteTab then
		palTabSel = fltk.choice(336, 464, 128, 24, "Palette table selection")
		palTabSel:align(FL.ALIGN_TOP)
		palTabSel:callback(setPalTypeCB)
		palTabSelOptions = {"HardwareMan's measured values", 'round(255*v/7)', 'Steps of 36', 'Steps of 32'}
		palTabSelLUT = generateLutFromList(palTabSelOptions)
		addItemsToChoice(palTabSel, palTabSelOptions)
		palTabSel:labelsize(12)


		spriteSizeSel = fltk.choice(336, 464, 128, 24, "Sprite size")
		spriteSizeSel:align(FL.ALIGN_TOP)
		spriteSizeSel:callback(setSpriteSizeCB)
		spriteSizeSel:labelsize(12)
		spriteSizeSel:hide()
	elseif tab==rgt.levelTab then
		initLevelEditor()
	end
end
