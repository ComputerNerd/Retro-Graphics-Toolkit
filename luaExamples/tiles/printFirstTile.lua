local p = projects.current
if p:have(project.tilesMask) then
	local pixels = p.tiles[1].pixels
	local zeroChar = string.byte('A')
	for y = 1, #pixels do
		local pixelRow = pixels[y]
		for x = 1, #pixelRow do
			io.write(string.char(zeroChar + pixelRow[x]))
		end
		io.write('\n')
	end
else
	p:haveMessage(project.tilesMask)
end
