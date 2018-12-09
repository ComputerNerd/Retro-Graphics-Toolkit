-- Changes the project based on ID.
if #projects < 2 then
	fl.alert('This example requires a minimum of two projects.')
	return
else
	set=tonumber(fl.input(string.format("Current project ID is: %d\nEnter the project ID you want to switch to from [1,%d].",projects.currentIdx, project.count)))
	if set~=nil then
		if project.set(set)==true then
			fl.alert(string.format("Project set to %d", projects.currentIdx))
		else
			fl.alert(string.format("ID %d is out of bounds or is already set to %d",set,set))
		end
	end
	rgt.redraw()
end
