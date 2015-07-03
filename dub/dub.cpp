/*
  ==============================================================================

   This file is part of the DUB bindings generator (http://lubyk.org/dub)
   Copyright (c) 2007-2012 by Gaspard Bucher (http://teti.ch).

  ------------------------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

  ==============================================================================
*/
#include "dub/dub.h"

#include <stdlib.h>  // malloc
#include <string.h>  // strlen strcmp
#include <assert.h>  // assert

#define DUB_EXCEPTION_BUFFER_SIZE 256  
#define TYPE_EXCEPTION_MSG "expected %s, found %s"
#define TYPE_EXCEPTION_SMSG "expected %s, found %s (using super)"
#define DEAD_EXCEPTION_MSG  "using deleted %s"
#define DUB_MAX_IN_SHIFT 4294967296

#if LUA_VERSION_NUM > 501
#define DUB_LUA_FIVE_TWO
#else
#define DUB_LUA_FIVE_ONE
#endif 

#define DUB_INIT_CODE "local class = ...\nif class.new then\nsetmetatable(class, {\n __call = function(lib, ...)\n   return lib.new(...)\n end,\n})\nend\n"
#define DUB_INIT_ERR "[string \"Dub init code\"]"
// Define the callback error function. We store the error function in
// self._errfunc so that it can also be used from Lua (this error function
// captures the currently global 'print' which is useful for remote network objects).
#define DUB_ERRFUNC "local self, print = ...\n\
local errfunc = function(...)\n\
  local err = self.error\n\
  if err then\n\
    err(self, ...)\n\
  else\n\
    print('error', ...)\n\
  end\n\
end\n\
self._errfunc = errfunc\n\
return errfunc"

using namespace dub;

void dub::printStack(lua_State *L, const char *msg) {
  int top = lua_gettop(L);
  if (msg) {
    printf("============ %s (%i)\n", msg, top);
  } else {
    printf("============ (%i)\n", top);
  }
  for(int i=1; i<=top; ++i) {
    if (lua_isstring(L, i)) {
      printf("  \"%s\"\n", lua_tostring(L, i));
    } else {
      printf("  %s\n", lua_typename(L, lua_type(L, i)));
    }

  }
  printf("===============================\n");
}  

// ======================================================================
// =============================================== dub::Exception
// ======================================================================
Exception::Exception(const char *format, ...) {
  char buffer[DUB_EXCEPTION_BUFFER_SIZE];
  va_list args;
  va_start(args, format);
    vsnprintf(buffer, DUB_EXCEPTION_BUFFER_SIZE, format, args);
  va_end(args);
  message_ = buffer;
}

Exception::~Exception() throw() {}

const char* Exception::what() const throw() {
  return message_.c_str();
}


TypeException::TypeException(lua_State *L, int narg, const char *type, bool is_super) :
  Exception(is_super ? TYPE_EXCEPTION_SMSG : TYPE_EXCEPTION_MSG, type, luaL_typename(L, narg)) {}

// ======================================================================
// =============================================== dub::Object
// ======================================================================
void Object::dub_pushobject(lua_State *L, void *ptr, const char *tname, bool gc) {
  DubUserdata *udata = (DubUserdata*)lua_newuserdata(L, sizeof(DubUserdata));
  udata->ptr = ptr;
  udata->gc  = gc;
  if (dub_userdata_) {
    // We already have a userdata. Push a new userdata (copy to this item,
    // should never gc).
    assert(!gc);
    udata->gc = false;
  } else {
    // First initialization.
    dub_userdata_ = udata;
  }
  // the userdata is now on top of the stack

  // set metatable (contains methods)
  lua_getfield(L, LUA_REGISTRYINDEX, tname);
  lua_setmetatable(L, -2);
  // <udata>
}

