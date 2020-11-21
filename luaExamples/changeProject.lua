-- Changes the project based on ID.
if #projects < 2 then
	fltk.alert('This example requires a minimum of two projects.')
	return
else
	set=tonumber(fl.input(string.format("Current project ID is: %d\nEnter the project ID you want to switch to from [1,%d].",projects.currentIdx, #projects)))
	if set~=nil then
		if project.set(set)==true then
			fltk.alert(string.format("Project set to %d", projects.currentIdx))
		else
			fltk.alert(string.format("ID %d is out of bounds or is already set to %d", set, set))
		end
	end
	rgt.redraw()
end
