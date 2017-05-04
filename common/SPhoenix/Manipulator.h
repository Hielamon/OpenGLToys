#pragma once

#include <SPhoenix/utils.h>
#include <SPhoenix/core.h>
#include <SPhoenix/ManipulatorBase.h>

namespace SP
{
	class Manipulator : public ManipulatorBase
	{
	public:
		Manipulator(Camera *pCam)
		{
			if (!pCam)
			{
				SP_CERR("the camera prepared for initial the Manipulator cannot be NULL");
				exit(-1);
			}
			mpCam = pCam;
			mfovy = mpCam->mfovy;
			maspect = mpCam->maspect;
			mzNear = mpCam->mzNear;
			mzFar = mpCam->mzFar;

			meye = mpCam->meye;
			mup = mpCam->mup;
			mcenter = mpCam->mcenter;

			memset(mkeyState, GL_FALSE, KEY_COUNT);
			memset(mmouseButtonState, GL_FALSE, MOUSE_COUNT);
			
		}

		~Manipulator() {}

		void registerCallBacks()
		{
			if (!mpCam)
			{
				SP_CERR("Cannot get callback for a empty camera pointer");
				exit(-1);
			}

			GLFWwindow *curWinPtr = mpCam->getGLFWWinPtr();
			assert(curWinPtr != nullptr);

			glfwSetWindowUserPointer(curWinPtr, this);

			//Register callbacks to the current window
			_registerKeyCallBack(curWinPtr);
			_registerScrollCallBack(curWinPtr);
			_registerMouseButtonCallBack(curWinPtr);
			_registerCursorPosCallBack(curWinPtr);
			
		}

	protected:
		virtual void keyCallBackImpl(GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			mkeyState[key] = action;

			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				if (action == GLFW_PRESS)
				{
					glfwSetWindowShouldClose(window, GL_TRUE);
				}
				break;
			case GLFW_KEY_R:
				mpCam->setProjectionMatrix(mfovy, maspect, mzNear, mzFar);
				mpCam->setViewMatrix(meye, mcenter, mup);
				break;
			case GLFW_KEY_TAB:
				if (action == GLFW_PRESS)
				{
					GLint rastMode;
					glGetIntegerv(GL_POLYGON_MODE, &rastMode);
					GLint bias = rastMode - GL_POINT;
					bias = (bias + 1) % 3;
					glPolygonMode(GL_FRONT_AND_BACK, GL_POINT + bias);
				}
				break;
			default:
				break;
			}
		}

		virtual void scrollCallBackImpl(GLFWwindow *window, double xoffset, double yoffset)
		{
			float curFovy = mpCam->mfovy;

			curFovy -= float(yoffset);

			curFovy = std::max(1.0f, std::min(curFovy, 180.0f));
			mpCam->setProjectionMatrix(curFovy, mpCam->maspect);
		}

		virtual void mouseButtonCallBackImpl(GLFWwindow *window, int button, int action, int mods)
		{
			mmouseButtonState[button] = action;
		}

		virtual void cursorPosCallBackImpl(GLFWwindow *window, double xpos, double ypos)
		{
			double dx = xpos - mpreX;
			double dy = ypos - mpreY;

			if (abs(dx) >= 1 || abs(dy) >= 1)
			{
				
				float dxRad = glm::radians(dx);
				float dyRad = glm::radians(dy);
				if (mmouseButtonState[GLFW_MOUSE_BUTTON_LEFT])
				{
					std::cout << "dx = " << dx << std::endl;
					std::cout << "dy = " << dy << std::endl;

					glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), dxRad, glm::vec3(0.0f, 1.0f, 0.0f));
					rotate = glm::rotate(rotate, dyRad, glm::vec3(1.0f, 0.0f, 0.0f));

					glm::vec4 eyeHomo(mpCam->meye, 1.0f);
					eyeHomo = rotate * eyeHomo;
					glm::vec3 eye(eyeHomo);

					glm::vec4 upHomo(mpCam->mup, 0.0f);
					upHomo = rotate * upHomo;
					glm::vec3 up(upHomo);

					mpCam->setViewMatrix(eye, mpCam->mcenter, up);

				}
				else if (mmouseButtonState[GLFW_MOUSE_BUTTON_MIDDLE])
				{

				}
				mpreX = xpos;
				mpreY = ypos;
			}
		}

	protected:
		Camera *mpCam;

		float mfovy, maspect, mzNear, mzFar;
		glm::vec3 meye, mcenter, mup;

		//indicate whether the key is pressed
		bool mkeyState[KEY_COUNT];

		//indicate whether the mouse button is pressed
		bool mmouseButtonState[MOUSE_COUNT];

		//record pre-frame's cursor corrdinate
		double mpreX, mpreY;

	private:

		void _registerKeyCallBack(GLFWwindow *curWinPtr)
		{
			GLFWkeyfun keyfun = [](GLFWwindow *window, int key, int scancode, int action, int mode)
			{
				static_cast<Manipulator*>(glfwGetWindowUserPointer(window))->keyCallBackImpl(
					window, key, scancode, action, mode
				);
			};
			glfwSetKeyCallback(curWinPtr, keyfun);
		}

		void _registerScrollCallBack(GLFWwindow *curWinPtr)
		{

			GLFWscrollfun scrollfun = [](GLFWwindow *window, double xoffset, double yoffset)
			{
				static_cast<Manipulator*>(glfwGetWindowUserPointer(window))->scrollCallBackImpl(
					window, xoffset, yoffset
				);
			};
			glfwSetScrollCallback(curWinPtr, scrollfun);
		}

		void _registerMouseButtonCallBack(GLFWwindow *curWinPtr)
		{
			GLFWmousebuttonfun mousebuttonfun = [](GLFWwindow *window, int button, int action, int mods)
			{
				static_cast<Manipulator*>(glfwGetWindowUserPointer(window))->mouseButtonCallBackImpl(
					window, button, action, mods
				);
			};
			glfwSetMouseButtonCallback(curWinPtr, mousebuttonfun);
		}

		void _registerCursorPosCallBack(GLFWwindow *curWinPtr)
		{
			GLFWcursorposfun cursorposfun = [](GLFWwindow *window, double xpos, double ypos)
			{
				static_cast<Manipulator*>(glfwGetWindowUserPointer(window))->cursorPosCallBackImpl(
					window, xpos, ypos
				);
			};
			glfwSetCursorPosCallback(curWinPtr, cursorposfun);
		}
	};
}



