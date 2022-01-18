#include "mvViewport.h"
#include "mvTypes.h"


mv_internal void 
enable_cursor(mvViewport& viewport)
{
	viewport.cursorEnabled = true;
	glfwSetInputMode(viewport.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetInputMode(viewport.handle, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

mv_internal void 
disable_cursor(mvViewport& viewport)
{
	viewport.cursorEnabled = false;
	glfwSetInputMode(viewport.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(viewport.handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

mv_internal void 
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	mvViewport& viewport = *(mvViewport*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		if (viewport.cursorEnabled)
			disable_cursor(viewport);
		else
			enable_cursor(viewport);
	}
}

mv_internal void 
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	mvViewport& viewport = *(mvViewport*)glfwGetWindowUserPointer(window);
	float xoffset = xpos - viewport.lastX;
	float yoffset = ypos - viewport.lastY;
	viewport.deltaX = xoffset;
	viewport.deltaY = yoffset;
	viewport.lastX = xpos;
	viewport.lastY = ypos;
}

mv_internal void 
framebuffer_resize(GLFWwindow* window, int width, int height)
{
	mvViewport& viewport = *(mvViewport*)glfwGetWindowUserPointer(window);
	viewport.resized = true;
}

void
initialize_viewport(mvViewport& viewport, int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	viewport.handle = glfwCreateWindow(width, height, "Dear Py3D", nullptr, nullptr);
	glfwSetCursorPosCallback(viewport.handle, mouse_callback);
	glfwSetKeyCallback(viewport.handle, key_callback);
	glfwSetWindowSizeCallback(viewport.handle, framebuffer_resize);

	viewport.width = width;
	viewport.height = height;
	viewport.running = true;
	glfwSetWindowUserPointer(viewport.handle, &viewport);
}

void 
process_viewport_events(mvViewport& viewport)
{
	viewport.deltaX = 0.0;
	viewport.deltaY = 0.0;
	viewport.running = !glfwWindowShouldClose(viewport.handle);
	glfwPollEvents();
}
