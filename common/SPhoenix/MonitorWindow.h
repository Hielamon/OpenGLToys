#pragma once
#include "Core.h"

namespace SP
{
	//The Monitor Window for showing the scene by cameras
	class MonitorWindow : public GLWindowBase
	{
	public:
		MonitorWindow(const std::string &win_name = "Untitled",
					  int width = 0, int height = 0)
			: GLWindowBase(win_name, width, height), mFrameCostTime(0),
			mDefaultCameraIndex(0)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			//We set the default Camera(same size with window) in mvpCamera[0],
			//The Scene will be transformed to fit the mvpCamera[0]
			std::shared_ptr<CameraMini> pCamera =
				std::make_shared<CameraMini>(mWidth, mHeight, 0, 0);

			//Add the default camera to the first element of the mvpCamera
			addCamera(pCamera);
		}

		~MonitorWindow() {}

		/**************************************************************************/
		/*****************************Camera operation*****************************/
		//Add the camera to the MonitorWindow, meanwhile setup the camera
		void addCamera(const std::shared_ptr<Camera> &pCamera)
		{
			if (pCamera.use_count() == 0)
			{
				SP_CERR("The current camera added to the MonitorWindow is empty");
				return;
			}

			pCamera->setup(mWidth, mHeight);
			mvpCamera.push_back(pCamera);
		}

		std::shared_ptr<Camera> getCamera(int index)
		{
			if (index < 0 || index >= mvpCamera.size())
			{
				SP_CERR("The index for getting camera is not valid");
				return nullptr;
			}

			return mvpCamera[index];
		}

		std::shared_ptr<Camera> getDefaultCamera()
		{
			return getCamera(mDefaultCameraIndex);
		}

		std::vector<std::shared_ptr<Camera>> getAllCameras()
		{
			return mvpCamera;
		}

		int getNumCamera()
		{
			return mvpCamera.size();
		}

		//Adjust the keyCamera pose to the (1,1,1) corner of pScene's cube bounding box
		//And you can rigidly set the synchronous rigidly adjustment to the vpCamera
		void adjustCameraToScene(const std::shared_ptr<Camera> &pKeyCamera,
								 const std::shared_ptr<Scene> &pScene,
								 const std::vector<std::shared_ptr<Camera>> &vpCamera =
								 std::vector<std::shared_ptr<Camera>>())
		{
			if (pScene.use_count() == 0 || pKeyCamera.use_count() == 0)
			{
				SP_CERR("The pScene or pKeyCamera is empty");
				return;
			}

			BBox bbox = pScene->getTotalBBox();
			glm::mat4 keyVMatrix = pKeyCamera->getViewMatrix();
			pKeyCamera->adjustCameraPose(bbox);
			glm::mat4 dT = glm::inverse(pKeyCamera->getViewMatrix()) * keyVMatrix;

			glm::mat3 dR = glm::mat3(dT);
			glm::vec3 dTs = glm::vec3(dT[3]);

			//Rigidly adjust the pose of vpCamera, except for the keyCamera
			for (size_t i = 0; i < vpCamera.size(); i++)
			{
				if (vpCamera[i] == pKeyCamera) continue;

				glm::vec3 eye, center, up;
				vpCamera[i]->getCameraPose(eye, center, up);

				eye = dR * eye + dTs;
				center = dR * center + dTs;
				up = dR * up;

				vpCamera[i]->setViewMatrix(eye, center, up);
			}
		}

		//Reset the viewport of cameras
		void resetCamerasViewport(const std::vector<std::shared_ptr<Camera>> &vpCamera)
		{
			for (size_t i = 0; i < vpCamera.size(); i++)
			{
				int cx, cy, cw, ch;
				vpCamera[i]->getCanvas(cx, cy, cw, ch);
				vpCamera[i]->setViewport(0, 0, cw, ch);
			}
		}

