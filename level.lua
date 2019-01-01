--[[
	This file is part of Retro Graphics Toolkit

	Retro Graphics Toolkit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or any later version.

	Retro Graphics Toolkit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
	Copyright Sega16 (or whatever you wish to call me) (2012-2018)
--]]
lvlCurLayer=0
curLayerInfo=nil
editX,editY=0,0
szX,szY=0,0

function setSizePer()
	local src=curLayerInfo.src&3
	local plane=(curLayerInfo.src>>2)+1

	rgt.syncProject()

	local p = projects.current
	local chunks = p.chunks

	local tileW = p.tiles.width
	local tileH = p.tiles.height
	local tilemap = p.tilemaps[plane]

	if src==level.TILES then
		szX = tileW
		szY = tileH
	elseif src==level.BLOCKS then
		szX = tilemap.width * tileW
		szY = tilemap.height * tileH
	elseif src==level.CHUNKS then
		szX = chunks.width * tileW
		szY = chunks.height * tileH
		if chunks.useBlocks ~= false then
			szX = szX * tilemap.width
			szY = szY * tilemap.height
		end
	end
end

lvlZoom=1
scrollX,scrollY=0,0
xOff,yOff=168,72

function setScrollBounds()
	local w,h=szX*lvlZoom*curLayerInfo.w,szY*lvlZoom*curLayerInfo.h
	if w<rgt.w()-xOff then
		lvlScrollx:hide()
		scrollX=0
	else
		lvlScrollx:show()
		lvlScrollx:bounds(0,curLayerInfo.w)
	end
	if h<rgt.h()-yOff then
		lvlScrolly:hide()
		scrollY=0
	else
		lvlScrolly:show()
		lvlScrolly:bounds(0,curLayerInfo.h)
	end
end

function lvlsetlayer(layer)
	lvlCurLayer=layer
	layerName:value(level.names[lvlCurLayer+1])
	curLayerInfo=level.getInfo(lvlCurLayer)
	wLayer:value(curLayerInfo.w)
	hLayer:value(curLayerInfo.h)
	if editX>=curLayerInfo.w then
		editX=curLayerInfo.w-1
	end
	if editY>=curLayerInfo.h then
		editY=curLayerInfo.h-1
	end
	lvlSrc:value(curLayerInfo.src&3)
	setScrollBounds()
	setSizePer()
	if level.objamt[lvlCurLayer+1]>0 then
		spriteSel:maximum(level.objamt[lvlCurLayer+1]-1)
		updateSpriteSliders(level.getObj(lvlCurLayer,spriteEdit))
	end
end

function checkCurLayerBounds()
	if lvlCurLayer>=level.amt then
		lvlsetlayer(level.amt-1)
	end
end

function lvlsetLayerCB(unused)
	lvlsetlayer(layerSel:get_index())
	rgt.damage()
end

function lvlappend(unused)
	level.setLayerAmt(level.amt+1)
	layerSel:add(level.names[level.amt])
	rgt.redraw()
end

function lvldelete(unused)
	if level.amt>1 then
		level.removeLayer(lvlCurLayer)
		layerSel:remove(lvlCurLayer)
		checkCurLayerBounds()
		rgt.redraw()
	else
		fl.alert("You must have a minimum of one layer. If do not want a level disable it via means of the have level option in project settings")
	end
end

function lvlscrollCB(unused)
	scrollX=lvlScrollx:value()
	scrollY=lvlScrolly:value()
	rgt.redraw()
end

showSprites=true

function lvlshowspirtes(unused)
	showSprites=spriteBtn:value()~=0
	rgt.damage()
end

function setLayerName(unused)
	level.setLayerName(lvlCurLayer,layerName:value())
	layerSel:replace(lvlCurLayer,level.names[lvlCurLayer+1])
	rgt.redraw()
end

function setLvlzoomCB(unused)
	lvlZoom=tSizeScrl:value()
	setScrollBounds()
	rgt.damage()
end

function invalidSource(src)
	fl.alert(string.format("%d is not a valid source",src))
end

