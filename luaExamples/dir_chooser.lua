local dir=fl.dir_chooser() -- The three parameters are optional
if dir == nil or dir == '' then
	fl.alert("No directory specified")
else
	fl.message(dir)
end
