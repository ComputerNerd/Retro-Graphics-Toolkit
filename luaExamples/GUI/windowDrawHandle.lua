	-- To allows for more control over a window, Lua functions can be called for draw and handle
	win=Fl_Window.new(640,480)
function draw()
	win:baseDraw()
	fl.color(0,0,255)
	fl.circle(Fl.event_x(),Fl.event_y(),25)
	fl.draw(string.format("%d %d",Fl.event_x(),Fl.event_y()),10,10)
end
function handle(e)
	-- print(fl.eventnames(e))
	if e==FL.MOVE then
		win:redraw()
	end
	if win:baseHandle(e)~=0 then
		return 1 --The event was used
	end
	--Process unused events
	return 0
end
	win:setDrawFunction("draw")
	win:setHandleFunction("handle")
	-- Fl_Window.set[replace with either Draw or Handle]Function(win) This is how you would restore the function to default which simply calls "baseDraw" or "baseHandle"
	win:End()
	win:show()
	while win:shown()~=0 do
		Fl.wait()
	end
