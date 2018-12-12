local fname=fl.file_chooser("Save hello word to") -- All parameters are optional
if not (fname == nil or fname == '') then
	local file=assert(io.open(fname,"w"))
	io.output(file)
	io.write("Hello world")
	io.close(file)
end
