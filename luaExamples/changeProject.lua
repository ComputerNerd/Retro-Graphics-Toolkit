-- Changes the project based on ID.
set=tonumber(fl.input(string.format("Current project ID is: %d\nEnter the project ID you want to switch to from [0,%d)",project.curProjectID,project.count)))
if set~=nil then
	if project.set(set)==true then
		fl.alert(string.format("Project set to %d",project.curProjectID))
	else
		fl.alert(string.format("ID %d is out of bounds or is already set to %d",set,set))
	end
end
rgt.redraw()
