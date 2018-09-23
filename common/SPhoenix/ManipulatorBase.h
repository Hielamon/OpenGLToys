#pragma once
#include "Utils.h"

namespace SP
{
	class ManipulatorBase
	{
	public:
		virtual void registerCallBacks() = 0;

		//Some tasks need to be processed for every frame
		//Such as the movement of cameras
		virtual void doFrameTasks()
		{
			return;
		}

	protected:
		ManipulatorBase() {}
		~ManipulatorBase() {}
	};
}