// ======================================================================
// =============================================== dub::Thread
// ======================================================================
void Thread::dub_pushobject(lua_State *L, void *ptr, const char *tname, bool gc) {
  if (dub_L) {
    if (!strcmp(tname, dub_typename_)) {
      // Pushing same type again.

      // We do not care about gc being false here since we share the same userdata
      // object.
      // push self
      lua_pushvalue(dub_L, 1);
      lua_xmove(dub_L, L, 1);
      // <self>
    } else {
      // Type cast.
      assert(!gc);
      dub::pushudata(L, ptr, tname, gc);
      // <udata>
    }
    return;
  }

  // initialization

  //--=============================================== setup super
  lua_newtable(L);
  // <self>
  Object::dub_pushobject(L, ptr, tname, gc);
  // <self> <udata>
  dub_typename_ = tname;
  lua_pushlstring(L, "super", 5);
  // <self> <udata> 'super'
  lua_pushvalue(L, -2);
  // <self> <udata> 'super' <udata>
  lua_rawset(L, -4); // <self>.super = <udata>
  // <self> <udata>

  //--=============================================== setup metatable on self
  lua_getfield(L, LUA_REGISTRYINDEX, tname);
  // <self> <udata> <mt>
  lua_setmetatable(L, -3); // setmetatable(self, mt)
  // <self> <udata>
  
  //--=============================================== setup lua thread
  // Create env table used for garbage collection protection.
  lua_newtable(L);
  // <self> <udata> <env>
  lua_pushvalue(L, -1);
  // <self> <udata> <env> <env>
#ifdef DUB_LUA_FIVE_ONE
  if (!lua_setfenv(L, -3)) { // setfenv(udata, env)
    // <self> <udata> <env>
    lua_pop(L, 3);
    // 
    throw Exception("Could not set userdata env on '%s'.", lua_typename(L, lua_type(L, -3)));
  }
#else
  lua_setuservalue(L, -3);
  // <self> <udata> <env>
#endif

  // <self> <udata> <env>
  dub_L = lua_newthread(L);
  // <self> <udata> <env> <thread>

  // Store the thread in the userdata environment table so it is not 
  // garbage collected too soon.
  luaL_ref(L, -2);
  // <self> <udata> <env>

  //--=============================================== prepare error function
  int error = luaL_loadbuffer(L, DUB_ERRFUNC, strlen(DUB_ERRFUNC), "Dub error function");
  if (error) {
    throw Exception("Error evaluating error function code (%s).", lua_tostring(L, -1));
  }
  // <self> <udata> <env> (errloader)

  lua_pushvalue(L, -4);
  // <self> <udata> <env> (errloader) <self>
  
#ifdef DUB_LUA_FIVE_ONE
  lua_getfield(L, LUA_GLOBALSINDEX, "print");
#else
  lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
  // <self> <udata> <env> <errloader> <self> <_G>
  lua_getfield(L, -1, "print");
  // <self> <udata> <env> (errloader) <self> <_G> (print)
  lua_remove(L, -2);
#endif

  // <self> <udata> <env> (errloader) <self> (print)
  
  error = lua_pcall(L, 2, 1, 0);
  if (error) {
    throw Exception("Error executing error function code (%s).", lua_tostring(L, -1));
  }
  

  // <self> <udata> <env> <errfunc>
  lua_remove(L, -2);
  // <self> <udata> <errfunc>
  lua_remove(L, -2);
  // <self> <errfunc>

  //--=============================================== prepare thread stack
  // Transfer a copy of <self> to thread stack
  lua_pushvalue(L, -2);
  // <self> <errfunc> <self>
  lua_pushvalue(L, -2);
  // <self> <errfunc> <self> <errfunc>
  lua_xmove(L, dub_L, 2);
  lua_pop(L, 1);

  // dub_L: <self> <errfunc>
  // L:     <self>
}

bool Thread::dub_pushcallback(const char *name) const {
  lua_State *L = const_cast<lua_State *>(dub_L);
  lua_getfield(L, 1, name);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return false;
  } else {
    lua_pushvalue(L, 1);
    // ... <func> <self>
    return true;
  }
}

void Thread::dub_pushvalue(const char *name) const {
  lua_State *L = const_cast<lua_State *>(dub_L);
  lua_getfield(L, 1, name);
}

bool Thread::dub_call(int param_count, int retval_count) const {
  lua_State *L = const_cast<lua_State *>(dub_L);
  int status = lua_pcall(L, param_count, retval_count, 2);
  if (status) {
    if (status == LUA_ERRRUN) {
      // failure properly handled by the error handler
    } else if (status == LUA_ERRMEM) {
      // memory allocation failure
      fprintf(stderr, "Memory allocation failure (%s).\n", lua_tostring(dub_L, -1));
    } else {
      // error in error handler
      fprintf(stderr, "Error in error handler (%s).\n", lua_tostring(dub_L, -1));
    }
    lua_pop(dub_L, 1);
    return false;
  }
  return true;
}




