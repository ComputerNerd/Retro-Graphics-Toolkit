-- Color conversion code from http://sputnik.freewisdom.org/lib/colors/
-----------------------------------------------------------------------------
-- Converts an HSL triplet to RGB
-- (see http://homepages.cwi.nl/~steven/css/hsl.html).
-- 
-- @param h              hue (0-360)
-- @param s              saturation (0.0-1.0)
-- @param L              lightness (0.0-1.0)
-- @return               an R, G, and B component of RGB
-----------------------------------------------------------------------------

function hsl_to_rgb(h, s, L)
	local m1, m2
	if L<=0.5 then
		m2 = L*(s+1.)
	else
		m2 = L+s-L*s
	end
	m1 = L*2.-m2

	local function _h2rgb(m1, m2, h)
		if h<0. then h = h+1 end
		if h>1. then h = h-1 end
		if h*6.<1. then
			return m1+(m2-m1)*h*6.
		elseif h*2.<1. then
			return m2
		elseif h*3.<2. then
			return m1+(m2-m1)*(2./3.-h)*6.
		else
			return m1
		end
	end

	return _h2rgb(m1, m2, h+1./3.), _h2rgb(m1, m2, h), _h2rgb(m1, m2, h-1./3.)
end
function idxHelper(idx)
	local ent = projects.current.palette[idx]

	local r = ent.r
	local g = ent.g
	local b = ent.b

	local h,s,l=rgt.rgbToHsl(r/255.,g/255.,b/255.)
	r,g,b=hsl_to_rgb((h+shift)%1.,s,l)
	return math.floor(r * 255.), math.floor(g * 255.), math.floor(b *255.)
end
function draw(win)
	local p = projects.current
	win:super_draw()
	if shift~=nil then
		for row=0,p.palette.rowCnt-1 do
			for idx=0,p.palette.perRow-1 do
				local r,g,b=idxHelper(row*p.palette.perRow+idx + 1)
				fl.rectf((idx+1)*16,(row+1)*16,16,16,r,g,b)
			end
		end
		if p.palette.haveAlt~=0 then
			for row=0,p.palette.rowCntAlt-1 do
				for idx=0,p.palette.perRowAlt-1 do
					local r,g,b=idxHelper(row*p.palette.perRowAlt+idx+p.palette.cnt + 1)
					fl.rectf((idx+1)*16+(p.palette.perRow+1)*16,(row+1)*16,16,16,r,g,b)
				end
			end
		end
	end
end
shift=0.
function setShift(unused)
	shift=sld:value()
	win:redraw()
end
ok=0
function btnCB(val)
	ok=val
	win:hide()
end

function PaletteWindow()
	win = fltk.double_window_sub(320, 200, 'Shift hue by')
	win:override_draw(draw)

	return win
end

local p = projects.current
if p:have(project.palMask) then
	local p = projects.current
	win = PaletteWindow()
	win:set_modal()
	sld=fltk.value_slider(10,166,300,24)
	sld:type(FL.HOR_SLIDER)
	sld:callback(setShift)
	okbtn=fltk.button((320-64)/2-64-10,134,64,24,"OK")
	okbtn:callback(btnCB,1)
	cancelbtn=fltk.button((320-64)/2+64+10,134,64,24,"Cancel")
	cancelbtn:callback(btnCB)
	win:done()
	win:show()
	while win:shown() do
		Fl.wait()
	end
	if ok~=0 then
		undo.pushPaletteAll()
		for ent = 1, p.palette.cntTotal, 1 do
			local r,g,b=idxHelper(ent)
			p.palette[ent]:setRGB(r,g,b)
		end
		palette.fixSliders()
		rgt.damage()
	end
else
	project.haveMessage(project.palMask)
end
