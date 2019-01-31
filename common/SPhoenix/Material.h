#pragma once
#include "Texture.h"


#include <ft2build.h>
#include FT_FREETYPE_H  

namespace SP
{
	class Material
	{
	public:
		//The default material constructor will set the diffusecolor and 
		//ambientcolor to (1.0f, 1.0f, 1.0f, 1.0f) and set the specularcolor to
		//(0.0f, 0.0f, 0.0f, 0.0f)
		Material() : mTextureTotalCount(0), mDiffuseColor(glm::vec4(1.0f)),
			mAmbientColor(glm::vec4(1.0f)), mSpecularColor(glm::vec4(0.0f)),
			mShininess(DEFAULT_SHININESS), mShininessStrength(DEFAULT_SHININESS_STRENGTH),
			mWarpType(GL_REPEAT), mbUploaded(false) {}

		Material(const glm::vec4 &diffuseColor, const glm::vec4 &ambientColor,
				 const glm::vec4 &specularColor, float shininess = DEFAULT_SHININESS,
				 float shininessStrength = DEFAULT_SHININESS_STRENGTH)
			: mTextureTotalCount(0), mDiffuseColor(diffuseColor), mAmbientColor(ambientColor),
			mSpecularColor(specularColor), mShininess(shininess),
			mShininessStrength(shininessStrength), mWarpType(GL_REPEAT), mbUploaded(false) {}

		Material(const glm::vec4 &diffAndambiColor, const glm::vec4 &specularColor,
				 float shininess = DEFAULT_SHININESS, 
				 float shininessStrength = DEFAULT_SHININESS_STRENGTH)
			: mTextureTotalCount(0), mDiffuseColor(diffAndambiColor),
			mAmbientColor(diffAndambiColor), mSpecularColor(specularColor),
			mShininess(shininess), mShininessStrength(shininessStrength),
			mWarpType(GL_REPEAT), mbUploaded(false) {}

		Material(const glm::vec4 &uniformColor, float shininess = DEFAULT_SHININESS,
				 float shininessStrength = DEFAULT_SHININESS_STRENGTH)
			: mTextureTotalCount(0), mDiffuseColor(uniformColor), mAmbientColor(uniformColor),
			mSpecularColor(uniformColor), mShininess(shininess),
			mShininessStrength(shininessStrength), mWarpType(GL_REPEAT), mbUploaded(false) {}

		~Material()
		{
			clearUploaded();
		}

		void reset()
		{
			mmTextypeToTex.clear();
			mTextureTotalCount = 0;

			if (mbUploaded)
				clearUploaded();
		}

		void setTexWarpType(GLint warpType = GL_REPEAT)
		{
			mWarpType = warpType;
		}

		void setShininess(const GLfloat& shininess)
		{
			mShininess = shininess;
		}

		void setShininessStrength(const GLfloat& shininessStrength)
		{
			mShininessStrength = shininessStrength;
		}

		void setDiffuseColor(const glm::vec4 &diffuseColor)
		{
			mDiffuseColor = diffuseColor;
		}

		void setAmbientColor(const glm::vec4 &ambientColor)
		{
			mAmbientColor = ambientColor;
		}

		void setSpecularColor(const glm::vec4 &specularColor)
		{
			mSpecularColor = specularColor;
		}

		void addTexture(const std::shared_ptr<Texture> &pTexture)
		{
			TextureType curType = pTexture->getType();
			if (mmTextypeToTex.find(curType) == mmTextypeToTex.end())
			{
				mmTextypeToTex[curType] = std::vector<std::shared_ptr<Texture>>();
			}
			std::vector<std::shared_ptr<Texture>> &vpTexture = mmTextypeToTex[curType];

			if (vpTexture.size() >= 1)
			{
				std::shared_ptr<Texture> &pLastTexture = vpTexture[vpTexture.size() - 1];
				if (pTexture->getHeight() != pLastTexture->getHeight() ||
					pTexture->getWidth() != pLastTexture->getWidth() || 
					pTexture->getChannel() != pLastTexture->getChannel())
				{
					SP_CERR("The texture size(width, height or channel) for the same \
							type in a material is not consistent");
							
					exit(-1);
				}
			}

			vpTexture.push_back(pTexture);
			mTextureTotalCount++;
		}

		int getTextypeCount(TextureType type)
		{
			if (mmTextypeToTex.find(type) != mmTextypeToTex.end())
			{
				return mmTextypeToTex[type].size();
			}
			else
			{
				return 0;
			}

		}

