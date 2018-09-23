#pragma once

#include <memory>
#include <algorithm>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

#include "GlobalMacros.h"


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

inline void RigidTransformLookAt(glm::mat4 &rigid, glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up)
{
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), eye);
	glm::vec3 zDir = eye - center;
	glm::vec3 zAxis = glm::normalize(zDir);
	glm::vec3 xAxis = glm::normalize(glm::cross(up, zAxis));
	glm::vec3 yAxis = glm::cross(zAxis, xAxis);

	glm::vec4 col1 = glm::vec4(xAxis, 0.0f);
	glm::vec4 col2 = glm::vec4(yAxis, 0.0f);
	glm::vec4 col3 = glm::vec4(zAxis, 0.0f);
	glm::vec4 col4 = glm::vec4(glm::vec3(0.0f), 1.0f);

	glm::mat4 rotate = glm::mat4(col1, col2, col3, col4);
	glm::mat4 Cam2World = translate * rotate;

	glm::vec4 eyeHomo(glm::vec3(0.0f), 1.0f);
	eye = glm::vec3(Cam2World * rigid * eyeHomo);

	float zDistance = glm::dot(zDir, zDir);
	zDistance = sqrtf(zDistance);

	glm::vec4 centerHomo(0.0f, 0.0f, -zDistance, 1.0f);
	center = glm::vec3(Cam2World * rigid * centerHomo);

	glm::vec4 upHomo(0.0f, 1.0f, 0.0f, 0.0f);
	up = glm::vec3(Cam2World * rigid * upHomo);

	/*glm::vec4 eyeHomo(eye, 1.0f);
	eye = glm::vec3(rigid * eyeHomo);

	glm::vec4 centerHomo(center, 1.0f);
	center = glm::vec3(rigid * centerHomo);

	glm::vec4 upHomo(up, 0.0f);
	up = glm::vec3(rigid * upHomo);*/
}

