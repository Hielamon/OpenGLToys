#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <SPhoenix/global_macro.h>

/**Get the resolution of primary screen*/
inline glm::u32vec2 GetScreenResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (mode == NULL)
	{
		SP_CERR("cannot get the screen's resolution");
		return glm::u32vec2(-1, -1);
	}
	return glm::u32vec2(mode->width, mode->height);
}