		bool replaceTexture(const std::shared_ptr<Texture> &pTexture, int index)
		{
			TextureType curType = pTexture->getType();
			if (mmTextypeToTex.find(curType) == mmTextypeToTex.end())
			{
				SP_CERR("There is not any texture has the same type the input texture");
				return false;
			}

			std::vector<std::shared_ptr<Texture>> &vpTexture = mmTextypeToTex[curType];

			if (index >= 0 && index < vpTexture.size() )
			{
				std::shared_ptr<Texture> pOtherTexture;
				if(index + 1 < vpTexture.size())
					pOtherTexture = vpTexture[index + 1];
				else if(index - 1 >= 0)
					pOtherTexture = vpTexture[index - 1];

				if (pOtherTexture.use_count() != 0 &&
					(pTexture->getHeight() != pOtherTexture->getHeight() ||
					 pTexture->getWidth() != pOtherTexture->getWidth() ||
					 pTexture->getChannel() != pOtherTexture->getChannel()))
				{
					SP_CERR("The texture size(width, height or channel) for the same \
							type in a material is not consistent");

					return false;
				}

				vpTexture[index] = pTexture;

				if (mbUploaded)
				{
					clearUploaded();
					uploadToDevice();
				}
			}
		}

		int getTextureTotalCount()
		{
			return mTextureTotalCount;
		}

		std::vector<std::shared_ptr<Texture>> getTextures(TextureType type)
		{
			std::vector<std::shared_ptr<Texture>> vpTexture;
			if (mmTextypeToTex.find(type) != mmTextypeToTex.end())
			{
				vpTexture = mmTextypeToTex[type];
			}

			return vpTexture;
		}

		//Get the macro for the shader codes, which defines some necessary variable 
		//Such as texture type the material holds, which is picked from the
		//TextypeToMacro of the TextureGlobal
		virtual std::string getShaderMacros()
		{
			std::string macros = "";
			
			if (mTextureTotalCount == 0) return macros;

			macros += "#define HAVE_TEXTURE\n";

			std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;
			for (iter = mmTextypeToTex.begin(); iter != mmTextypeToTex.end(); iter++)
			{
				macros += TextureGlobal::getInstance().TextypeToMacro[iter->first];
			}

			return macros;
		}

		//upload the material information to the device
		//If the mbUploaded is true, this function will do nothing
		virtual void uploadToDevice()
		{
			if (mbUploaded) return;

			std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;
			for (iter = mmTextypeToTex.begin(); iter != mmTextypeToTex.end(); iter++)
			{
				//We just use the int value of the texture type as the Unit ID
				int textureUnit = iter->first;
				_uploadArrayTexture(textureUnit, iter->second);
			}

			//Set the existence of texture maps
			mbDiffuseMap = mmTexunitToTexbuffer.find(Tex_DIFFUSE) != mmTexunitToTexbuffer.end();
			mbAmbientMap = mmTexunitToTexbuffer.find(Tex_AMBIENT) != mmTexunitToTexbuffer.end();
			mbSpecularMap = mmTexunitToTexbuffer.find(Tex_SPECULAR) != mmTexunitToTexbuffer.end();

			mbUploaded = true;
		}

		//clear the uploaded buffers and reset the mbUploaded to false
		virtual void clearUploaded()
		{
			if (mbUploaded)
			{
				std::map<int, GLuint>::iterator iter;
				for (iter = mmTexunitToTexbuffer.begin();
					 iter != mmTexunitToTexbuffer.end(); iter++)
				{
					glDeleteTextures(1, &(iter->second));
				}
				mmTexunitToTexbuffer.clear();
				mbUploaded = false;
			}
		}

		//Material Activate function which is applied before every draw commands
		//, and after the useProgram statement.
		//This function need two inputs : bValidTexCoord and bValidVertexColor
		//If the texture coords are invalid there is a valid vertex color array
		//The active funtion will just update the shinness paramater,
		//otherwise it will intelligently choose
		//whether to use texture maps or the Blinn-Phong uniform color
		virtual void active(const GLuint &programID,
							bool bValidTexCoord = true,
							bool bValidVertexColor = false)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current material has not been uploaded befor activing");
				return;
			}

			GLint uShininessLoc = glGetUniformLocation(programID, "material.uShininess");
			glUniform1f(uShininessLoc, mShininess);

