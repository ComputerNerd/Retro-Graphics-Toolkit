#include "global.h"
/*Some users may not be programmers and will not run a debugger these error messages should make it easier for them to convey a glitch to me
All they have to do is copy and paste the error message*/
void malloc_error(int line,const char * file,const char * function,int bytes)
{
	fl_alert("malloc error in file %s function %s line %d\nNumber of bytes attempted %d",file,function,line,bytes);
}
void realloc_error(int line,const char * file,const char * function,int bytes)
{
	fl_alert("realloc error in file %s function %s line %d\nNumber of bytes attempted %d",file,function,line,bytes);
}
void default_trigger(int line,const char * file,const char * function)
{
	/*!
	In a switch statment sometimes there should be no defualt action so this function gets called when the varible does not compare with any cases
	*/
	fl_alert("Default triggered in file %s function %s line %d",file,function,line);
}
