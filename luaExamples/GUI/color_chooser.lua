-- Note this function is a bit different than the standard fl_color_chooser function in the sense that instead of passing r,g,b by reference r,g,b are returned
local ret,r,g,b=fl.color_chooser("Pick in rgb") -- mode is an optional parameter
if ret~=0 then
	fltk.message(string.format("RGB values are: %d %d %d",math.floor(r * 255),math.floor(g * 255),math.floor(b * 255)))
end
ret,r,g,b=fl.color_chooser("Pick in hsv",3)
if ret~=0 then
	fltk.message(string.format("Converted to RGB: %d %d %d",math.floor(r * 255),math.floor(g * 255),math.floor(b * 255)))
end
ret,r,g,b=fl.color_chooser("Pick a color",-1,1.0) -- You can also specify default values in r,g,b order there is no need to specify all three when doing so note that these can of course be changed by the user even when said channel has a default value.
if ret~=0 then
	fltk.message(string.format("RGB values are: %d %d %d",math.floor(r * 255),math.floor(g * 255),math.floor(b * 255)))
end