			GLint uShininessStrengthLoc =
				glGetUniformLocation(programID, "material.uShininessStrength");
			glUniform1f(uShininessStrengthLoc, mShininessStrength);

			if (bValidTexCoord)
			{
				std::map<TextureType, std::string> &nameMap = TextureGlobal::getInstance().TextypeToMaterialName;
				//std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;
				//for (iter = mmTextypeToTex.begin(); iter != mmTextypeToTex.end(); iter++)
				//{
				//	int samplerLoc = glGetUniformLocation(programID, nameMap[iter->first].c_str());
				//	glUniform1i(samplerLoc, int(iter->first));
				//	//GL_DEBUG_ALL;
				//}

				std::map<int, GLuint>::iterator iter_;
				for (iter_ = mmTexunitToTexbuffer.begin();
					 iter_ != mmTexunitToTexbuffer.end(); iter_++)
				{
					TextureType type = TextureType(iter_->first);
					int samplerLoc = glGetUniformLocation(programID, nameMap[type].c_str());
					glUniform1i(samplerLoc, int(iter_->first));

					glActiveTexture(GL_TEXTURE0 + iter_->first);
					//glBindTexture(GL_TEXTURE_2D, iter_->second);
					glBindTexture(GL_TEXTURE_2D_ARRAY, iter_->second);
				}
				glActiveTexture(GL_TEXTURE0);
			}
			else if (bValidVertexColor)
			{
				//When there is no texture used, but there is vertex color array
				//We dont need to upload any diffuse, ambient or specular
				return;
			}

			bool bDiffuse = !(mbDiffuseMap && bValidTexCoord);
			bool bAmbient = !(mbAmbientMap && bValidTexCoord) && bDiffuse;
			bool bSpecular = !(mbSpecularMap && bValidTexCoord);

