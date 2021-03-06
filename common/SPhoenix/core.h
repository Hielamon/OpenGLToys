#pragma once

#include <SPhoenix/utils.h>
#include <SPhoenix/Shader.h>
#include <SPhoenix/Scene.h>
#include <SPhoenix/ManipulatorBase.h>


namespace SP
{
	class GlfwConfigure
	{
	public:

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

			atexit(glfwTerminate);
		}

		~GlfwConfigure() {}
	};
	extern GlfwConfigure _glfwconfig;

	/**OpenGL window base*/
	class GLWindowBase
	{
	public:
		GLWindowBase(const std::string &win_name = "Untitled", int width = 0, int height = 0)
		{
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
			: GLWindowBase(camName, width, height)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_DEPTH_TEST);

			setProjectionMatrix();
			mviewMatrix = glm::mat4(1.0f);
		}

		~Camera(){}

		std::shared_ptr<SceneUtil> getSceneUtil(int index)
		{
			if (index < 0 || index >= mvpSceneUtil.size())
			{
				SP_CERR("Invalid index to mvSceneUtil");
				exit(-1);
			}
			return mvpSceneUtil[index];
		}

		std::vector<std::shared_ptr<SceneUtil>> getSceneUtil()
		{
			return mvpSceneUtil;
		}

		void setProjectionMatrix(float fovy = 45.f, float aspect = 0.0f, float zNear = 0.01, float zFar = 100.f)
		{
			mfovy = fovy; mzNear = zNear; mzFar = zFar;
			maspect = aspect == 0.0f ? mwidth / float(mheight) : aspect;

			mprojectionMatrix = glm::perspective(glm::radians(mfovy), maspect, mzNear, mzFar);
		}

		void setViewMatrix(glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up)
		{
			meye = eye; mcenter = center; mup = up;

			mviewMatrix = glm::lookAt(meye, mcenter, mup);
		}

		void setManipulator(std::shared_ptr<ManipulatorBase> pManipulator)
		{
			mpManipulator = pManipulator;
			mpManipulator->registerCallBacks();
		}

		/**Add a Scene to a Camera's SceneUtil pointer vector, and return the SceneUtil ID*/
		GLuint addScene(Scene &scene)
		{
			mvpSceneUtil.push_back(std::make_shared<SceneUtil>(scene));
			return int(mvpSceneUtil.size()) - 1;
		}

		float mfovy, maspect, mzNear, mzFar;
		glm::vec3 meye, mcenter, mup;

	protected:

		virtual void runOnce()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (size_t i = 0; i < mvpSceneUtil.size(); i++)
			{
				GLuint programID = mvpSceneUtil[i]->getProgramID();

				GLint projectionLoc = glGetUniformLocation(programID, "projection");
				glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mprojectionMatrix));

				GLint viewLoc = glGetUniformLocation(programID, "view");
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mviewMatrix));

				mvpSceneUtil[i]->show();
			}
		}

	private:
		std::vector<std::shared_ptr<SceneUtil>> mvpSceneUtil;
		std::shared_ptr<ManipulatorBase> mpManipulator;

		glm::mat4 mviewMatrix;
		glm::mat4 mprojectionMatrix;
	};
}
