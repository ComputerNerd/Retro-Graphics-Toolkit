-- How to create a simple window and wait for the window to exit.
win=fltk.window(640,480)
win:done()
win:show()
while win:shown() do
	Fl.wait()
end
