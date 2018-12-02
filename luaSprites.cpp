/*
	This file is part of Retro Graphics Toolkit

	Retro Graphics Toolkit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or any later version.

	Retro Graphics Toolkit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
	Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#include "luaSprites.hpp"
#include "project.h"
#include "dub/dub.h"
static int sprite__set_(lua_State *L) {
	sprite *self = *((sprite **)dub::checksdata(L, 1, "sprite"));
	const char *key = luaL_checkstring(L, 2);

	if (!strcmp(key, "w")) {
		self->w = luaL_checkinteger(L, 3);
		return 0;
	}

	if (!strcmp(key, "h")) {
		self->h = luaL_checkinteger(L, 3);
		return 0;
	}

	if (!strcmp(key, "starttile")) {
		self->starttile = luaL_checkinteger(L, 3);
		return 0;
	}

	if (!strcmp(key, "offx")) {
		self->offx = luaL_checkinteger(L, 3);
		return 0;
	}

	if (!strcmp(key, "offy")) {
		self->offy = luaL_checkinteger(L, 3);
		return 0;
	}

	if (!strcmp(key, "loadat")) {
		self->loadat = luaL_checkinteger(L, 3);
		return 0;
	}

	if (!strcmp(key, "palrow")) {
		self->palrow = luaL_checkinteger(L, 3);
		return 0;
	}

	if (!strcmp(key, "hflip")) {
		self->hflip = lua_toboolean(L, 3);
		return 0;
	}

	if (!strcmp(key, "vflip")) {
		self->vflip = lua_toboolean(L, 3);
		return 0;
	}

	if (!strcmp(key, "prio")) {
		self->prio = lua_toboolean(L, 3);
		return 0;
	}

	return 0;
}
static int sprite__get_(lua_State *L) {
	sprite *self = *((sprite **)dub::checksdata_n(L, 1, "sprite", true));
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n", k);
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);

		if (!lua_isnil(L, -1))
			return 1;
		else
			lua_pop(L, 2);

		if (!strcmp("w", k)) {
			lua_pushinteger(L, self->w);
			return 1;
		}

		if (!strcmp("h", k)) {
			lua_pushinteger(L, self->h);
			return 1;
		}

		if (!strcmp("palrow", k)) {
			lua_pushinteger(L, self->palrow);
			return 1;
		}

		if (!strcmp("starttile", k)) {
			lua_pushinteger(L, self->starttile);
			return 1;
		}

		if (!strcmp("offx", k)) {
			lua_pushinteger(L, self->offx);
			return 1;
		}

		if (!strcmp("offy", k)) {
			lua_pushinteger(L, self->offy);
			return 1;
		}

		if (!strcmp("loadat", k)) {
			lua_pushinteger(L, self->loadat);
			return 1;
		}

		if (!strcmp("prio", k)) {
			lua_pushboolean(L, self->prio);
			return 1;
		}

		if (!strcmp("hflip", k)) {
			lua_pushboolean(L, self->hflip);
			return 1;
		}

		if (!strcmp("vflip", k)) {
			lua_pushboolean(L, self->vflip);
			return 1;
		}
	}

	return 0;
}
static int sprite___tostring(lua_State *L) {
	sprite *self = *((sprite **)dub::checksdata(L, 1, "sprite"));
	lua_pushfstring(L, "sprite table: %p\n\tWidth: %d\n\tHeight: %d\n\tPalette row: %d\n\tHorizontal flip: %s\n\tVertical flip: %s\n\tHigh priority: %s", self, self->w, self->h, self->palrow, (self->hflip ? "true" : "false"), (self->vflip ? "true" : "false"), (self->prio ? "true" : "false"));
	return 1;
}
static const struct luaL_Reg sprite_member_methods[] = {
	//{ "new" , settings_settings },
	{ "__newindex", sprite__set_ },
	{ "__index", sprite__get_ },
	{ "__tostring", sprite___tostring },
	{ "deleted", dub::isDeleted },
	{ NULL, NULL},
};
int luaopen_sprite(lua_State *L, class spriteGroup*self, unsigned idx)
{
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "sprite");
	// <mt>

	// register member methods
	dub::fregister(L, sprite_member_methods);
	// setup meta-table
	dub::setup(L, "sprite");
	// <mt>
	dub::pushudata(L, &self->list[idx], "sprite", true);
	return 1;
}

static int spriteGroup__set_(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata(L, 1, "spriteGroup"));
	const char *key = luaL_checkstring(L, 2);

	if (!strcmp(key, "name"))
		self->name.assign(luaL_checkstring(L, 3));

	return 0;
}

static int spriteGroup__get_(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata_n(L, 1, "spriteGroup", true));
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n", k);

		if (k >= 1 && k <= self->list.size()) {
			luaopen_sprite(L, self, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);

		if (!lua_isnil(L, -1))
			return 1;

		else
			lua_pop(L, 2);

		printf("k=%s\n", k);

		if (!strcmp("name", k)) {
			lua_pushstring(L, self->name.c_str());
			return 1;
		}
	}

	return 0;
}

static int spriteGroup__len_(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata(L, 1, "spriteGroup"));
	lua_pushinteger(L, self->list.size());
	return 1;
}

static int spriteGroup___tostring(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata(L, 1, "spriteGroup"));
	lua_pushfstring(L, "sprite group table: %p\nNamed: %s", self, self->name.c_str());
	return 1;
}

static const struct luaL_Reg spriteGroup_member_methods[] = {
	//{ "new" , settings_settings },
	{ "__newindex", spriteGroup__set_ },
	{ "__index", spriteGroup__get_ },
	{ "__len", spriteGroup__len_ },
	{ "__tostring", spriteGroup___tostring },
	{ "deleted", dub::isDeleted },
	{ NULL, NULL},
};

int luaopen_spriteGroup(lua_State *L, class sprites*self, unsigned idx)
{
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "spriteGroup");
	// <mt>

	// register member methods
	dub::fregister(L, spriteGroup_member_methods);
	// setup meta-table
	dub::setup(L, "spriteGroup");
	// <mt>
	dub::pushudata(L, &self->groups[idx], "spriteGroup", true);
	return 1;
}


static int spriteGroups__set_(lua_State *L) {
	sprites *self = *((sprites **)dub::checksdata(L, 1, "spriteGroups"));
	const char *key = luaL_checkstring(L, 2);

	if (!strcmp(key, "name"))
		self->name.assign(luaL_checkstring(L, 3));

	return 0;
}
static int spriteGroups__get_(lua_State *L) {
	sprites *self = *((sprites **)dub::checksdata_n(L, 1, "spriteGroups", true));
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n", k);

		if (k >= 1 && k <= self->groups.size()) {
			luaopen_spriteGroup(L, self, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);
		puts(lua_typename(L, lua_type(L, 0)));
		puts(lua_typename(L, lua_type(L, 1)));
		puts(lua_typename(L, lua_type(L, 2)));
		puts(lua_typename(L, lua_type(L, 3)));
		puts(lua_typename(L, lua_type(L, 4)));
		puts("__END__");
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);

		if (!lua_isnil(L, -1))
			return 1;

		else
			lua_pop(L, 2);

		printf("k=%s\n", k);

		if (!strcmp("name", k)) {
			lua_pushstring(L, self->name.c_str());
			return 1;
		}
	}

	return 0;
}
static int spriteGroups__len_(lua_State *L) {
	class sprites *self = *((class sprites **)dub::checksdata(L, 1, "spriteGroups"));
	lua_pushinteger(L, self->groups.size());
	return 1;
}

static int spriteGroups___tostring(lua_State *L) {
	class sprites *self = *((class sprites **)dub::checksdata(L, 1, "spriteGroups"));
	lua_pushfstring(L, "sprite groups table: %p\nNamed: %s", self, self->name.c_str());
	return 1;
}

static int spriteGroups_importSpriteSheet(lua_State *L) {
	class sprites *self = *((class sprites **)dub::checksdata(L, 1, "spriteGroups"));
	self->importSpriteSheet(luaL_checkstring(L, 2));
	return 0;
}

static const struct luaL_Reg spriteGroups_member_methods[] = {
	{ "__newindex", spriteGroups__set_ },
	{ "__index", spriteGroups__get_ },
	{ "__len", spriteGroups__len_ },
	{ "__tostring", spriteGroups___tostring },
	{ "importSpriteSheet", spriteGroups_importSpriteSheet },
	{ "deleted", dub::isDeleted },
	{ NULL, NULL},
};

int luaopen_spriteGroups(lua_State *L, unsigned idx) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "spriteGroups");
	// <mt>

	// register member methods
	dub::fregister(L, spriteGroups_member_methods);
	// setup meta-table
	dub::setup(L, "spriteGroups");
	// <mt>
	dub::pushudata(L, &currentProject->ms->sps[idx], "spriteGroups", true);
	return 1;
}

static int sprites__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);

	if (!strcmp(key, "name"))
		currentProject->ms->name.assign(luaL_checkstring(L, 3));

	return 0;
}

static int sprites__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n", k);

		if (k >= 1 && k <= currentProject->ms->sps.size()) {
			luaopen_spriteGroups(L, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);

		if (!lua_isnil(L, -1))
			return 1;

		else
			lua_pop(L, 2);

		printf("k=%s\n", k);

		if (!strcmp("name", k)) {
			lua_pushstring(L, currentProject->ms->name.c_str());
			return 1;
		}
	}

	return 0;
}

static int sprites__len_(lua_State *L) {
	lua_pushinteger(L, currentProject->ms->sps.size());
	return 1;
}

static int sprites___tostring(lua_State *L) {
	lua_pushfstring(L, "metasprite table: %p\nNamed: %s", currentProject->ms, currentProject->ms->name.c_str());
	return 1;
}

static const struct luaL_Reg sprites_member_methods[] = {
	{ "__newindex", sprites__set_},
	{ "__index", sprites__get_},
	{ "__len", sprites__len_},
	{ "__tostring", sprites___tostring},
	{ "deleted", dub::isDeleted},
	{ NULL, NULL},
};

int luaopen_sprites(lua_State *L) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "sprites");
	// <mt>

	// register member methods
	dub::fregister(L, sprites_member_methods);
	// setup meta-table
	dub::setup(L, "sprites");
	// <mt>
	dub::pushudata(L, currentProject->ms, "sprites", true);
	return 1;
}
