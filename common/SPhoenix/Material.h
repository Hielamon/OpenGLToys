#pragma once
#include "Texture.h"

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
			mShininess(DEFAULT_SHININESS), mbUploaded(false) {}

		Material(const glm::vec4 &diffuseColor, const glm::vec4 &ambientColor,
				 const glm::vec4 &specularColor, float shininess = DEFAULT_SHININESS)
			: mTextureTotalCount(0), mDiffuseColor(diffuseColor), mAmbientColor(ambientColor),
			mSpecularColor(specularColor), mShininess(shininess), mbUploaded(false) {}

		Material(const glm::vec4 &diffAndambiColor, const glm::vec4 &specularColor,
				 float shininess = DEFAULT_SHININESS)
			: mTextureTotalCount(0), mDiffuseColor(diffAndambiColor),
			mAmbientColor(diffAndambiColor), mSpecularColor(specularColor),
			mShininess(shininess), mbUploaded(false) {}

		Material(const glm::vec4 &uniformColor, float shininess = DEFAULT_SHININESS)
			: mTextureTotalCount(0), mDiffuseColor(uniformColor), mAmbientColor(uniformColor),
			mSpecularColor(uniformColor), mShininess(shininess), mbUploaded(false) {}

		~Material()
		{
			clearUploaded();
		}

		void setShininess(const GLfloat& shininess)
		{
			mShininess = shininess;
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
					pTexture->getWidth() != pLastTexture->getWidth())
				{
					SP_CERR("The texture size for the same type in a material is not consistent");
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
		//mTextypeToMacro of the TextureGlobal
		virtual std::string getShaderMacros()
		{

			std::string macros = "";

			if (mTextureTotalCount == 0) return macros;

			macros += "#define HAVE_TEXTURE\n";

			std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;
			for (iter = mmTextypeToTex.begin(); iter != mmTextypeToTex.end(); iter++)
			{
				macros += TextureGlobal::getInstance().mTextypeToMacro[iter->first];
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
			mbDiffuseMap = mmTextypeToTex.find(Tex_DIFFUSE) != mmTextypeToTex.end();
			mbAmbientMap = mmTextypeToTex.find(Tex_AMBIENT) != mmTextypeToTex.end();
			mbSpecularMap = mmTextypeToTex.find(Tex_SPECULAR) != mmTextypeToTex.end();

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

			if (bValidTexCoord)
			{
				std::map<TextureType, std::string> &nameMap = TextureGlobal::getInstance().mTextypeToMaterialName;
				std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;
				for (iter = mmTextypeToTex.begin(); iter != mmTextypeToTex.end(); iter++)
				{
					int samplerLoc = glGetUniformLocation(programID, nameMap[iter->first].c_str());
					glUniform1i(samplerLoc, int(iter->first));
				}

				std::map<int, GLuint>::iterator iter_;
				for (iter_ = mmTexunitToTexbuffer.begin();
					 iter_ != mmTexunitToTexbuffer.end(); iter_++)
				{
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

			if (bDiffuse) _uploadColor(programID, "material.uDiffuse", mDiffuseColor);
			if (bAmbient) _uploadColor(programID, "material.uAmbient", mAmbientColor);
			if (bSpecular) _uploadColor(programID, "material.uSpecular", mSpecularColor);
		}

	protected:
		//The map stores the texture as the Type-TextureVector pairs
		std::map<TextureType, std::vector<std::shared_ptr<Texture>>> mmTextypeToTex;

		//The uniform color arguments for Blinn-Phong lighting model
		glm::vec4 mDiffuseColor, mAmbientColor, mSpecularColor;
		GLfloat mShininess;

		//Record the total texture counts
		int mTextureTotalCount;

		//****** The inner variable for device ******//

		bool mbUploaded;

		//Indicate whether there is a *color map
		bool mbDiffuseMap, mbAmbientMap, mbSpecularMap;

		//The map between the texture unit and the texture buffer name;
		std::map<int, GLuint> mmTexunitToTexbuffer;

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
			int layerCount = vpTexture.size();

			GLuint textureBuffer;
			glGenTextures(1, &textureBuffer);
			mmTexunitToTexbuffer[textureUnit] = textureBuffer;

			glBindTexture(GL_TEXTURE_2D_ARRAY, textureBuffer);
			int levelNum = floor(log2(std::max(width, height))) + 1;
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, levelNum, GL_RGB8, width, height, layerCount);

			for (size_t i = 0; i < layerCount; i++)
			{
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1,
								GL_RGB, GL_UNSIGNED_BYTE, vpTexture[i]->getData());
			}

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			glActiveTexture(GL_TEXTURE0);
		}

		//upload the color uniform variable
		void _uploadColor(GLint programID, const char * varname, const glm::vec4 &color)
		{
			GLint uColorLoc = glGetUniformLocation(programID, varname);
			glUniform4f(uColorLoc, color.r, color.g, color.b, color.a);
		}
	};
}