// ======================================================================
// =============================================== dub::error
// ======================================================================
// This calls lua_Error after preparing the error message with line
// and number.
int dub::error(lua_State *L) {
  // ... <msg>
  luaL_where(L, 1);
  // ... <msg> <where>
  // Does it match 'Dub init code' ?
  const char *w = lua_tostring(L, -1);
  if (!strncmp(w, DUB_INIT_ERR, strlen(DUB_INIT_ERR))) {
    // error in ctor, show calling place, not dub init code.
    lua_pop(L, 1);
    luaL_where(L, 2);
  }
  // ... <msg> <where>
  lua_pushvalue(L, -2);
  // ... <msg> <where> <msg>
  lua_remove(L, -3);
  // ... <where> <msg>
  lua_concat(L, 2);
  return lua_error(L);
}




// ======================================================================
// =============================================== dub::protect
// ======================================================================

// TODO: Can we make this faster ?
inline void push_own_env(lua_State *L, int ud) {
#ifdef DUB_LUA_FIVE_ONE
  lua_getfenv(L, ud);
  // ... <udata> ... <env>
  lua_pushstring(L, ".");
  // ... <udata> ... <env> "."
  lua_rawget(L, -2); // <env>["."]
  // ... <udata> ... <env> <??>
  if (!lua_rawequal(L, -1, ud)) {
    // ... <udata> ... <env> <nil>
    // does not have it's own env table
    lua_pop(L, 2);
    // ... <udata> ... 
#else
  lua_getuservalue(L, ud);
  // ... <udata> ... <env/nil>
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
#endif
    // ... <udata> ... 
    // Create env table
    lua_newtable(L);
    // ... <udata> ... <env>
    lua_pushstring(L, ".");
    // ... <udata> ... <env> "."
    lua_pushvalue(L, ud);
    // ... <udata> ... <env> "." <udata>
    lua_rawset(L, -3); // env["."] = udata
    // ... <udata> ... <env>
    lua_pushvalue(L, -1);
    // ... <udata> ... <env> <env>
#ifdef DUB_LUA_FIVE_ONE
    if (!lua_setfenv(L, ud)) {
      luaL_error(L, "Could not set userdata env on '%s'.", lua_typename(L, lua_type(L, ud)));
    }
#else
    lua_setuservalue(L, ud);
#endif
    // ... <udata> ... <env>
  } else {
    // ... <udata> ... <env> <self>
    // has its own env table
    lua_pop(L, 1);
    // ... <udata> ... <env>
  }                            
}

void dub::protect(lua_State *L, int owner, int original, const char *key) {
  // Point to original to avoid original gc before owner.
  push_own_env(L, owner);
  // ... <env>
  lua_pushvalue(L, original);
  // ... <env> <original>
  lua_setfield(L, -2, key); // env["key"] = <original>
  // ... <env>
  lua_pop(L, 1);
  // ...
}

// ======================================================================
// =============================================== dub::pushudata
// ======================================================================

void dub::pushudata(lua_State *L, const void *cptr, const char *tname, bool gc) {
  // To avoid users spending time with const issues.
  void *ptr = const_cast<void*>(cptr);
  // If anything is changed here, it must be reflected in dub::Object::dub_pushobject.
  DubUserdata *userdata = (DubUserdata*)lua_newuserdata(L, sizeof(DubUserdata));
  userdata->ptr = ptr;
  if (!gc) {
    // Point to original (self) to avoid original gc.
    dub::protect(L, lua_gettop(L), 1, "_");
  }

  userdata->gc = gc;

  // the userdata is now on top of the stack
  luaL_getmetatable(L, tname);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    // create empty metatable on the fly for opaque types.
    luaL_newmetatable(L, tname);
  }
  // <udata> <mt>

  // set metatable (contains methods)
  lua_setmetatable(L, -2);
}

// ======================================================================
// =============================================== dub::check ...
// ======================================================================
// These methods are slight adaptations from luaxlib.c
// Copyright (C) 1994-2008 Lua.org, PUC-Rio.

