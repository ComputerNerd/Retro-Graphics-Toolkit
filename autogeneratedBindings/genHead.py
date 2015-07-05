boilerplate="""/*
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
	Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#pragma once
"""
import os
from glob import glob
inclst=''
addlib=''
for fname in sorted(glob('*.cpp')):
    print('Processing: '+fname)
    base,ext=os.path.splitext(fname)
    inclst+='#include "'+base+'.h"\n'
    baseName=base.replace('FLTK_','').replace('Lua','')
    addlib+='luaopen_'+base+'(L);\nlua_setglobal(L, "'+baseName+'");\n'
    txt=''
    with open(fname) as f:
        txt=f.read()
    if baseName.startswith("Fl_")==True:
        if 'using '+baseName+'::'+baseName+';' not in txt and '"callback"' in txt:
            rpl='Fl_Lua_'+baseName[3:]
            txt=txt.replace(baseName,rpl)
            newinc='#include <FL/'+baseName+'.H>'
            txt=txt.replace('#include <FL/'+rpl+'.H>',newinc)
            txt=txt.replace('int luaopen_FLTK_Fl_Lua','int luaopen_FLTK_Fl')
            idx=txt.index(newinc)+len(newinc)+1
            txt=txt[:idx]+'#include "cbHelper.h"\nclass '+rpl+':public '+baseName+'{\nusing '+baseName+'::'+baseName+';\npublic:\n\tstruct cbInfo ci;\n};\n'+txt[idx:]
            idx=txt.index('delete self;')-len('      ')-1
            txt=txt[:idx]+'\n\tif(self->ci.cb) free(self->ci.cb);\n'+txt[idx:]
            while 1:
                idx=txt.find('retval__ = new',idx)
                if idx<0:
                    break
                idx=txt.find('\n',idx)
                txt=txt[:idx]+'\n\tretval__->ci.L=L;\n\tretval__->ci.cb=0;'+txt[idx:]
            boilerplatefunc='''static int $(REPLACE)_callback(lua_State *L) {
	try {
		$(REPLACE) *self = *(($(REPLACE) **)dub::checksdata(L, 1, "FLTK.$(REPLACE)"));
		int top__ = lua_gettop(L);
		if (top__ >= 3) {
			self->ci.cb=strdup(luaL_checkstring(L,2));
			self->ci.udat=luaL_checkinteger(L,3);
			self->callback(luaWidgetCallbackHelper,&self->ci);
			return 0;
		} else if (top__ >= 2) {
			self->ci.cb=strdup(luaL_checkstring(L,2));
			self->ci.udat=0;
			self->callback(luaWidgetCallbackHelper,&self->ci);
			return 0;
		} else {
			lua_pushstring(L,self->ci.cb);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "callback: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "callback: Unknown exception");
	}
	return dub::error(L);
}
'''.replace('$(REPLACE)',rpl)
            idx=txt.index(boilerplatefunc.split('\n',1)[0])
            idx2=txt.find('\n}',idx)
            txt=txt.replace(txt[idx:idx2+3],boilerplatefunc)
    with open(fname,'w') as f:
        f.write(txt.replace('DUB_EXPORT','').replace('"type"','"Type"').replace('"end"','"End"'))
    with open(base+'.h','w') as f:
        f.write(boilerplate)
        f.write('int luaopen_'+base+'(lua_State *L);\n') 
with open('includes.h','w') as f:
    f.write(inclst)
with open('addlib.c','w') as f:
    f.write(addlib)
