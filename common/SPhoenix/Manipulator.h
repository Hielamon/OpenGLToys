#pragma once

#include "MonitorWindow.h"

namespace SP
{
	class WinManipulator : public ManipulatorBase
	{
	public:
		WinManipulator(const std::shared_ptr<GLWindowBase> &pGLWindowBase)
			: mpGLWindowBase(pGLWindowBase), mCursorPosX(0), mCursorPosY(0)
		{
			if (pGLWindowBase.use_count() == 0)
			{
				SP_CERR("the camera prepared for initial the Manipulator cannot be NULL");
				exit(-1);
			}
			
			memset(mKeyState, GL_FALSE, KEY_COUNT);
			memset(mMouseButtonState, GL_FALSE, MOUSE_COUNT);
		}

		WinManipulator() = delete;

		~WinManipulator() {
			removeCallBacks();
			std::cout << "Deconstruction of Manipulator" << std::endl;
		}

		void registerCallBacks()
		{
			GLFWwindow *curWinPtr = mpGLWindowBase->getGLFWWinPtr();
			assert(curWinPtr != nullptr);

			glfwSetWindowUserPointer(curWinPtr, this);

			//Register callbacks to the current window
			_registerKeyCallBack(curWinPtr);
			_registerScrollCallBack(curWinPtr);
			_registerMouseButtonCallBack(curWinPtr);
			_registerCursorPosCallBack(curWinPtr);
		}

		void removeCallBacks()
		{

			GLFWwindow *curWinPtr = mpGLWindowBase->getGLFWWinPtr();
			assert(curWinPtr != nullptr);

			_removeKeyCallBack(curWinPtr);
			_removeScrollCallBack(curWinPtr);
			_removeMouseButtonCallBack(curWinPtr);
			_removeCursorPosCallBack(curWinPtr);
		}

	protected:
		virtual void keyCallBackImpl(GLFWwindow *window, int key, int scancode, int action, int mods)
		{

			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				if (action == GLFW_PRESS)
				{
					std::cout << "Close the window" << std::endl;
					glfwSetWindowShouldClose(window, GL_TRUE);
				}
				break;
			/*case GLFW_KEY_R:
				mpCam->setProjectionMatrix(mfovy, maspect, mzNear, mzFar);
				if(mKeyState[GLFW_KEY_LEFT_CONTROL])
					mpCam->setViewMatrix(meye, mcenter, mup);
				mpCam->deleteManipulator();
				return;
				break;
			case GLFW_KEY_C:
				if (action == GLFW_PRESS)
				{
					mpCam->revertShowIDColor();
				}
				break;*/
			
			default:
				break;
			}

			mKeyState[key] = action;
		}

		virtual void scrollCallBackImpl(GLFWwindow *window, double xoffset, double yoffset)
		{
			/*float curFovy = mpCam->mfovy;

			curFovy -= float(yoffset);

			curFovy = std::max(1.0f, std::min(curFovy, 180.0f));
			mpCam->setProjectionMatrix(curFovy, mpCam->maspect);*/
		}

		virtual void mouseButtonCallBackImpl(GLFWwindow *window, int button, int action, int mods)
		{
			/*if (button == GLFW_MOUSE_BUTTON_LEFT &&
				action == GLFW_PRESS &&
				mMouseButtonState[button] == GLFW_RELEASE)
			{
				double x, y;
				glfwGetCursorPos(mpCam->getGLFWWinPtr(), &x, &y);
				//std::cout << "x = " << x << ", y = " << y <<  std::endl;
				glm::i32vec2 winSize = mpCam->getWindowSize();
				int bufferx = x, buffery = winSize.y - y;
				GLuint meshID = 0;
				meshID = mpCam->getPointMeshID(bufferx, buffery);

				if (mKeyState[GLFW_KEY_LEFT_CONTROL])
				{
					mpCam->addSelectedID(meshID);
					if (meshID > 0)
					{
						std::cout << "MeshID = " << meshID << std::endl;
					}
				}
				else if(mKeyState[GLFW_KEY_LEFT_SHIFT])
				{
					mpCam->deleteSelectedID(meshID);
				}
				else if(meshID == 0)
				{
					mpCam->setSelectedID(meshID);
				}
				
			}*/

			mMouseButtonState[button] = action;
		}

