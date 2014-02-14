/*
 This file is part of Retro Graphics Toolkit

    Retro Graphics Toolkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or any later version.

    Retro Graphics Toolkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
/*!
This is where all "project" releated stuff goes
For example the save project file function goes here
*/
#pragma once
#include "global.h"
#include "tilemap.h"
#include "class_tiles.h"
#include "classtilemap.h"
#define currentProjectVersionNUM 0
extern uint32_t curProjectID;
struct Project/*!<Holds all data needed for a project based system for examaple tile screen and level 1 are 2 seperate projects*/
{
	std::string Name;
	uint32_t gameSystem;
	tileMap * tileMapC;
	tiles * tileC;
	uint8_t rgbPal[256];
	uint8_t palDat[128];
	uint8_t palType[64];/*!<Sets 3 different types for each palette entry free locked and reserved*/
	
};
extern struct Project ** projects;
extern uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
extern struct Project * currentProject;
extern Fl_Slider* curPrj;
void initProject(void) __attribute__((constructor(101)));/*!< this needs to be ran before class constructors*/
bool appendProject();
bool removeProject(uint32_t id);
void switchProject(uint32_t id);
bool loadProject(uint32_t id);
bool saveProject(uint32_t id);
