-- Simple demonstration of the project string store.
-- The project string store lets you store arbitrary strings in a project which will be saved and loaded as a part of the project. 

-- Create a blank project to avoid accidentally messing up the users project.
project.append()
project.set(#projects)

local p = projects[#projects]

-- The name is long in hopes to reduce the chance of accidentally overwriting someone's project.
local projectFname = 'string-store-hello-project-file.rgp'
local f = io.open(projectFname)
if f == nil then
	-- The project doesn't exist. Add the test string to the project and save it.
	p.stringStore['hello'] = 'Hello world!'
	p:save(projectFname)
	fltk.message('The project has been saved as ' .. projectFname .. ' the next time you run this example the string will be loaded and displayed in a message dialog and the project will be erased.')
else
	f:close()
	p:load(projectFname)
	os.remove(projectFname)
	fltk.message(p.stringStore.hello) -- Both types of indexing work.
end