		virtual void cursorPosCallBackImpl(GLFWwindow *window, double xpos, double ypos)
		{
			mCursorPosX = xpos;
			mCursorPosY = ypos;
			//std::cout << "xpos = " << xpos << ";" << "ypos = " << ypos << std::endl;
		}

	protected:
		std::shared_ptr<GLWindowBase> mpGLWindowBase;

		//indicate whether the key is pressed
		bool mKeyState[KEY_COUNT];

		//indicate whether the mouse button is pressed
		bool mMouseButtonState[MOUSE_COUNT];

		//record current-frame's cursor corrdinate
		double mCursorPosX, mCursorPosY;

	private:

		void _registerKeyCallBack(GLFWwindow *curWinPtr)
		{
			GLFWkeyfun keyfun = [](GLFWwindow *window, int key, int scancode, int action, int mode)
			{
				static_cast<WinManipulator*>(glfwGetWindowUserPointer(window))->keyCallBackImpl(
					window, key, scancode, action, mode
				);
				/*static_cast<Manipulator*>(0)->keyCallBackImpl(
					window, key, scancode, action, mode
				);*/
			};
			glfwSetKeyCallback(curWinPtr, keyfun);
		}

		void _removeKeyCallBack(GLFWwindow *curWinPtr)
		{
			glfwSetKeyCallback(curWinPtr, NULL);
		}

		void _registerScrollCallBack(GLFWwindow *curWinPtr)
		{

			GLFWscrollfun scrollfun = [](GLFWwindow *window, double xoffset, double yoffset)
			{
				static_cast<WinManipulator*>(glfwGetWindowUserPointer(window))->scrollCallBackImpl(
					window, xoffset, yoffset
				);
			};
			glfwSetScrollCallback(curWinPtr, scrollfun);
		}

		void _removeScrollCallBack(GLFWwindow *curWinPtr)
		{
			glfwSetScrollCallback(curWinPtr, NULL);
		}

		void _registerMouseButtonCallBack(GLFWwindow *curWinPtr)
		{
			GLFWmousebuttonfun mousebuttonfun = [](GLFWwindow *window, int button, int action, int mods)
			{
				static_cast<WinManipulator*>(glfwGetWindowUserPointer(window))->mouseButtonCallBackImpl(
					window, button, action, mods
				);
			};
			glfwSetMouseButtonCallback(curWinPtr, mousebuttonfun);
		}

		void _removeMouseButtonCallBack(GLFWwindow *curWinPtr)
		{
			glfwSetMouseButtonCallback(curWinPtr, NULL);
		}

		void _registerCursorPosCallBack(GLFWwindow *curWinPtr)
		{
			GLFWcursorposfun cursorposfun = [](GLFWwindow *window, double xpos, double ypos)
			{
				static_cast<WinManipulator*>(glfwGetWindowUserPointer(window))->cursorPosCallBackImpl(
					window, xpos, ypos
				);
			};
			glfwSetCursorPosCallback(curWinPtr, cursorposfun);
		}

