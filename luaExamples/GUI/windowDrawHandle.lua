-- To allows for more control over a window, Lua functions can be called for draw and handle

function MyWindow(w, h, label)
	local win = fltk.double_window_sub(w, h, label)
	win:override_draw(draw)
	win:override_handle(handle)
	return win
end

function draw(win)
	print('draw')
	win:super_draw()
	fl.color(0,0,255)
	fl.circle(Fl.event_x(),Fl.event_y(),25)
	fl.draw(string.format("%d %d",Fl.event_x(),Fl.event_y()),10,10)
end
function handle(win, e)
	print('handle')
	-- print(fl.eventnames(e))
	if e==FL.MOVE then
		win:redraw()
	end
	if win:super_handle(e)~=0 then
		return 1 --The event was used
	end
	--Process unused events
	return 0
end
local mywin = MyWindow(640, 480, "Window draw and handle demonstration.")
mywin:done()
mywin:show()
while mywin:shown()~=0 do
	Fl.wait()
end
