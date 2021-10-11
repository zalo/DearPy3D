#include "mvContext.h"

extern mvContext* GContext = nullptr;

void 
CreateContext()
{
	GContext = new mvContext();
}

void 
DestroyContext()
{
	glfwTerminate();
	delete GContext;
	GContext = nullptr;
}
