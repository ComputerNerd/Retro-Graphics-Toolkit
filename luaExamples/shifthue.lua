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

if project.have(project.palMask) then
	shift=tonumber(fl.input("Shift hue by [0,1]","0"))+0.
	if shift~=nil then
		for ent=0,palette.cnt+palette.cntAlt-1,1 do
			local r,g,b=palette.getRGB(ent)
			local h,s,l=rgt.rgbToHsl(r/255.,g/255.,b/255.)
			r,g,b=hsl_to_rgb((h+shift)%1.,s,l)
			palette.setRGB(ent,r*255.,g*255.,b*255.)
		end
		palette.fixSliders()
	end
else
	project.haveMessage(project.palMask)
end
