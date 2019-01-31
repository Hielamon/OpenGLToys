#pragma once

#include "Utils.h"
#include <assimp/material.h>

namespace SP
{
	inline void SOILImageDeleter(unsigned char *d)
	{
		if (d) SOIL_free_image_data(d);
	}

	//The texture types which are used in our SPhoenix library
	enum TextureType
	{
		Tex_AMBIENT, Tex_DIFFUSE, Tex_SPECULAR/*, Tex_NORMALS*/,
		Tex_CUBE1, Tex_CUBE2
	};

	class TextureGlobal
	{
	public:
		~TextureGlobal() {}

		static TextureGlobal& getInstance()
		{
			static TextureGlobal texGSet;
			return texGSet;
		}

		std::map<aiTextureType, TextureType> aiTextypeToTextype;
		std::map<TextureType, std::string> TextypeToMacro;
		std::map<TextureType, std::string> TextypeToMaterialName;

	private:
		TextureGlobal()
		{
			aiTextypeToTextype = {
				{ aiTextureType_AMBIENT, Tex_AMBIENT },
				{ aiTextureType_DIFFUSE, Tex_DIFFUSE },
				{ aiTextureType_SPECULAR, Tex_SPECULAR }/*,
														{ aiTextureType_NORMALS, Tex_NORMALS }*/
			};

			TextypeToMacro = {
				{ Tex_AMBIENT , "#define AMBIENT_TEXTURE \n" },
				{ Tex_DIFFUSE , "#define DIFFUSE_TEXTURE \n" },
				{ Tex_SPECULAR, "#define SPECULAR_TEXTURE\n" },
				{ Tex_CUBE1, "#define CUBE_TEXTURE1\n" },
				{ Tex_CUBE2, "#define CUBE_TEXTURE2\n" },
			};

			TextypeToMaterialName =
			{
				{ Tex_AMBIENT , "material.ambient_maps" },
				{ Tex_DIFFUSE , "material.diffuse_maps" },
				{ Tex_SPECULAR, "material.specular_maps" },
				{ Tex_CUBE1, "cube_map1" },
				{ Tex_CUBE2, "cube_map2" },
			};
		}
	};

	class Texture
	{
	public:
		Texture(TextureType type = TextureType::Tex_DIFFUSE)
			: mWidth(-1), mHeight(-1), mChannels(-1),
			mForceChannels(3), mType(type) {}

		Texture(const std::string &imagePath, TextureType type)
			: mForceChannels(3)
		{
			unsigned char * data = SOIL_load_image(imagePath.c_str(),
												   &mWidth, &mHeight,
												   &mChannels, /*mForceChannels*/SOIL_LOAD_AUTO);
			
			
			if (!data)
			{
				SP_CERR("Failed to Open Texture File: " + imagePath);
				mbValid = false;
				return;
			}

			std::shared_ptr<unsigned char> pData(data, SOILImageDeleter);
			mpData = pData;
			mType = type;
			mImagePath = imagePath;
			mbValid = true;
		}

		Texture(const std::shared_ptr<unsigned char> &pData, int width, int height, int channels, TextureType type)
			: mpData(pData), mWidth(width), mHeight(height), mChannels(channels), mType(type) {}

		~Texture() {}

		TextureType getType() 
		{
			return mType;
		}

		int getWidth()
		{
			return mWidth;
		}

		int getHeight()
		{
			return mHeight;
		}

		int getChannel()
		{
			return mChannels;
		}

		bool IsValid()
		{
			return mbValid;
		}

		const unsigned char * getData()
		{
			return mpData.get();
		}

		void clockWise90()
		{
			int totalSize = mWidth*mHeight*mForceChannels;
			unsigned char *dataTemp = new unsigned char[totalSize];

			unsigned char *data = mpData.get();
			memcpy(dataTemp, data, totalSize * sizeof(unsigned char));

			//Do clockWise90 transform
			int H = mWidth, W = mHeight;
			for (size_t i = 0; i < H; i++)
			{
				for (size_t j = 0; j < W; j++)
				{
					int offset = (i * W + j) * 3;
					int offset_ = ((W - 1 - j)*mWidth + i) * 3;
					for (size_t k = 0; k < 3; k++)
					{
						*(data + offset + k) = *(dataTemp + offset_ + k);
					}
				}
			}
			mWidth = W;
			mHeight = H;
			delete [] dataTemp;
		}

		void antiClockWise90()
		{
			int totalSize = mWidth*mHeight*mForceChannels;
			unsigned char *dataTemp = new unsigned char[totalSize];

			unsigned char *data = mpData.get();
			memcpy(dataTemp, data, totalSize * sizeof(unsigned char));

			//Do clockWise90 transform
			int H = mWidth, W = mHeight;
			for (size_t i = 0; i < H; i++)
			{
				for (size_t j = 0; j < W; j++)
				{
					int offset = (i * W + j) * 3;
					int offset_ = (j*mWidth + H - 1 -i) * 3;
					for (size_t k = 0; k < 3; k++)
					{
						*(data + offset + k) = *(dataTemp + offset_ + k);
					}
				}
			}
			mWidth = W;
			mHeight = H;
			delete [] dataTemp;
		}

	protected:

		//The mChannels are the original channaels of texture
		int mWidth, mHeight, mChannels;
		int mForceChannels;
		bool mbValid;

		std::string mImagePath;
		TextureType mType;
		std::shared_ptr<unsigned char> mpData;
	};
}