			if (bDiffuse) uploadColor(programID, "material.uDiffuse", mDiffuseColor);
			if (bAmbient) uploadColor(programID, "material.uAmbient", mAmbientColor);
			if (bSpecular) uploadColor(programID, "material.uSpecular", mSpecularColor);
		}

	protected:
		//The map stores the texture as the Type-TextureVector pairs
		std::map<TextureType, std::vector<std::shared_ptr<Texture>>> mmTextypeToTex;

		//The uniform color arguments for Blinn-Phong lighting model
		glm::vec4 mDiffuseColor, mAmbientColor, mSpecularColor;
		GLfloat mShininess, mShininessStrength;

		//Record the total texture counts
		int mTextureTotalCount;

		//****** The inner variable for device ******//
		bool mbUploaded;

		GLint mWarpType;

		//Indicate whether there is a *color map
		bool mbDiffuseMap, mbAmbientMap, mbSpecularMap;

		//The map between the texture unit and the texture buffer name;
		std::map<int, GLuint> mmTexunitToTexbuffer;

	protected:
		//upload the color uniform variable
		void uploadColor(GLint programID, const char * varname, const glm::vec4 &color)
		{
			GLint uColorLoc = glGetUniformLocation(programID, varname);
			glUniform4f(uColorLoc, color.r, color.g, color.b, color.a);
		}

	private:
		//upload the array texture for the corresponding texture type
		//Actually we just use the enum value of the texture type as
		//the texture unit, so we name the texture type as texture unit
		void _uploadArrayTexture(int textureUnit,
								 const std::vector<std::shared_ptr<Texture>> &vpTexture)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);

			int width = vpTexture[0]->getWidth();
			int height = vpTexture[0]->getHeight();
			int channel = vpTexture[0]->getChannel();
			int layerCount = vpTexture.size();

			if (width <= 0 || height <= 0) return;

			GLuint textureBuffer;
			glGenTextures(1, &textureBuffer);
			mmTexunitToTexbuffer[textureUnit] = textureBuffer;

			glBindTexture(GL_TEXTURE_2D_ARRAY, textureBuffer);
			int levelNum = floor(log2(std::max(width, height))) + 1;

			int internalFormat, Format;
			switch (channel)
			{
			case 1:
				internalFormat = GL_R8;
				Format = GL_RED;
				break;
			case 2:
				internalFormat = GL_RG8;
				Format = GL_RG;
				break;
			case 3:
				internalFormat = GL_RGB8;
				Format = GL_RGB;
				break;
			case 4:
				internalFormat = GL_RGBA8;
				Format = GL_RGBA;
				break;
			default:
				SP_CERR("Unknown channels");
				exit(-1);
				break;
			}

			glTexStorage3D(GL_TEXTURE_2D_ARRAY, levelNum, internalFormat, width, height, layerCount);

			for (size_t i = 0; i < layerCount; i++)
			{
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1,
								Format, GL_UNSIGNED_BYTE, vpTexture[i]->getData());
			}

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, mWarpType);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, mWarpType);

			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			glActiveTexture(GL_TEXTURE0);
		}

	
	};

	class MaterialFBO : public Material
	{
	public:
		MaterialFBO() {}
		~MaterialFBO() {}

		bool setTexbuffer(int texUnit, GLuint buffer)
		{
			mmTexunitToTexbuffer[texUnit] = buffer;
			return true;
		}

		bool getTexbuffer(int texUnit, GLuint &buffer)
		{
			if (mmTexunitToTexbuffer.find(texUnit) ==
				mmTexunitToTexbuffer.end())
				return false;

			return buffer = mmTexunitToTexbuffer[texUnit];
		}

		//Get the macro for the shader codes, which defines some necessary variable 
		//Such as texture type the material holds, which is picked from the
		//TextypeToMacro of the TextureGlobal
		virtual std::string getShaderMacros()
		{
			std::string macros = "";

			//if (mmTexunitToTexbuffer.size() == 0) return macros;

			macros += "#define HAVE_TEXTURE\n";

			macros += TextureGlobal::getInstance().TextypeToMacro[TextureType::Tex_DIFFUSE];

			return macros;
		}

		virtual void active(const GLuint &programID,
							bool bValidTexCoord = true,
							bool bValidVertexColor = false)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current material has not been uploaded befor activing");
				return;
			}

			GLint uShininessLoc = glGetUniformLocation(programID, "material.uShininess");
			glUniform1f(uShininessLoc, mShininess);

			GLint uShininessStrengthLoc =
				glGetUniformLocation(programID, "material.uShininessStrength");
			glUniform1f(uShininessStrengthLoc, mShininessStrength);

			if (bValidTexCoord)
			{
				std::map<TextureType, std::string> &nameMap = TextureGlobal::getInstance().TextypeToMaterialName;
				std::map<int, GLuint>::iterator iter_;
				for (iter_ = mmTexunitToTexbuffer.begin();
					 iter_ != mmTexunitToTexbuffer.end(); iter_++)
				{
					TextureType type = TextureType(iter_->first);
					
					int samplerLoc = glGetUniformLocation(programID, nameMap[type].c_str());
					glUniform1i(samplerLoc, int(iter_->first));

					glActiveTexture(GL_TEXTURE0 + iter_->first);
					//glBindTexture(GL_TEXTURE_2D, iter_->second);
					glBindTexture(GL_TEXTURE_2D, iter_->second);
				}
				glActiveTexture(GL_TEXTURE0);
			}
			else if (bValidVertexColor)
			{
				return;
			}

			bool bDiffuse = !(mbDiffuseMap && bValidTexCoord);
			bool bAmbient = !(mbAmbientMap && bValidTexCoord) && bDiffuse;
			bool bSpecular = !(mbSpecularMap && bValidTexCoord);

			if (bDiffuse) uploadColor(programID, "material.uDiffuse", mDiffuseColor);
			if (bAmbient) uploadColor(programID, "material.uAmbient", mAmbientColor);
			if (bSpecular) uploadColor(programID, "material.uSpecular", mSpecularColor);
		}

	};

	//The mvpCubeFaceTexture vector holds the six face texture of the cube,
	//following the order:right, left, top, bottom, back, front
	class MaterialCube : public Material
	{
	public:
		MaterialCube()
		{
			mWarpType = GL_CLAMP_TO_EDGE;
			mvpCubeFaceTexture.reserve(6);
		}

		~MaterialCube() {}

		//The input vector holds the six face texture of the cube,
		//following the order:right, left, top, bottom, back, front
		void setCubeTextures(const std::vector<std::shared_ptr<Texture>> &vpTexture)
		{
			if (vpTexture.size() != 6)
			{
				SP_CERR("The size of textures for setting cubemap is not 6");
				return;
			}
			mvpCubeFaceTexture = vpTexture;
		}

		virtual void uploadToDevice()
		{
			if (mbUploaded) return;

			if (mvpCubeFaceTexture.size() != 6)
			{
				SP_CERR("The size of textures for uploading cubemap is not 6");
				return;
			}

			//Check the empty texture
			for (size_t i = 0; i < 6; i++)
			{
				if (mvpCubeFaceTexture[i].use_count() == 0)
				{
					SP_CERR("The " << i << "th texture of the cubeMap is empty");
					return;
				}
			}

			//Check the same size
			int width = mvpCubeFaceTexture[0]->getWidth();
			int height = mvpCubeFaceTexture[0]->getHeight();
			for (size_t i = 1; i < 6; i++)
			{
				if (mvpCubeFaceTexture[i]->getWidth() != width)
				{
					SP_CERR("The " << i << "th texture's width is not same with pre");
					return;
				}

				if (mvpCubeFaceTexture[i]->getHeight() != height)
				{
					SP_CERR("The " << i << "th texture's height is not same with pre");
					return;
				}
			}

			int textureUnit = Tex_CUBE1;
			_uploadCubeTexture(textureUnit, mvpCubeFaceTexture);

			Material::uploadToDevice();
		}

		virtual void active(const GLuint &programID,
							bool bValidTexCoord = true,
							bool bValidVertexColor = false)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current MaterialCube has not been uploaded befor activing");
				return;
			}

			std::map<TextureType, std::string> &nameMap = 
				TextureGlobal::getInstance().TextypeToMaterialName;

			{
				int samplerLoc = glGetUniformLocation(programID, nameMap[Tex_CUBE1].c_str());
				glUniform1i(samplerLoc, Tex_CUBE1);
				//GL_DEBUG_ALL;
			}

			glActiveTexture(GL_TEXTURE0 + Tex_CUBE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mmTexunitToTexbuffer[Tex_CUBE1]);
			glActiveTexture(GL_TEXTURE0);
		}

	private:
		//holds the six face texture of the cube, following the order:
		//right, left, top, bottom, back, front
		std::vector<std::shared_ptr<Texture>> mvpCubeFaceTexture;

	private:
		void _uploadCubeTexture(int textureUnit,
								const std::vector<std::shared_ptr<Texture>> &vpTexture)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);

			int width = vpTexture[0]->getWidth();
			int height = vpTexture[0]->getHeight();
			int layerCount = vpTexture.size();

			GLuint cubeMapBuffer;
			glGenTextures(1, &cubeMapBuffer);
			mmTexunitToTexbuffer[textureUnit] = cubeMapBuffer;

			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapBuffer);

			for (size_t i = 0; i < vpTexture.size(); i++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
							 GL_RGB8, width, height, 0,
							 GL_RGB, GL_UNSIGNED_BYTE, vpTexture[i]->getData());
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//since texture coordinates that are exactly between two faces might 
			//not hit an exact face (due to some hardware limitations) so by using
			// GL_CLAMP_TO_EDGE OpenGL always return their edge values whenever
			// we sample between faces.
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mWarpType);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mWarpType);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mWarpType);

			//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glActiveTexture(GL_TEXTURE0);
		}
	};

	class MaterialCubeFBO : public Material
	{
	public:
		MaterialCubeFBO()
		{
			mvpCubeFaceTextureBuffer.resize(6, 0);
		}

		~MaterialCubeFBO() {}

		//The input vector holds the six face texture of the cube,
		//following the order:right, left, top, bottom, back, front
		/*void setCubeFaceTextureBuffer(int layer, GLuint faceBuffer)
		{
			if (layer >= 0 && layer < 6)
			{
				mvpCubeFaceTextureBuffer[layer] = faceBuffer;
				glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeMapBuffer);
				glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + layer, faceBuffer);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}
		}*/

		void setCubeTextureBuffer1(GLuint cubeBuffer)
		{
			mCubeMapBuffer1 = cubeBuffer;
			int textureUnit = Tex_CUBE1;
			mmTexunitToTexbuffer[textureUnit] = mCubeMapBuffer1;
		}

		void setCubeTextureBuffer2(GLuint cubeBuffer)
		{
			mCubeMapBuffer2 = cubeBuffer;
			int textureUnit = Tex_CUBE2;
			mmTexunitToTexbuffer[textureUnit] = mCubeMapBuffer2;
		}

		virtual void uploadToDevice()
		{
			if (mbUploaded) return;

			Material::uploadToDevice();
		}

		virtual void active(const GLuint &programID,
							bool bValidTexCoord = true,
							bool bValidVertexColor = false)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current MaterialCube has not been uploaded befor activing");
				return;
			}

			std::map<TextureType, std::string> &nameMap =
				TextureGlobal::getInstance().TextypeToMaterialName;

			std::map<int, GLuint>::iterator iter_;
			for (iter_ = mmTexunitToTexbuffer.begin();
				 iter_ != mmTexunitToTexbuffer.end(); iter_++)
			{
				TextureType type = TextureType(iter_->first);

				int samplerLoc = glGetUniformLocation(programID, nameMap[type].c_str());
				glUniform1i(samplerLoc, int(iter_->first));

				glActiveTexture(GL_TEXTURE0 + iter_->first);
				//glBindTexture(GL_TEXTURE_2D, iter_->second);
				glBindTexture(GL_TEXTURE_CUBE_MAP, iter_->second);
			}
			glActiveTexture(GL_TEXTURE0);
		}

	private:
		//holds the six face texture of the cube, following the order:
		//right, left, top, bottom, back, front
		std::vector<GLuint> mvpCubeFaceTextureBuffer;

		GLuint mCubeMapBuffer1;
		GLuint mCubeMapBuffer2;
	};


	class MaterialText : public Material
	{
	public:
		MaterialText(GLuint lineHeight = 24,
					 glm::vec4 charColor = glm::vec4(1.0f))
			: mbValidText(false), mCharColor(charColor)
		{
			if (FT_Init_FreeType(&mFT))
			{
				SP_CERR("ERROR::FREETYPE: Could not init FreeType Library");
				return;
			}

			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			std::string FrontPath = __currentPATH + "/Fronts/arial.ttf";

			if (FT_New_Face(mFT, FrontPath.c_str(), 0, &mFace))
			{
				SP_CERR("ERROR::FREETYPE: Failed to load font from " + FrontPath);
				return;
			}

			FT_Set_Pixel_Sizes(mFace, 0, lineHeight);

			std::vector<unsigned char *> vpData;
			for (size_t i = 0; i < TEXT_CHAR_NUM; i++)
			{
				// Load character glyph 
				if (FT_Load_Char(mFace, i, FT_LOAD_RENDER))
				{
					SP_CERR("ERROR::FREETYTPE: Failed to load Glyph");
					return;
				}

				int width = mFace->glyph->bitmap.width;
				int height = mFace->glyph->bitmap.rows;
				int bearingX = mFace->glyph->bitmap_left;
				int bearingY = mFace->glyph->bitmap_top;
				int advance = mFace->glyph->advance.x;
				unsigned char *data = mFace->glyph->bitmap.buffer;

				int pixelCount = width * height;
				unsigned char *data_ = new unsigned char[pixelCount];
				memcpy(data_, data, pixelCount * sizeof(unsigned char));

				vpData.push_back(data_);
				mvGlyphSize.push_back({ width, height });
				mvGlyphBearing.push_back({ bearingX, bearingY });
				mvAdvance.push_back(advance >> 6);
			}

			int maxWidth = mvGlyphSize[0].x, maxHeight = mvGlyphSize[0].y;
			std::for_each(mvGlyphSize.begin(), mvGlyphSize.end(),
						  [&](const glm::vec2 &glyphSize) 
			{
				if (glyphSize.x > maxWidth) maxWidth = glyphSize.x;
				if (glyphSize.y > maxHeight) maxHeight = glyphSize.y;
			});

			for (size_t i = 0; i < TEXT_CHAR_NUM; i++)
			{
				int pixelCount = maxWidth * maxHeight;
				unsigned char *pUData = new unsigned char[pixelCount];
				memset(pUData, 0, pixelCount);

				int width = mvGlyphSize[i].x, height = mvGlyphSize[i].y;
				unsigned char *data = vpData[i];

				//Copy the origin bitmap to the new bitmap(left-top)
				//std::cout << " char = " << i << "(" << char(i) << "):" << std::endl;

				for (size_t m = 0, r = 0, ur = 0; m < height; 
					 m++, r+= width, ur += maxWidth)
				{
					for (size_t n = 0, c = 0, uc = 0; n < width;
						 n++, c++, uc++)
					{
						pUData[ur + uc] = data[r + c];
						//std::cout << std::setw(4) << int(pUData[ur + uc]);
						//std::cout << std::setw(4) << int(data[m*width + n]);
					}
					//std::cout << std::endl;
				}

				if (data) delete[] data;

				std::shared_ptr<unsigned char> pData(pUData,
													 [](unsigned char *d) 
				{
					delete[] d;
				});

				std::shared_ptr<Texture> pTexture = 
					std::make_shared<Texture>(pData, maxWidth, maxHeight, 
											  1, TextureType::Tex_DIFFUSE);

				addTexture(pTexture);
			}

			mUWidth = maxWidth;
			mUHeight = maxHeight;

			mUWidthInv = 1.0 / mUWidth;
			mUHeightInv = 1.0 / mUHeight;

			mbValidText = true;
		}

		bool isValidText()
		{
			return mbValidText;
		}

		~MaterialText() 
		{
			FT_Done_Face(mFace);
			FT_Done_FreeType(mFT);
		}

		void setCharColor(glm::vec4 charColor)
		{
			mCharColor = charColor;
		}

		//If the input c is not valid, return -1
		int getMaterialLayer(const char &c)
		{
			return c < 0 || c >= TEXT_CHAR_NUM ? -1 : int(c);
		}

		//Get the texcoord of the c after unifying texture
		bool getTexCoord(const char &c, glm::vec2 &lb, glm::vec2 &rb,
						 glm::vec2 &rt, glm::vec2 &lt)
		{
			if (c < 0 || c >= TEXT_CHAR_NUM)
			{
				SP_CERR("The input char is not valid now");
				return false;
			}

			int cID = int(c);
			int width = mvGlyphSize[cID].x, height = mvGlyphSize[cID].y;

			lb.x = 0.0f; lb.y = (mUHeight - height) * mUHeightInv;
			rb.x = width * mUWidthInv; rb.y = lb.y;
			rt.x = rb.x; rt.y = 1.0f;
			lt.x = 0.0f; lt.y = 1.0f;

			return true;
		}

		glm::vec2 getGlyphSize(const char &c)
		{
			if (c < 0 || c >= TEXT_CHAR_NUM)
			{
				SP_CERR("The input char is not valid now");
				return glm::vec2(-1.0f);
			}

			return mvGlyphSize[int(c)];
		}

		glm::vec2 getGlyphBearing(const char &c)
		{
			if (c < 0 || c >= TEXT_CHAR_NUM)
			{
				SP_CERR("The input char is not valid now");
				return glm::vec2(0.0f);;
			}

			return mvGlyphBearing[int(c)];
		}

		GLuint getGlyphAdvance(const char &c)
		{
			if (c < 0 || c >= TEXT_CHAR_NUM)
			{
				SP_CERR("The input char is not valid now");
				return 0;
			}

			return mvAdvance[int(c)];
		}


		virtual void uploadToDevice()
		{
			if (!mbValidText)
			{
				SP_CERR("The current MaterialText has not valid text set");
				return;
			}
			Material::uploadToDevice();
		}

		virtual void active(const GLuint &programID,
							bool bValidTexCoord = true,
							bool bValidVertexColor = false)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current MaterialText has not been uploaded befor activing");
				return;
			}

			uploadColor(programID, "uCharColor", mCharColor);

			std::map<TextureType, std::string> &nameMap =
				TextureGlobal::getInstance().TextypeToMaterialName;

			{
				int samplerLoc = glGetUniformLocation(programID, nameMap[Tex_DIFFUSE].c_str());
				glUniform1i(samplerLoc, Tex_DIFFUSE);
				//GL_DEBUG_ALL;
			}

			glActiveTexture(GL_TEXTURE0 + Tex_DIFFUSE);
			glBindTexture(GL_TEXTURE_2D_ARRAY, mmTexunitToTexbuffer[Tex_DIFFUSE]);
			glActiveTexture(GL_TEXTURE0);
		}

	private:
		FT_Library mFT;
		FT_Face mFace;

		//The uniform width and height for arranging array texture
		int mUWidth, mUHeight;
		float mUWidthInv, mUHeightInv;

		//vec2[0] = width, vec2[1] = height
		std::vector<glm::vec2> mvGlyphSize;

		// Offset from baseline to left/top of glyph
		std::vector<glm::vec2> mvGlyphBearing;

		// Offset to advance to next glyph
		std::vector<GLuint> mvAdvance;

		bool mbValidText;
		glm::vec4 mCharColor;

	};
}
