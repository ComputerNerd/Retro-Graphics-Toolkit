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
	Copyright Sega16 (or whatever you wish to call me) (2012-2019)
*/
#include "luaHelpers.hpp"
#include "luaSprites.hpp"
#include "callbacksprites.h"
#include "project.h"
#include "dub/dub.h"
static int sprite__set_(lua_State *L) {
	getProjectIDX
	const size_t metaSpriteIDX = idxPtr[1];
	const size_t spriteGroupIDX = idxPtr[2];
	const size_t spriteIDX = idxPtr[3];
	sprite *self = &projects->at(projectIDX).ms->sps[metaSpriteIDX].groups[spriteGroupIDX].list[spriteIDX];
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
	checkAlreadyExists

	getProjectIDX
	const size_t metaSpriteIDX = idxPtr[1];
	const size_t spriteGroupIDX = idxPtr[2];
	const size_t spriteIDX = idxPtr[3];
	const sprite *self = &projects->at(projectIDX).ms->sps[metaSpriteIDX].groups[spriteGroupIDX].list[spriteIDX];

	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER)
		int k = luaL_checkinteger(L, 2);

	else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

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
	getProjectIDX
	const size_t metaSpriteIDX = idxPtr[1];
	const size_t spriteGroupIDX = idxPtr[2];
	const size_t spriteIDX = idxPtr[3];
	const sprite *self = &projects->at(projectIDX).ms->sps[metaSpriteIDX].groups[spriteGroupIDX].list[spriteIDX];
	lua_pushfstring(L, "metasprites[%d].groups[%d][%d] sprite table: %p\n\tWidth: %d\n\tHeight: %d\n\tPalette row: %d\n\tHorizontal flip: %s\n\tVertical flip: %s\n\tHigh priority: %s",
	                metaSpriteIDX, spriteGroupIDX, spriteIDX,
	                self, self->w, self->h, self->palrow, (self->hflip ? "true" : "false"), (self->vflip ? "true" : "false"), (self->prio ? "true" : "false"));
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
int luaopen_sprite(lua_State *L, unsigned projectIDX, unsigned metaSpriteIDX, unsigned groupIDX, unsigned spriteIDX)
{
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "sprite");
	// <mt>

	// register member methods
	dub::fregister(L, sprite_member_methods);
	// setup meta-table
	dub::setup(L, "sprite");

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 4);
	idxUserData[0] = projectIDX;
	idxUserData[1] = metaSpriteIDX;
	idxUserData[2] = groupIDX;
	idxUserData[3] = spriteIDX;

	luaL_getmetatable(L, "sprite");
	lua_setmetatable(L, -2);
	return 1;
}

static int spriteGroup__set_(lua_State *L) {
	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const size_t sgIDX = idxPtr[2];
	spriteGroup *self = &projects->at(projectIDX).ms->sps[msIDX].groups[sgIDX];

	const char *key = luaL_checkstring(L, 2);

	if (!strcmp(key, "name"))
		self->name.assign(luaL_checkstring(L, 3));

	return 0;
}

static int spriteGroup__get_(lua_State *L) {
	checkAlreadyExists

	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const size_t sgIDX = idxPtr[2];
	const spriteGroup *self = &projects->at(projectIDX).ms->sps[msIDX].groups[sgIDX];
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);

		if (k >= 1 && k <= self->list.size()) {
			luaopen_sprite(L, projectIDX, msIDX, sgIDX, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("name", k)) {
			lua_pushstring(L, self->name.c_str());
			return 1;
		}
	}

	return 0;
}

static int spriteGroup__len_(lua_State *L) {
	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const size_t sgIDX = idxPtr[2];
	const spriteGroup *self = &projects->at(projectIDX).ms->sps[msIDX].groups[sgIDX];
	lua_pushinteger(L, self->list.size());
	return 1;
}

static int spriteGroup___tostring(lua_State *L) {
	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const size_t sgIDX = idxPtr[2];
	const spriteGroup *self = &projects->at(projectIDX).ms->sps[msIDX].groups[sgIDX];
	lua_pushfstring(L, "sprite group table: %p\nNamed: %s", self, self->name.c_str());
	return 1;
}

static int spriteGroup_dither(lua_State *L) {
	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const size_t sgIDX = idxPtr[2];
	ditherSpriteAsImage(msIDX, sgIDX, &projects->at(projectIDX));
	return 0;
}

static int spriteGroup_draw(lua_State *L) {
	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const size_t sgIDX = idxPtr[2];

	int32_t outx, outy;
	projects->at(projectIDX).ms->sps[msIDX].draw(sgIDX, luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0), luaL_optinteger(L, 4, 0), lua_toboolean(L, 5), &outx, &outy);
	lua_pushinteger(L, outx);
	lua_pushinteger(L, outy);
	return 2;
}

static const struct luaL_Reg spriteGroup_member_methods[] = {
	//{ "new" , settings_settings },
	{ "__newindex", spriteGroup__set_ },
	{ "__index", spriteGroup__get_ },
	{ "__len", spriteGroup__len_ },
	{ "__tostring", spriteGroup___tostring },
	{ "dither", spriteGroup_dither },
	{ "draw", spriteGroup_draw },
	{ "deleted", dub::isDeleted },
	{ NULL, NULL},
};

