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
	Copyright Sega16 (or whatever you wish to call me) (2012-2016)
--]]
function mysplit(inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={} ; i=1
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                t[i] = str
                i = i + 1
        end
        return t
end
function packCommaStr(w,pstr,hexstr)
	local str=''
	w=w:gsub('%[','')
	w=w:gsub('%]','')
	w=w:gsub('{','')
	w=w:gsub('}','')
	w=w:gsub('=','')
	for n in w:gmatch('([^,]+)') do
		local nu
		if n:sub(1,#hexstr)==hexstr then
			nu=tonumber(n:sub(#hexstr+1),16)
		else
			nu=tonumber(n)
		end
		if nu==nil then
			fl.alert('tonumber() returned nil.\nInput was: '..n)
		else
			str=str..string.pack(pstr,nu)
		end
	end
	return str
end
function filereaderProcessText(tp,relptr,offbits,be,t,fname)-- Converts text to tables
	if relptr==true then
		-- Not yet supported.
		fl.alert('Error: should be unreachable')
	else
		if tp==rgt.tBinary then
			return {{fl.filename_name(fname),t}}-- Simply return a table with one element which contains the input
		elseif tp==rgt.tCheader then
			t=t:gsub("/%*.-%*/", "" )--From http://rosettacode.org/wiki/Strip_block_comments#Lua
			t=t:gsub("//.-[^\r\n]+", "" )
			t=t:gsub("static","")
			t=t:gsub("const","")
			t=t:gsub("long long","int64_t")
			t=t:gsub("uint","unsigned int")--Solve two problems at once. Simplifies handling of uintx_t and allows for uint to be used as unsigned int.
			t=t:gsub(";","")
			t = t:gsub('#include.*["%<].*["%>]', '') -- Remove include statements.
			t = t:gsub('(\r?\n)%s*\r?\n', '%1') -- Remove blank lines. Based on: http://lua-users.org/wiki/SciteDeleteBlankLines
			local sizes={char='b',short='h',int='i',long='j',int8_t='b',int16_t='h',int32_t='i',int64_t='j'}
			local array={}--{unsigned,size,{data}}
			local outArray
			local firstArray=true
			local needsName
			local idx=0
			local be=false
			local unsigned
			local holdType
			for w in t:gmatch("%S+") do
				if firstArray then
					idx=idx+1
					array[idx]={'',''}
					firstArray=false
					outArray=true
					needsName=true
					unsigned=false
					holdType=''
				end
				if outArray then
					if w=='unsigned' then
						unsigned=true
					elseif sizes[w]~=nil then
						local chr=sizes[w]
						if unsigned then
							chr=chr:upper()
						end
						if be then
							holdType='>'..chr
						else
							holdType='<'..chr
						end
						outArray=false
					else
						fl.alert('Unknown variable keyword: '..w..'\nPlease modify filereader.lua to add support for it. Look for the table called sizes')
						return nil
					end
				else
					if needsName then
						w=mysplit(w,'[')
						array[idx][1]=w[1]
						if w[2]~=nil then
							array[idx][2]=array[idx][2]..packCommaStr(w[2],holdType,'0x')
						end
						needsName=false
					else
						array[idx][2]=array[idx][2]..packCommaStr(w,holdType,'0x')
						if w:find('}')~=nil then
							firstArray=true
						end
					end
				end
			end
			return array
		elseif tp==rgt.tBEX or tp==rgt.tASM then
			local sizes={}
			local holdType
			if tp==rgt.tASM then
				t=t:gsub(";.-[^\r\n]+", "" )
				if be then
					sizes={['dc.b']='>B',['dc.w']='>H',['dc.l']='>I'}
				else
					sizes={['dc.b']='<B',['dc.w']='<H',['dc.l']='<I'}
				end
				local holdType=sizes['dc.b']
			else
				t=t:gsub("'.-[^\r\n]+", "" )
				if be then
					sizes={data='>B',dataint='>H',datalong='>I'}
				else
					sizes={data='<B',dataint='<H',datalong='<I'}
				end
				local holdType=sizes['data']
			end
			local array={}
			local anyArray=false
			local idx=0
			for w in t:gmatch("%S+") do
				if w:find(':')~=nil then
					idx=idx+1
					array[idx]={w:gsub(':',''),''}
					anyArray=true
				else
					if anyArray then
						if sizes[w]~=nil then
							holdType=sizes[w]
						else
							array[idx][2]=array[idx][2]..packCommaStr(w,holdType,'$')
						end
					else
						fl.alert('Unhandled word: '..w)
						return nil
					end
				end
			end
			return array
		end
	end
	return nil
end
