#pragma once
#include "Texture.h"

namespace SP
{
	class MaterialUtil;

	class Material
	{
	public:
		Material() : mNumTextures(0), mDiffuseColor(glm::vec4(1.0f)),
			mAmbientColor(glm::vec4(1.0f)), mSpecularColor(glm::vec4(0.0f)),
			mShininess(DEFAULT_SHININESS) {}

		Material(const glm::vec4 &diffuseColor, const glm::vec4 &ambientColor,
				 const glm::vec4 &specularColor, float shininess = DEFAULT_SHININESS)
			: mNumTextures(0), mDiffuseColor(diffuseColor), mAmbientColor(ambientColor),
			mSpecularColor(specularColor), mShininess(shininess) {}

		Material(const glm::vec4 &diffAndambiColor, const glm::vec4 &specularColor,
				 float shininess = DEFAULT_SHININESS)
			: mNumTextures(0), mDiffuseColor(diffAndambiColor),
			mAmbientColor(diffAndambiColor), mSpecularColor(specularColor),
			mShininess(shininess) {}

		Material(const glm::vec4 &uniformColor, float shininess = DEFAULT_SHININESS)
			: mNumTextures(0), mDiffuseColor(uniformColor), mAmbientColor(uniformColor),
			mSpecularColor(uniformColor), mShininess(shininess) {}

		~Material() {}

		friend class MaterialUtil;

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
			if (mmpTypeTexture.find(curType) == mmpTypeTexture.end())
			{
				mmpTypeTexture[curType] = std::vector<std::shared_ptr<Texture>>();
			}
			std::vector<std::shared_ptr<Texture>> &vpTexture = mmpTypeTexture[curType];

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
			mNumTextures++;
		}

		int getTypeNum(TextureType type)
		{
			if (mmpTypeTexture.find(type) != mmpTypeTexture.end())
			{
				return mmpTypeTexture[type].size();
			}
			else
			{
				return 0;
			}

		}

		int getAllTexturesNum()
		{
			return mNumTextures;
		}

		std::vector<std::shared_ptr<Texture>> getTextures(TextureType type)
		{
			std::vector<std::shared_ptr<Texture>> vpTexture;
			if (mmpTypeTexture.find(type) != mmpTypeTexture.end())
			{
				vpTexture = mmpTypeTexture[type];
			}
			
			return vpTexture;
		}
		
		virtual std::string getShaderMacros()
		{
			std::string macros = mNumTextures == 0 ? 
				"" : "#define HAVE_TEXTURE\n";

			std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;
			for (iter = mmpTypeTexture.begin(); iter != mmpTypeTexture.end(); iter++)
			{
				macros += TextureGlobal::getInstance().typeMacroMap[iter->first];
			}

			return macros;
		}

		virtual std::shared_ptr<MaterialUtil> createUtil(const std::shared_ptr<Material>
															&pMaterial)
		{
			std::shared_ptr<MaterialUtil> pMaterialUtil;
			if (pMaterial->IsUploaded())
			{
				pMaterialUtil = pMaterial->mpMaterialUtil.lock();
			}
			else
			{
				pMaterialUtil = std::make_shared<MaterialUtil>(pMaterial);
			}

			return pMaterialUtil;
		}

		//For indicating whether the material has been uploaded to the GPU memory
		void setMaterialUtil(const std::shared_ptr<MaterialUtil> &pMaterialUtil)
		{
			mpMaterialUtil = pMaterialUtil;
		}

		//Clearing the state of uploading to the GPU memory
		void resetMaterialUtil()
		{
			mpMaterialUtil.reset();
		}

		//Get if the material has been uploaded by accessing the mpMaterialUtil's state
		bool IsUploaded()
		{
			std::shared_ptr<MaterialUtil> pMaterialUtil = mpMaterialUtil.lock();
			return pMaterialUtil.use_count() != 0;
		}

	protected:
		std::map<TextureType, std::vector<std::shared_ptr<Texture>>> mmpTypeTexture;

		glm::vec4 mDiffuseColor, mAmbientColor;
		glm::vec4 mSpecularColor;
		GLfloat mShininess;

		//For indicating whether the material has been uploaded to the GPU memory
		std::weak_ptr<MaterialUtil> mpMaterialUtil;

