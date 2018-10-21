#pragma once

#include <memory>
#include <algorithm>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>

#include <iomanip>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

#include "GlobalMacros.h"

namespace SP
{
	class SPConfigure
	{
	public:
		static SPConfigure &getInstance()
		{
			static SPConfigure SPConfig;
			return SPConfig;
		}

		glm::vec3 FollowedEye, FollowedUp;
		glm::vec4 CameraColor, FollowedCameraColor;
		float DefaultShininess, DefaultShininessStrength;

	private:
		SPConfigure() :
			FollowedEye(0.0f, 0.1f, 0.3f),
			FollowedUp(0.0f, 1.0f, 0.0f),
			CameraColor(1.0f, 0.0f, 0.0f, 1.0f),
			FollowedCameraColor(1.0f, 1.0f, 0.0f, 1.0f),
			DefaultShininess(32.f), DefaultShininessStrength(0.5f)
		{
			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			std::string configFile = __currentPATH + "/SPConfigure.txt";

			std::fstream cfs(configFile, std::ios::in);
			if (!cfs.is_open())
			{
				SP_CERR("The the configure file doesn't exist in " + configFile);
				printDefaultSetting();
				return;
			}

			std::stringstream ioStr;
			std::string line;
			while (std::getline(cfs, line))
			{
				std::string label;
				ioStr.clear();
				ioStr << line;
				ioStr >> label;
				if (label == "FollowedEye")
				{
					ioStr >> FollowedEye.x >> FollowedEye.y >> FollowedEye.z;
				}
				else if (label == "FollowedUp")
				{
					ioStr >> FollowedUp.x >> FollowedUp.y >> FollowedUp.z;
				}
				else if (label == "CameraColor")
				{
					ioStr >> CameraColor.r >> CameraColor.g >>
						CameraColor.b >> CameraColor.w;
				}
				else if (label == "FollowedCameraColor")
				{
					ioStr >> FollowedCameraColor.r >> FollowedCameraColor.g >>
						FollowedCameraColor.b >> FollowedCameraColor.w;
				}
				else if (label == "DefaultShininess")
				{
					ioStr >> DefaultShininess;
				}
				else if (label == "DefaultShininessStrength")
				{
					ioStr >> DefaultShininessStrength;
				}
			}
		}

		void printDefaultSetting()
		{
			std::cout << "The default setting is :" << std::endl;

			std::cout << "FollowedEye : [" << FollowedEye.x << ", " << FollowedEye.y
				<< ", " << FollowedEye.z << "]" << std::endl;

			std::cout << "FollowedUp : [" << FollowedUp.x << ", " << FollowedUp.y
				<< ", " << FollowedUp.z << "]" << std::endl;

			std::cout << "CameraColor : [" << CameraColor.r << ", " << CameraColor.g
				<< ", " << CameraColor.b << ", " << CameraColor.w << "]" << std::endl;

			std::cout << "FollowedCameraColor : [" << FollowedCameraColor.r << ", " << FollowedCameraColor.g
				<< ", " << FollowedCameraColor.b << ", " << FollowedCameraColor.w << "]" << std::endl;

			std::cout << "DefaultShininess : " << DefaultShininess << std::endl;
			std::cout << "DefaultShininessStrength : " << DefaultShininessStrength << std::endl;
		}
	};

#define DEFAULT_SHININESS SPConfigure::getInstance().DefaultShininess
#define DEFAULT_SHININESS_STRENGTH SPConfigure::getInstance().DefaultShininessStrength

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

}





