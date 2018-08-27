#pragma once

#include "utils.h"
#include "Shader.h"
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
	//extern GlfwConfigure _glfwconfig;

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
			mheight = height == 0 ? screenSize[1] *0.5 : height;
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
				glfwWindowHint(GLFW_SAMPLES, 8);
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
			//glEnable(GL_MULTISAMPLE);
			glEnable(GL_DEPTH_TEST);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			//Using uniform buffers
			glGenBuffers(1, &mViewUBO);
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferData(GL_UNIFORM_BUFFER, 144, NULL, GL_DYNAMIC_DRAW);
			glGenBuffers(GL_UNIFORM_BUFFER, 0);
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);
			
			//Using the render buffer object to get the clicked meshID
			/*glGenRenderbuffers(1, &mMeshIDRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, mMeshIDRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_R32UI, mwidth, mheight);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, mMeshIDRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);*/

			/*GLenum buffers[1] = { GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(1, buffers);*/
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				std::cout << "The colors buffers is not valid" << std::endl;
			}

			//Set the default projection matrix and view matrix
			setProjectionMatrix();
			setViewMatrix();
		}

		~Camera(){}


		void setProjectionMatrix(float fovy = 45.f, float aspect = 0.0f, float zNear = 0.01, float zFar = 100.f)
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
			mpSceneUtil = std::make_shared<SceneUtil>(pScene);
			pScene->setSceneUtil(mpSceneUtil);

			//Update the model matrix according the scene bounding box
			BBox sceneBBox = pScene->getBoundingBox();
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

			float scale = mzFar / (f*10);
			f *= scale;
			glm::mat4 ms;
			ms = glm::scale(ms, glm::vec3(scale, scale, scale));
			pScene->setTopModelMatrix(ms*mt);

			glm::vec3 eye(0.0f, 0.0f, f);
			setViewMatrix(eye, center, up);
		}

		void addMeshToScene(const std::shared_ptr<Mesh>& pMesh,
					 const std::shared_ptr<ShaderCodes> &pShaderCodes = nullptr)
		{
			if (mpSceneUtil.use_count() != 0)
			{
				std::shared_ptr<MeshUtil> pMeshUtil = std::make_shared<MeshUtil>(pMesh);
				pMesh->setMeshUtil(pMeshUtil);

				mpSceneUtil->addMeshUtil(pMeshUtil, pShaderCodes);

				if (mpColorSceneUtil.use_count() != 0)
				{
					std::shared_ptr<MeshColorIDUtil> pMeshColorIDUtil =
						std::make_shared<MeshColorIDUtil>(pMeshUtil);
					mpColorSceneUtil->addMeshUtil(
						std::static_pointer_cast<MeshUtil>(pMeshColorIDUtil),
						mpColorIDShader);
				}
			}
		}

		//mfovy is the angle FOV in y direction
		float mfovy, maspect, mzNear, mzFar;
		glm::vec3 meye, mcenter, mup;
		bool mbShowIDColor;

	protected:
		virtual void runOnce()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if (mpSceneUtil.use_count() == 0) return;

			if (mbShowIDColor)
			{
				if (mpColorSceneUtil.use_count() == 0)
				{
					//Initialize the mpColorSceneUtil;
					std::string __currentPATH = __FILE__;
					__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
					mpColorIDShader = std::make_shared<ShaderCodes>(
						__currentPATH + "/Shaders/SPhoenixScene-MeshIDColor.vert",
						__currentPATH + "/Shaders/SPhoenixScene-MeshIDColor.frag");

					mpColorSceneUtil = std::make_shared<SceneUtil>(*mpSceneUtil);
					mpColorSceneUtil->reset();
					std::vector<std::shared_ptr<MeshUtil>> vExistedMesh = mpSceneUtil->getMeshUtils();
					for (size_t i = 0; i < vExistedMesh.size(); i++)
					{
						std::shared_ptr<MeshColorIDUtil> pMeshColorIDUtil =
							std::make_shared<MeshColorIDUtil>(vExistedMesh[i]);
						mpColorSceneUtil->addMeshUtil(
							std::static_pointer_cast<MeshUtil>(pMeshColorIDUtil),
							mpColorIDShader);
					}
				}

				mpColorSceneUtil->draw();
			}
			else
			{
				mpSceneUtil->draw();
			}
		}

	private:
		std::shared_ptr<SceneUtil> mpSceneUtil;

		//mpColorSceneUtil is used to showing the id colored scene
		std::shared_ptr<SceneUtil> mpColorSceneUtil;
		std::shared_ptr<ShaderCodes> mpColorIDShader;

		std::shared_ptr<ManipulatorBase> mpManipulator;

		glm::mat4 mviewMatrix;
		glm::mat4 mprojectionMatrix;
		GLuint mViewUBO;

		GLuint mMeshIDRBO;
	};
}
