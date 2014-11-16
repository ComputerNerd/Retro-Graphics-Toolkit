	-- How to create a simple window and wait for exit.
	win=Fl_Window.new(640,480)
	Fl_Window.End(win)
	Fl_Window.show(win)
	while Fl_Window.shown(win)~=0 do
		Fl.wait()
	end

