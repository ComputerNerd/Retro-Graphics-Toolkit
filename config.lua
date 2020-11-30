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
	Copyright Sega16 (or whatever you wish to call me) (2012-2020)
--]]

--[[
This file allows you control various aspects of Retro Graphics Toolkit.
For purposes of organization files are separated and included in this main file.
For example menu.lua allows you to edit the shortcut keys to various menu items.

Warning: This file is executed very early. Before the project objects are ready.
--]]

serpent = require("serpent.src.serpent")
-- You must install compat53 and bit32 to use luajit.

-- There are a few ways to go about this. One is to install locally. This is helpful if these libraries cannot be installed with your package manager and you don't want to run as root.
-- luarocks --local --lua-version 5.1 install compat53
-- luarocks --local --lua-version 5.1 install bit32
-- Then prior to running Retro Graphics Toolkit run
-- eval $(luarocks --lua-version 5.1 path)

require("compat53")
bit32 = require('bit32')


Fl.scheme('plastic')
if is_headless == 0 then
	dofile "menu.lua"
	dofile "callbacks.lua"
	dofile "gui.lua"
	dofile "level.lua"
end
dofile "project.lua"
dofile "system.lua"
dofile "paletteValidation.lua"
dofile "tiles.lua"
dofile "TMS9918.lua"
if is_headless == 0 then
	dofile "filereader.lua"
end
--print('Code will execute here after the window is created but before the GUI controls are added')
