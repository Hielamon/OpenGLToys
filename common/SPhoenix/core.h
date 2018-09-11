#pragma once

#include "Utils.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "ManipulatorBase.h"


namespace SP
{
	class GlfwConfigure
	{
	public:
		static void Setup()
		{
			static GlfwConfigure glfwConf;
		}

	private:

		GlfwConfigure()
		{
			if (!glfwInit())
			{
				SP_CERR("Failed to Init the glfw");
				exit(-1);
			}
			else
			{
				SP_LOG("Succeed to Init the glfw library");
			}

			//atexit(glfwTerminate);
		}

		~GlfwConfigure()
		{
			SP_LOG("Terminate the glfw library");
			glfwTerminate();
		}
	};

	/**OpenGL window base*/
	class GLWindowBase
	{
	public:
		GLWindowBase(const std::string &win_name = "Untitled", int width = 0, int height = 0)
		{
			GlfwConfigure::Setup();

			glm::u32vec2 screenSize = GetScreenResolution();
			assert(screenSize[0] != -1);
			mwidth = width == 0 ? screenSize[0] * 0.5 : width;
			mheight = height == 0 ? screenSize[1] * 0.5 : height;
			mwinName = win_name;

			mglfwWinPtr = _initWindow(mwidth, mheight, mwinName);
			assert(mglfwWinPtr != nullptr);

			mopened = GL_TRUE;
		}

		~GLWindowBase()
		{
			if (mopened && mglfwWinPtr != nullptr)
			{
				glfwDestroyWindow(mglfwWinPtr);
				mopened = false;
			}
		}

		GLFWwindow *getGLFWWinPtr()
		{
			return mglfwWinPtr;
		}

		glm::i32vec2 getWindowSize()
		{
			return glm::i32vec2(mwidth, mheight);
		}

		bool isOpened()
		{
			return mopened;
		}

		virtual bool isShutdown()
		{
			return glfwWindowShouldClose(mglfwWinPtr);
		}

		bool run()
		{
			while (!isShutdown())
			{
				runOnce();

				glfwPollEvents();
				glfwSwapBuffers(mglfwWinPtr);
			}

			return GL_TRUE;
		}

		virtual void runOnce() = 0;
	protected:
		/**GLFW window pointer*/
		GLFWwindow * mglfwWinPtr;

		/**window size*/
		int mwidth, mheight;

		/**windows name*/
		std::string mwinName;

		/**indicate the window state, TODO : maybe change to a enum*/
		bool mopened;

	private:
		GLFWwindow * _initWindow(int &width, int &height, std::string &winName)
		{
			{
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				//glfwWindowHint(GLFW_SAMPLES, mNumSamples);
				glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
			}

			GLFWwindow *window = glfwCreateWindow(width, height, winName.c_str(), NULL, NULL);
			if (window == nullptr)
			{
				SP_CERR("Failed to CreateWindow ect...");
				exit(-1);
			}

			glfwMakeContextCurrent(window);

			glewExperimental = GL_TRUE;
			if (glewInit() != GLEW_OK)
			{
				SP_CERR("Failed to initialize GLEW");
				glfwDestroyWindow(window);
				exit(-1);
			}

			return window;
		}
	};

