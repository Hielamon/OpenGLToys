#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_access.hpp>

#include <SOIL.h>
#include <SPhoenix/Shader.h>
#include <SPhoenix/Scene.h>
#include <SPhoenix/utils.h>

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
				glfwDestroyWindow(mglfwWinPtr);
		}

		bool run()
		{
			if (prepare() == GL_FALSE)
			{
				return GL_FALSE;
			}

			while (!glfwWindowShouldClose(mglfwWinPtr))
			{
				runOnce();

				glfwPollEvents();
				glfwSwapBuffers(mglfwWinPtr);
			}

			return GL_TRUE;
		}

	protected:
		virtual bool prepare() = 0;

		virtual void runOnce() = 0;


	private:
		/**GLFW window pointer*/
		GLFWwindow * mglfwWinPtr;

		/**window size*/
		int mwidth, mheight;

		/**windows name*/
		std::string mwinName;

		/**indicate the window state, TODO : maybe change to a enum*/
		bool mopened;

		GLFWwindow * _initWindow()
		{
			{
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
		}

		~Camera(){}

		/***/
		void addScene(Scene &scene)
		{
			SceneUtil sceneUtil(scene);
			mvSceneUtil.push_back(sceneUtil);
		}

	protected:
		virtual bool prepare()
		{
			return GL_TRUE;
		}

		virtual void runOnce()
		{
			for (size_t i = 0; i < mvSceneUtil.size(); i++)
			{
				mvSceneUtil[i].show();
			}
		}

	private:
		std::vector<SceneUtil> mvSceneUtil;
	};
}