		int mNumTextures;
	};

	class MaterialUtil
	{
	public:
		MaterialUtil() = delete;

		MaterialUtil(const std::shared_ptr<Material> &pMaterial)
			: mpMaterial(pMaterial)
		{
			std::map<TextureType, std::vector<std::shared_ptr<Texture>>> &mpTypeTexture =
				mpMaterial->mmpTypeTexture;
			std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;

			for (iter = mpTypeTexture.begin(); iter != mpTypeTexture.end(); iter++)
			{
				int textureUnit = iter->first;
				glActiveTexture(GL_TEXTURE0 + textureUnit);

				int width = iter->second[0]->getWidth();
				int height = iter->second[0]->getHeight();
				int layerCount = iter->second.size();

				GLuint texture;
				glGenTextures(1, &texture);
				mmTextureUnit[textureUnit] = texture;

				glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
				int levelNum = floor(log2(std::max(width, height))) + 1;
				glTexStorage3D(GL_TEXTURE_2D_ARRAY, levelNum, GL_RGB8, width, height, layerCount);

				for (size_t i = 0; i < layerCount; i++)
				{
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1,
									GL_RGB, GL_UNSIGNED_BYTE, iter->second[i]->getData());
				}

				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
				
				glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

				//Test the 2D texture
				/*glBindTexture(GL_TEXTURE_2D, texture);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, iter->second[0]->getData());
				int levles = floor(log2(std::max(width, height))) + 1;
				glTexStorage2D(GL_TEXTURE_2D, levles, GL_RGB8, width, height);

				for (size_t i = 0; i < layerCount; i++)
				{
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
									GL_RGB, GL_UNSIGNED_BYTE, iter->second[i]->getData());
				}

				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glBindTexture(GL_TEXTURE_2D, 0);*/
			}
			glActiveTexture(GL_TEXTURE0);

			//Set the existence of texture maps
			mbDiffuseMap = mpTypeTexture.find(Tex_DIFFUSE) != mpTypeTexture.end();
			mbAmbientMap = mpTypeTexture.find(Tex_AMBIENT) != mpTypeTexture.end();
			mbSpecularMap = mpTypeTexture.find(Tex_SPECULAR) != mpTypeTexture.end();
		}

		~MaterialUtil () 
		{
			std::map<int, GLuint>::iterator iter;
			for (iter = mmTextureUnit.begin(); iter != mmTextureUnit.end(); iter++)
			{
				glDeleteTextures(1, &(iter->second));
			}
		}

		virtual std::string getShaderMacros()
		{
			return mpMaterial->getShaderMacros();
		}

		void activeMaterial(bool bValidTexCoord = true,
							bool bValidVertexColor = false)
		{
			GLint programID;
			glGetIntegerv(GL_CURRENT_PROGRAM, &programID);

			GLint uShininessLoc = glGetUniformLocation(programID, "material.uShininess");
			glUniform1f(uShininessLoc, mpMaterial->mShininess);

			if (bValidTexCoord)
			{
				std::map<TextureType, std::string> &nameMap = TextureGlobal::getInstance().typeMaterialNameMap;
				std::map<TextureType, std::vector<std::shared_ptr<Texture>>>::iterator iter;
				for (iter = mpMaterial->mmpTypeTexture.begin();
					 iter != mpMaterial->mmpTypeTexture.end(); iter++)
				{
					int samplerLoc = glGetUniformLocation(programID, nameMap[iter->first].c_str());
					glUniform1i(samplerLoc, int(iter->first));
				}

				std::map<int, GLuint>::iterator iter_;
				for (iter_ = mmTextureUnit.begin(); iter_ != mmTextureUnit.end(); iter_++)
				{
					glActiveTexture(GL_TEXTURE0 + iter_->first);
					//glBindTexture(GL_TEXTURE_2D, iter_->second);
					glBindTexture(GL_TEXTURE_2D_ARRAY, iter_->second);
				}
				glActiveTexture(GL_TEXTURE0);
			}
			else if(bValidVertexColor)
			{
				//When there is no texture used, but there is vertex color array
				//We dont need to upload any diffuse, ambient or specular
				return;
			}
			
			bool bDiffuse = !(mbDiffuseMap && bValidTexCoord);
			bool bAmbient = !(mbAmbientMap && bValidTexCoord) && bDiffuse;
			bool bSpecular = !(mbSpecularMap && bValidTexCoord);

			if (bDiffuse)
			{
				GLint uDiffuseLoc = glGetUniformLocation(programID, "material.uDiffuse");
				glm::vec4 &uColor = mpMaterial->mDiffuseColor;
				glUniform4f(uDiffuseLoc, uColor.r, uColor.g, uColor.b, uColor.a);
			}
			if (bAmbient)
			{
				GLint uAmbientLoc = glGetUniformLocation(programID, "material.uAmbient");
				glm::vec4 &uColor = mpMaterial->mAmbientColor;
				glUniform4f(uAmbientLoc, uColor.r, uColor.g, uColor.b, uColor.a);
			}
			if (bSpecular)
			{
				GLint uSpecularLoc = glGetUniformLocation(programID, "material.uSpecular");
				glm::vec4 &uColor = mpMaterial->mSpecularColor;
				glUniform4f(uSpecularLoc, uColor.r, uColor.g, uColor.b, uColor.a);
			}
		}

	protected:
		std::shared_ptr<Material> mpMaterial;

		//Indicate whether there is a *color map
		bool mbDiffuseMap, mbAmbientMap, mbSpecularMap;

		//The map between the texture unit and the texture;
		std::map<int, GLuint> mmTextureUnit;
	};
}
