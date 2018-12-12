-- How to create a simple window and wait for the window to exit.
win=Fl_Window.new(640,480)
win:End()
win:show()
while win:shown()~=0 do
	Fl.wait()
end

