-- Note this function is a bit different than the standard fl_color_chooser function in the sense that instead of passing r,g,b by reference r,g,b are returned
local ret,r,g,b=fl.color_chooser("Pick in rgb") -- mode is an optional parameter
if ret~=0 then
	fl.message(string.format("RGB values are: %d %d %d",r*255,g*255,b*255))
end
ret,r,g,b=fl.color_chooser("Pick in hsv",3)
if ret~=0 then
	fl.message(string.format("Converted to RGB: %d %d %d",r*255,g*255,b*255))
end
ret,r,g,b=fl.color_chooser("Pick a red",-1,1.0) -- You can also specify default values in r,g,b order there is no need to specify all three when doing so note that these can of course be changed by the user even when specified
if ret~=0 then
	fl.message(string.format("RGB values are: %d %d %d",r*255,g*255,b*255))
end
