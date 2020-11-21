local dir=fl.dir_chooser() -- The three parameters are optional
if dir == nil or dir == '' then
	fltk.alert("No directory specified")
else
	fltk.message(dir)
end