lua_Number dub::checknumber(lua_State *L, int narg) throw(TypeException) {
#ifdef DUB_LUA_FIVE_ONE
  lua_Number d = lua_tonumber(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
    throw TypeException(L, narg, lua_typename(L, LUA_TNUMBER));
  return d;
#else
  int isnum = 0;
  lua_Number d = lua_tonumberx(L, narg, &isnum);
  if (!isnum)
    throw TypeException(L, narg, lua_typename(L, LUA_TNUMBER));
  return d;
#endif
}

lua_Integer dub::checkinteger(lua_State *L, int narg) throw(TypeException) {
#ifdef DUB_LUA_FIVE_ONE
  lua_Integer d = lua_tointeger(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
    throw TypeException(L, narg, lua_typename(L, LUA_TNUMBER));
  return d;
#else
  int isnum = 0;
  lua_Integer d = lua_tointegerx(L, narg, &isnum);
  if (!isnum)
    throw TypeException(L, narg, lua_typename(L, LUA_TNUMBER));
  return d;
#endif
}

const char *dub::checklstring(lua_State *L, int narg, size_t *len) throw(TypeException) {
  const char *s = lua_tolstring(L, narg, len);
  if (!s) throw TypeException(L, narg, lua_typename(L, LUA_TSTRING));
  return s;
}

void **dub::checkudata(lua_State *L, int ud, const char *tname, bool keep_mt) throw(dub::Exception) {
  void **p = (void**)lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {
        // same (correct) metatable
        if (!keep_mt) {
          lua_pop(L, 2);
        } else {
          // keep 1 metatable on top (needed by bindings)
          lua_pop(L, 1);
        }
        if (!*p) {
          throw dub::Exception(DEAD_EXCEPTION_MSG, tname);
        }
        return p;
      }
    }
  }
  throw TypeException(L, ud, tname); /* else error */
  return NULL;  /* to avoid warnings */
}


static inline void **dub_cast_ud(lua_State *L, int ud, const char *tname) {
  // .. <ud> ... <mt> <mt>
  lua_pop(L, 1);
  // ... <ud> ... <mt>
  // TODO: optmize by putting this cast value in the registry.
  lua_pushlstring(L, "_cast_", 6);
  // ... <ud> ... <mt> "_cast_"
  lua_rawget(L, -2);
  if (lua_isfunction(L, -1)) {
    lua_pushvalue(L, ud);
    lua_pushstring(L, tname);
    // ... <ud> ... <mt> cast_func <ud> "OtherType"
    lua_call(L, 2, 1);
    // ... <ud> ... <mt> <ud>
    void **p = (void**)lua_touserdata(L, -1);
    if (p != NULL) {
      // done
      return p;
    }
  }

  // ... <ud> ... <mt> nil
  // Does not change stack size (only last element).
  return NULL;
}

static inline void **getsdata(lua_State *L, int ud, const char *tname, bool keep_mt) throw() {
  void **p = (void**)lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {
        // same (correct) metatable
        lua_pop(L, keep_mt ? 1 : 2);
      } else {
        p = dub_cast_ud(L, ud, tname);
        // ... <ud> ... <ud> <mt/nil>
        if (p && keep_mt) {
          lua_remove(L, -2);
        } else {
          lua_pop(L, 2);
        }
      }
    }
  } else if (lua_istable(L, ud)) {
    if (ud < 0) {
      ud = lua_gettop(L) + 1 + ud;
    }
    // get p from super
    // ... <ud> ...
    // TODO: optimize by storing key in registry ?
    lua_pushlstring(L, "super", 5);
    // ... <ud> ... 'super'
    lua_rawget(L, ud);
    // ... <ud> ... <ud?>
    p = (void**)lua_touserdata(L, -1);
    if (p != NULL) {
      // ... <ud> ... <ud>
      if (lua_getmetatable(L, -1)) {  /* does it have a metatable? */
        // ... <ud> ... <ud> <mt>
        lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
        // ... <ud> ... <ud> <mt> <mt>
        if (lua_rawequal(L, -1, -2)) {
          // same (correct) metatable
          lua_remove(L, -3);
          // ... <ud> ... <mt> <mt>
          lua_pop(L, keep_mt ? 1 : 2);
        } else {
          lua_remove(L, -3);
          // ... <ud> ... <mt> <mt>
          p = dub_cast_ud(L, ud, tname);
          // ... <ud> ... <ud> <mt/nil>
          if (p && keep_mt) {
            lua_remove(L, -2);
            // ... <ud> ... <mt>
          } else {
            lua_pop(L, 2);
            // ... <ud> ...
          }
        }
      } else {
        lua_pop(L, 1);
        // ... <ud> ...
      }
    } else {
      lua_pop(L, 1);
      // ... <ud> ...
    }
  }
  return p;
}

void **dub::checksdata_n(lua_State *L, int ud, const char *tname, bool keep_mt) {
  void **p = getsdata(L, ud, tname, keep_mt);
  if (!p) {
    luaL_error(L, TYPE_EXCEPTION_MSG, tname, luaL_typename(L, ud));
  } else if (!*p) {
    // dead object
    luaL_error(L, DEAD_EXCEPTION_MSG, tname);
  }
  return p;
}

