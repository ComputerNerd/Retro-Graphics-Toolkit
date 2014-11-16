	-- To allows for more control over a window, lua functions can be called for draw and handle
	win=Fl_Window.new(640,480)
function draw()
	Fl_Window.baseDraw(win)
	fl.color(0,0,255)
	fl.circle(Fl.event_x(),Fl.event_y(),25)
	fl.draw(string.format("%d %d",Fl.event_x(),Fl.event_y()),10,10)
end
function handle(e)
	-- print(fl.eventnames(e))
	if e==FL.MOVE then
		Fl_Window.redraw(win)
	end
	if Fl_Window.baseHandle(win,e)~=0 then
		return 1 --The event was used
	end
	--Process unused events
	return 0
end
	Fl_Window.setDrawFunction(win,"draw")
	Fl_Window.setHandleFunction(win,"handle")
	-- Fl_Window.set[replace with either Draw or Handle]Function(win) This is how you would restore the function to default which simply calls "baseDraw" or "baseHandle"
	Fl_Window.End(win)
	Fl_Window.show(win)
	while Fl_Window.shown(win)~=0 do
		Fl.wait()
	end
