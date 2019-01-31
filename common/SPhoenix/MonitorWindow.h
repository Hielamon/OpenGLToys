#pragma once
#include "GLWindowBase.h"
#include "UICamera.h"
#include "OmniCamera.h"

namespace SP
{
	//The Monitor Window for showing the scenee by cameras
	class MonitorWindow : public GLWindowBase
	{
	public:
		MonitorWindow(const std::string &win_name = "Untitled",
					  int width = 0, int height = 0, bool visible = true)
			: GLWindowBase(win_name, width, height, visible), mFrameCostTime(0),
			mDefaultCameraIndex(0), mFirstRunCameraIdx(0), mbShowText(true)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LINE_SMOOTH);
			glLineWidth(3.0f);
			glPointSize(3.0f);
			glDepthFunc(GL_LEQUAL);
			//glEnable(GL_CULL_FACE);
			//glCullFace(GL_BACK);

			//For Opengl Transparent
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//For transfer buffers with any size
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);

			//Set up the UI Scene
			mpUIScene = std::make_shared<TwoDScene>();
			mpUIScene->uploadToDevice();

			mpUICamera = std::make_shared<UICamera>(mWidth, mHeight, 0, 0);
			mpUICamera->setup(mWidth, mHeight);

			mpTextScene = std::make_shared<TextScene>();
			mpTextScene->uploadToDevice();

			//We set the default Camera(same size with window) in mvpCamera[0],
			//The Scene will be transformed to fit the mvpCamera[0]
			std::shared_ptr<Camera> pCamera =
				std::make_shared<Camera>(mWidth, mHeight, 0, 0);

			//Add the default camera to the first element of the mvpCamera
			addCamera(pCamera);
		}

		~MonitorWindow() {}

		void setWindowVisible(bool bVisible)
		{
			if (bVisible)
			{
				glfwShowWindow(mGLFWWinPtr);
			}
			else
			{
				glfwHideWindow(mGLFWWinPtr);
			}
		}

		void setDoShowText(bool bShowText)
		{
			mbShowText = bShowText;
		}

		void setBackgroundColor(glm::vec4 color = glm::vec4(0.0f))
		{
			glClearColor(color.r, color.g, color.b, color.a);
		}

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
			mvpScene.push_back(pCamera->getAttachedScene());

			//Add camera border to the mpUIScene
			if(pCamera->getShowCanvas())
			{
				int cx, cy, cw, ch;
				pCamera->getCanvas(cx, cy, cw, ch);
				addRectangle(cx, cy, cw, ch, 1, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			}
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
				vpCamera[i]->setViewport(cx, cy, cw, ch);
				//vpCamera[i]->setViewport(0, 0, mWidth, mHeight);
			}
		}

		//Swap the default camera, we will swap the default camera with the 
		//input index camera and swap the canvas size, if the input index is valid
		void swapCanvasWithDefaultCamera(int index)
		{
			std::shared_ptr<Camera> pCamera = getCamera(index);
			if (pCamera.use_count() != 0)
			{
				//swap the canvas with the default camera
				int cx, cy, cw, ch;
				pCamera->getCanvas(cx, cy, cw, ch);

				std::shared_ptr<Camera> pDefaultCamera = getDefaultCamera();
				int cx_, cy_, cw_, ch_;
				pDefaultCamera->getCanvas(cx_, cy_, cw_, ch_);

				pCamera->setCanvas(cx_, cy_, cw_, ch_);
				pCamera->setViewport(cx_, cy_, cw_, ch_);
				//pCamera->setViewport(0, 0, mWidth, mHeight);

				pDefaultCamera->setCanvas(cx, cy, cw, ch);
				pDefaultCamera->setViewport(cx, cy, cw, ch);
				//pDefaultCamera->setViewport(0, 0, mWidth, mHeight);
			}
		}

		void setFirstRunCamera(int firstRunCameraIdx)
		{
			if (firstRunCameraIdx >= 0 && firstRunCameraIdx < mvpCamera.size())
				mFirstRunCameraIdx = firstRunCameraIdx;
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

		//Set the scenee to the MonitorWindow, meanwhile upload the scenee to device
		//This function all adjust(transform) the scenee to the world center and 
		//scale the scenee, meanwhile move the first camera in the monitor to the 
		//(1, 1, 1) corner of the moved scenee cube bounding box, keep the relative 
		//pose between the cameras synchronous
		void setScene(const std::shared_ptr<Scene> &pScene)
		{
			if (pScene->getNumMesh() == 0)return;
			pScene->uploadToDevice();

			if (mpScene.use_count() != 0)
			{
				mvpScene[0] = pScene;
			}
			else
			{
				mvpScene.insert(mvpScene.begin(), pScene);
			}

			mpScene = pScene;

			std::shared_ptr<Camera> pDefaultCamera = getDefaultCamera();
			adjustSceneToCamera(mpScene, pDefaultCamera);

			adjustCameraToScene(pDefaultCamera, mpScene);
		}

		std::shared_ptr<Scene> getScene()
		{
			return mpScene;
		}

		void setSkyBoxScene(const std::shared_ptr<Scene> &pSkyBoxScene)
		{
			pSkyBoxScene->uploadToDevice();
			mpSkyBoxScene = pSkyBoxScene;
			mvpScene.push_back(pSkyBoxScene);
		}

		/******************************Scene operation*****************************/
		/**************************************************************************/

		/**************************************************************************/
		/********************************UI operation*******************************/

		//bottom-left coordinate
		void addRectangle(int x, int y, int width, int height, int border = 1,
						  glm::vec4 color = glm::vec4(1.0f))
		{
			/*float wInv = 1.0f / mWidth, hInv = 1.0f / mHeight;
			float w_ = 2.0f * width * wInv, h_ = 2.0f * height * hInv;
			float x_ = 2.0f * x * wInv - 1.0f;
			float y_ = 2.0f * y * hInv - 1.0f;*/

			PrimitiveType pType = border > 0 ? PrimitiveType::LINES :
				PrimitiveType::TRIANGLES;

			std::vector<glm::vec3> vertices(4);
			std::vector<GLuint> indices;
			{
				/*vertices[0] = glm::vec3(x_, y_, -1.0f);
				vertices[1] = glm::vec3(x_ + w_, y_, -1.0f);
				vertices[2] = glm::vec3(x_ + w_, y_ + h_, -1.0f);
				vertices[3] = glm::vec3(x_, y_ + h_, -1.0f);*/
				vertices[0] = glm::vec3(x, y, -1.0f);
				vertices[1] = glm::vec3(x + width, y, -1.0f);
				vertices[2] = glm::vec3(x + width, y + height, -1.0f);
				vertices[3] = glm::vec3(x, y + height, -1.0f);

				if (border > 0)
				{
					indices =
					{
						0, 1,
						1, 2,
						2, 3,
						3, 0
					};
				}
				else
				{
					indices =
					{
						0, 1, 2,
						0, 2, 3
					};
				}
				
			}
			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vertices, indices, pType);
			pVA->addInstance();

			std::shared_ptr<Material> pMatrial = std::make_shared<Material>(color);
			std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA, pMatrial);
			assert(mpUIScene.use_count() != 0);
			mpUIScene->addMesh(pMesh);
		}

		void showText()
		{
			mpUICamera->roughRenderScene(mpTextScene);
		}

		void setText(std::string text, glm::vec2 originPt,
					 float scale = 1.0f, glm::vec4 color = glm::vec4(1.0f))
		{
			mpTextScene->setText(text, originPt, scale, color);
		}

		/********************************UI operation*******************************/
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
			std::stringstream ioStr;
			while (!isShutdown())
			{
				start_ = std::chrono::high_resolution_clock::now();

				runOnce();

				frameCount++;

				end = std::chrono::high_resolution_clock::now();
				cost = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
				mFrameCostTime = cost.count() * 1e-6;

				if (mFrameCostTime > 1000.0)
				{
					float tpf = mFrameCostTime / frameCount;
					float fps = frameCount * 1000.0 / mFrameCostTime;

					ioStr.str("");
					ioStr << "frame rate = " << std::fixed << std::setprecision(2)
						<<  fps << " fps ; frame time = " << tpf << " ms/frame";
						

					setText(ioStr.str(), glm::vec2(10.0f, 10.0f), 0.5f,
							glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));

					frameCount = 0;
					start = end;
				}

				cost = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_);
				mFrameCostTime = cost.count() * 1e-6;

				//std::cout << "frameID = " << frameID << std::endl;
				frameID++;
			}
		}

		virtual void runOnce()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			if (mpManipulator.use_count() != 0)
				mpManipulator->doFrameTasks();

			runCamera(mFirstRunCameraIdx);
			for (size_t i = 0; i < mvpCamera.size(); i++)
			{
				if(i != mFirstRunCameraIdx)
					runCamera(i);
			}

			if (mpUIScene->getNumMesh() > 0)
				mpUICamera->renderScene(mpUIScene);

			if (mbShowText) showText();

			glfwPollEvents();
			glfwSwapBuffers(mGLFWWinPtr);

		}

	protected:
		void adjustSceneToCamera(const std::shared_ptr<Scene> &pScene,
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

			BBox sceneeBBox = pScene->getTotalBBox();
			glm::vec3 minVertex = sceneeBBox.getMinVertex();
			glm::vec3 maxVertex = sceneeBBox.getMaxVertex();
			glm::vec3 sceneCenter = (minVertex + maxVertex)*0.5f;

			pCamera->adjustCameraPose(sceneeBBox);
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

		void runCamera(int cameraIdx)
		{
			std::shared_ptr<Mesh> &pCameraShape = mvpCamera[cameraIdx]->getCameraShape();
			if (pCameraShape.use_count() != 0)
			{
				pCameraShape->setAccessible(false);
				mvpCamera[cameraIdx]->renderSceneArray(mvpScene);
				pCameraShape->setAccessible(true);
			}
			else
			{
				mvpCamera[cameraIdx]->renderSceneArray(mvpScene);
			}
			
		}

	protected:
		std::shared_ptr<Scene> mpScene;
		std::shared_ptr<Scene> mpSkyBoxScene;
		std::vector<std::shared_ptr<Scene>> mvpScene;

		std::shared_ptr<Scene> mpUIScene;
		std::shared_ptr<UICamera> mpUICamera;

		std::shared_ptr<TextScene> mpTextScene;

		//The first element mvpCamera[0] is the default camera
		//which has the same canvas size with the window, and must be
		//set in the constructor of this class or inherited class
		std::vector<std::shared_ptr<Camera>> mvpCamera;

	protected:
		//Cost time for the current frame, in millisecond
		float mFrameCostTime;

		//The index of the default camera in the mvpCamera vector
		int mDefaultCameraIndex;
		int mFirstRunCameraIdx;

		bool mbShowText;
	};
}