static int luaopen_spriteGroup(lua_State *L, unsigned projectIDX, unsigned metaSpriteIDX, unsigned groupIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "spriteGroup");
	// <mt>

	// register member methods
	dub::fregister(L, spriteGroup_member_methods);
	// setup meta-table
	dub::setup(L, "spriteGroup");

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 3);
	idxUserData[0] = projectIDX;
	idxUserData[1] = metaSpriteIDX;
	idxUserData[2] = groupIDX;

	luaL_getmetatable(L, "spriteGroup");
	lua_setmetatable(L, -2);
	return 1;
}


static int spriteGroups__set_(lua_State *L) {
	getProjectIDX
	const size_t msIDX = idxPtr[1];
	class sprites *self = &projects->at(projectIDX).ms->sps[msIDX];
	const char *key = luaL_checkstring(L, 2);

	if (!strcmp(key, "name"))
		self->name.assign(luaL_checkstring(L, 3));

	return 0;
}
static int spriteGroups__get_(lua_State *L) {
	checkAlreadyExists

	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const class sprites *self = &projects->at(projectIDX).ms->sps[msIDX];

	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);

		if (k >= 1 && k <= self->groups.size()) {
			luaopen_spriteGroup(L, projectIDX, msIDX, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("name", k)) {
			lua_pushstring(L, self->name.c_str());
			return 1;
		}
	}

	return 0;
}

static int spriteGroups__len_(lua_State *L) {
	getProjectIDX
	size_t msIDX = idxPtr[1];
	const class sprites *self = &projects->at(projectIDX).ms->sps[msIDX];
	lua_pushinteger(L, self->groups.size());
	return 1;
}

static int spriteGroups___tostring(lua_State *L) {
	getProjectIDX
	const size_t msIDX = idxPtr[1];
	const class sprites *self = &projects->at(projectIDX).ms->sps[msIDX];
	lua_pushfstring(L, "sprite groups table: %p\nNamed: %s", self, self->name.c_str());
	return 1;
}

static int spriteGroups_importSpriteSheet(lua_State *L) {
	getProjectIDX
	size_t msIDX = idxPtr[1];
	projects->at(projectIDX).ms->sps[msIDX].importSpriteSheet(luaL_checkstring(L, 2));
	return 0;
}

static int spriteGroups_dither(lua_State *L) {
	getProjectIDX
	size_t msIDX = idxPtr[1];
	ditherGroupAsImage(msIDX, &projects->at(projectIDX));
	return 0;
}

static const struct luaL_Reg spriteGroups_member_methods[] = {
	{ "__newindex", spriteGroups__set_ },
	{ "__index", spriteGroups__get_ },
	{ "__len", spriteGroups__len_ },
	{ "__tostring", spriteGroups___tostring },
	{ "importSpriteSheet", spriteGroups_importSpriteSheet },
	{ "dither", spriteGroups_dither },
	{ "deleted", dub::isDeleted },
	{ NULL, NULL},
};

static int luaopen_spriteGroups(lua_State *L, unsigned projectIDX, unsigned metaSpriteIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "spriteGroups");
	// <mt>

	// register member methods
	dub::fregister(L, spriteGroups_member_methods);
	// setup meta-table
	dub::setup(L, "spriteGroups");
	// <mt>

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	idxUserData[0] = projectIDX;
	idxUserData[1] = metaSpriteIDX;

	luaL_getmetatable(L, "spriteGroups");
	lua_setmetatable(L, -2);
	return 1;
}

static int sprites__set_(lua_State *L) {
	getProjectIDX
	const char *key = luaL_checkstring(L, 2);

	if (!strcmp(key, "name"))
		projects->at(projectIDX).ms->name.assign(luaL_checkstring(L, 3));

	return 0;
}

static int sprites__get_(lua_State *L) {
	checkAlreadyExists
	int type = lua_type(L, 2);

	getProjectIDX

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);

		if (k >= 1 && k <= projects->at(projectIDX).ms->sps.size()) {
			luaopen_spriteGroups(L, projectIDX, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("name", k)) {
			lua_pushstring(L, projects->at(projectIDX).ms->name.c_str());
			return 1;
		}
	}

	return 0;
}

static int sprites__len_(lua_State *L) {
	getProjectIDX
	lua_pushinteger(L, projects->at(projectIDX).ms->sps.size());
	return 1;
}

static int sprites___tostring(lua_State *L) {
	getProjectIDX
	const struct metasprites* ms = projects->at(projectIDX).ms;
	lua_pushfstring(L, "metasprite table: %p\nNamed: %s", ms, ms->name.c_str());
	return 1;
}

static int metaSprites_dither(lua_State *L) {
	getProjectIDX

	for (unsigned i = 0; i < projects->at(projectIDX).ms->sps.size(); ++i)
		ditherGroupAsImage(i, &projects->at(projectIDX));

	return 0;
}

static const struct luaL_Reg sprites_member_methods[] = {
	{ "__newindex", sprites__set_},
	{ "__index", sprites__get_},
	{ "__len", sprites__len_},
	{ "__tostring", sprites___tostring},
	{ "dither", metaSprites_dither},
	{ "deleted", dub::isDeleted},
	{ NULL, NULL},
};

int luaopen_MetaSprites(lua_State *L, size_t projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "metasprites");
	// <mt>

	// register member methods
	dub::fregister(L, sprites_member_methods);
	// setup meta-table
	dub::setup(L, "metasprites");
	// <mt>
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "metasprites");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}
