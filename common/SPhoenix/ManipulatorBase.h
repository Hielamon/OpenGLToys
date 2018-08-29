#pragma once
#include "Utils.h"

namespace SP
{
	class ManipulatorBase
	{
	public:
		virtual void registerCallBacks() = 0;

	protected:
		ManipulatorBase() {}
		~ManipulatorBase() {}
	};
}



