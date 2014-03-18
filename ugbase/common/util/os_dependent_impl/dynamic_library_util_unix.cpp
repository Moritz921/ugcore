// created by Sebastian Reiter
// s.b.reiter@gmail.com
// 10.01.2013 (d,m,y)

#include <dlfcn.h>
#include "../dynamic_library_util.h"
#include <string>

namespace ug{

DynLibHandle OpenLibrary(const char* fileName)
{
	DynLibHandle dlh = dlopen(fileName, RTLD_LAZY);
	if(dlh == NULL)
		throw std::string(dlerror());
	return dlh;
}

bool CloseLibrary(DynLibHandle h)
{
	return dlclose(h);
}

void* GetLibraryProcedure(DynLibHandle h, const char* procName)
{
	return dlsym(h, procName);
}

}// end of namespace
