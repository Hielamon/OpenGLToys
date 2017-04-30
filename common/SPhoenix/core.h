#pragma once

#include <SPhoenix/utils.h>
#include <SPhoenix/Shader.h>
#include <SPhoenix/Scene.h>
//#include <SPhoenix/Manipulator.h>


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

			mglfwWinPtr = _initWindow();
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

		bool run()
		{
			/*if (prepare() == GL_FALSE)
			{
				return GL_FALSE;
			}*/

			while (!isShutdown())
			{
				runOnce();

				glfwPollEvents();
				glfwSwapBuffers(mglfwWinPtr);
			}

			return GL_TRUE;
		}

		virtual void runOnce() = 0;

		virtual bool isShutdown()
		{
			return glfwWindowShouldClose(mglfwWinPtr);
		}

	
		//virtual bool prepare() = 0;

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
		GLFWwindow * _initWindow()
		{
			{
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				glfwWindowHint(GLFW_SAMPLES, 8);
				glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
			}

			GLFWwindow *window = glfwCreateWindow(mwidth, mheight, mwinName.c_str(), NULL, NULL);
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
			mviewMatrix = glm::mat4(1.0f);

			mfovy = glm::radians(45.f);
			maspect = 1.0f;
			mzNear = 0.01;
			mzFar = 100.f;

			//mprojectionMatrix = glm::mat4(1.0f);
			mprojectionMatrix = glm::perspective(mfovy, maspect, mzNear, mzFar);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_DEPTH_TEST);
		}

		~Camera(){}

		/**Set a scene to show mass*/
		GLuint addScene(Scene &scene)
		{
			SceneUtil sceneUtil(scene);
			mvSceneUtil.push_back(sceneUtil);
			return int(mvSceneUtil.size()) - 1;
		}

		void setFrustum(float fovyDeg = 45.f, float aspect = 1.0, float zNear = 0.01, float zFar = 100.f)
		{
			mfovy = glm::radians(fovyDeg);
			maspect = aspect;
			mzNear = zNear;
			mzFar = zFar;

			mprojectionMatrix = glm::perspective(mfovy, maspect, mzNear, mzFar);
		}

		void setKeyCallback(GLFWkeyfun keyCallBack)
		{
			glfwSetKeyCallback(mglfwWinPtr, keyCallBack);
		}

		SceneUtil& getSceneUtil(int index)
		{
			if (index < 0 || index >= mvSceneUtil.size())
			{
				SP_CERR("Invalid index to mvSceneUtil");
				exit(-1);
			}
			return mvSceneUtil[index];
		}

		glm::mat4 mviewMatrix;

	protected:
		/*virtual bool prepare()
		{
			if (mvSceneUtil.empty())
			{
				SP_CERR("There isn't any scene for preparation");
				return GL_FALSE;
			}

			return GL_TRUE;
		}*/

		virtual void runOnce()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (size_t i = 0; i < mvSceneUtil.size(); i++)
			{
				GLuint programID = mvSceneUtil[i].getProgramID();

				GLint projectionLoc = glGetUniformLocation(programID, "projection");
				glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mprojectionMatrix));

				GLint viewLoc = glGetUniformLocation(programID, "view");
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mviewMatrix));

				mvSceneUtil[i].show();
			}
		}

	private:
		std::vector<SceneUtil> mvSceneUtil;

		glm::mat4 mprojectionMatrix;

		float mfovy, maspect, mzNear, mzFar;
	};
}
