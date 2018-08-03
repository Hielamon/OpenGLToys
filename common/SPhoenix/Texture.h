#pragma once

#include "utils.h"

namespace SP
{
	class Texture
	{
	public:
		Texture() : mwidth(-1), mheight(-1), mchannels(-1) {}
		Texture(const std::string &imagePath) : mwidth(-1), mheight(-1), mchannels(-1) {}
		~Texture() {}

	protected:
		int mwidth, mheight, mchannels;
		std::string mimagePath;
		std::shared_ptr<unsigned char> mpData;
	};
}