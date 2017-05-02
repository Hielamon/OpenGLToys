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

		GLFWkeyfun getKeyCallBack(Camera *pCam)
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

		GLFWscrollfun getScrollCallBack(Camera *pCam)
		{
			if (!pCam)
			{
				SP_CERR("Cannot get callback for a empty camera pointer");
				exit(-1);
			}
			mpCam = pCam;

			glfwSetWindowUserPointer(mpCam->getGLFWWinPtr(), this);

			GLFWscrollfun result = [](GLFWwindow *window, double xoffset, double yoffset)
			{
				static_cast<Manipulator*>(glfwGetWindowUserPointer(window))->scrollCallBack(
					window, xoffset, yoffset
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
			if (key == GLFW_KEY_D /*&& action == GLFW_PRESS*/)
			{
				glm::mat4 rotate_m = glm::rotate(mpCam->getSceneUtil(0).getModelMatrix(), glm::radians(5.f), glm::vec3(0.0f, 1.0f, 0.0f));
				
				mpCam->getSceneUtil(0).setModelMatrix(rotate_m);
			}
			if (key == GLFW_KEY_A /*&& action == GLFW_PRESS*/)
			{
				glm::mat4 rotate_m = glm::rotate(mpCam->getSceneUtil(0).getModelMatrix(), glm::radians(-5.f), glm::vec3(0.0f, 1.0f, 0.0f));

				mpCam->getSceneUtil(0).setModelMatrix(rotate_m);
			}
			if (key == GLFW_KEY_W /*&& action == GLFW_PRESS*/)
			{
				glm::mat4 rotate_m = glm::rotate(mpCam->getSceneUtil(0).getModelMatrix(), glm::radians(-5.f), glm::vec3(1.0f, 0.0f, 0.0f));

				mpCam->getSceneUtil(0).setModelMatrix(rotate_m);
			}
			if (key == GLFW_KEY_S /*&& action == GLFW_PRESS*/)
			{
				glm::mat4 rotate_m = glm::rotate(mpCam->getSceneUtil(0).getModelMatrix(), glm::radians(5.f), glm::vec3(1.0f, 0.0f, 0.0f));

				mpCam->getSceneUtil(0).setModelMatrix(rotate_m);
			}
			if (key == GLFW_KEY_R && action == GLFW_PRESS)
			{
				if (mpCam->getSceneUtil(0).getModelMatrix() != glm::mat4(1.0f))
				{
					if(preModelMatrix == glm::mat4(1.0f))preModelMatrix = mpCam->getSceneUtil(0).getModelMatrix();
					mpCam->getSceneUtil(0).setModelMatrix(glm::mat4(1.0f));
				}
				else
				{
					mpCam->getSceneUtil(0).setModelMatrix(preModelMatrix);
				}
			}

			if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
			{
				GLint rastMode;
				glGetIntegerv(GL_POLYGON_MODE, &rastMode);
				GLint bias = rastMode - GL_POINT;
				bias = (bias + 1) % 3;
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT + bias);

			}
		}

		void scrollCallBack(GLFWwindow *window, double xoffset, double yoffset)
		{
			float curFovy = glm::degrees(mpCam->mfovy);

			curFovy -= yoffset;

			curFovy = std::max(1.0f, std::min(curFovy, 180.0f));
			
			mpCam->setFrustum(curFovy, mpCam->maspect);

		}

	private:

		Camera *mpCam;
		glm::mat4 preModelMatrix;
	};
}



