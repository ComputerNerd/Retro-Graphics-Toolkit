-- Retro Graphics Toolkit is compiled with the Zlib Lua binding from https://github.com/brimworks/lua-zlib
-- That is where you will find documentation for The Zlib binding.
major, minor, patch = zlib.version()
fl.message(string.format('%d.%d.%d', major, minor, patch))