		void _removeCursorPosCallBack(GLFWwindow *curWinPtr)
		{
			glfwSetCursorPosCallback(curWinPtr, NULL);
		}
	};

	class MonitorManipulator : public WinManipulator
	{
	public:
		MonitorManipulator(const std::shared_ptr<MonitorWindow> &pMonitorWindow)
			: WinManipulator(pMonitorWindow), mpMonitorWindow(pMonitorWindow),
			mMoveDirCount(0), mFollowedCameraIdx(0), mMaximizeCameraIdx(0)
		{
			mvMoveDir.resize(6, glm::vec3(0.0f));
			mAcEyeVary = glm::vec3(0.0f);
			mAcUpVary = glm::vec3(0.0f);
		}

		~MonitorManipulator() {}

		/*void getCameraMoveVariables(int &MoveDirCount,
									std::vector<glm::vec3> &vMoveDir)
		{
			MoveDirCount = mMoveDirCount;
			vMoveDir = mvMoveDir;
		}*/

		//Some tasks need to be processed for every frame
		//Such as the movement of cameras
		virtual void doFrameTasks()
		{
			std::vector<std::shared_ptr<Camera>> vpCamera =
				mpMonitorWindow->getAllCameras();
			std::shared_ptr<Camera> pDefaultCamera = mpMonitorWindow->getDefaultCamera();

			float frameCostTime = mpMonitorWindow->getFrameCostTime();

			//Do the Pre Rotation
			/*if (vpCamera.size() >= 2)
			{
				JoyStick3D &joystick1 = vpCamera[1]->getJoyStick3D();
				if (joystick1.getDoRotate())
				{
					glm::vec3 eye, center, up;
					vpCamera[1]->getCameraPose(eye, center, up);
					joystick1.executeTimeRotationViewSpace(eye, center, up,
														   frameCostTime);
					vpCamera[1]->setViewMatrix(eye, center, up);
				}
			}*/

			//Do camera movement
			if (mMoveDirCount > 0 && mpMonitorWindow.use_count() != 0)
			{
				glm::vec3 direction = glm::vec3(0.0f);
				for (size_t i = 0; i < mvMoveDir.size(); i++)
				{
					direction += mvMoveDir[i];
				}

				if (glm::dot(direction, direction) > 1e-1)
				{
					std::shared_ptr<Camera> pCurrentCamera = vpCamera[mMaximizeCameraIdx];
					std::shared_ptr<Camera> pFollowedCamera = vpCamera[mFollowedCameraIdx];
					std::shared_ptr<Camera> pReferCamera = pCurrentCamera;
						
					if (pDefaultCamera == pReferCamera &&
						pDefaultCamera != pFollowedCamera)
					{
						pReferCamera = pFollowedCamera;
					}

					glm::mat4 referVM = pReferCamera->getViewMatrix();

					if (mKeyState[GLFW_KEY_LEFT_ALT])
					{
						JoyStickGlobal::getInstance().accelerateAutoMiniSpeed(
							CAMERA_MOVE_ACSPEED);
					}
					else if(mKeyState[GLFW_KEY_LEFT_CONTROL])
					{
						JoyStickGlobal::getInstance().accelerateAutoMiniSpeed(
							CAMERA_MOVE_DESPEED);
					}

					/*glm::vec3 eye1, center1, up1;
					glm::vec3 eye2, center2, up2;
					vpCamera[0]->getCameraPose(eye1, center1, up1);
					vpCamera[1]->getCameraPose(eye2, center2, up2);
					glm::vec3 eyeDiff = eye2 - eye1;
					glm::vec3 upDiff = up2 - up1;
					std::cout << "before eyeDiff = " << eyeDiff.x << "," << eyeDiff.y << "," << eyeDiff.z << std::endl;
					std::cout << "before upDiff = " << upDiff.x << "," << upDiff.y << "," << upDiff.z << std::endl;*/

					direction = glm::transpose(glm::mat3(referVM))
						* direction;

					JoyStick3D &referJoy = pReferCamera->getJoyStick3D();
					referJoy.setJoyStickSpace(glm::mat4(1.0f));
					referJoy.setTranslateDir(direction);
					referJoy.setDoTranslate(true);

					if (mKeyState[GLFW_KEY_LEFT_SHIFT] ||
						mKeyState[GLFW_KEY_LEFT_ALT])
					{
						referJoy.accelerateTsVelocity(CAMERA_MOVE_ACSPEED);
					}
					else
					{
						referJoy.accelerateTsVelocity(CAMERA_MOVE_DESPEED);
					}

					pReferCamera->excuteJoyStick3DTranslate(frameCostTime);
					referJoy.setDoTranslate(false);

					//If the default camera was anchored, excute the follow
					if (pDefaultCamera != pFollowedCamera && 
						pFollowedCamera == pReferCamera)
					{
						//Move the default camera to the indicated position
						followCamera(pFollowedCamera, pDefaultCamera,
									 SPConfigure::getInstance().FollowedEye,
									 SPConfigure::getInstance().FollowedUp);
					}

					/*vpCamera[0]->getCameraPose(eye1, center1, up1);
					vpCamera[1]->getCameraPose(eye2, center2, up2);
					glm::vec3 eyeDiff_ = eye2 - eye1;
					glm::vec3 upDiff_ = up2 - up1;
					std::cout << "after eyeDiff = " << eyeDiff_.x << "," << eyeDiff_.y << "," << eyeDiff_.z << std::endl;
					std::cout << "after upDiff = " << upDiff_.x << "," << upDiff_.y << "," << upDiff_.z << std::endl;

					glm::vec3 eyeVary = eyeDiff_ - eyeDiff;
					glm::vec3 upVary = upDiff_ - upDiff;
					mAcEyeVary += eyeVary;
					mAcUpVary += upVary;
					std::cout << "eye vary = " << eyeVary.x << "," << eyeVary.y << "," << eyeVary.z << std::endl;
					std::cout << "up  vary = " << upVary.x << "," << upVary.y << "," << upVary.z << std::endl;
					std::cout << "mAcEyeVary = " << mAcEyeVary.x << "," << mAcEyeVary.y << "," << mAcEyeVary.z << std::endl;
					std::cout << "mAcUpVary  = " << mAcUpVary.x << "," << mAcUpVary.y << "," << mAcUpVary.z << std::endl;
					std::cout << std::endl;
					std::cout << std::endl;*/
					//std::cout << "direction = " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
					//std::cout << "mMoveDirCount = " << mMoveDirCount << std::endl;
				}
			}

			WinManipulator::doFrameTasks();
			return;
		}

	protected:
		std::shared_ptr<MonitorWindow> mpMonitorWindow;

		//For doing camera movement
		int mMoveDirCount;
		//key: W, S, A, D, E, Q
		std::vector<glm::vec3> mvMoveDir;

		//The index of camera which the default camera will follow
		int mFollowedCameraIdx;
		//The index of camera which will be set the major maximize screen
		int mMaximizeCameraIdx;

		glm::vec3 mAcEyeVary, mAcUpVary;

	protected:
		virtual void keyCallBackImpl(GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			switch (key)
			{
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

			case GLFW_KEY_SPACE:
				if (action == GLFW_PRESS && mKeyState[GLFW_KEY_LEFT_ALT])
				{
					//Retive the default camera canvas with the current maximized camera
					mpMonitorWindow->swapCanvasWithDefaultCamera(mMaximizeCameraIdx);
					mMaximizeCameraIdx = (mMaximizeCameraIdx + 1) % mpMonitorWindow->getNumCamera();
					//Maximize the mMaximizeCameraIdx camera
					mpMonitorWindow->swapCanvasWithDefaultCamera(mMaximizeCameraIdx);
					mpMonitorWindow->setFirstRunCamera(mMaximizeCameraIdx);
				}
				break;

			case GLFW_KEY_R:
				if (action == GLFW_PRESS)
				{
					std::vector<std::shared_ptr<Camera>> vpCamera =
						mpMonitorWindow->getAllCameras();

					std::shared_ptr<Camera> pDefaultCamera = 
						mpMonitorWindow->getDefaultCamera();

					if (mKeyState[GLFW_KEY_LEFT_CONTROL])
					{
						std::shared_ptr<Scene> pScene = mpMonitorWindow->getScene();

						if (pScene.use_count() != 0)
						{
							std::shared_ptr<Camera> pCurrentCamera = vpCamera[mMaximizeCameraIdx];
							std::shared_ptr<Camera> pFollowedCamera = vpCamera[mFollowedCameraIdx];
							std::shared_ptr<Camera> pReferCamera = pCurrentCamera;

							if (pDefaultCamera == pReferCamera &&
								pDefaultCamera != pFollowedCamera)
							{
								pReferCamera = pFollowedCamera;
							}

							std::vector<std::shared_ptr<Camera>> vAnchorCamera;
							if (pDefaultCamera != pFollowedCamera &&
								pFollowedCamera == pReferCamera)
							{
								vAnchorCamera.push_back(pDefaultCamera);
							}

							mpMonitorWindow->adjustCameraToScene(pReferCamera,
																 pScene, vAnchorCamera);
						}
					}
					
					//Uniform all camera rotation to the default camera
					/*if (mKeyState[GLFW_KEY_LEFT_SHIFT])
					{
						glm::mat4 viewMatrix0 = pDefaultCamera->getViewMatrix();
						mpMonitorWindow->unifyCamerasDirection(pDefaultCamera,
															   vpCamera);
					}*/

					//Reset the viewport of cameras
					mpMonitorWindow->resetCamerasViewport(vpCamera);
				}
				break;

			case GLFW_KEY_HOME:
				if (action == GLFW_PRESS)
				{
					std::vector<std::shared_ptr<Camera>> vpCamera =
						mpMonitorWindow->getAllCameras();

					std::shared_ptr<Camera> pDefaultCamera =
						mpMonitorWindow->getDefaultCamera();

					std::shared_ptr<Camera> pCurrentCamera = vpCamera[mMaximizeCameraIdx];
					std::shared_ptr<Camera> pFollowedCamera = vpCamera[mFollowedCameraIdx];
					std::shared_ptr<Camera> pReferCamera = pCurrentCamera;

					if (pDefaultCamera == pReferCamera &&
						pDefaultCamera != pFollowedCamera)
					{
						pReferCamera = pFollowedCamera;
					}

					glm::mat4 referVM = pReferCamera->getViewMatrix();

					glm::mat3 R0 = glm::mat3(referVM);
					glm::vec3 Ts0 = glm::vec3(referVM[3]);

					std::vector<std::shared_ptr<Camera>> vBundleCamera;
					vBundleCamera.push_back(pReferCamera);
					if (pDefaultCamera != pFollowedCamera &&
						pFollowedCamera == pReferCamera)
					{
						vBundleCamera.push_back(pDefaultCamera);
					}

					//Home the pReferCamera and adjust the follow default camera
					//if it follows
					for (size_t i = 0; i < vBundleCamera.size(); i++)
					{
						//vpCamera[i]->setViewport(0, 0, cw, ch);
						glm::vec3 eye, center, up;
						vBundleCamera[i]->getCameraPose(eye, center, up);
						eye = R0 * eye + Ts0;
						center = R0 * center + Ts0;
						up = R0 * up;
						vBundleCamera[i]->setViewMatrix(eye, center, up);
					}
				}
				break;

			case GLFW_KEY_F:
				if (action == GLFW_PRESS )
				{
					std::vector<std::shared_ptr<Camera>> vpCamera =
						mpMonitorWindow->getAllCameras();

					std::shared_ptr<Camera> pDefaultCamera =
						mpMonitorWindow->getDefaultCamera();

					if (mKeyState[GLFW_KEY_LEFT_ALT])
					{
						std::shared_ptr<Camera> pFollowedCamera = vpCamera[mFollowedCameraIdx];
						if (pFollowedCamera != pDefaultCamera)
						{
							//Change the camera shape color according to the configure
							std::shared_ptr<Mesh> pMesh = pFollowedCamera->getCameraShape();
							if (pMesh.use_count() != 0)
								pMesh->getMaterial()->setDiffuseColor(
									SPConfigure::getInstance().CameraColor);
						}

						mFollowedCameraIdx = (mFollowedCameraIdx + 1) % vpCamera.size();
						if (mKeyState[GLFW_KEY_LEFT_CONTROL]) mFollowedCameraIdx = 0;

						pFollowedCamera = vpCamera[mFollowedCameraIdx];
						if (pFollowedCamera != pDefaultCamera)
						{
							//Move the default camera to the indicated position
							followCamera(pFollowedCamera, pDefaultCamera,
										 SPConfigure::getInstance().FollowedEye,
										 SPConfigure::getInstance().FollowedUp);

							//Change the camera shape color according to the configure
							std::shared_ptr<Mesh> pMesh = pFollowedCamera->getCameraShape();
							if (pMesh.use_count() != 0)
								pMesh->getMaterial()->setDiffuseColor(
									SPConfigure::getInstance().FollowedCameraColor);
						}
					}
					else if(mKeyState[GLFW_KEY_LEFT_CONTROL])
					{
						std::shared_ptr<Camera> pCurrentCamera = vpCamera[mMaximizeCameraIdx];
						std::shared_ptr<Camera> pFollowedCamera = vpCamera[mFollowedCameraIdx];
						std::shared_ptr<Camera> pReferCamera = pCurrentCamera;

						if (pDefaultCamera == pReferCamera &&
							pDefaultCamera != pFollowedCamera)
						{
							pReferCamera = pFollowedCamera;
						}

						std::shared_ptr<Scene> pScene = mpMonitorWindow->getScene();
						BBox bbox = pScene->getTotalBBox();
						glm::vec3 minVertex = bbox.getMinVertex();
						glm::vec3 maxVertex = bbox.getMaxVertex();
						glm::vec3 sceneeCenter = (minVertex + maxVertex)*0.5f;

						float fovy, aspect, zNear, zFar;
						pReferCamera->getFrustum(fovy, aspect, zNear, zFar);

						glm::vec3 center(0.0f, 0.0f, 0.0f);
						glm::vec3 up(0.0f, 1.0f, 0.0f);
						float boxDepth = maxVertex.z - minVertex.z;
						float boxWidth = maxVertex.x - minVertex.x;
						float boxHeight = maxVertex.y - minVertex.y;
						float tanHalfFovy = tan(fovy*0.5f);
						float fy = boxDepth*0.5f + boxHeight*0.5f / tanHalfFovy;
						float fx = boxDepth*0.5f + boxWidth*0.5f / (tanHalfFovy*aspect);
						float f = std::max(fx, fy);

						glm::vec3 eye(0.0f, 0.0f, f);
						pReferCamera->setViewMatrix(eye, center, up);

						//If the default camera was anchored, excute the follow
						if (pDefaultCamera != pFollowedCamera &&
							pFollowedCamera == pReferCamera)
						{
							//Move the default camera to the indicated position
							followCamera(pFollowedCamera, pDefaultCamera,
										 SPConfigure::getInstance().FollowedEye,
										 SPConfigure::getInstance().FollowedUp);
						}
						
					}
				}
				break;
			/*case GLFW_KEY_RIGHT:
			{
				std::shared_ptr<Camera> pCameraMini = mpMonitorWindow->getCamera(1);
				int offsetX, offsetY;
				pCameraMini->getCanvasOffset(offsetX, offsetY);
				offsetX += 5;
				pCameraMini->setCanvasOffset(offsetX, offsetY);
			}
				break;
			case GLFW_KEY_LEFT:
			{
				std::shared_ptr<Camera> pCameraMini = mpMonitorWindow->getCamera(1);
				int offsetX, offsetY;
				pCameraMini->getCanvasOffset(offsetX, offsetY);
				offsetX -= 5;
				pCameraMini->setCanvasOffset(offsetX, offsetY);
			}
			break;
			case GLFW_KEY_UP:
			{
				std::shared_ptr<Camera> pCameraMini = mpMonitorWindow->getCamera(1);
				int offsetX, offsetY;
				pCameraMini->getCanvasOffset(offsetX, offsetY);
				offsetY += 5;
				pCameraMini->setCanvasOffset(offsetX, offsetY);
			}
			break;
			case GLFW_KEY_DOWN:
			{
				std::shared_ptr<Camera> pCameraMini = mpMonitorWindow->getCamera(1);
				int offsetX, offsetY;
				pCameraMini->getCanvasOffset(offsetX, offsetY);
				offsetY -= 5;
				pCameraMini->setCanvasOffset(offsetX, offsetY);
			}
			break;*/
			default:
				break;
			}

			//Do the camera travelling for all cameras' joysticks
			//If the rigid all cameras movement is not locked
			if (1)
			{
				glm::vec3 tmpDir(0.0f);
				int index = -1;
				switch (key)
				{
				case GLFW_KEY_W:
					index = 0;
					tmpDir = glm::vec3(0.0f, 0.0f, -1.0f);
					break;
				case GLFW_KEY_S:
					index = 1;
					tmpDir = glm::vec3(0.0f, 0.0f, 1.0f);
					break;
				case GLFW_KEY_A:
					index = 2;
					tmpDir = glm::vec3(-1.0f, 0.0f, 0.0f);
					break;
				case GLFW_KEY_D:
					index = 3;
					tmpDir = glm::vec3(1.0f, 0.0f, 0.0f);
					break;
				case GLFW_KEY_E:
					//In world space
					index = 4;
					tmpDir = glm::vec3(0.0f, 1.0f, 0.0f);
					tmpDir = glm::mat3(mpMonitorWindow->getDefaultCamera()->getViewMatrix())
						* tmpDir;
					break;
				case GLFW_KEY_Q:
					//In world space
					index = 5;
					tmpDir = glm::vec3(0.0f, -1.0f, 0.0f);
					tmpDir = glm::mat3(mpMonitorWindow->getDefaultCamera()->getViewMatrix())
						* tmpDir;
					break;
				default:
					break;
				}
				
				int PreMoveDirCount = mMoveDirCount;
				if (index >= 0 /*&& index < 6*/)
				{
					if (action == GLFW_PRESS)
					{
						mMoveDirCount++;
						mvMoveDir[index] = tmpDir;
					}
					else if (action == GLFW_RELEASE)
					{
						mMoveDirCount--;
						mvMoveDir[index] = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
			}

			//If the second camera rotation is not locked
			/*if (mpMonitorWindow->getNumCamera() >= 2)
			{
				std::shared_ptr<Camera> pCamera1 = mpMonitorWindow->getCamera(1);
				JoyStick3D &joystick1 = pCamera1->getJoyStick3D();
				glm::mat4 viewMatrix1 = pCamera1->getViewMatrix();
				int index = -1;
				glm::vec3 axis(0.0f);

				switch (key)
				{
				case GLFW_KEY_LEFT:
					index = 0;
					axis = glm::vec3(0.0f, -1.0f, 0.0f);
					
					joystick1.setJoyStickSpace(viewMatrix1);
					break;
				case GLFW_KEY_RIGHT:
					index = 1;
					axis = glm::vec3(0.0f, 1.0f, 0.0f);
					
					joystick1.setJoyStickSpace(viewMatrix1);
					break;
				case GLFW_KEY_UP:
					index = 2;
					axis = glm::vec3(1.0f, 0.0f, 0.0f);
					joystick1.setJoyStickSpace(viewMatrix1);
					break;
				case GLFW_KEY_DOWN:
					index = 3;
					axis = glm::vec3(-1.0f, 0.0f, 0.0f);
					joystick1.setJoyStickSpace(viewMatrix1);
					break;
				default:
					break;
				}

				if (index != -1)
				{
					if (action == GLFW_PRESS)
					{
						joystick1.setDoRotate(true);
						joystick1.setRotateAxis(axis);
					}
					else if(action == GLFW_RELEASE)
					{
						joystick1.setDoRotate(false);
					}
				}
			}*/
			
			WinManipulator::keyCallBackImpl(window, key, scancode, action, mods);
		}

		virtual void scrollCallBackImpl(GLFWwindow *window, double xoffset, double yoffset)
		{
			float scale = 1.0f;
			if (yoffset < -9) yoffset = -9;
			scale += (yoffset * 0.1);
			int vx, vy, vw, vh;

			std::shared_ptr<Camera> pCurrentCamera = 
				mpMonitorWindow->getCamera(mMaximizeCameraIdx);

			//pDefaultCamera = mpMonitorWindow->getCamera(1);

			//scale the viewport of camera
			pCurrentCamera->getViewport(vx, vy, vw, vh);

			vw *= scale;
			vh *= scale;

			int cx, cy, cw, ch;
			pCurrentCamera->getCanvas(cx, cy, cw, ch);

			double scaleXPos = mCursorPosX - cx;
			double scaleYPos = mpMonitorWindow->getWindowSize()[1] - mCursorPosY - cy;

			double vXPos = scaleXPos - vx;
			double vYPos = scaleYPos - vy;

			vx -= vXPos*(scale - 1.0f);
			vy -= vYPos*(scale - 1.0f);

			if(vw < MAX_VIEWPORT_SIZE && vh < MAX_VIEWPORT_SIZE)
				pCurrentCamera->setViewport(vx, vy, vw, vh);
		}

		virtual void mouseButtonCallBackImpl(GLFWwindow *window, int button, int action, int mods)
		{
			/*if (button == GLFW_MOUSE_BUTTON_LEFT &&
			action == GLFW_PRESS &&
			mMouseButtonState[button] == GLFW_RELEASE)
			{
			double x, y;
			glfwGetCursorPos(mpCam->getGLFWWinPtr(), &x, &y);
			//std::cout << "x = " << x << ", y = " << y <<  std::endl;
			glm::i32vec2 winSize = mpCam->getWindowSize();
			int bufferx = x, buffery = winSize.y - y;
			GLuint meshID = 0;
			meshID = mpCam->getPointMeshID(bufferx, buffery);

			if (mKeyState[GLFW_KEY_LEFT_CONTROL])
			{
			mpCam->addSelectedID(meshID);
			if (meshID > 0)
			{
			std::cout << "MeshID = " << meshID << std::endl;
			}
			}
			else if(mKeyState[GLFW_KEY_LEFT_SHIFT])
			{
			mpCam->deleteSelectedID(meshID);
			}
			else if(meshID == 0)
			{
			mpCam->setSelectedID(meshID);
			}

			}*/


			WinManipulator::mouseButtonCallBackImpl(window, button, action, mods);
		}

		virtual void cursorPosCallBackImpl(GLFWwindow *window, double xpos, double ypos)
		{
			double dx = xpos - mCursorPosX;
			double dy = ypos - mCursorPosY;

			if (mMouseButtonState[GLFW_MOUSE_BUTTON_RIGHT] && 
				(abs(dx) >= 1.0 || abs(dy) >= 1.0))
			{
				std::shared_ptr<Camera> pDefaultCamera =
					mpMonitorWindow->getDefaultCamera();

				std::shared_ptr<Camera> pCurrentCamera = 
					mpMonitorWindow->getCamera(mMaximizeCameraIdx);
				std::shared_ptr<Camera> pFollowedCamera = 
					mpMonitorWindow->getCamera(mFollowedCameraIdx);
				std::shared_ptr<Camera> pReferCamera = pCurrentCamera;

				if (pDefaultCamera == pReferCamera &&
					pDefaultCamera != pFollowedCamera)
				{
					pReferCamera = pFollowedCamera;
				}
				
				float fovy, aspect, zNear, zFar;
				pCurrentCamera->getFrustum(fovy, aspect, zNear, zFar);
				float tanHalfFovy = std::tan(fovy*0.5);
				float tanHalfFovx = tanHalfFovy * aspect;
				float fovx = std::atan(tanHalfFovx) * 2;

				int cx, cy, cw, ch;
				pCurrentCamera->getCanvas(cx, cy, cw, ch);

				float dxRad = fovx * dx / ch;
				float dyRad = fovx * dy / cw;

				JoyStick3D joystick;
				joystick.setDoRotate(true);
				float angle = 0.0f;

				glm::mat4 referVM = pReferCamera->getViewMatrix();

				if (abs(dx) > abs(dy))
				{
					glm::mat4 RInv = glm::mat4(glm::mat3(referVM));
					RInv = glm::transpose(RInv);
					glm::mat4 Twl = RInv*referVM;
					joystick.setJoyStickSpace(Twl);
					joystick.setRotateAxis(glm::vec3(0.0f, 1.0f, 0.0f));
					angle = -dxRad;
				}
				else
				{
					joystick.setJoyStickSpace(referVM);
					joystick.setRotateAxis(glm::vec3(1.0f, 0.0f, 0.0f));

					angle = -dyRad;
				}
				
				glm::vec3 eye, center, up;
				pReferCamera->getCameraPose(eye, center, up);

				joystick.executeRotation(eye, center, up, angle);
				pReferCamera->setViewMatrix(eye, center, up);

				//If the default camera was anchored, excute the follow
				if (pDefaultCamera != pFollowedCamera &&
					pFollowedCamera == pReferCamera)
				{
					followCamera(pFollowedCamera, pDefaultCamera,
								 SPConfigure::getInstance().FollowedEye,
								 SPConfigure::getInstance().FollowedUp);
				}
			}

			WinManipulator::cursorPosCallBackImpl(window, xpos, ypos);
		}

	protected:
		void followCamera(const std::shared_ptr<Camera> &pFollowedCamera,
						  const std::shared_ptr<Camera> &pCamera, 
						  const glm::vec3 &relativeEye,
						  const glm::vec3 &relativeUp)
		{
			glm::mat4 targetVM = pFollowedCamera->getViewMatrix();
			glm::mat3 targetRInv = glm::transpose(glm::mat3(targetVM));
			glm::vec3 targetTsInv = -targetRInv * glm::vec3(targetVM[3]);

			glm::vec3 eye = targetRInv * relativeEye + targetTsInv;
			glm::vec3 center = targetTsInv;
			glm::vec3 up = targetRInv * relativeUp;
			pCamera->setViewMatrix(eye, center, up);
		}
	};
}



