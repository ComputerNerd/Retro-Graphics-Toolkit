#!/bin/sh
# Make a 7z archive for Windows users.
htmlmin help-src.html help.html
rm -f RetroGraphicsToolkit.exe.7z
mv serpent serpent-tmp
mkdir serpent
cp -r serpent-tmp/src serpent
7z a -t7z -m0=lzma -mx=9 -mlc=8 -mmc=1000000000 -mfb=273 -ms=on RetroGraphicsToolkit.exe.7z ./RetroGraphicsToolkit.exe ./lua51.dll ./bit32.dll ./string.dll ./table.dll ./utf8.dll ./lua-doc/ ./luajit-doc/ ./lua ./*.lua ./Contributing ./README.md luaExamples/ headlessExamples/ ./libgcc_s_dw2-1.dll ./Manual/ help.html ./luajit.exe ./serpent/
rm -rf serpent
mv serpent-tmp serpent
