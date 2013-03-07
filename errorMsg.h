#pragma once
void malloc_error(int line,const char * file,const char * function,int bytes);
void realloc_error(int line,const char * file,const char * function,int bytes);
void default_trigger(int line,const char * file,const char * function);
#define show_malloc_error(bytes_error) malloc_error(__LINE__,__FILE__,__FUNCTION__,bytes_error);
#define show_realloc_error(bytes_error) realloc_error(__LINE__,__FILE__,__FUNCTION__,bytes_error);
#define show_default_error default_trigger(__LINE__,__FILE__,__FUNCTION__);
