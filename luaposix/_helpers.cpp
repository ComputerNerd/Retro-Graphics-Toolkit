#include "_helpers.h"
#include "config.h"

/* ========================= *
 * Bad argument diagnostics. *
 * ========================= */


int argtypeerror(lua_State *L, int narg, const char *expected)
{
	const char *got = luaL_typename(L, narg);
	return luaL_argerror(L, narg,
		lua_pushfstring(L, "%s expected, got %s", expected, got));
}

void checktype(lua_State *L, int narg, int t, const char *expected)
{
	if (lua_type(L, narg) != t)
		argtypeerror (L, narg, expected);
}

lua_Integer checkinteger(lua_State *L, int narg, const char *expected)
{
	lua_Integer d = lua_tointeger(L, narg);
	if (d == 0 && !lua_isinteger(L, narg))
		argtypeerror(L, narg, expected);
	return d;
}

int checkint(lua_State *L, int narg)
{
	return (int)checkinteger(L, narg, "int");
}

long checklong(lua_State *L, int narg)
{
	return (long)checkinteger(L, narg, "int");
}

int optboolean(lua_State *L, int narg, int def)
{
	if (lua_isnoneornil(L, narg))
		return def;
	checktype (L, narg, LUA_TBOOLEAN, "boolean or nil");
	return (int)lua_toboolean(L, narg);
}

int optint(lua_State *L, int narg, lua_Integer def)
{
	if (lua_isnoneornil(L, narg))
		return (int) def;
	return (int)checkinteger(L, narg, "int or nil");
}

const char * optstring(lua_State *L, int narg, const char *def)
{
	const char *s;
	if (lua_isnoneornil(L, narg))
		return def;
	s = lua_tolstring(L, narg, NULL);
	if (!s)
		argtypeerror(L, narg, "string or nil");
	return s;
}

void checknargs(lua_State *L, int maxargs)
{
	int nargs = lua_gettop(L);
	lua_pushfstring(L, "no more than %d argument%s expected, got %d",
		        maxargs, maxargs == 1 ? "" : "s", nargs);
	luaL_argcheck(L, nargs <= maxargs, maxargs + 1, lua_tostring (L, -1));
	lua_pop(L, 1);
}

/* Try a lua_getfield from the table on the given index. On success the field
 * is pushed and 0 is returned, on failure nil and an error message is pushed and 2
 * is returned */
void checkfieldtype(lua_State *L, int index, const char *k, int expect_type, const char *expected)
{
	int got_type;
	lua_getfield(L, index, k);
	got_type = lua_type(L, -1);

	if (expected == NULL)
		expected = lua_typename(L, expect_type);

	lua_pushfstring(L, "%s expected for field '%s', got %s",
		expected, k, got_type == LUA_TNIL ? "no value" : lua_typename(L, got_type));
	luaL_argcheck(L, got_type == expect_type, index, lua_tostring(L, -1));
	lua_pop(L, 1);
}

#define NEXT_IKEY	-2
#define NEXT_IVALUE	-1
void
checkismember(lua_State *L, int index, int n, const char *const S[])
{
	/* Diagnose non-string type field names. */
	int got_type = lua_type(L, NEXT_IKEY);
	luaL_argcheck(L, lua_isstring(L, NEXT_IKEY), index,
		lua_pushfstring(L, "invalid %s field name", lua_typename(L, got_type)));

	/* Check field name is listed in S. */
	{
		const char *k = lua_tostring(L, NEXT_IKEY);
		int i;
		for (i = 0; i < n; ++i)
			if (STREQ(S[i], k)) return;
	}


	/* Diagnose invalid field name. */
	luaL_argcheck(L, 0, index,
		lua_pushfstring(L, "invalid field name '%s'", lua_tostring(L, NEXT_IKEY)));
}
#undef NEXT_IKEY
#undef NEXT_IVALUE
#undef checkfieldnames
void checkfieldnames(lua_State *L, int index, int n, const char * const S[])
{
	for (lua_pushnil(L); lua_next(L, index); lua_pop(L, 1))
		checkismember(L, index, n, S);
}

int checkintfield(lua_State *L, int index, const char *k)
{
	int r;
	checkfieldtype(L, index, k, LUA_TNUMBER, "int");
	r = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return r;
}

int checknumberfield(lua_State *L, int index, const char *k)
{
	int r;
	checkfieldtype(L, index, k, LUA_TNUMBER, "number");
	r = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return r;
}

const char * checklstringfield(lua_State *L, int index, const char *k, size_t *plen)
{
	const char *r;
	checkfieldtype(L, index, k, LUA_TSTRING, NULL);
	r = lua_tolstring(L, -1, plen);
	lua_pop(L, 1);
	return r;
}

int optintfield(lua_State *L, int index, const char *k, int def)
{
	int got_type;
	lua_getfield(L, index, k);
	got_type = lua_type(L, -1);
	lua_pop(L, 1);
	if (got_type == LUA_TNONE || got_type == LUA_TNIL)
		return def;
	return checkintfield(L, index, k);
}

const char * optstringfield(lua_State *L, int index, const char *k, const char *def)
{
	const char *r;
	int got_type;
	got_type = lua_type(L, -1);
	lua_pop(L, 1);
	if (got_type == LUA_TNONE || got_type == LUA_TNIL)
		return def;
	return checkstringfield(L, index, k);
}

int pusherror(lua_State *L, const char *info)
{
	lua_pushnil(L);
	if (info==NULL)
		lua_pushstring(L, strerror(errno));
	else
		lua_pushfstring(L, "%s: %s", info, strerror(errno));
	lua_pushinteger(L, errno);
	return 3;
}
int pushresult(lua_State *L, int i, const char *info)
{
	if (i==-1)
		return pusherror(L, info);
	return pushintresult(i);
}

void badoption(lua_State *L, int i, const char *what, int option)
{
	luaL_argerror(L, i,
		lua_pushfstring(L, "invalid %s option '%c'", what, option));
}



/* ================== *
 * Utility functions. *
 * ================== */


int binding_notimplemented(lua_State *L, const char *fname, const char *libname)
{
	lua_pushnil(L);
	lua_pushfstring(L, "'%s' is not implemented by host %s library",
			fname, libname);
	return 2;
}

