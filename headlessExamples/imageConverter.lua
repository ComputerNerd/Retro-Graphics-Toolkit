local olddir=unistd.getcwd()
unistd.chdir(libgen.dirname(arg[0]))
local argparse = require "argparse"
unistd.chdir(olddir)
local parser = argparse()
	:name 'Image converter'
	:description 'Converts (an) image(s) to tiles with a map and palette'
parser:option('-d --data','C: C source, A: Assembly, Be: Bex, Bi: Binary','C')
	:args('1')
parser:option('-s --system','G: Sega Genesis, N: NES, GG: Game Gear, M: Master System','G')
	:args('1')
parser:option('-m --tile-map-compression','Specify as index 0="Uncompressed",1="Nemesis",2="Kosinski",3="Enigma",4="Saxman",5="Comper"',0)
	:args('1')
parser:option('-t --tile-compression','Specify as index 0="Uncompressed",1="Nemesis",2="Kosinski",3="Enigma",4="Saxman",5="Comper"',0)
	:args('1')
parser:argument('images','You can specify one or more images.\nIn the same directory the output will be stored with the same base filename but different extensions and with one of the following appended to the base filename:\n\t_tiles\n\t_palette\n\t_map')
	:args('+')
local args = parser:parse()

local sysmap={G=project.segaGenesis,N=project.NES,GG=project.gameGear,M=project.masterSystem}
local datatmap={C=rgt.Cheader,A=rgt.ASM,Be=rgt.BEX,Bi=rgt.Binary}
local sys=sysmap[args.system]
local datat=datatmap[args.data]
local datatExtmap={[rgt.Cheader]='.c',[rgt.ASM]='.asm',[rgt.BEX]='.bex',[rgt.Binary]='.bin'}
local datatExt=datatExtmap[datat]

for k,v in pairs(args.images) do
	if sys~=project.segaGenesis then
		print('Setting to',sys)
		project.setSystem(sys)
	end
	rgt.syncProject()
	tilemaps.loadImage(v)
	local set=settings.new()
	--[[
	struct settings{//TODO avoid hardcoding palette row amount
		bool sprite;//Are we generating the palette for a sprite
		unsigned off[MAX_ROWS_PALETTE];//Offsets for each row
		unsigned alg;//Which algorithm should be used
		bool ditherAfter;//After color quantization should the image be dithered
		bool entireRow;//If true dither entire tilemap at once or false dither each row separately
		unsigned colSpace;//Which colorspace should the image be quantized in
		unsigned perRow[MAX_ROWS_PALETTE];//How many colors will be generated per row
		bool useRow[MAX_ROWS_PALETTE];
		unsigned rowAuto;
		int rowAutoEx[2];
	};
	--]]
	set.sprite=false
	for i=1,palette.rowCnt do
		set:off(i,0)
	end
	set.alg=0
	set.ditherAfter=true
	set.entireRow=false
	set.colSpace=0
	for i=1,palette.rowCnt do
		set:perRow(i,palette.perRow)
	end
	for i=1,palette.rowCnt do
		set:useRow(i,true)
	end
	set.rowAuto=1
	set:rowAutoEx(1,0)
	set:rowAutoEx(2,0)
	tilemaps.generatePalette(set)
	tile.removeDuplicate(false)
	baselabel=libgen.basename(fl.filename_setext(v,'')):gsub('-','_'):gsub(' ','_')
	palette.save(fl.filename_setext(v,'_pal'..datatExt),0,palette.cnt,sys==project.NES,datat,false,baselabel..'_pal')
	tilemaps.save(0,fl.filename_setext(v,'_map'..datatExt),datat,false,args.tile_map_compression,baselabel..'_map',fl.filename_setext(v,'_attributes'..datatExt),baselabel..'_attributes')
	tile.save(fl.filename_setext(v,'_tiles'..datatExt),datat,false,args.tile_compression,baselabel..'_tiles')
	if datat==rgt.Cheader then
		function createHeader(fname,lbl,extra)
			io.output(fname)
			io.write('extern const uint8_t '..lbl..'[];\n')
			if extra~=nil and extra~='' then
				io.write(extra)
			end
			io.close()
		end
		createHeader(fl.filename_setext(v,'_pal'..'.h'),baselabel..'_pal',nil)
		maplbl=baselabel..'_map'
		maplblu=maplbl:upper()
		createHeader(fl.filename_setext(v,'_map'..'.h'),maplbl,string.format("#define %s_W %d\n#define %s_H %d\n",maplblu,tilemaps.width[1],maplblu,tilemaps.heightA[1]))
		tileslbl=baselabel..'_tiles'
		tileslblu=tileslbl:upper()
		createHeader(fl.filename_setext(v,'_tiles'..'.h'),tileslbl,string.format('#define %s_CNT %d\n',tileslblu,tile.amt))
		if sys==project.NES then
			createHeader(fl.filename_setext(v,'_attributes'..'.h'),baselabel..'_attributes',nil)
		end
		io.output(fl.filename_setext(v,'.h'))
		io.write('#include "'..fl.filename_setext(v,'_pal'..'.h')..'"\n')
		io.write('#include "'..fl.filename_setext(v,'_map'..'.h')..'"\n')
		io.write('#include "'..fl.filename_setext(v,'_tiles'..'.h')..'"\n')
		if sys==project.NES then
			io.write('#include "'..fl.filename_setext(v,'_attributes'..'.h')..'"\n')
		end
		io.close()
	end
	--project.save(fl.filename_setext(v,'.rgt'))
	project.append()
	project.remove(0)
end
