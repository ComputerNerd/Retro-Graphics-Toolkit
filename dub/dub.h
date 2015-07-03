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
#ifndef DUB_BINDING_GENERATOR_DUB_H_
#define DUB_BINDING_GENERATOR_DUB_H_

#include <string.h>  // strlen strcmp

#ifndef DUB_ASSERT_KEY
#define DUB_ASSERT_KEY(k, m) strcmp(k, m)
// Use this to avoid the overhead of strcmp in get/set of public attributes.
// if you avoid strcmp, bad keys can map to any other key.
//#define DUB_ASSERT_KEY(k, m) false
#endif
#define KEY_EXCEPTION_MSG "invalid key '%s'"

typedef int LuaStackSize;

#ifndef DUB_EXPORT
#ifdef _WIN32
#define DUB_EXPORT extern "C" __declspec(dllexport)
#else
#define DUB_EXPORT extern "C"
#endif
#endif

#include <lua.h>
#include <lauxlib.h>

#include <string>    // std::string for Exception
#include <exception> // std::exception

// Helpers to check for explicit 'false' or 'true' return values.
#define lua_isfalse(L,i) (lua_isboolean(L,i) && !lua_toboolean(L,i))
#define lua_istrue(L,i)  (lua_isboolean(L,i) && lua_toboolean(L,i))
#define luaL_checkboolean(L,n) (lua_toboolean(L,n))

struct DubUserdata {
  void *ptr;
  bool gc;
};

// ======================================================================
// =============================================== dub::Exception
// ======================================================================
namespace dub {

/** All exceptions raised by dub::check.. are instances of this class. All
 * exceptions deriving from std::exception have their message displayed
 * in Lua (through lua_error).
 */
class Exception : public std::exception {
  std::string message_;
public:
  explicit Exception(const char *format, ...);
  ~Exception() throw();
  const char* what() const throw();
};

class TypeException : public Exception {
public:
  explicit TypeException(lua_State *L, int narg, const char *type, bool is_super = false);
};

/** This class allows an object to be deleted from either C++ or Lua.
 * When Lua deletes the object, dub_destroy is called. When C++ deletes
 * the object, the related userdata is invalidated.
 */
class Object {
public:
  Object() : dub_userdata_(NULL) {}

  /** The destructor marks the userdata as deleted so that Lua no
   * longer tries to access it.
   */
  virtual ~Object() {
    if (dub_userdata_) {
      // Protect from gc.
      dub_userdata_->gc = false;
      // Invalidate Lua userdata.
      dub_userdata_->ptr = NULL;
    }
  }

  /** This is called on object instanciation by dub instead of
   * dub::pushudata to setup dub_userdata_.
   *
   */
  void dub_pushobject(lua_State *L, void *ptr, const char *type_name, bool gc = true);

protected:
  /** Pointer to the userdata. *userdata => pointer to C++ object.
   */
  DubUserdata *dub_userdata_;
};

/** This class creates a 'self' table and prepares a thread
 * that can be used for callbacks from C++ to Lua.
 */
class Thread : public Object {
public:
  Thread()
    : dub_L(NULL) {}
  /** This is called on object instanciation by dub to create the lua
   * thread, prepare the <self> table and setup metamethods. This is
   * called instead of dub::pushudata.
   * <udata> <mt>
   */
  void dub_pushobject(lua_State *L, void *ptr, const char *type_name, bool gc = true);

  /** Push function 'name' found in <self> on the stack with <self> as
   * first argument.
   *
   * Constness is there to make it easy to implement callbacks like
   * int rowCount() const, without requiring users to fiddle with constness
   * which is not a notion part of Lua anyway.
   */
  bool dub_pushcallback(const char *name) const;

  /** Push any lua value from self on the stack.
   */
  void dub_pushvalue(const char *name) const;
  
  /** Execute the protected call. If an error occurs, dub tries to find
   * an 'error' function in <self> and calls this function with the
   * error string. If no error function is found, the error message is
   * just printed out to stderr.
   */
  bool dub_call(int param_count, int retval_count) const;

  /** Lua thread that contains <self> on stack position 1. This lua thread
   * is public to ease object state manipulation from C++ (but stack *must
   * not* be messed up).
   */
  lua_State *dub_L;

protected:
  /** Type name (allows faster check for cast).
   */
  const char *dub_typename_;
};

// ======================================================================
// =============================================== dub::pushclass
// ======================================================================

// To ease storing a LuaRef in a void* pointer.
struct DubRef {
  int ref;

