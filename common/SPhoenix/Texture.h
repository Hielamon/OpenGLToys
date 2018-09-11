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
		Tex_AMBIENT, Tex_DIFFUSE, Tex_SPECULAR/*, Tex_NORMALS*/
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

		std::map<aiTextureType, TextureType> mAitextypeToTextype;
		std::map<TextureType, std::string> mTextypeToMacro;
		std::map<TextureType, std::string> mTextypeToMaterialName;

	private:
		TextureGlobal()
		{
			mAitextypeToTextype = {
				{ aiTextureType_AMBIENT, Tex_AMBIENT },
				{ aiTextureType_DIFFUSE, Tex_DIFFUSE },
				{ aiTextureType_SPECULAR, Tex_SPECULAR }/*,
														{ aiTextureType_NORMALS, Tex_NORMALS }*/
			};

			mTextypeToMacro = {
				{ Tex_AMBIENT , "#define AMBIENT_TEXTURE \n" },
				{ Tex_DIFFUSE , "#define DIFFUSE_TEXTURE \n" },
				{ Tex_SPECULAR, "#define SPECULAR_TEXTURE\n" }
			};

			mTextypeToMaterialName =
			{
				{ Tex_AMBIENT , "material.ambient_maps" },
				{ Tex_DIFFUSE , "material.diffuse_maps" },
				{ Tex_SPECULAR, "material.specular_maps" }
			};
		}
	};

	class Texture
	{
	public:
		Texture() : mWidth(-1), mHeight(-1), mChannels(-1) {}

		Texture(const std::string &imagePath, TextureType type)
		{
			unsigned char * data = SOIL_load_image(imagePath.c_str(), &mWidth, &mHeight, &mChannels, 3/*SOIL_LOAD_AUTO*/);
			if (!data)
			{
				SP_CERR("Failed to Open Texture File: " + imagePath);
				mbValid = false;
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

		bool IsValid()
		{
			return mbValid;
		}

		const unsigned char * getData()
		{
			return mpData.get();
		}

	protected:

		//The mChannels are the original channaels of texture
		int mWidth, mHeight, mChannels;
		bool mbValid;

		std::string mImagePath;
		TextureType mType;
		std::shared_ptr<unsigned char> mpData;
	};
}