void **dub::issdata(lua_State *L, int ud, const char *tname, int type) {
  if (type == LUA_TUSERDATA || type == LUA_TTABLE) {
    void **p = getsdata(L, ud, tname, false);
    if (!p) {
      return NULL;
    } else if (!*p) {
      // dead object
      throw dub::Exception(DEAD_EXCEPTION_MSG, tname);
    } else {
      return p;
    }
  } else {
    return NULL;
  }
}

void **dub::checksdata(lua_State *L, int ud, const char *tname, bool keep_mt) throw(dub::Exception) {
  void **p = getsdata(L, ud, tname, keep_mt);
  if (!p) {
    throw dub::TypeException(L, ud, tname);
  } else if (!*p) {
    // dead object
    throw dub::Exception(DEAD_EXCEPTION_MSG, tname);
  }
  return p;
}

void **dub::checksdata_d(lua_State *L, int ud, const char *tname) throw(dub::Exception) {
  void **p = getsdata(L, ud, tname, false);
  if (!p) {
    throw dub::TypeException(L, ud, tname);
  }
  // do not check for dead objects
  return p;
}

// ======================================================================
// =============================================== dub::setup
// ======================================================================

void dub::setup(lua_State *L, const char *type_name) {
  // meta-table should be on top
  // <mt>
  lua_getfield(L, -1, "__index");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_pushvalue(L, -1);
    // <mt>.__index = <mt>
    lua_setfield(L, -2, "__index");
  } else {
    // We already have a custom __index metamethod.
    lua_pop(L, 1);
  }
  // <mt>
  lua_pushstring(L, type_name);
  // <mt> "type_name"
  // <mt>."type" = "type_name"
  lua_setfield(L, -2, "type");

  // <mt>

  // Setup the __call meta-table with an upvalue
  // printf("%s\n", DUB_INIT_CODE);
  /*
  local class = ...
  -- new can be nil for abstract types
  if class.new then
    setmetatable(class, {
      __call = function(lib, ...)
        -- We could keep lib.new in an upvalue but this
        -- prevents rewriting class.new in Lua which is
        -- very useful.
        return lib.new(...)
      end,
    })
  end
  */
  int error = luaL_loadbuffer(L, DUB_INIT_CODE, strlen(DUB_INIT_CODE), "Dub init code");
  if (error) {
    fprintf(stderr, "%s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
  } else {
    // <mt> <func>
    lua_pushvalue(L, -2);
    // <mt> <func> <mt>
    error = lua_pcall(L, 1, 0, 0);
    if (error) {
      fprintf(stderr, "%s", lua_tostring(L, -1));
      lua_pop(L, 1);  /* pop error message from the stack */
    }
  }
  // free(lua_code);
  // <mt>
}

int dub::hash(const char *str, int sz) {
  unsigned int h = 0;
  int c;

  while ( (c = *str++) ) {
    // FIXME: integer constant is too large for 'long' type
    unsigned int h1 = (h << 6)  % DUB_MAX_IN_SHIFT;
    unsigned int h2 = (h << 16) % DUB_MAX_IN_SHIFT;
    h = c + h1 + h2 - h;
    h = h % DUB_MAX_IN_SHIFT;
  }
  return h % sz;
}

// register constants in the table at the top
void dub::register_const(lua_State *L, const dub::const_Reg*l) {
  for (; l->name; l++) {
    // push each constant into the table at top
    lua_pushnumber(L, l->value);
    lua_setfield(L, -2, l->name);
  }
}

// This is called whenever we ask for obj:deleted() in Lua
int dub::isDeleted(lua_State *L) {
  void **p = (void**)lua_touserdata(L, 1);
  if (p == NULL && lua_istable(L, 1)) {
    // get p from super
    // <ud>
    // TODO: optimize by storing key in registry ?
    lua_pushlstring(L, "super", 5);
    // <ud> 'super'
    lua_rawget(L, 1);
    // <ud> <ud?>
    p = (void**)lua_touserdata(L, 2);
    lua_pop(L, 1);
    // <ud>
  }
  lua_pushboolean(L, p && !*p);
  return 1;
}

// Compatibility with luaL_register on lua 5.1 and 5.2
void dub::fregister(lua_State *L, const luaL_Reg *l) {
#ifdef DUB_LUA_FIVE_ONE
  luaL_register(L, NULL, l);
#else
  luaL_setfuncs(L, l, 0);
#endif
}


