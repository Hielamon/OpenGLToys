#pragma once
#include "utils.h"

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



