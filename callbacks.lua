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
	Copyright Sega16 (or whatever you wish to call me) (2012-2015)
--]]

--[[
-- This file allows for drawing on the main screen and getting event callbacks via Lua.
-- The draw callback allows for drawing on the screen. There are two callbacks:
-- The first drawing callback is called BEFORE Retro Graphics Toolkit does any custom drawing.
-- The second drawing callback is called AFTER  Retro Graphics Toolkit does any custom drawing.
-- This allows you to draw over and under elements that are drawn By Retro Graphics Toolkit.
-- Both callbacks take one parameter which tells you what tab you are on.
-- Please use the values in the rgt table instead of hard coding numbers.
-- These functions must not return any values
--]]
function drawCBfirst(tab)
	
end
function drawCBlast(tab)
	
end
--[[
-- The event callback takes two arguments.
-- The first of which is the event type.
-- You can use the FL table to tell what type of event it is. Do not hard code numbers.
-- The second of which is the tab ID.
-- See draw callback notes on the tab parameter.
-- This function must return one value that is zero or one.
-- ]]
function eventCB(e,tab)
	--print(fl.eventnames(e),e,tab)
	--Return 1 if this was used for A WIDGET ONLY. 0 otherwise.
	if e==FL.SHORTCUT then
		if Fl.event_ctrl()~=0 then
			if Fl.event_key(FL.Page_Up)~=0 then
				rgt.setTab(tab+1)
				return 1
			end
			if Fl.event_key(FL.Page_Down)~=0 then
				rgt.setTab(tab-1)
				return 1
			end
		end
	end
	return 0
end
function switchTab(oldtab,newtab)
	--print(oldtab,newtab)
end
