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
	Copyright Sega16 (or whatever you wish to call me) (2012-2020)
*/
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/names.h>
#include "dub/dub.h"
#include "luafltk.hpp"
#include "luaHelpers.hpp"
static int luafl_ask(lua_State*L) {
	lua_pushboolean(L, fl_ask("%s", luaL_optstring(L, 1, "Default message")));
	return 1;
}
static int luafl_beep(lua_State*L) {
	fl_beep();
	return 0;
}
static int luafl_color_chooser(lua_State*L) {
	double r, g, b;
	r = luaL_optnumber(L, 3, 0.);
	g = luaL_optnumber(L, 4, 0.);
	b = luaL_optnumber(L, 5, 0.);
	int ret = fl_color_chooser(luaL_optstring(L, 1, "Select a color"), r, g, b, luaL_optinteger(L, 2, -1));

	if (ret) {
		lua_pushinteger(L, ret);
		lua_pushnumber(L, r);
		lua_pushnumber(L, g);
		lua_pushnumber(L, b);
		return 4;
	} else {
		lua_pushinteger(L, ret);
		return 1;
	}
}
static int luafl_dir_chooser(lua_State*L) {
	lua_pushstring(L, fl_dir_chooser(luaL_optstring(L, 1, "Choose a directory"), luaL_optstring(L, 2, NULL), luaL_optinteger(L, 3, 0)));
	return 1;
}
static int luafl_file_chooser(lua_State*L) {
	lua_pushstring(L, fl_file_chooser(luaL_optstring(L, 1, "Choose a file"), luaL_optstring(L, 2, NULL), luaL_optstring(L, 3, 0), luaL_optinteger(L, 4, 0)));
	return 1;
}
static int luafl_input(lua_State*L) {
	lua_pushstring(L, fl_input(luaL_optstring(L, 1, "Enter text"), luaL_optstring(L, 2, NULL)));
	return 1;
}
static int luafl_password(lua_State*L) {
	fl_password(luaL_optstring(L, 1, "Enter text"), luaL_optstring(L, 2, NULL));
	return 0;
}
static int luafl_eventnames(lua_State*L) {
	lua_pushstring(L, fl_eventnames[luaL_checkinteger(L, 1)]);
	return 1;
}
static int luafl_point(lua_State*L) {
	fl_point(luaL_optinteger(L, 1, 0), luaL_optinteger(L, 2, 0));
	return 0;
}
static int luafl_rect(lua_State*L) {
	int x = luaL_optinteger(L, 1, 0), y = luaL_optinteger(L, 2, 0), w = luaL_optinteger(L, 3, 0), h = luaL_optinteger(L, 4, 0);

	if (lua_type(L, 5) == LUA_TNUMBER)
		fl_rect(x, y, w, h, luaL_optinteger(L, 5, 0));
	else
		fl_rect(x, y, w, h);

	return 0;
}
static int luafl_color(lua_State*L) {
	if (lua_type(L, 2) == LUA_TNUMBER && lua_type(L, 3) == LUA_TNUMBER)
		fl_color(luaL_optinteger(L, 1, 0), luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0));
	else
		fl_color((unsigned)luaL_optinteger(L, 1, 0));

	return 0;
}
static int luafl_arc(lua_State*L) {
	if (lua_type(L, 6) == LUA_TNUMBER)
		fl_arc(luaL_optinteger(L, 1, 0), luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0), luaL_optinteger(L, 4, 0), luaL_optnumber(L, 5, 0), luaL_optnumber(L, 6, 0));
	else
		fl_arc(luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0), luaL_optnumber(L, 4, 0.0), luaL_optnumber(L, 5, 0.0));

	return 0;
}
static int luafl_begin_complex_polygon(lua_State*L) {
	fl_begin_complex_polygon();
	return 0;
}
static int luafl_begin_line(lua_State*L) {
	fl_begin_line();
	return 0;
}
static int luafl_begin_loop(lua_State*L) {
	fl_begin_loop();
	return 0;
}
static int luafl_begin_points(lua_State*L) {
	fl_begin_points();
	return 0;
}
static int luafl_begin_polygon(lua_State*L) {
	fl_begin_polygon();
	return 0;
}
static int luafl_circle(lua_State*L) {
	fl_circle(luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0));
	return 0;
}
static int luafl_end_complex_polygon(lua_State*L) {
	fl_end_complex_polygon();
	return 0;
}
static int luafl_end_line(lua_State*L) {
	fl_end_line();
	return 0;
}
static int luafl_end_loop(lua_State*L) {
	fl_end_loop();
	return 0;
}
static int luafl_end_points(lua_State*L) {
	fl_end_points();
	return 0;
}
static int luafl_end_polygon(lua_State*L) {
	fl_end_polygon();
	return 0;
}
static int luafl_draw(lua_State*L) {
	if (lua_type(L, 1) == LUA_TNUMBER) {
		int parm[3];
		parm[0] = luaL_optinteger(L, 1, 0);
		parm[1] = luaL_optinteger(L, 3, 0);
		parm[2] = luaL_optinteger(L, 4, 0);
		const char*str = lua_tostring(L, 2);

		if (lua_type(L, 5) == LUA_TNUMBER) //n specified
			fl_draw(parm[0], str, parm[1], parm[2], luaL_optinteger(L, 5, 0));
		else
			fl_draw(parm[0], str, parm[1], parm[2]);

	} else {
		const char*str = lua_tostring(L, 1);
		int parm[2];
		parm[0] = luaL_optinteger(L, 2, 0);
		parm[1] = luaL_optinteger(L, 3, 0);

		if (lua_type(L, 4) == LUA_TNUMBER) //n specified
			fl_draw(str, parm[0], parm[1], luaL_optinteger(L, 4, 0));
		else
			fl_draw(str, parm[0], parm[1]);
	}

	return 0;
}
/** void fl_rectf(int x, int y, int w, int h)
 * inc/fl_draw.h:206
 */