		//Swap the default camera, we will swap the default camera with the 
		//input index camera and swap the canvas size, if the input index is valid
		void swapWithDefaultCamera(int index)
		{
			std::shared_ptr<Camera> pCamera = getCamera(index);
			if (pCamera.use_count() != 0)
			{
				//swap the canvas with the default camera
				int cx, cy, cw, ch;
				pCamera->getCanvas(cx, cy, cw, ch);
				/*pCamera->getCanvasOffset();
				pCamera->getCanvasSize();*/

				std::shared_ptr<Camera> pDefaultCamera = getDefaultCamera();
				int cx_, cy_, cw_, ch_;
				pDefaultCamera->getCanvas(cx_, cy_, cw_, ch_);
				/*pDefaultCamera->getCanvasOffset();
				pDefaultCamera->getCanvasSize();*/

				pCamera->setCanvas(cx_, cy_, cw_, ch_);
				/*pCamera->setCanvasOffset();
				pCamera->setCanvasSize();*/
				pCamera->setViewport(0, 0, cw_, ch_);

				pDefaultCamera->setCanvas(cx, cy, cw, ch);
				/*pDefaultCamera->setCanvasOffset();
				pDefaultCamera->setCanvasSize();*/
				pDefaultCamera->setViewport(0, 0, cw, ch);

				//swap the pointer in the mvpCamera
				mvpCamera[index] = pDefaultCamera;
				mvpCamera[mDefaultCameraIndex] = pCamera;
			}
		}

		//Uniform cameras rotation to the  keyCamera
		void unifyCamerasDirection(const std::shared_ptr<Camera> &pKeyCamera,
								   const std::vector<std::shared_ptr<Camera>> &vpCamera)
		{
			if (pKeyCamera.use_count() == 0 || pKeyCamera.use_count() == 0)
			{
				SP_CERR("The pScene or pKeyCamera is empty");
				return;
			}

			glm::mat4 viewMatrix0 = pKeyCamera->getViewMatrix();
			glm::mat3 R0 = glm::mat3(viewMatrix0);

			for (size_t i = 1; i < vpCamera.size(); i++)
			{
				if (vpCamera[i] == pKeyCamera) continue;

				JoyStick3D joysticki;
				glm::mat4 viewMatrixi = vpCamera[i]->getViewMatrix();
				glm::mat3 Ri = glm::mat3(viewMatrixi);
				glm::mat3 Ri0 = Ri * glm::transpose(R0);

				joysticki.setJoyStickSpace(viewMatrixi);
				glm::vec3 eye, center, up;
				vpCamera[i]->getCameraPose(eye, center, up);
				joysticki.executeRotation(eye, center, up, Ri0);
				vpCamera[i]->setViewMatrix(eye, center, up);
			}
		}

		/*****************************Camera operation*****************************/
		/**************************************************************************/


		/**************************************************************************/
		/******************************Scene operation*****************************/

		//Set the scene to the MonitorWindow, meanwhile upload the scene to device
		//This function all adjust(transform) the scene to the world center and 
		//scale the scene, meanwhile move the first camera in the monitor to the 
		//(1, 1, 1) corner of the moved scene cube bounding box, keep the relative 
		//pose between the cameras synchronous
		void setScene(const std::shared_ptr<Scene> &pScene)
		{
			pScene->uploadToDevice();
			mpScene = pScene;

			std::shared_ptr<Camera> defaultCamera = getDefaultCamera();
			_adjustSceneToCamera(mpScene, defaultCamera);

			adjustCameraToScene(defaultCamera, mpScene, mvpCamera);
		}

		std::shared_ptr<Scene> getScene()
		{
			return mpScene;
		}

		void setSkyBoxScene(const std::shared_ptr<Scene> &pSkyBoxScene)
		{
			pSkyBoxScene->uploadToDevice();
			mpSkyBoxScene = pSkyBoxScene;
		}

		/******************************Scene operation*****************************/
		/**************************************************************************/

		float getFrameCostTime()
		{
			return mFrameCostTime;
		}

