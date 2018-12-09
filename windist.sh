#!/bin/sh
#Make a 7z archive for windows users, used for releasing the windows version of Retro Graphics Toolkit
rm -f RetroGraphicsToolkit.exe.7z
cp ./Manual/Manual.pdf .
7z a -t7z -m0=lzma -mx=9 -mlc=8 -mmc=1000000000 -mfb=273 -ms=on RetroGraphicsToolkit.exe.7z ./RetroGraphicsToolkit.exe ./lua53.dll ./*.lua ./Contributing ./README.md luaExamples/ headlessExamples/ ./Manual.pdf

