local function square(x)
	return x*x
end
local function manIt(c_r,c_i,maxit)
	--Based on my (Sega16's) Mandelbrot explorer https://github.com/ComputerNerd/Mandelbrot-Casio-Prizm-Explorer
	local ckr,cki;
	local p,ptot=0,8
	local z_r = c_r
	local z_i = c_i
	local zrsqr = (z_r * z_r)
	local zisqr = (z_i * z_i)
	local q=square(c_r-0.25)+square(c_i)
	if ((q*(q+(c_r-0.25)))) < (square(c_i)/4.) then
		return 0,0,0
	end
	repeat
		ckr = z_r
		cki = z_i
		ptot=ptot+ptot
		if ptot > maxit then
			ptot = maxit
		end
		while p<ptot do
			z_i=(square(z_r+z_i))-zrsqr-zisqr
			z_i=z_i+c_i
			z_r=zrsqr-zisqr+c_r
			zrsqr=square(z_r)
			zisqr=square(z_i)
			if (zrsqr + zisqr) > 4. then
				local color=p*16777215//maxit
				return color&255,(color>>8)&255,(color>>16)&255
			end
			if (z_r == ckr) and (z_i == cki) then
				return 0,0,0
			end
			p=p+1
		end
	until ptot ~= maxit
	return 0,0,0
end

local p = projects.current

if p:have(project.mapMask) then
	local maxit = tonumber(fl.input("Maximum iterations","16000"))
	if maxit ~= nil then
		local tilemap = p.tilemaps.current
		p.tiles:setAmt(tilemap.width * tilemap.height)

		for j = 0, tilemap.height - 1 do
			local row = tilemap[j + 1]
			for i = 1, tilemap.width do
				row[i].tile = j * tilemap.width + i
			end
		end

		local width = tilemap.width * p.tiles.width
		local height = tilemap.hAll * p.tiles.height

		-- [-2,1],[-1,1]
		local sx, sy = 3/width, 2/height
		local x, y
		y = -1
		local img = {}
		for i = 0,height - 1 do
			x = -2
			for j = 0, width * 3 - 1, 3 do
				local r,g,b = manIt(x, y, maxit)
				img[(i * width * 3) + j + 1] = r
				img[(i * width * 3) + j + 2] = g
				img[(i * width * 3) + j + 3] = b
				x = x + sx
			end
			y = y + sy
		end
		-- Paramaters: image, -1 for all rows, has alpha channel false for 3 bpp, true for 4 bpp, copy to true color tiles, convert
		tilemap:imageToTiles(img, -1, false, true)
		rgt.redraw()
	end
else
	p:haveMessage(project.mapMask)
end