		void run()
		{
			std::chrono::time_point<std::chrono::steady_clock> start, start_, end;
			std::chrono::nanoseconds cost;
			std::vector<float> vFPS;
			int frameCount = 0, frameID = 0;
			start = std::chrono::high_resolution_clock::now();
			std::cout << std::endl;
			while (!isShutdown())
			{
				start_ = std::chrono::high_resolution_clock::now();

				runOnce();

				glfwPollEvents();
				glfwSwapBuffers(mGLFWWinPtr);

				frameCount++;

				end = std::chrono::high_resolution_clock::now();
				cost = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
				mFrameCostTime = cost.count() * 1e-6;

				if (mFrameCostTime > 1000.0)
				{
					float tpf = mFrameCostTime / frameCount;
					float fps = frameCount * 1000.0 / mFrameCostTime;
					std::cout << "\rframe rate = " << fps <<
						"fps ; frame time = " << tpf << " ms/frame" << std::flush;
					frameCount = 0;
					start = end;
				}

				cost = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_);
				mFrameCostTime = cost.count() * 1e-6;

				//std::cout << "frameID = " << frameID << std::endl;
				frameID++;
			}
			std::cout << std::endl;
		}

		virtual void runOnce()
		{
			if (mpManipulator.use_count() != 0)
				mpManipulator->doFrameTasks();

			std::vector<std::shared_ptr<Scene>> vpScene;
			if (mpScene.use_count() != 0) vpScene.push_back(mpScene);

			for (size_t i = 0; i < mvpCamera.size(); i++)
			{
				std::shared_ptr<Scene> pCameraShapeScene =
					mvpCamera[i]->getCameraShapeScene();

				if (pCameraShapeScene.use_count() != 0)
					vpScene.push_back(pCameraShapeScene);
			}

			if (mpSkyBoxScene.use_count() != 0) vpScene.push_back(mpSkyBoxScene);

			for (size_t i = 0; i < mvpCamera.size(); i++)
			{
				mvpCamera[i]->renderOneFrame(vpScene);
				//mvpCamera[i]->renderOneFrame(mpScene);
			}
		}

	protected:
		std::shared_ptr<Scene> mpScene;
		std::shared_ptr<Scene> mpSkyBoxScene;

		//The first element mvpCamera[0] is the default camera
		//which has the same canvas size with the window, and must be
		//set in the constructor of this class or inherited class
		std::vector<std::shared_ptr<Camera>> mvpCamera;

	private:
		//Cost time for the current frame, in millisecond
		float mFrameCostTime;

		//The index of the default camera in the mvpCamera vector
		int mDefaultCameraIndex;

	private:
		void _adjustSceneToCamera(const std::shared_ptr<Scene> &pScene,
								  const std::shared_ptr<Camera> &pCamera)
		{
			if (pScene.use_count() == 0 || pCamera.use_count() == 0)
			{
				SP_CERR("Invalid to adjust for empty pScene or empty pCamera");
				return;
			}

			/*int p[2] = { 0 };
			glGetIntegerv(GL_MAX_VIEWPORT_DIMS, p);
			std::cout << "max width = " << p[0] << ";" << "max height = " << p[1] << std::endl;*/

			BBox sceneBBox = pScene->getTotalBBox();
			glm::vec3 minVertex = sceneBBox.getMinVertex();
			glm::vec3 maxVertex = sceneBBox.getMaxVertex();
			glm::vec3 sceneCenter = (minVertex + maxVertex)*0.5f;

			pCamera->adjustCameraPose(sceneBBox);
			glm::vec3 eye, center, up;
			pCamera->getCameraPose(eye, center, up);
			glm::vec3 halfDiagonal = minVertex - sceneCenter;
			glm::vec3 viewAxis = eye - sceneCenter;
			float depthRange = std::sqrt(glm::dot(viewAxis, viewAxis)) +
				std::sqrt(glm::dot(halfDiagonal, halfDiagonal));

			glm::mat4 mt;
			mt = glm::translate(mt, -sceneCenter);

			float fovy, aspect, zNear, zFar;
			pCamera->getFrustum(fovy, aspect, zNear, zFar);

			float scale = zFar * 0.5 / depthRange;
			glm::mat4 ms;
			ms = glm::scale(ms, glm::vec3(scale, scale, scale));
			pScene->transformMesh(ms*mt);
		}

		//Move the cameras in the default camera corrdinate
		/*void _excuteCamerasMove(float millisecond)
		{
			glm::mat4 viewMatrix0 = mpMonitorWindow->getDefaultCamera()->getViewMatrix();
			for (size_t i = 0; i < mvpCamera.size(); i++)
			{
				JoyStick3D &joystick = mvpCamera[i]->getJoyStick3D();
				if (!joystick.getDoRotate() && !joystick.getDoTranslate())
					continue;
				mvpCamera[i]->excuteJoyStick3D(millisecond);
			}
		}*/
	};
}
