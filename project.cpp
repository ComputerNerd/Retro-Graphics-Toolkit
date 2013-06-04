#include "project.h"
#include <FL/Fl.H>
#include <FL/Fl_Hor_Value_Slider.H>
struct Project ** projects;
uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
struct Project * currentProject;
Fl_Slider* curPrj;
void initProject()
{
	projects = (struct Project **) malloc(sizeof(void *));
	projects[0] = new struct Project;
	currentProject=projects[0];
	projects_count=1;
	currentProject->tileC = new tiles;
	currentProject->tileMapC = new tileMap;
	currentProject->Name.assign("Add a description here");
	memset(currentProject->palDat,0,128);
	memset(currentProject->rgbPal,0,192);
}
bool appendProject()
{
	projects = (struct Project **) realloc(projects,(projects_count+1)*sizeof(void *));
	if (projects == 0)
	{
		show_realloc_error((projects_count+1)*sizeof(void *))
		return false;
	}
	projects[projects_count] = new struct Project;
	currentProject=projects[projects_count];
	currentProject->tileC = new tiles;
	currentProject->tileMapC = new tileMap;
	currentProject->Name.assign("Add a description here");
	projects_count++;
	memset(currentProject->palDat,0,128);
	memset(currentProject->rgbPal,0,192);
	return true;
}
bool removeProject(uint32_t id)
{
	//removes selected project
	if (projects_count == 0)
	{
		fl_alert("You must have atleast one project did you think I could just let you have null pointers?");
		return false;
	}
	delete projects[id]->tileC;
	delete projects[id]->tileMapC;
	delete projects[id];
	if (projects_count == 1)
		return true;
	projects_count--;
	return true;
}
