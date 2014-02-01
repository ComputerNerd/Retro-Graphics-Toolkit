/*!
This is where all "project" releated stuff goes
For example the save project file function goes here
*/
#pragma once
#include "global.h"
#include "tilemap.h"
#include "class_tiles.h"
#include "classtilemap.h"
struct Project/*!<Holds all data needed for a project based system for examaple tile screen and level 1 are 2 seperate projects*/
{
	std::string Name;
	uint8_t gameSystem;
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
void initProject(void);/*!< this needs to be called before using addProject*/
bool appendProject();
bool removeProject(uint32_t id);
