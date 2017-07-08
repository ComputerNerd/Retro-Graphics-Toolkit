/*
 * POSIX library for Lua 5.1, 5.2 & 5.3.
 * Copyright (C) 2013-2016 Gary V. Vaughan
 * Copyright (C) 2010-2013 Reuben Thomas <rrt@sc3d.org>
 * Copyright (C) 2008-2010 Natanael Copa <natanael.copa@gmail.com>
 * Clean up and bug fixes by Leo Razoumov <slonik.az@gmail.com> 2006-10-11
 * Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br> 07 Apr 2006 23:17:49
 * Based on original by Claudio Terra for Lua 3.x.
 * With contributions by Roberto Ierusalimschy.
 * With documentation from Steve Donovan 2012
 */

#ifndef LUAPOSIX__HELPERS_C
#define LUAPOSIX__HELPERS_C 1
#define PACKAGE_STRING "luaposix"
#define PACKAGE_NAME "luaposix"

#include "config.h"

#include <errno.h>
#ifndef  __MINGW32__
#include <grp.h>
#include <pwd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>		/* for _POSIX_VERSION */

#if !defined PATH_MAX
# define PATH_MAX 1024
#endif

/* Some systems set _POSIX_C_SOURCE over _POSIX_VERSION! */
#if _POSIX_C_SOURCE >= 200112L || _POSIX_VERSION >= 200112L || _XOPEN_SOURCE >= 600
# define LPOSIX_2001_COMPLIANT 1
#endif

#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
# define LPOSIX_2008_COMPLIANT 1
# ifndef LPOSIX_2001_COMPLIANT
#   define LPOSIX_2001_COMPLIANT
# endif
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM < 503
#  define lua_isinteger lua_isnumber
#  if LUA_VERSION_NUM == 501
#    include "compat-5.2.c"
#  endif
#endif

#if LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
#  define lua_objlen lua_rawlen
#  define lua_strlen lua_rawlen
#  define luaL_openlib(L,n,l,nup) luaL_setfuncs((L),(l),(nup))
#  define luaL_register(L,n,l) (luaL_newlib(L,l))
#endif

#ifndef STREQ
#  define STREQ(a, b)     (strcmp (a, b) == 0)
#endif

/* Mark unused parameters required only to match a function type
   specification. */
#ifdef __GNUC__
#  define LPOSIX_UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define LPOSIX_UNUSED(x) UNUSED_ ## x
#endif

/* LPOSIX_STMT_BEG/END are used to create macros that expand to a
   single compound statement in a portable way. */
#if defined __GNUC__ && !defined __STRICT_ANSI__ && !defined __cplusplus
#  define LPOSIX_STMT_BEG	(void)(
#  define LPOSIX_STMT_END	)
#else
#  if (defined sun || defined __sun__)
#    define LPOSIX_STMT_BEG	if (1)
#    define LPOSIX_STMT_END	else (void)0
#  else
#    define LPOSIX_STMT_BEG	do
#    define LPOSIX_STMT_END	while (0)
#  endif
#endif


/* The extra indirection to these macros is required so that if the
   arguments are themselves macros, they will get expanded too.  */
#define LPOSIX__SPLICE(_s, _t)	_s##_t
#define LPOSIX_SPLICE(_s, _t)	LPOSIX__SPLICE(_s, _t)

#define LPOSIX__STR(_s)		#_s
#define LPOSIX_STR(_s)		LPOSIX__STR(_s)

/* The +1 is to step over the leading '_' that is required to prevent
   premature expansion of MENTRY arguments if we didn't add it.  */
#define LPOSIX__STR_1(_s)	(#_s + 1)
#define LPOSIX_STR_1(_s)	LPOSIX__STR_1(_s)

#define LPOSIX_CONST(_f)	LPOSIX_STMT_BEG {			\
					lua_pushinteger(L, _f);		\
					lua_setfield(L, -2, #_f);	\
				} LPOSIX_STMT_END

#define LPOSIX_FUNC(_s)		{LPOSIX_STR_1(_s), (_s)}

#define pushokresult(b)	pushboolresult((int) (b) == OK)

#ifndef errno
extern int errno;
#endif


#define pushintegerfield(k,v) LPOSIX_STMT_BEG {				\
	lua_pushinteger(L, (lua_Integer) v); lua_setfield(L, -2, k);	\
} LPOSIX_STMT_END

#define pushnumberfield(k,v) LPOSIX_STMT_BEG {				\
	lua_pushnumber(L, (lua_Number) v); lua_setfield(L, -2, k);	\
} LPOSIX_STMT_END

#define pushstringfield(k,v) LPOSIX_STMT_BEG {				\
	if (v) {							\
		lua_pushstring(L, (const char *) v);			\
		lua_setfield(L, -2, k);					\
	}								\
} LPOSIX_STMT_END

#define pushlstringfield(k,v,l) LPOSIX_STMT_BEG {			\
	if (l > 0 && v) {						\
		lua_pushlstring(L, (const char *) v, (size_t) l);	\
		lua_setfield(L, -2, k);					\
	}								\
} LPOSIX_STMT_END

#define pushliteralfield(k,v) LPOSIX_STMT_BEG {				\
	if (v) {							\
		lua_pushliteral(L, v);					\
		lua_setfield(L, -2, k);					\
	}								\
} LPOSIX_STMT_END

#define settypemetatable(t) LPOSIX_STMT_BEG {				\
	if (luaL_newmetatable(L, t) == 1)				\
		pushliteralfield("_type", t);				\
	lua_setmetatable(L, -2);					\
} LPOSIX_STMT_END

#define setintegerfield(_p, _n) pushintegerfield(LPOSIX_STR(_n), _p->_n)
#define setnumberfield(_p, _n) pushnumberfield(LPOSIX_STR(_n), _p->_n)
#define setstringfield(_p, _n) pushstringfield(LPOSIX_STR(_n), _p->_n)


#define checkstringfield(L,i,s) (checklstringfield(L,i,s,NULL))

#define pushboolresult(b)	(lua_pushboolean(L, (b)), 1)

#define pushintresult(n)	(lua_pushinteger(L, (n)), 1)

#define pushstringresult(s)	(lua_pushstring(L, (s)), 1)

void checkfieldnames(lua_State *L, int index, int n, const char * const S[]);
#define checkfieldnames(L,i,S) (checkfieldnames)(L,i,sizeof(S)/sizeof(*S),S)

int argtypeerror(lua_State *L, int narg, const char *expected);
void checktype(lua_State *L, int narg, int t, const char *expected);
lua_Integer checkinteger(lua_State *L, int narg, const char *expected);
int checkint(lua_State *L, int narg);
long checklong(lua_State *L, int narg);
int optboolean(lua_State *L, int narg, int def);
int optint(lua_State *L, int narg, lua_Integer def);
const char * optstring(lua_State *L, int narg, const char *def);
void checknargs(lua_State *L, int maxargs);
void checkfieldtype(lua_State *L, int index, const char *k, int expect_type, const char *expected);
int checkintfield(lua_State *L, int index, const char *k);
int checknumberfield(lua_State *L, int index, const char *k);
const char * checklstringfield(lua_State *L, int index, const char *k, size_t *plen);
int optintfield(lua_State *L, int index, const char *k, int def);
const char * optstringfield(lua_State *L, int index, const char *k, const char *def);
int pusherror(lua_State *L, const char *info);
int pushresult(lua_State *L, int i, const char *info);
void badoption(lua_State *L, int i, const char *what, int option);
int binding_notimplemented(lua_State *L, const char *fname, const char *libname);
#endif /*LUAPOSIX__HELPERS_C*/
