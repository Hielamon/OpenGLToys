#pragma once

#include "utils.h"
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

	class Texture
	{
	public:
		Texture() : mwidth(-1), mheight(-1), mchannels(-1) {}

		Texture(const std::string &imagePath, TextureType type)
		{
			unsigned char * data = SOIL_load_image(imagePath.c_str(), &mwidth, &mheight, &mchannels, 3/*SOIL_LOAD_AUTO*/);
			if (!data)
			{
				SP_CERR("Failed to Open Texture File: " + imagePath);
				exit(-1);
			}


			std::shared_ptr<unsigned char> pData(data, SOILImageDeleter);
			mpData = pData;
			mtype = type;
			mimagePath = imagePath;
		}

		Texture(const std::shared_ptr<unsigned char> &pData, int width, int height, int channels, TextureType type)
			: mpData(pData), mwidth(width), mheight(height), mchannels(channels), mtype(type) {}

		~Texture() {}

		TextureType getType() 
		{
			return mtype;
		}

		int getWidth()
		{
			return mwidth;
		}

		int getHeight()
		{
			return mheight;
		}

		const unsigned char * getData()
		{
			return mpData.get();
		}

	protected:

		//The mchannels are the original channaels of texture
		int mwidth, mheight, mchannels;

		std::string mimagePath;
		TextureType mtype;
		std::shared_ptr<unsigned char> mpData;
	};

	class TextureGlobal
	{
	public:
		TextureGlobal()
		{
			aiTypeMap = {
				{ aiTextureType_AMBIENT, Tex_AMBIENT },
				{ aiTextureType_DIFFUSE, Tex_DIFFUSE },
				{ aiTextureType_SPECULAR, Tex_SPECULAR }/*,
				{ aiTextureType_NORMALS, Tex_NORMALS }*/
			};

			typeMacroMap = {
				{ Tex_AMBIENT , "#define AMBIENT_TEXTURE \n" },
				{ Tex_DIFFUSE , "#define DIFFUSE_TEXTURE \n" },
				{ Tex_SPECULAR, "#define SPECULAR_TEXTURE\n" }
			};

			typeMaterialNameMap = 
			{
				{ Tex_AMBIENT , "material.ambient_maps" },
				{ Tex_DIFFUSE , "material.diffuse_maps" },
				{ Tex_SPECULAR, "material.specular_maps" }
			};
		}

		~TextureGlobal() {}

		static TextureGlobal& getInstance()
		{
			static TextureGlobal texGSet;
			return texGSet;
		}

		std::map<aiTextureType, TextureType> aiTypeMap;
		std::map<TextureType, std::string> typeMacroMap;
		std::map<TextureType, std::string> typeMaterialNameMap;
		
	};
}