static int FLTK_fl_rectf(lua_State *L) {
	try {
		int top__ = lua_gettop(L);

		if (top__ >= 7) {
			int x = dub::checkinteger(L, 1);
			int y = dub::checkinteger(L, 2);
			int w = dub::checkinteger(L, 3);
			int h = dub::checkinteger(L, 4);
			unsigned char r = dub::checkinteger(L, 5);
			unsigned char g = dub::checkinteger(L, 6);
			unsigned char b = dub::checkinteger(L, 7);
			fl_rectf(x, y, w, h, r, g, b);
			return 0;
		} else if (top__ >= 5) {
			int x = dub::checkinteger(L, 1);
			int y = dub::checkinteger(L, 2);
			int w = dub::checkinteger(L, 3);
			int h = dub::checkinteger(L, 4);
			int c = dub::checkinteger(L, 5);
			fl_rectf(x, y, w, h, c);
			return 0;
		} else {
			int x = dub::checkinteger(L, 1);
			int y = dub::checkinteger(L, 2);
			int w = dub::checkinteger(L, 3);
			int h = dub::checkinteger(L, 4);
			fl_rectf(x, y, w, h);
			return 0;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_rectf: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_rectf: Unknown exception");
	}

	return lua_error(L);
}
/** void fl_draw_box(int, int x, int y, int w, int h, int)
 * inc/fl_draw.h:641
 */
static int FLTK_fl_draw_box(lua_State *L) {
	try {
		int p1 = dub::checkinteger(L, 1);
		int x = dub::checkinteger(L, 2);
		int y = dub::checkinteger(L, 3);
		int w = dub::checkinteger(L, 4);
		int h = dub::checkinteger(L, 5);
		int p6 = dub::checkinteger(L, 6);
		fl_draw_box((Fl_Boxtype)p1, x, y, w, h, p6);
		return 0;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_draw_box: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_draw_box: Unknown exception");
	}

	return lua_error(L);
}
/** const char* fl_filename_name(const char *filename)
 * inc/filename.h:54
 */
static int FLTK_fl_filename_name(lua_State *L) {
	try {
		const char *filename = dub::checkstring(L, 1);
		lua_pushstring(L, fl_filename_name(filename));
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_name: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_name: Unknown exception");
	}

	return lua_error(L);
}

/** const char* fl_filename_ext(const char *buf)
 * inc/filename.h:55
 */
static int FLTK_fl_filename_ext(lua_State *L) {
	try {
		const char *buf = dub::checkstring(L, 1);
		lua_pushstring(L, fl_filename_ext(buf));
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_ext: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_ext: Unknown exception");
	}

	return lua_error(L);
}

/** char* fl_filename_setext(char *to, int tolen, const char *ext)
 * inc/filename.h:56
 */
static int FLTK_fl_filename_setext(lua_State *L) {
	try {
		char *to = (char*)dub::checkstring(L, 1);
		const char *ext = dub::checkstring(L, 2);
		size_t blen = strlen(to) + strlen(ext);
		char*tmpbuf = (char*)malloc(blen);
		strcpy(tmpbuf, to);
		lua_pushstring(L, fl_filename_setext(tmpbuf, blen, ext));
		free(tmpbuf);
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_setext: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_setext: Unknown exception");
	}

	return lua_error(L);
}

/** int fl_filename_expand(char *to, int tolen, const char *from)
 * inc/filename.h:57
 */
static int FLTK_fl_filename_expand(lua_State *L) {
	try {
		char *to = (char*)dub::checkstring(L, 1);
		int tolen = dub::checkinteger(L, 2);
		const char *from = dub::checkstring(L, 3);
		lua_pushnumber(L, fl_filename_expand(to, tolen, from));
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_expand: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_expand: Unknown exception");
	}

	return lua_error(L);
}

/** int fl_filename_absolute(char *to, int tolen, const char *from)
 * inc/filename.h:58
 */
static int FLTK_fl_filename_absolute(lua_State *L) {
	try {
		char *to = (char*)dub::checkstring(L, 1);
		int tolen = dub::checkinteger(L, 2);
		const char *from = dub::checkstring(L, 3);
		lua_pushnumber(L, fl_filename_absolute(to, tolen, from));
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_absolute: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_absolute: Unknown exception");
	}

	return lua_error(L);
}

/** int fl_filename_relative(char *to, int tolen, const char *from)
 * inc/filename.h:59
 */
static int FLTK_fl_filename_relative(lua_State *L) {
	try {
		char *to = (char*)dub::checkstring(L, 1);
		int tolen = dub::checkinteger(L, 2);
		const char *from = dub::checkstring(L, 3);
		lua_pushnumber(L, fl_filename_relative(to, tolen, from));
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_relative: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_relative: Unknown exception");
	}

	return lua_error(L);
}

/** int fl_filename_match(const char *name, const char *pattern)
 * inc/filename.h:60
 */
static int FLTK_fl_filename_match(lua_State *L) {
	try {
		const char *name = dub::checkstring(L, 1);
		const char *pattern = dub::checkstring(L, 2);
		lua_pushnumber(L, fl_filename_match(name, pattern));
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_match: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_match: Unknown exception");
	}

	return lua_error(L);
}

/** int fl_filename_isdir(const char *name)
 * inc/filename.h:61
 */
static int FLTK_fl_filename_isdir(lua_State *L) {
	try {
		const char *name = dub::checkstring(L, 1);
		lua_pushnumber(L, fl_filename_isdir(name));
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_isdir: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_isdir: Unknown exception");
	}

	return lua_error(L);
}

/** int fl_filename_list(const char *d, struct dirent ***l, Fl_File_Sort_F *s=fl_numericsort)
 * inc/filename.h:125
 */
static int FLTK_fl_filename_list(lua_State *L) {
	try {
		int top__ = lua_gettop(L);

		if (top__ >= 3) {
			const char *d = dub::checkstring(L, 1);
			dirent** *l = *((dirent** **)dub::checksdata(L, 2, "dirent**"));
			Fl_File_Sort_F *s = *((Fl_File_Sort_F **)dub::checksdata(L, 3, "Fl_File_Sort_F"));
			lua_pushnumber(L, fl_filename_list(d, l, s));
			return 1;
		} else {
			const char *d = dub::checkstring(L, 1);
			dirent** *l = *((dirent** **)dub::checksdata(L, 2, "dirent**"));
			lua_pushnumber(L, fl_filename_list(d, l));
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_list: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_list: Unknown exception");
	}

	return lua_error(L);
}

/** void fl_filename_free_list(struct dirent ***l, int n)
 * inc/filename.h:127
 */
static int FLTK_fl_filename_free_list(lua_State *L) {
	try {
		dirent** *l = *((dirent** **)dub::checksdata(L, 1, "dirent**"));
		int n = dub::checkinteger(L, 2);
		fl_filename_free_list(l, n);
		return 0;
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_filename_free_list: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_filename_free_list: Unknown exception");
	}

	return lua_error(L);
}
static const luaL_Reg lua_flAPI[] = {
	{"ask", luafl_ask},
	{"beep", luafl_beep},
	{"color_chooser", luafl_color_chooser},
	{"dir_chooser", luafl_dir_chooser},
	{"file_chooser", luafl_file_chooser},
	{"input", luafl_input},
	{"password", luafl_password},
	{"eventnames", luafl_eventnames},
	{"point", luafl_point},
	{"rect", luafl_rect},
	{"rectf", FLTK_fl_rectf},
	{"color", luafl_color},
	{"arc", luafl_arc},
	{"begin_complex_polygon", luafl_begin_complex_polygon},
	{"begin_line", luafl_begin_line},
	{"begin_loop", luafl_begin_loop},
	{"begin_points", luafl_begin_points},
	{"begin_polygon", luafl_begin_polygon},
	{"circle", luafl_circle},
	{"end_complex_polygon", luafl_end_complex_polygon},
	{"end_line", luafl_end_line},
	{"end_loop", luafl_end_loop},
	{"end_points", luafl_end_points},
	{"end_polygon", luafl_end_polygon},
	{"draw", luafl_draw},
	{"draw_box", FLTK_fl_draw_box},
	{"filename_name", FLTK_fl_filename_name },
	{"filename_ext", FLTK_fl_filename_ext },
	{"filename_setext", FLTK_fl_filename_setext },
	{"filename_expand", FLTK_fl_filename_expand },
	{"filename_absolute", FLTK_fl_filename_absolute },
	{"filename_relative", FLTK_fl_filename_relative },
	{"filename_match", FLTK_fl_filename_match },
	{"filename_isdir", FLTK_fl_filename_isdir },
	{"filename_list", FLTK_fl_filename_list },
	{"filename_free_list", FLTK_fl_filename_free_list },
	{0, 0}
};
static int luaFl_check(lua_State*L) {
	lua_pushinteger(L, Fl::check());
	return 1;
}
static int luaFl_wait(lua_State*L) {
	if (lua_type(L, 1) == LUA_TNUMBER)
		lua_pushinteger(L, Fl::wait(luaL_checknumber(L, 1)));
	else
		lua_pushinteger(L, Fl::wait());

	return 1;
}
static int luaFl_event_x(lua_State*L) {
	lua_pushinteger(L, Fl::event_x());
	return 1;
}
static int luaFl_event_y(lua_State*L) {
	lua_pushinteger(L, Fl::event_y());
	return 1;
}
static int luaFl_event_key(lua_State*L) {
	if (lua_type(L, 1) == LUA_TNUMBER)
		lua_pushinteger(L, Fl::event_key(luaL_checknumber(L, 1)));
	else
		lua_pushinteger(L, Fl::event_key());

	return 1;
}
static int luaFl_event_ctrl(lua_State*L) {
	lua_pushinteger(L, Fl::event_ctrl());
	return 1;
}
static int luaFl_event_alt(lua_State*L) {
	lua_pushinteger(L, Fl::event_alt());
	return 1;
}
static int luaFl_event_shift(lua_State*L) {
	lua_pushinteger(L, Fl::event_shift());
	return 1;
}
static int luaFl_event_length(lua_State*L) {
	lua_pushinteger(L, Fl::event_length());
	return 1;
}
static int luaFl_event_text(lua_State*L) {
	lua_pushlstring(L, Fl::event_text(), Fl::event_length());
	return 1;
}
static int luaFl_event_button(lua_State*L) {
	lua_pushinteger(L, Fl::event_button());
	return 1;
}
static int luaFl_scheme(lua_State*L) {
	Fl::scheme(lua_tostring(L, 1));
	return 0;
}
static const luaL_Reg lua_FlAPI[] = {
	{"check", luaFl_check},
	{"wait", luaFl_wait},
	{"event_x", luaFl_event_x},
	{"event_y", luaFl_event_y},
	{"event_key", luaFl_event_key},
	{"event_ctrl", luaFl_event_ctrl},
	{"event_alt", luaFl_event_alt},
	{"event_shift", luaFl_event_shift},
	{"event_length", luaFl_event_length},
	{"event_text", luaFl_event_text},
	{"event_button", luaFl_event_button},
	{"scheme", luaFl_scheme},
	{0, 0}
};
static const keyPair FLconsts[] = {
	{"MENU_INACTIVE", FL_MENU_INACTIVE},
	{"MENU_TOGGLE", FL_MENU_TOGGLE},
	{"MENU_VALUE", FL_MENU_VALUE},
	{"MENU_RADIO", FL_MENU_RADIO},
	{"MENU_INVISIBLE", FL_MENU_INVISIBLE},
	{"SUBMENU_POINTER", FL_SUBMENU_POINTER},
	{"SUBMENU", FL_SUBMENU},
	{"MENU_DIVIDER", FL_MENU_DIVIDER},
	{"MENU_HORIZONTAL", FL_MENU_HORIZONTAL},
	{"SHIFT", FL_SHIFT},
	{"CAPS_LOCK", FL_CAPS_LOCK},
	{"CTRL", FL_CTRL},
	{"ALT", FL_ALT},
	{"NUM_LOCK", FL_NUM_LOCK},
	{"META", FL_META},
	{"SCROLL_LOCK", FL_SCROLL_LOCK},
	{"BUTTON1", FL_BUTTON1},
	{"BUTTON2", FL_BUTTON2},
	{"BUTTON3", FL_BUTTON3},
	{"BUTTONS", FL_BUTTONS},
	{"Button", FL_Button},
	{"BackSpace", FL_BackSpace},
	{"Tab", FL_Tab},
	{"Iso_Key", FL_Iso_Key},
	{"Enter", FL_Enter},
	{"Pause", FL_Pause},
	{"Scroll_Lock", FL_Scroll_Lock},
	{"Escape", FL_Escape},
#if (FL_MAJOR_VERSION>=1 && FL_MINOR_VERSION>=3 && FL_PATCH_VERSION>=3)
	{"Kana", FL_Kana},
	{"Eisu", FL_Eisu},
	{"Yen", FL_Yen},
	{"JIS_Underscore", FL_JIS_Underscore},
#endif
	{"Home", FL_Home},
	{"Left", FL_Left},
	{"Up", FL_Up},
	{"Right", FL_Right},
	{"Down", FL_Down},
	{"Page_Up", FL_Page_Up},
	{"Page_Down", FL_Page_Down},
	{"End", FL_End},
	{"Print", FL_Print},
	{"Insert", FL_Insert},
	{"Menu", FL_Menu},
	{"Help", FL_Help},
	{"Num_Lock", FL_Num_Lock},
	{"KP", FL_KP},
	{"KP_Enter", FL_KP_Enter},
	{"KP_Last", FL_KP_Last},
	{"F", FL_F},
	{"F_Last", FL_F_Last},
	{"Shift_L", FL_Shift_L},
	{"Shift_R", FL_Shift_R},
	{"Control_L", FL_Control_L},
	{"Control_R", FL_Control_R},
	{"Caps_Lock", FL_Caps_Lock},
	{"Meta_L", FL_Meta_L},
	{"Meta_R", FL_Meta_R},
	{"Alt_L", FL_Alt_L},
	{"Alt_R", FL_Alt_R},
	{"Delete", FL_Delete},
	{"NORMAL_BUTTON", FL_NORMAL_BUTTON},
	{"TOGGLE_BUTTON", FL_TOGGLE_BUTTON},
	{"RADIO_BUTTON", FL_RADIO_BUTTON},
	{"HIDDEN_BUTTON", FL_HIDDEN_BUTTON},
	{"VERT_SLIDER", FL_VERT_SLIDER},
	{"HOR_SLIDER", FL_HOR_SLIDER},
	{"VERT_FILL_SLIDER", FL_VERT_FILL_SLIDER},
	{"HOR_FILL_SLIDER", FL_HOR_FILL_SLIDER},
	{"VERT_NICE_SLIDER", FL_VERT_NICE_SLIDER},
	{"HOR_NICE_SLIDER", FL_HOR_NICE_SLIDER},
	{"VERTICAL", FL_VERTICAL},
	{"HORIZONTAL", FL_HORIZONTAL},
	{"LEFT_MOUSE", FL_LEFT_MOUSE},
	{"MIDDLE_MOUSE", FL_MIDDLE_MOUSE},
	{"RIGHT_MOUSE", FL_RIGHT_MOUSE},
	{"ALIGN_CENTER", FL_ALIGN_CENTER},
	{"ALIGN_TOP", FL_ALIGN_TOP},
	{"ALIGN_BOTTOM", FL_ALIGN_BOTTOM},
	{"ALIGN_LEFT", FL_ALIGN_LEFT},
	{"ALIGN_RIGHT", FL_ALIGN_RIGHT},
	{"ALIGN_INSIDE", FL_ALIGN_INSIDE},
	{"ALIGN_TEXT_OVER_IMAGE", FL_ALIGN_TEXT_OVER_IMAGE},
	{"ALIGN_IMAGE_OVER_TEXT", FL_ALIGN_IMAGE_OVER_TEXT},
	{"ALIGN_CLIP", FL_ALIGN_CLIP},
	{"ALIGN_WRAP", FL_ALIGN_WRAP},
	{"ALIGN_IMAGE_NEXT_TO_TEXT", FL_ALIGN_IMAGE_NEXT_TO_TEXT},
	{"ALIGN_TEXT_NEXT_TO_IMAGE", FL_ALIGN_TEXT_NEXT_TO_IMAGE},
	{"ALIGN_IMAGE_BACKDROP", FL_ALIGN_IMAGE_BACKDROP},
	{"ALIGN_TOP_LEFT", FL_ALIGN_TOP_LEFT},
	{"ALIGN_TOP_RIGHT", FL_ALIGN_TOP_RIGHT},
	{"ALIGN_BOTTOM_LEFT", FL_ALIGN_BOTTOM_LEFT},
	{"ALIGN_BOTTOM_RIGHT", FL_ALIGN_BOTTOM_RIGHT},
	{"ALIGN_LEFT_TOP", FL_ALIGN_LEFT_TOP},
	{"ALIGN_RIGHT_TOP", FL_ALIGN_RIGHT_TOP},
	{"ALIGN_LEFT_BOTTOM", FL_ALIGN_LEFT_BOTTOM},
	{"ALIGN_RIGHT_BOTTOM", FL_ALIGN_RIGHT_BOTTOM},
	{"ALIGN_NOWRAP", FL_ALIGN_NOWRAP},
	{"ALIGN_POSITION_MASK", FL_ALIGN_POSITION_MASK},
	{"ALIGN_IMAGE_MASK", FL_ALIGN_IMAGE_MASK},
	{"HELVETICA", FL_HELVETICA},
	{"HELVETICA_BOLD", FL_HELVETICA_BOLD},
	{"HELVETICA_ITALIC", FL_HELVETICA_ITALIC},
	{"HELVETICA_BOLD_ITALIC", FL_HELVETICA_BOLD_ITALIC},
	{"COURIER", FL_COURIER},
	{"COURIER_BOLD", FL_COURIER_BOLD},
	{"COURIER_ITALIC", FL_COURIER_ITALIC},
	{"COURIER_BOLD_ITALIC", FL_COURIER_BOLD_ITALIC},
	{"TIMES", FL_TIMES},
	{"TIMES_BOLD", FL_TIMES_BOLD},
	{"TIMES_ITALIC", FL_TIMES_ITALIC},
	{"TIMES_BOLD_ITALIC", FL_TIMES_BOLD_ITALIC},
	{"SYMBOL", FL_SYMBOL},
	{"SCREEN", FL_SCREEN},
	{"SCREEN_BOLD", FL_SCREEN_BOLD},
	{"ZAPF_DINGBATS", FL_ZAPF_DINGBATS},
	{"FREE_FONT", FL_FREE_FONT},
	{"BOLD", FL_BOLD},
	{"ITALIC", FL_ITALIC},
	{"BOLD_ITALIC", FL_BOLD_ITALIC},
	{"FOREGROUND_COLOR", FL_FOREGROUND_COLOR},
	{"BACKGROUND2_COLOR", FL_BACKGROUND2_COLOR},
	{"INACTIVE_COLOR", FL_INACTIVE_COLOR},
	{"SELECTION_COLOR", FL_SELECTION_COLOR},
	{"GRAY0", FL_GRAY0},
	{"DARK3", FL_DARK3},
	{"DARK2", FL_DARK2},
	{"DARK1", FL_DARK1},
	{"BACKGROUND_COLOR", FL_BACKGROUND_COLOR},
	{"LIGHT1", FL_LIGHT1},
	{"LIGHT2", FL_LIGHT2},
	{"LIGHT3", FL_LIGHT3},
	{"BLACK", FL_BLACK},
	{"RED", FL_RED},
	{"GREEN", FL_GREEN},
	{"YELLOW", FL_YELLOW},
	{"BLUE", FL_BLUE},
	{"MAGENTA", FL_MAGENTA},
	{"CYAN", FL_CYAN},
	{"DARK_RED", FL_DARK_RED},
	{"DARK_GREEN", FL_DARK_GREEN},
	{"DARK_YELLOW", FL_DARK_YELLOW},
	{"DARK_BLUE", FL_DARK_BLUE},
	{"DARK_MAGENTA", FL_DARK_MAGENTA},
	{"DARK_CYAN", FL_DARK_CYAN},
	{"WHITE", FL_WHITE},
	{"FREE_COLOR", FL_FREE_COLOR},
	{"NUM_FREE_COLOR", FL_NUM_FREE_COLOR},
	{"GRAY_RAMP", FL_GRAY_RAMP},
	{"NUM_GRAY", FL_NUM_GRAY},
	{"GRAY", FL_GRAY},
	{"COLOR_CUBE", FL_COLOR_CUBE},
	{"NUM_RED", FL_NUM_RED},
	{"NUM_GREEN", FL_NUM_GREEN},
	{"NUM_BLUE", FL_NUM_BLUE},
	{"CURSOR_DEFAULT", FL_CURSOR_DEFAULT},
	{"CURSOR_ARROW", FL_CURSOR_ARROW},
	{"CURSOR_CROSS", FL_CURSOR_CROSS},
	{"CURSOR_WAIT", FL_CURSOR_WAIT},
	{"CURSOR_INSERT", FL_CURSOR_INSERT},
	{"CURSOR_HAND", FL_CURSOR_HAND},
	{"CURSOR_HELP", FL_CURSOR_HELP},
	{"CURSOR_MOVE", FL_CURSOR_MOVE},
	{"CURSOR_NS", FL_CURSOR_NS},
	{"CURSOR_WE", FL_CURSOR_WE},
	{"CURSOR_NWSE", FL_CURSOR_NWSE},
	{"CURSOR_NESW", FL_CURSOR_NESW},
	{"CURSOR_N", FL_CURSOR_N},
	{"CURSOR_NE", FL_CURSOR_NE},
	{"CURSOR_E", FL_CURSOR_E},
	{"CURSOR_SE", FL_CURSOR_SE},
	{"CURSOR_S", FL_CURSOR_S},
	{"CURSOR_SW", FL_CURSOR_SW},
	{"CURSOR_W", FL_CURSOR_W},
	{"CURSOR_NW", FL_CURSOR_NW},
	{"CURSOR_NONE", FL_CURSOR_NONE},
	{"READ", FL_READ},
	{"WRITE", FL_WRITE},
	{"EXCEPT", FL_EXCEPT},
	{"RGB", FL_RGB},
	{"INDEX", FL_INDEX},
	{"SINGLE", FL_SINGLE},
	{"DOUBLE", FL_DOUBLE},
	{"ACCUM", FL_ACCUM},
	{"ALPHA", FL_ALPHA},
	{"DEPTH", FL_DEPTH},
	{"STENCIL", FL_STENCIL},
	{"RGB8", FL_RGB8},
	{"MULTISAMPLE", FL_MULTISAMPLE},
	{"STEREO", FL_STEREO},
	{"FAKE_SINGLE", FL_FAKE_SINGLE},
	{"IMAGE_WITH_ALPHA", FL_IMAGE_WITH_ALPHA},
	{"DAMAGE_CHILD", FL_DAMAGE_CHILD},
	{"DAMAGE_EXPOSE", FL_DAMAGE_EXPOSE},
	{"DAMAGE_SCROLL", FL_DAMAGE_SCROLL},
	{"DAMAGE_OVERLAY", FL_DAMAGE_OVERLAY},
	{"DAMAGE_USER1", FL_DAMAGE_USER1},
	{"DAMAGE_USER2", FL_DAMAGE_USER2},
	{"DAMAGE_ALL", FL_DAMAGE_ALL},
	{"WHEN_NEVER", FL_WHEN_NEVER},
	{"WHEN_CHANGED", FL_WHEN_CHANGED},
	{"WHEN_NOT_CHANGED", FL_WHEN_NOT_CHANGED},
	{"WHEN_RELEASE", FL_WHEN_RELEASE},
	{"WHEN_RELEASE_ALWAYS", FL_WHEN_RELEASE_ALWAYS},
	{"WHEN_ENTER_KEY", FL_WHEN_ENTER_KEY},
	{"WHEN_ENTER_KEY_ALWAYS", FL_WHEN_ENTER_KEY_ALWAYS},
	{"WHEN_ENTER_KEY_CHANGED", FL_WHEN_ENTER_KEY_CHANGED},
	{"NO_BOX", FL_NO_BOX},
	{"FLAT_BOX,", FL_FLAT_BOX},
	{"UP_BOX,", FL_UP_BOX},
	{"DOWN_BOX,", FL_DOWN_BOX},
	{"UP_FRAME,", FL_UP_FRAME},
	{"DOWN_FRAME,", FL_DOWN_FRAME},
	{"THIN_UP_BOX,", FL_THIN_UP_BOX},
	{"THIN_DOWN_BOX,", FL_THIN_DOWN_BOX},
	{"THIN_UP_FRAME,", FL_THIN_UP_FRAME},
	{"THIN_DOWN_FRAME,", FL_THIN_DOWN_FRAME},
	{"ENGRAVED_BOX,", FL_ENGRAVED_BOX},
	{"EMBOSSED_BOX,", FL_EMBOSSED_BOX},
	{"ENGRAVED_FRAME,", FL_ENGRAVED_FRAME},
	{"EMBOSSED_FRAME,", FL_EMBOSSED_FRAME},
	{"BORDER_BOX,", FL_BORDER_BOX},
	{"_SHADOW_BOX,", _FL_SHADOW_BOX},
	{"BORDER_FRAME,", FL_BORDER_FRAME},
	{"_SHADOW_FRAME,", _FL_SHADOW_FRAME},
	{"_ROUNDED_BOX,", _FL_ROUNDED_BOX},
	{"_RSHADOW_BOX,", _FL_RSHADOW_BOX},
	{"_ROUNDED_FRAME,", _FL_ROUNDED_FRAME},
	{"_RFLAT_BOX,", _FL_RFLAT_BOX},
	{"_ROUND_UP_BOX,", _FL_ROUND_UP_BOX},
	{"_ROUND_DOWN_BOX,", _FL_ROUND_DOWN_BOX},
	{"_DIAMOND_UP_BOX,", _FL_DIAMOND_UP_BOX},
	{"_DIAMOND_DOWN_BOX,", _FL_DIAMOND_DOWN_BOX},
	{"_OVAL_BOX,", _FL_OVAL_BOX},
	{"_OSHADOW_BOX,", _FL_OSHADOW_BOX},
	{"_OVAL_FRAME,", _FL_OVAL_FRAME},
	{"_OFLAT_BOX,", _FL_OFLAT_BOX},
	{"_PLASTIC_UP_BOX,", _FL_PLASTIC_UP_BOX},
	{"_PLASTIC_DOWN_BOX,", _FL_PLASTIC_DOWN_BOX},
	{"_PLASTIC_UP_FRAME,", _FL_PLASTIC_UP_FRAME},
	{"_PLASTIC_DOWN_FRAME,", _FL_PLASTIC_DOWN_FRAME},
	{"_PLASTIC_THIN_UP_BOX,", _FL_PLASTIC_THIN_UP_BOX},
	{"_PLASTIC_THIN_DOWN_BOX,", _FL_PLASTIC_THIN_DOWN_BOX},
	{"_PLASTIC_ROUND_UP_BOX,", _FL_PLASTIC_ROUND_UP_BOX},
	{"_PLASTIC_ROUND_DOWN_BOX,", _FL_PLASTIC_ROUND_DOWN_BOX},
	{"_GTK_UP_BOX,", _FL_GTK_UP_BOX},
	{"_GTK_DOWN_BOX,", _FL_GTK_DOWN_BOX},
	{"_GTK_UP_FRAME,", _FL_GTK_UP_FRAME},
	{"_GTK_DOWN_FRAME,", _FL_GTK_DOWN_FRAME},
	{"_GTK_THIN_UP_BOX,", _FL_GTK_THIN_UP_BOX},
	{"_GTK_THIN_DOWN_BOX,", _FL_GTK_THIN_DOWN_BOX},
	{"_GTK_THIN_UP_FRAME,", _FL_GTK_THIN_UP_FRAME},
	{"_GTK_THIN_DOWN_FRAME,", _FL_GTK_THIN_DOWN_FRAME},
	{"_GTK_ROUND_UP_BOX,", _FL_GTK_ROUND_UP_BOX},
	{"_GTK_ROUND_DOWN_BOX,", _FL_GTK_ROUND_DOWN_BOX},
#if (FL_MAJOR_VERSION>=1 && FL_MINOR_VERSION>=3 && FL_PATCH_VERSION>=3)
	{"_GLEAM_UP_BOX,", _FL_GLEAM_UP_BOX},
	{"_GLEAM_DOWN_BOX,", _FL_GLEAM_DOWN_BOX},
	{"_GLEAM_UP_FRAME,", _FL_GLEAM_UP_FRAME},
	{"_GLEAM_DOWN_FRAME,", _FL_GLEAM_DOWN_FRAME},
	{"_GLEAM_THIN_UP_BOX,", _FL_GLEAM_THIN_UP_BOX},
	{"_GLEAM_THIN_DOWN_BOX,", _FL_GLEAM_THIN_DOWN_BOX},
	{"_GLEAM_ROUND_UP_BOX,", _FL_GLEAM_ROUND_UP_BOX},
	{"_GLEAM_ROUND_DOWN_BOX,", _FL_GLEAM_ROUND_DOWN_BOX},
#endif
	{"FREE_BOXTYPE", FL_FREE_BOXTYPE},
};


extern "C" {
int moonfltk_open_main(lua_State *L);
}

void createFLTKbindings(lua_State *L) {
	//FLTK bindings
	luaL_newlib(L, lua_flAPI);
	lua_setglobal(L, "fl");

	luaL_newlib(L, lua_FlAPI);
	lua_setglobal(L, "Fl");

	lua_createtable(L, 0, FL_FULLSCREEN + arLen(FLconsts));

	for (unsigned x = 0; x <= FL_FULLSCREEN; ++x)
		mkKeyunsigned(L, fl_eventnames[x] + 3, x);

	for (unsigned x = 0; x < arLen(FLconsts); ++x)
		mkKeyunsigned(L, FLconsts[x].key, FLconsts[x].pair);

	lua_setglobal(L, "FL");

	moonfltk_open_main(L);
	lua_setglobal(L, "fltk");
}
