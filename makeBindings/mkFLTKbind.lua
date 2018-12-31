local lub = require 'lub'
local dub = require 'dub'

local inspector = dub.Inspector {
  INPUT    = {
    lub.path '|FL',
  },
}
local binder = dub.LuaBinder()

binder:bind(inspector, {
  -- Mandatory library name. This is used as prefix for class types.
  lib_name = 'FLTK',

  output_directory = lub.path '|bind',

  -- Remove this part in included headers
  header_base = lub.path '|FL',

  -- Open the library with require 'xml.core' (not 'xml') because
  -- we want to add some more Lua methods inside 'xml.lua'.
  luaopen    = 'FLTK',
})
