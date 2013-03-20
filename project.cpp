#include "project.h"
struct Project ** projects;
uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
struct Project * currentProject;
void initProject()
{
	projects = (struct Project **) malloc(sizeof(void *));
	projects[0] = new struct Project;
	currentProject=projects[0];
	projects_count=1;
	currentProject->tileC = new tiles;
	currentProject->tileMapC = new tileMap;
	currentProject->Name.assign("Add a description here");
	
}
bool addProjectAfter(uint32_t id)
{
	projects_count++;
	projects = (struct Project **) realloc(projects,projects_count*sizeof(void *));
	//if (id != projects_count)
	//	memmove(arr+id, arr+id+1, (projects_count-id-1)*sizeof(void *));
	return true;
}
