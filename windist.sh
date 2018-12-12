#!/bin/sh
# Make a 7z archive for Windows users.
rm -f RetroGraphicsToolkit.exe.7z
cp ./Manual/Manual.pdf .
7z a -t7z -m0=lzma -mx=9 -mlc=8 -mmc=1000000000 -mfb=273 -ms=on RetroGraphicsToolkit.exe.7z ./RetroGraphicsToolkit.exe ./lua53.dll ./Manual.pdf ./*.lua ./Contributing ./README.md luaExamples/ headlessExamples/

