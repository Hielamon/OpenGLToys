#pragma once

#include <SPhoenix/utils.h>
#include <SPhoenix/core.h>

namespace SP
{
	class Manipulator
	{
	public:
		Manipulator() {}
		~Manipulator() {}

		GLFWkeyfun getCallBack(Camera *pCam)
		{
			if (!pCam)
			{
				SP_CERR("Cannot get callback for a empty camera pointer");
				exit(-1);
			}
			mpCam = pCam;

			glfwSetWindowUserPointer(mpCam->getGLFWWinPtr(), this);

			GLFWkeyfun result = [](GLFWwindow *window, int key, int scancode, int action, int mode)
			{
				static_cast<Manipulator*>(glfwGetWindowUserPointer(window))->keyCallBackImpl(
					window, key, scancode, action, mode
				);
			};
			return result;
		}

		void keyCallBackImpl(GLFWwindow *window, int key, int scancode, int action, int mode)
		{
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, GL_TRUE);
			}
			/*if (key == GLFW_KEY_S && action == GLFW_PRESS)
			{
				for (size_t i = 0; i < glwindow_event_instance->m_geometry_array.size(); i++)
				{
					glwindow_event_instance->m_geometry_array[i].m_is_show = !glwindow_event_instance->m_geometry_array[i].m_is_show;
				}
			}*/
			if (key == GLFW_KEY_R /*&& action == GLFW_PRESS*/)
			{
				glm::mat4 rotate_m = glm::rotate(mpCam->getSceneUtil(0).getModelMatrix(), glm::radians(8.f), glm::vec3(0.0f, 1.0f, 0.0f));
				
				mpCam->getSceneUtil(0).setModelMatrix(rotate_m);
			}
			if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
			{
				GLint rastMode;
				glGetIntegerv(GL_POLYGON_MODE, &rastMode);
				GLint bias = rastMode - GL_POINT;
				/*GLboolean isMode;
				glGetBooleanv(GL_POLYGON_OFFSET_LINE, &isMode);
				if (isMode == GL_FALSE)
				{
					std::cout << "Right and strange output" << std::endl;
				}*/
				
				bias = (bias + 1) % 3;
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT + bias);

				/*std::cout << "bias = " << bias << std::endl;
				std::cout << "rastMode[0] = " << rastMode << std::endl;
				switch (bias)
				{
				case 0:
					std::cout << "GL_POINT" << std::endl;
					glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
					break;
				case 1:
					std::cout << "GL_LINE" << std::endl;
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					break;
				case 2:
					std::cout << "GL_FILL" << std::endl;
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					break;
				default:
					break;
				}*/
			}
		}

	private:

		Camera *mpCam;
	};
}