	/**Camera class : execute the rendering loop*/
	class Camera : public GLWindowBase
	{
	public:
		Camera(int width, int height, const std::string &camName = "Untitled")
			: GLWindowBase(camName, width, height), mbShowIDColor(false)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_DEPTH_TEST);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			//Using uniform buffers
			glGenBuffers(1, &mViewUBO);
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferData(GL_UNIFORM_BUFFER, 144, NULL, GL_DYNAMIC_DRAW);
			glGenBuffers(GL_UNIFORM_BUFFER, 0);
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);

			//Set the default projection matrix and view matrix
			setProjectionMatrix();
			setViewMatrix();
		}

		~Camera()
		{
			glDeleteBuffers(1, &mViewUBO);
		}


		void setProjectionMatrix(float fovy = 60.f, float aspect = 0.0f, float zNear = 0.01, float zFar = 100.f)
		{
			mfovy = fovy; mzNear = zNear; mzFar = zFar;
			maspect = aspect == 0.0f ? mwidth / float(mheight) : aspect;

			mprojectionMatrix = glm::perspective(glm::radians(mfovy), maspect, mzNear, mzFar);
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(mprojectionMatrix));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void setViewMatrix(glm::vec3 &eye = glm::vec3(0.0f, 0.0f, 0.0f),
						   glm::vec3 &center = glm::vec3(0.0f, 0.0f, -1.0f),
						   glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f))
		{
			meye = eye; mcenter = center; mup = up;

			mviewMatrix = glm::lookAt(meye, mcenter, mup);
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(mviewMatrix));
			glBufferSubData(GL_UNIFORM_BUFFER, 128, 12, glm::value_ptr(meye));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void setManipulator(std::shared_ptr<ManipulatorBase> pManipulator)
		{
			mpManipulator = pManipulator;
			mpManipulator->registerCallBacks();
		}

		/**Add a Scene to a Camera's SceneUtil pointer vector, and return the SceneUtil ID*/
		void setScene(std::shared_ptr<Scene> &pScene)
		{
			mpScene = pScene;
			pScene->uploadToDevice();

			//Update the model matrix according the scene bounding box
			BBox sceneBBox = pScene->getTotalBBox();
			glm::vec3 minVertex = sceneBBox.getMinVertex();
			glm::vec3 maxVertex = sceneBBox.getMaxVertex();
			glm::vec3 sceneCenter = (minVertex + maxVertex)*0.5f;
			glm::mat4 mt;
			mt = glm::translate(mt, -sceneCenter);


			glm::vec3 center(0.0f, 0.0f, 0.0f);
			glm::vec3 up(0.0f, 1.0f, 0.0f);
			float boxDepth = maxVertex.z - minVertex.z;
			float boxWidth = maxVertex.x - minVertex.x;
			float boxHeight = maxVertex.y - minVertex.y;
			float tanHalfFovy = tan(glm::radians(mfovy)*0.5f);
			float fy = boxDepth*0.5f + boxHeight*0.5f / tanHalfFovy;
			float fx = boxDepth*0.5f + boxWidth*0.5f / (tanHalfFovy*maspect);
			float f = std::max(fx, fy);

			float scale = mzFar / (f * 10);
			f *= scale;
			glm::mat4 ms;
			ms = glm::scale(ms, glm::vec3(scale, scale, scale));

			//TODO: we need to traverse all instance and set
			//      new model matrix attributes
			//pScene->setTopModelMatrix(ms*mt);

			glm::vec3 eye(0.0f, 0.0f, f);
			setViewMatrix(eye, center, up);
		}

		void revertShowIDColor()
		{
			mbShowIDColor = !mbShowIDColor;
		}

		virtual void addMeshToScene(const std::shared_ptr<Mesh>& pMesh,
									const std::shared_ptr<ShaderProgram> 
									&pShaderProgramTmp = nullptr)
		{
			if (mpScene.use_count() != 0)
			{
				mpScene->addMesh(pMesh, pShaderProgramTmp);
				//We can check the size of meshes in mpColorScene
				//If which is not equal to the size of meshes in mpScene
				//We will set the mbRefreshColorScene
				//mbRefreshColorScene = true;
			}
		}

		//mfovy is the angle FOV in y direction
		float mfovy, maspect, mzNear, mzFar;
		glm::vec3 meye, mcenter, mup;


	protected:
		Camera() {}

		virtual void runOnce()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if (mpScene.use_count() == 0) return;

			if (mbShowIDColor)
			{
				if (mpColorScene.use_count() == 0 || 
					mpColorScene->getNumMesh() != mpScene->getNumMesh()) 
					generateColorScene();

				mpColorScene->draw();
			}
			else
			{
				mpScene->draw();
			}
		}

		//Generate the mpColorSceneUtil, According the exsited mpSceneUtil
		void generateColorScene()
		{
			mpColorScene = std::make_shared<SceneColorID>(mpScene);
			mpColorScene->uploadToDevice();
		}

	protected:
		std::shared_ptr<Scene> mpScene;

		//mpColorSceneUtil is used to showing the id colored scene
		std::shared_ptr<SceneColorID> mpColorScene;
		bool mbShowIDColor;

		std::shared_ptr<ManipulatorBase> mpManipulator;

		glm::mat4 mviewMatrix;
		glm::mat4 mprojectionMatrix;
		GLuint mViewUBO;
	};

	/**Camera class with FBO technique*/
	class CameraFBO : public Camera
	{
	public:
		CameraFBO(int width, int height, const std::string &camName = "Untitled")
			: Camera(width, height, camName)
		{
			glDepthFunc(GL_LEQUAL);

			//Create MultiSample FBO
			{
				glGenFramebuffers(1, &mMSFBO);
				glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

				//Create and attach the color, depth 
				//and extra buffers Color buffers

				//MSAA for anti-aliasing
				glGenTextures(1, &mColorTexture);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mColorTexture);
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_RGB,
										mwidth, mheight, GL_TRUE);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
									   GL_TEXTURE_2D_MULTISAMPLE, mColorTexture, 0);

				glGenTextures(1, &mMSMeshIDTexture);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMSMeshIDTexture);
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_R32UI,
										mwidth, mheight, GL_TRUE);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
									   GL_TEXTURE_2D_MULTISAMPLE, mMSMeshIDTexture, 0);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

				GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
				glDrawBuffers(2, buffers);

				//Depth and stencil renderbuffer
				glGenRenderbuffers(1, &mMSDepthStencilRBO);
				glBindRenderbuffer(GL_RENDERBUFFER, mMSDepthStencilRBO);
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, mNumSamples,
												 GL_DEPTH24_STENCIL8, mwidth, mheight);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
										  GL_RENDERBUFFER, mMSDepthStencilRBO);

				glBindRenderbuffer(GL_RENDERBUFFER, 0);

				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				{
					SP_CERR("The MultSamples FrameBuffer is not complete");
					exit(-1);
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			//Create RAW FBO
			{
				glGenFramebuffers(1, &mFBO);
				glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

				glGenTextures(1, &mMeshIDTexture);
				glBindTexture(GL_TEXTURE_2D, mMeshIDTexture);
				/*glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, mwidth, mheight, 0, GL_R,
				GL_UNSIGNED_INT, NULL);*/
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, mwidth, mheight, 0,
							 GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
				/*GLenum error = glGetError();
				if (error == GL_NO_ERROR)
				{
				std::cout << " ERROR :  GL_NO_ERROR" << std::endl;
				}*/
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
									   GL_TEXTURE_2D, mMeshIDTexture, 0);

				GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, buffers);

				//Depth and stencil renderbuffer
				/*glGenRenderbuffers(1, &mDepthStencilRBO);
				glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRBO);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mwidth, mheight);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
				GL_RENDERBUFFER, mDepthStencilRBO);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);*/

				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != 
					GL_FRAMEBUFFER_COMPLETE)
				{
					SP_CERR("The Raw FrameBuffer is not complete");
					exit(-1);
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}

		GLuint getPointMeshID(GLint x, GLint y)
		{
			//Blit the MultiSample MeshID to Sigle Sample MeshID
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBlitFramebuffer(x, y, x + 1, y + 1, x, y, x + 1, y + 1,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			GLuint meshID;
			glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &meshID);
			//GL_DEBUG_ALL;
			return meshID;
		}

		void setSelectedID(const GLuint &meshID)
		{
			mvSelectedMeshID.clear();
			if (meshID > 0) addSelectedID(meshID);
		}

		void addSelectedID(const GLuint &meshID)
		{
			if (meshID > 0) mvSelectedMeshID.push_back(meshID);
		}

		void deleteSelectedID(const GLuint &meshID)
		{
			std::list<GLuint>::iterator diter = std::find(mvSelectedMeshID.begin(),
														  mvSelectedMeshID.end(), meshID);
			if (diter != mvSelectedMeshID.end())
			{
				mvSelectedMeshID.erase(diter);
			}
		}

		~CameraFBO()
		{
			glDeleteFramebuffers(1, &mFBO);
		}

	protected:
		virtual void runOnce()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);
			Camera::runOnce();


			if (mpSceneSelected.use_count() == 0 ||
				mpSceneSelected->getNumMesh() != mpScene->getNumMesh())
				generateSelectedScene();

			if (mvSelectedMeshID.size() > 0)
			{
				mpSceneSelected->drawByMeshIDs(mvSelectedMeshID);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Copy the color buffer from mFBO to the default framebuffer
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_BACK_LEFT);

			glBlitFramebuffer(0, 0, mwidth, mheight, 0, 0, mwidth, mheight,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);

		}

		//Generate the mpSelectedSceneUtil, According the exsited mpSceneUtil
		void generateSelectedScene()
		{
			mpSceneSelected = std::make_shared<SceneSelected>(mpScene);
			mpSceneSelected->uploadToDevice();
		}

	protected:
		//The multisample FBO mMSFBO for anti-aliasing,
		//the raw FBO mFBO acting as the temporary FBO
		GLuint mMSFBO, mFBO;

		GLuint mColorTexture, mMSMeshIDTexture, mMeshIDTexture;
		GLuint mMSDepthStencilRBO, mDepthStencilRBO;

		/**Samples for Super sample Anti-aliasing*/
		static const int mNumSamples = 8;

		//mpSelectedSceneUtil is used to showing the selected mesh
		std::shared_ptr<SceneSelected> mpSceneSelected;
		std::list<GLuint> mvSelectedMeshID;
	};

}
