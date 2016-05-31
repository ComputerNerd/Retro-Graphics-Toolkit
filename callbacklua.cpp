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
	Copyright Sega16 (or whatever you wish to call me) (2012-2016)
*/
#include "callbacklua.h"
#include "project.h"
#include "class_global.h"
#include "runlua.h"
int curScript=-1;
void appendLuaScript(Fl_Widget*,void*){
	window->luaEditProject->show();
	unsigned name=currentProject->lScrpt.size();
	std::string nstr=std::to_string(name);
	currentProject->lScrpt.emplace_back(luaScript{std::string(""),nstr});
	window->luaScriptSel->add(nstr.c_str(),0,switchCurLuaScript,(void*)(intptr_t)name);
	window->luaScriptSel->value(name);
	switchCurLuaScript(nullptr,(void*)(intptr_t)name);
}
void deleteLuaScript(Fl_Widget*,void*){
	if(curScript>=0){
		currentProject->lScrpt.erase(currentProject->lScrpt.begin()+curScript);
		window->luaScriptSel->remove(curScript);
		if(currentProject->lScrpt.size()){
			if(curScript>=currentProject->lScrpt.size()){
				curScript=currentProject->lScrpt.size()-1;
				window->luaScriptName->value(currentProject->lScrpt[curScript].name.c_str());
				window->luaBufProject->text(currentProject->lScrpt[curScript].str.c_str());
			}
		}else
			window->luaEditProject->hide();
	}
	window->redraw();
}
void setNameLuaScript(Fl_Widget*w,void*){
	Fl_Input*i=(Fl_Input*)w;
	if(curScript>=0){
		currentProject->lScrpt[curScript].name.assign(i->value());
		window->luaScriptSel->replace(curScript,currentProject->lScrpt[curScript].name.c_str());
	}
	window->redraw();
}
void switchCurLuaScript(Fl_Widget*,void*o){
	if(curScript>=0)
		currentProject->lScrpt[curScript].str.assign(window->luaBufProject->text());
	curScript=(intptr_t)o;
	window->luaScriptName->value(currentProject->lScrpt[curScript].name.c_str());
	window->luaBufProject->text(currentProject->lScrpt[curScript].str.c_str());
	window->redraw();
}
void runCurLuaScript(Fl_Widget*,void*){
	if(curScript>=0){
		lua_State*L=createLuaState();
		currentProject->lScrpt[curScript].str.assign(window->luaBufProject->text());
		runLua(L,currentProject->lScrpt[curScript].str.c_str(),false);
	}
}
