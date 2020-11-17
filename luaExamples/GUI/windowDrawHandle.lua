-- To allows for more control over a window, Lua functions can be called for draw and handle

function MyWindow(w, h, label)
	local win = fltk.double_window_sub(w, h, label)
	win:override_handle(handle)
	win:override_draw(draw)
	return win
end

function draw(win)
	win:super_draw()

	local xPos = fltk.event_x()
	local yPos = fltk.event_y()

	fltk.color(0, 0, 255)
	fltk.circle(xPos, yPos, 25)

	local mouseXYstr = string.format("%d %d", xPos, yPos)
	fltk.font(win:labelfont(), win:labelsize())
	fltk.color(win:labelcolor())
	fltk.draw(mouseXYstr, 10, 10)
end
function handle(win, e)
	if e == 'move' then
		win:redraw()
	end
	return win:super_handle(e)
end
mywin = MyWindow(640, 480, "Window draw and handle demonstration.")
mywin:done()
mywin:show()
while mywin:shown() do
	Fl.wait()
end
