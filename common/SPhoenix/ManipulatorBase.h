#pragma once
#include <SPhoenix/utils.h>

namespace SP
{
	class ManipulatorBase
	{
	public:
		ManipulatorBase() {}
		~ManipulatorBase() {}

		virtual void registerCallBacks() = 0;
	};
}



