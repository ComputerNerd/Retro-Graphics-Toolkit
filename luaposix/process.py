#!/usr/bin/python2.7
# Fixes luaposix to run on Retro Graphics Toolkit
import glob,os,subprocess
def fixit(i):
    with open(i) as f:
        old=f.read()
    with open(i+'pp','w') as f:
        f.write(old.replace('#include "_helpers.c"','#include "_helpers.h"').replace('LUALIB_API','').replace('LUA_API',''))
    os.remove(i)
for i in glob.glob('ext/posix/*.c'):
    if 'posix.c' in i:
        os.remove(i)
    else:
        fixit(i)
for i in glob.glob('ext/posix/sys/*.c'):
    fixit(i)
old=os.getcwd()
os.chdir('ext/posix/')
out=subprocess.check_output(['grep','--no-filename','-Ir','luaopen']).strip()
os.chdir(old)
with open('posix.h','w') as f:
    for l in out.split('\n'):
        f.write(''.join(['int ',l,';\n']))
print(out.replace('(lua_State *L)','(L);'))
