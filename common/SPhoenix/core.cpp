#include <SPhoenix/core.h>

namespace SP
{
	GlfwConfigure _glfwconfig;

	void KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mode)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	//GLWindow *GLWindow::glwindow_instance;
}