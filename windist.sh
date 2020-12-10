#!/bin/sh
# Make a 7z archive for Windows users.
i586-w64-mingw32-strip lua-5.1.5/src/*.exe RetroGraphicsToolkit.exe
htmlmin help-src.html help.html
rm -f RetroGraphicsToolkit.exe.7z
mv serpent serpent-tmp
mkdir serpent
cp -r serpent-tmp/src serpent
7z a -t7z -m0=lzma -mx=9 -mlc=7 -mmc=1000000000 -mfb=273 -ms=on RetroGraphicsToolkit.exe.7z ./RetroGraphicsToolkit.exe ./lua51.dll ./bit32.dll ./string.dll ./table.dll ./utf8.dll ./lua-doc/ ./luajit-doc/ ./lua ./*.lua ./Contributing ./README.md luaExamples/ headlessExamples/ ./libgcc_s_dw2-1.dll ./Manual/ help.html ./luajit.exe ./serpent/ lua-5.1.5/src/lua51.dll lua-5.1.5/src/lua.exe lua-5.1.5/src/luac.exe
rm -rf serpent
mv serpent-tmp serpent
# For testing in a virtual machine.
#mkisofs -o RetroGraphicsToolkit.iso ./RetroGraphicsToolkit.exe.7z