function selSlideUpdateMax(src)
	local p = projects.current
	local chunks = p.chunks

	if src==level.TILES then
		selSlide:maximum(#p.tiles - 1)
	elseif src==level.BLOCKS then
		local plane=(curLayerInfo.src>>2)+1
		local tilemap = p.tilemaps[plane]
		selSlide:maximum(tilemap.hAll // tilemap.h - 1)
	elseif src==level.CHUNKS then
		selSlide:maximum(#chunks - 1)
	else
		invalidSource(src)
	end
end

editModeLevel=false

function drawLevel()
	xOff=168*rgt.w()//800
	yOff=72*rgt.h()//600
	setScrollBounds()
	local xs,ys=xOff,yOff
	local src=curLayerInfo.src&3
	rgt.syncProject()
	local p = projects.current
	for j=scrollY,curLayerInfo.h-1 do
		local xxs=xs
		for i=scrollX,curLayerInfo.w-1 do
			--print('j',j,'i',i,'xxs',xxs,'ys',ys)
			local a=level.getXY(lvlCurLayer,i,j)
			local plane=(curLayerInfo.src>>2)
			--print('a',a,'a.dat',a.dat,'lvlCurLayer',lvlCurLayer)
			if src==level.TILES then
				local tile = p.tiles[a.id + 1]
				if tile ~= nil then
					tile:draw(xxs,ys,lvlZoom,(a.dat>>2)&3,a.dat&1,a.dat&2,false,0--[[TODO extended attributes--]],plane,false)
				end
			elseif src==level.BLOCKS then
				tilemaps.drawBlock(plane,a.id,xxs,ys,a.dat&3,lvlZoom)
			elseif src==level.CHUNKS then
				local chunks = p.chunks
				if a.id<#chunks then
					chunks[a.id + 1]:draw(xxs,ys,lvlZoom,0,0)
				else
					print('a.id>=chunks.amt',a.id,#chunks)
				end
			else
				invalidSource(src)
			end
			if editModeLevel and editX==i and editY==j then
				fl.draw_box(--[[FL.EMBOSSED_FRAME--]]13,xxs,ys,szX*lvlZoom+1,szY*lvlZoom+1,0)
			end
			xxs=xxs+szX*lvlZoom
			if xxs>=rgt.w() then
				break
			end
		end
		ys=ys+szY*lvlZoom
		if ys>=rgt.h() then
			break
		end
	end
	if showSprites then
		if level.objamt[lvlCurLayer+1]>0 then
			for i=0,level.objamt[lvlCurLayer+1]-1 do
				local info = level.getObj(lvlCurLayer, i)

				local ms = projects[info.prjid + 1].metasprites[info.metaid + 1]
				local sg = ms[info.groupid + 1]
				sg:draw((info.x*lvlZoom)+xOff-(szX*lvlZoom),(info.y*lvlZoom)+yOff-(szY*lvlZoom),lvlZoom,true)
			end
		end
	end
	selSlideUpdateMax(src)
	if level.objamt[lvlCurLayer+1]>0 then
		updateSpriteSliders(level.getObj(lvlCurLayer,spriteEdit))
	end
end
selectedIdx=0
function selSlideCB(unused)
	selectedIdx=selSlide:value()
	if editModeLevel then
		local i=level.getXY(lvlCurLayer,editX,editY)
		i.id=selectedIdx
		rgt.damage()
	end
end
function idBondsChk(idraw)
	if idraw<0 then
		return 0
	elseif idraw>selSlide:maximum() then
		return selSlide:maximum()
	else
		return idraw
	end
end
editSpriteMode=false
function editSpritesCB(unused)
	editSpriteMode=editSpriteBtn:value()~=0
end
spriteEdit=0
function updateSpriteSliders(info)
	if level.objamt[lvlCurLayer+1]>0 then
		spriteSel:show()
		spritePrj:show()
		spriteMeta:show()
		spriteGroup:show()
		local oldID=project.curProjectID
		spritePrj:maximum(#projects - 1)
		if info.prjid~=oldID then
			project.set(info.prjid)
		end
		local metasprites = projects.current.metasprites
		spriteMeta:maximum(#metasprites - 1)
		spriteGroup:maximum(#metasprites[spriteMeta:value() + 1] - 1)
		if project.curProjectID~=oldID then
			project.set(oldID)
		end
		spriteSel:value(spriteEdit)
		spritePrj:value(info.prjid)
		spriteMeta:value(info.metaid)
		spriteGroup:value(info.groupid)
	else
		editSpriteMode=false
		spriteSel:hide()
		spritePrj:hide()
		spriteMeta:hide()
		spriteGroup:hide()
	end
end
function setSpriteEditCB(unused)
	spriteEdit=spriteSel:value()
	updateSpriteSliders(level.getObj(lvlCurLayer,spriteEdit))
	rgt.redraw()
end
prj,meta,group=0,0,0
function setSpritePrj(unused)
	prj=spritePrj:value()
	local info=level.getObj(lvlCurLayer,spriteEdit)
	info.prjid=prj
	updateSpriteSliders(info)
	rgt.redraw()
end
function setSpriteMeta(unused)
	meta=spriteMeta:value()
	local info=level.getObj(lvlCurLayer,spriteEdit)
	info.metaid=meta
	updateSpriteSliders(info)
	rgt.redraw()
end
function setSpriteGroup(unused)
	group=spriteGroup:value()
	local info=level.getObj(lvlCurLayer,spriteEdit)
	info.groupid=group
	updateSpriteSliders(info)
	rgt.redraw()
end
function handleLevel(e)
	if editSpriteMode then
		if e==FL.PUSH then
			local x,y=Fl.event_x()-xOff,Fl.event_y()-yOff
			if x>=0 and y>=0 then
				x=(x//lvlZoom)+(scrollX*szX)
				y=(y//lvlZoom)+(scrollY*szY)
				level.appendObj(lvlCurLayer)
				spriteSel:maximum(level.objamt[lvlCurLayer+1]-1)
				spriteEdit=level.objamt[lvlCurLayer+1]-1
				spriteSel:value(spriteEdit)
				local info=level.getObj(lvlCurLayer,spriteEdit)
				updateSpriteSliders(info)
				info.x=x
				info.y=y
				info.prjid=prj
				info.metaid=meta
				info.groupid=group
				rgt.damage()
			end
		end
	else
		local shouldDamage=0
		if e==FL.PUSH then
			local x,y=Fl.event_x(),Fl.event_y()
			x=((x-xOff)//(szX*lvlZoom))+scrollX
			y=((y-yOff)//(szY*lvlZoom))+scrollY
			if x<curLayerInfo.w and y<curLayerInfo.h and x>=0 and y>=0 then
				if Fl.event_button()==FL.LEFT_MOUSE then
					if not (editModeLevel and editX==x and editY==y) then
						local info=level.getXY(lvlCurLayer,x,y)
						info.id=selectedIdx
					end
					editModeLevel=false
				else
					if editModeLevel then
						editModeLevel=false
					else
						local info=level.getXY(lvlCurLayer,x,y)
						selectedIdx=info.id
						selSlide:value(info.id)
						editX=x
						editY=y
						editModeLevel=true
					end
				end
				rgt.damage()
			end
		elseif e==FL.SHORTCUT then
			t=Fl.event_text()
			local addT=0
			if t=='a' then 
				addT=-1
				shouldDamage=1
			elseif t=='s' then 
				addT=1
				shouldDamage=1
			end
			local info=level.getXY(lvlCurLayer,x,y)
			if editModeLevel then
				if t=='h' then
					if editX>0 then
						editX=editX-1
						shouldDamage=1
					end
				elseif t=='j' then
					if editY<(curLayerInfo.h-1) then
						editY=editY+1
						shouldDamage=1
					end
				elseif t=='k' then
					if editY>0 then
						editY=editY-1
						shouldDamage=1
					end
				elseif t=='l' then
					if editX<(curLayerInfo.w-1) then
						editX=editX+1
						shouldDamage=1
					end
				end
				selectedIdx=idBondsChk(info.id+addT)
				info.id=selectedIdx
				selSlide:value(selectedIdx)
			else
				selectedIdx=idBondsChk(info.id+addT)
				selSlide:value(selectedIdx)
			end
		end
		if shouldDamage~=0 then
			rgt.damage()
		end
	end
end
function setLayerSrc(val)
	local p = projects.current
	local plane = (curLayerInfo.src >> 2) + 1
	if val == level.BLOCKS and p.tilemaps[plane].useBlocks ~= true then
		fl.alert("You must first enable blocks in the plane editor")
		lvlSrc:value(curLayerInfo.src&3)
		return
	end
	curLayerInfo.src=(curLayerInfo.src&(~3))|val
	rgt.syncProject()--Needed for selSlideUpdateMax
	selSlideUpdateMax(val)
	setSizePer()
end
function setLayerSrcCB(unused)
	setLayerSrc(lvlSrc:get_index())
	rgt.damage()
end
function resizeLayerCB(unused)
	level.resizeLayer(lvlCurLayer,wLayer:value(),hLayer:value())
	lvlsetlayer(lvlCurLayer)
	setScrollBounds()
	rgt.redraw()
end
function loadS1layout(unused)
	rgt.syncProject()
	local p = projects.current
	if p:have(project.levelMask|project.chunksMask) then
		local fname=fl.file_chooser("Load layout")
		if not (fname == nil or fname == '') then
			local file=assert(io.open(fname,'rb'))
			local str=file:read("*a")--Lua strings can have embedded zeros
			io.close(file)
			local w,h=str:byte(1)+1,str:byte(2)+1
			level.resizeLayer(lvlCurLayer,w,h)
			lvlsetlayer(lvlCurLayer)
			setLayerSrc(level.CHUNKS)
			lvlSrc:value(curLayerInfo.src&3)
			wLayer:value(w)
			hLayer:value(h)
			local idx=3
			for j=0,h-1 do
				for i=0,w-1 do
					info=level.getXY(lvlCurLayer,i,j)
					info.id=str:byte(idx)&127
					info.dat=str:byte(idx)&128
					idx=idx+1
				end
			end
		end
	else
		project.haveMessage(project.levelMask|project.chunksMask)
	end
end
function saveS1layout(unused)
	rgt.syncProject()
	local p = projects.current
	if p:have(project.levelMask) then
		if curLayerInfo.w<=256 and curLayerInfo.h<=256 then
			local fname=fl.file_chooser("Save layout")
			if not (fname == nil or fname == '') then
				local file=assert(io.open(fname,'wb'))
				file:write(string.char(curLayerInfo.w-1))
				file:write(string.char(curLayerInfo.h-1))
				for j=0,curLayerInfo.h do
					for i=0,curLayerInfo.w do
						str=level.getXY(lvlCurLayer,i,j)
						file:write(string.char((info.id&127)|(info.dat&128)))
					end
				end
			end
		else
			fl.alert("The header only allows one byte for width and height. The maximum width and height is 256")
		end
	else
		p:haveMessage(project.levelMask)
	end
end
function appendSpriteCB(unused)
	level.appendObj(lvlCurLayer)
	spriteSel:maximum(level.objamt[lvlCurLayer+1]-1)
	local info=level.getObj(lvlCurLayer,spriteEdit)
	updateSpriteSliders(info)
	rgt.redraw()
end
function deleteSpriteCB(unused)
	if level.objamt[lvlCurLayer+1]>0 then
		level.delObj(lvlCurLayer,spriteEdit)
		if level.objamt[lvlCurLayer+1]>0 then
			spriteSel:maximum(level.objamt[lvlCurLayer+1]-1)
			if spriteEdit>=level.objamt[lvlCurLayer+1] then
				spriteEdit=level.objamt[lvlCurLayer+1]-1
				spriteSel:value(spriteEdit)
			end
		else
			spriteSel:maximum(0)
			spriteEdit=0
		end
		updateSpriteSliders(info)
	else
		editSpriteMode=false
		spriteSel:hide()
		spritePrj:hide()
		spriteMeta:hide()
		spriteGroup:hide()
	end
	rgt.redraw()
end
function initLevelEditor()
	--Note Lua's scoping rules are different than C/C++. All these variables are in fact globals and thus you can access them in the callbacks
	curLayerInfo=level.getInfo(0)
	setSizePer()
	layerSel=fltk.choice(4,64,136,24,"Layer selection:")
	layerSel:align(FL.ALIGN_TOP)
	layerSel:callback(lvlsetLayerCB)
	layerSel:tooltip('Sets the current layer')
	for i=1,#level.names do
		layerSel:add(level.names[i])
	end
	layerName=fltk.input(4,108,136,24,"Layer name:")
	layerName:align(FL.ALIGN_TOP)
	layerName:tooltip('Sets the name for the current layer')
	layerName:callback(setLayerName)
	layerName:value(level.names[1])
	appendBtn=fltk.button(4,144,64,24,"Append layer")
	appendBtn:callback(lvlappend)
	appendBtn:labelsize(11)
	appendSpriteBtn=fltk.button(72,144,64,24,"Append sprite")
	appendSpriteBtn:callback(appendSpriteCB)
	appendSpriteBtn:labelsize(11)
	deleteBtn=fltk.button(4,176,64,24,"Delete layer")
	deleteBtn:labelsize(11)
	deleteBtn:callback(lvldelete)
	deleteSpriteBtn=fltk.button(72,176,64,24,"Delete sprite")
	deleteSpriteBtn:labelsize(11)
	deleteSpriteBtn:callback(deleteSpriteCB)
	spriteBtn=fltk.light_button(4,204,136,24,"Show sprites")
	spriteBtn:box(FL.NO_BOX)
	spriteBtn:down_box(3)--FL_DOWN_BOX and variables beyond that are read as nil for some reason even though functions such as print_r (the function I found here: https://gist.github.com/nrk/31175 ) display their value
	spriteBtn:selection_color(FL.FOREGROUND_COLOR)
	spriteBtn:callback(lvlshowspirtes)
	spriteBtn:type(FL.TOGGLE_BUTTON)
	spriteBtn:value(true)
	selSlide = fltk.value_slider(4,240,136,24,'Index selection')
	selSlide:align(FL.ALIGN_TOP)
	selSlide:tooltip('Sets current (tile|blocks|chunk)')
	selSlide:callback(selSlideCB)
	selSlide:step(1)
	selSlide:maximum(0)
	selSlide:type(FL.HOR_SLIDER)

	tSizeScrl = fltk.value_slider(4,280,136,24,"Tile zoom factor")
	tSizeScrl:align(FL.ALIGN_TOP)
	tSizeScrl:tooltip('Sets the zoom of the tiles')
	tSizeScrl:type(FL.HOR_SLIDER)
	tSizeScrl:step(1)
	tSizeScrl:maximum(16)
	tSizeScrl:minimum(1)
	tSizeScrl:value(1)
	tSizeScrl:callback(setLvlzoomCB)
	wLayer=fltk.int_input(48,308,92,24,"Width:")
	wLayer:value(1)
	wLayer:callback(resizeLayerCB)
	hLayer=fltk.int_input(48,340,92,24,"Height:")
	hLayer:value(1)
	hLayer:callback(resizeLayerCB)
	lvlSrc=fltk.choice(4,380,136,24,"Source selection:")
	lvlSrc:align(FL.ALIGN_TOP)
	lvlSrc:add('Tiles')
	lvlSrc:add('Blocks')
	lvlSrc:add('Chunks')
	lvlSrc:callback(setLayerSrcCB)

	editSpriteBtn=fltk.light_button(4,404,136,24,"Click add sprites?")
	editSpriteBtn:box(FL.NO_BOX)
	editSpriteBtn:down_box(3)
	editSpriteBtn:selection_color(FL.FOREGROUND_COLOR)
	editSpriteBtn:callback(editSpritesCB)
	editSpriteBtn:type(FL.TOGGLE_BUTTON)
	spriteSel=fltk.value_slider(4,436,136,24,"Sprite selection")
	spriteSel:align(FL.ALIGN_TOP)
	spriteSel:type(FL.HOR_SLIDER)
	spriteSel:step(1)
	spriteSel:minimum(0)
	spriteSel:maximum(0)
	spriteSel:value(0)
	spriteSel:callback(setSpriteEditCB)
	spriteSel:hide()
	spritePrj=fltk.value_slider(4,480,136,24,"Sprite's project ID")
	spritePrj:align(FL.ALIGN_TOP)
	spritePrj:type(FL.HOR_SLIDER)
	spritePrj:step(1)
	spritePrj:minimum(0)
	spritePrj:maximum(0)
	spritePrj:value(0)
	spritePrj:callback(setSpritePrj)
	spritePrj:hide()
	spriteMeta=fltk.value_slider(4,524,136,24,"Sprite's meta ID")
	spriteMeta:align(FL.ALIGN_TOP)
	spriteMeta:type(FL.HOR_SLIDER)
	spriteMeta:step(1)
	spriteMeta:minimum(0)
	spriteMeta:maximum(0)
	spriteMeta:value(0)
	spriteMeta:callback(setSpriteMeta)
	spriteMeta:hide()
	spriteGroup = fltk.value_slider(4,564,136,24,"Sprite's group ID")
	spriteGroup:align(FL.ALIGN_TOP)
	spriteGroup:type(FL.HOR_SLIDER)
	spriteGroup:step(1)
	spriteGroup:minimum(0)
	spriteGroup:maximum(0)
	spriteGroup:value(0)
	spriteGroup:callback(setSpriteGroup)
	spriteGroup:hide()

	lvlScrolly=fltk.scrollbar(144,48,24,544)
	lvlScrolly:callback(lvlscrollCB)
	lvlScrolly:hide()
	lvlScrolly:linesize(1)
	lvlScrollx=fltk.scrollbar(168,48,624,24)
	lvlScrollx:type(FL.HORIZONTAL)
	lvlScrollx:callback(lvlscrollCB)
	lvlScrollx:hide()
	lvlScrollx:linesize(1)
end
