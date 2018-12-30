#pragma once
#include "lua.h"
#ifdef __cplusplus
extern "C" {
#endif
LUALIB_API int luaopen_zlib(lua_State * const L);
#ifdef __cplusplus
}
#endif
