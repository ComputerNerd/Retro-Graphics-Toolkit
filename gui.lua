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
lvlCurLayer=0
function lvlsetLayer(unused)
	
end
function lvlappend(unused)

end
function lvldelete(unused)

end
function lvlscroll(unused)

end
function lvlsolo(unused)

end
function lvlshowspirtes(unused)

end
function drawLevel()
	local xs,ys=168*rgt.w()//800,72*rgt.h()//600
end
editModeLevel=0
function handleLevel(e)
	
end
function tabConfig(tab)
	if tab==rgt.levelTab then
		--Note Lua's scoping rules are different than C/C++. All these variables are in fact globals so thus you can access them in the callbacks
		layerSel=Fl_Choice.new(4,48,136,24)--TODO dub cannot yet handle enumerations so align is not working correctly. This means I cannot have a top aligned label.
		layerSel:callback('lvlsetLayer')
		layerSel:tooltip('Sets the current layer')
		appendBtn=Fl_Button.new(4,80,136,24,"Append new layer")
		appendBtn:callback('lvlappend')
		deleteBtn=Fl_Button.new(4,112,136,24,"Delete selected layer")
		deleteBtn:callback('lvldelete')
		soloBtn=Fl_Button.new(4,144,136,24,"Show only selected layer")
		soloBtn:callback('lvlsolo')
		soloBtn:Type(FL.TOGGLE_BUTTON)--TODO fix the binding for functions using Fl_Box so that this can be made to look like a checkbox,
		soloBtn:labelsize(11)
		spriteBtn=Fl_Button.new(4,176,136,24,"Show sprites")
		spriteBtn:callback('lvlshowspirtes')
		spriteBtn:Type(FL.TOGGLE_BUTTON)
		spriteBtn:value(1)
		tSizeScrl=Fl_Value_Slider.new(4,208,136,24)--TODO add top aligned label
		tSizeScrl:tooltip('Sets the zoom of the tiles')
		tSizeScrl:Type(FL.HOR_SLIDER)
		tSizeScrl:step(1)
		tSizeScrl:maximum(16.)
		tSizeScrl:minimum(1.)
		lvlScrollx=Fl_Scrollbar.new(144,48,24,544)
		lvlScrollx:callback('lvlscroll')
		lvlScrollx:hide()
		lvlScrolly=Fl_Scrollbar.new(168,48,624,24)
		lvlScrolly:Type(FL.HORIZONTAL)
		lvlScrolly:callback('lvlscroll')
		lvlScrolly:hide()
	end
end