  static int set(lua_State *L, void **ptr, int id) {
    if (lua_isnil(L, id)) {
      cleanup(L, ptr);
    } else {
      DubRef *ref;
      if (*ptr) {
        ref = (DubRef*)*ptr;
        luaL_unref(L, LUA_REGISTRYINDEX, ref->ref);
      } else {
        ref = new DubRef();
        *ptr = ref;
      }
      ref->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    return 0;
  }

  static int push(lua_State *L, void *ptr) {
    if (ptr) {
      DubRef *ref = (DubRef*)ptr;
      lua_rawgeti(L, LUA_REGISTRYINDEX, ref->ref);
      return 1;
    } else {
      return 0;
    }
  }

  static void cleanup(lua_State *L, void **ptr) {
    if (*ptr) {
      DubRef *ref = (DubRef*)*ptr;
      luaL_unref(L, LUA_REGISTRYINDEX, ref->ref);
      delete ref;
      *ptr = NULL;
    }
  }
};

/** Push a custom type on the stack.
 * Since the value is passed as a pointer, we assume it has been created
 * using 'new' and Lua can safely call delete when it needs to garbage-
 * -collect it.
 *
 * Constness: we const cast all passed values to ease passing read-only
 * arguments without requiring users to fiddle with constness which is not
 * a notion part of Lua anyway.
 */
void pushudata(lua_State *L, const void *ptr, const char *type_name, bool gc = true);

template<class T>
struct DubFullUserdata {
  T *ptr;
  T obj;
};

template<class T>
void pushfulldata(lua_State *L, const T &obj, const char *type_name) {
  DubFullUserdata<T> *copy = (DubFullUserdata<T>*)lua_newuserdata(L, sizeof(DubFullUserdata<T>));
  copy->obj = obj;
  // now **copy gives back the object.
  copy->ptr = &copy->obj;

  // the userdata is now on top of the stack

  // set metatable (contains methods)
  luaL_getmetatable(L, type_name);
  lua_setmetatable(L, -2);
}

template<class T>
void pushclass(lua_State *L, const T &obj, const char *type_name) {
  T *copy = new T(obj);
  pushudata(L, (void*)copy, type_name);
}

// ======================================================================
// =============================================== dub::pushclass2
// ======================================================================

/** Push a custom type on the stack and give it the pointer to the userdata.
 * Passing the userdata enables early deletion from C++ that safely
 * invalidates the userdatum by calling 
 */
template<class T>
void pushclass2(lua_State *L, T *ptr, const char *type_name) {
  T **userdata = (T**)lua_newuserdata(L, sizeof(T*));
  *userdata = ptr;

  // Store pointer in class so that it can set it to NULL on destroy with
  // *userdata = NULL
  ptr->luaInit((void**)userdata);
  // <udata>
  luaL_getmetatable(L, type_name);
  // <udata> <mt>
  lua_setmetatable(L, -2);
  // <udata>
}

// ======================================================================
// =============================================== constants
// ======================================================================

typedef struct const_Reg {
  const char *name;
  double value;
} const_Reg;

// register constants in the table at the top
void register_const(lua_State *L, const const_Reg *l);

// ======================================================================
// =============================================== dub_check ...
// ======================================================================

// These provide the same funcionality as their equivalent luaL_check... but they
// throw std::exception which can be caught (eventually to call lua_error).
lua_Number checknumber(lua_State *L, int narg) throw(dub::TypeException);
lua_Integer checkinteger(lua_State *L, int narg) throw(dub::TypeException);
const char *checklstring(lua_State *L, int narg, size_t *len) throw(dub::TypeException);
void **checkudata(lua_State *L, int ud, const char *tname, bool keep_mt = false) throw(dub::Exception);

// Super aware userdata calls (finds userdata inside provided table with table.super).
void **checksdata(lua_State *L, int ud, const char *tname, bool keep_mt = false) throw(dub::Exception);
// Super aware userdata calls that DOES NOT check for dangling pointers (used in 
// __gc binding).
void **checksdata_d(lua_State *L, int ud, const char *tname) throw(dub::Exception);
// Return pointer if the type is correct. Used to resolve overloaded functions when there
// is no other alternative (arg count, native types). We return the pointer so that we can
// optimize away the corresponding 'dub_checksdata'.
void **issdata(lua_State *L, int ud, const char *tname, int type);
// Does not throw exceptions. This method behaves exactly like luaL_checkudata but searches
// for table.super before calling lua_error. We cannot use throw() because of differing
// implementations for luaL_error (luajit throws an exception on luaL_error).
void **checksdata_n(lua_State *L, int ud, const char *tname, bool keep_mt = false);

inline const char *checkstring(lua_State *L, int narg) throw(dub::TypeException) {
  return checklstring(L, narg, NULL);
}

inline int checkboolean(lua_State *L, int narg) throw() {
  return lua_toboolean(L, narg);
}

// This calls lua_Error after preparing the error message with line
// and number.
int error(lua_State *L);

// This is a Lua binding called whenever we ask for obj:deleted() in Lua
int isDeleted(lua_State *L);

/** Protect garbage collection from pointers stored in objects or
 * retrieved in userdata copies.
 */
void protect(lua_State *L, int owner, int original, const char *key);

/** Prepare index function, setup 'type' field and __call metamethod.
 */
void setup(lua_State *L, const char *class_name);

// sdbm function: taken from http://www.cse.yorku.ca/~oz/hash.html
// This version is slightly adapted to cope with different
// hash sizes (and to be easy to write in Lua).
int hash(const char *str, int sz);

// Simple debugging function to print stack content.
void printStack(lua_State *L, const char *msg = NULL);

// Compatibility with luaL_register on lua 5.1 and 5.2
void fregister(lua_State *L, const luaL_Reg *l);


} // dub
  
#endif // DUB_BINDING_GENERATOR_DUB_H_

