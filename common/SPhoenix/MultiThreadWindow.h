#pragma once

#include <SPhoenix/MonitorWindow.h>

namespace SP
{
	class MultiThreadWindow
	{
	public:
		MultiThreadWindow(const std::string &majorWinName = "Untitled",
						  int width = 0, int height = 0)
		{

		}


		~MultiThreadWindow() {}

	private:
		/**major window size*/
		int mWidth, mHeight;

		std::vector<std::thread> mvWindowThread;
	};
}