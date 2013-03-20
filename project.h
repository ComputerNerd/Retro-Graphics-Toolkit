/*!
This is where all "project" releated stuff goes
For example the save project file function goes here
*/
#pragma once
#include "global.h"
#include "tilemap.h"
#include "class_tiles.h"
struct Project/*!<Holds all data needed for a project based system for examaple tile screen and level 1 are 2 seperate projects*/
{
	std::string Name;
	uint8_t gameSystem;
	tileMap * tileMapC;
	tiles * tileC;
	uint8_t rgbPal[192];
	uint8_t palDat[128];
	
};
extern struct Project ** projects;
extern uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
extern struct Project * currentProject;
void initProject();/*!< this needs to be called before using addProject*/
bool addProjectAfter(uint32_t id);
bool removeProject(uint32_t id);
