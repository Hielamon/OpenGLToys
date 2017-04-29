#pragma once

#include <memory>
#include <algorithm>

#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <SOIL.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

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



