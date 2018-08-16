#pragma once
#include "utils.h"

namespace SP
{
	class VertexArrayUtil;
	class VertexArrayTcUtil;

	//The basic vertexarray, just holds the vertices, normals(, indices) and a global object color
	//And now we consider just the triangle drawing vertexarray
	class VertexArray
	{
	public:
		//VertexArray() = delete;
		~VertexArray() {}

		VertexArray(const std::vector<glm::vec3> &vertices,
					const std::vector<glm::vec3> &normals,
					const std::vector<GLuint> &indices = std::vector<GLuint>(),
					const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f))
			: mNumExistedAttri(2)
		{
			mpvVertice = std::make_shared<std::vector<glm::vec3>>(vertices);
			mpvNormal = std::make_shared<std::vector<glm::vec3>>(normals);
			mpvIndice = std::make_shared<std::vector<GLuint>>(indices);
			mColor = color;
		}

		VertexArray(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					const std::shared_ptr<std::vector<glm::vec3>> &pNormals,
					const std::shared_ptr<std::vector<GLuint>> &pIndices = 
					std::make_shared<std::vector<GLuint>>(),
					const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f))
			: mpvVertice(pVertices), mpvNormal(pNormals), mpvIndice(pIndices), 
			mColor(color), mNumExistedAttri(2) {}

		friend class VertexArrayUtil;
		
		virtual std::string getShaderMacros()
		{
			return std::string("#define UNIFORM_COLOR\n");
		}


		virtual std::shared_ptr<VertexArrayUtil> createUtil(const std::shared_ptr<VertexArray>
															&pVertexArray)
		{
			std::shared_ptr<VertexArrayUtil> pVertexArrayUtil;
			if (pVertexArray->IsUploaded())
			{
				pVertexArrayUtil = pVertexArray->mpVertexArrayUtil.lock();
			}
			else
			{
				pVertexArrayUtil = std::make_shared<VertexArrayUtil>(pVertexArray);
			}

			return pVertexArrayUtil;
		}

		//For indicating whether the VertexArray has been uploaded to the GPU memory
		void setVertexArrayUtil(const std::shared_ptr<VertexArrayUtil> &pVertexArrayUtil)
		{
			mpVertexArrayUtil = pVertexArrayUtil;
		}

		//Clearing the state of uploading to the GPU memory
		void resetVertexArrayUtil()
		{
			mpVertexArrayUtil.reset();
		}

		//Get if the VertexArray has been uploaded by accessing the mpVertexArrayUtil's state
		bool IsUploaded()
		{
			std::shared_ptr<VertexArrayUtil> pVAUtil = mpVertexArrayUtil.lock();
			return pVAUtil.use_count() != 0;
		}

		//Get the number of existed attributions
		int getNumExistedAttri()
		{
			return mNumExistedAttri;
		}

	protected:
		std::shared_ptr<std::vector<glm::vec3>> mpvVertice;
		std::shared_ptr<std::vector<glm::vec3>> mpvNormal;
		std::shared_ptr<std::vector<GLuint>> mpvIndice;
		glm::vec3 mColor;

		//The number of existed attributions;
		//For the VertexArray , there are only vertice and normal ,
		//So that the value for the VertexArray is 2
		int mNumExistedAttri;

		//For indicating whether the VertexArray has been uploaded to the GPU memory
		std::weak_ptr<VertexArrayUtil> mpVertexArrayUtil;
	};

	class VertexArrayUtil
	{
	public:
		VertexArrayUtil() = delete;

		VertexArrayUtil(const std::shared_ptr<VertexArray> &pVertexArray)
			: mpVertexArray(pVertexArray)
		{
			std::vector<glm::vec3> &vVertice = *(pVertexArray->mpvVertice);
			glGenBuffers(1, &mVerticeVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mVerticeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vVertice[0])*vVertice.size(), &vVertice[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			std::vector<glm::vec3> &vNormal = *(pVertexArray->mpvNormal);
			glGenBuffers(1, &mNormalVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vNormal[0])*vNormal.size(), &vNormal[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			mNumDrawVertice = vVertice.size();

			std::vector<GLuint> &vIndice = *(pVertexArray->mpvIndice);
			mbDrawElements = vIndice.empty() ? false : true;
			if (mbDrawElements)
			{
				glGenBuffers(1, &mEBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vIndice[0])*vIndice.size(), &vIndice[0], GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				mNumDrawVertice = vIndice.size();
			}

			glGenVertexArrays(1, &mVAO);
			{
				glBindVertexArray(mVAO);
				glBindBuffer(GL_ARRAY_BUFFER, mVerticeVBO);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
				glEnableVertexAttribArray(1);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
			}
			glBindVertexArray(0);

			mbUploadUColor = mpVertexArray->getShaderMacros() == std::string("#define UNIFORM_COLOR\n");
		}

		~VertexArrayUtil()
		{
			glDeleteVertexArrays(1, &mVAO);
			glDeleteBuffers(1, &mVerticeVBO);
			glDeleteBuffers(1, &mNormalVBO);
			if (mbDrawElements)
			{
				glDeleteBuffers(1, &mEBO);
			}
		}

		virtual void draw()
		{
			if (mbUploadUColor)
			{
				GLint programID;
				glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
				GLint uColorLoc = glGetUniformLocation(programID, "uColor");
				glm::vec3 &uColor = mpVertexArray->mColor;
				glUniform3f(uColorLoc, uColor.r, uColor.g, uColor.b);
			}

			glBindVertexArray(mVAO);
			if (mbDrawElements)
			{
				glDrawElements(GL_TRIANGLES, mNumDrawVertice, GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, 0, mNumDrawVertice);
			}
		}

		virtual void drawInstanced(int count)
		{
			if (count <= 0) return;

			if (mbUploadUColor)
			{
				GLint programID;
				glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
				GLint uColorLoc = glGetUniformLocation(programID, "uColor");
				glm::vec3 &uColor = mpVertexArray->mColor;
				glUniform3f(uColorLoc, uColor.r, uColor.g, uColor.b);
			}

			glBindVertexArray(mVAO);
			if (mbDrawElements)
			{
				glDrawElementsInstanced(GL_TRIANGLES, mNumDrawVertice, GL_UNSIGNED_INT, 0, count);
			}
			else
			{
				glDrawArraysInstanced(GL_TRIANGLES, 0, mNumDrawVertice, count);
			}
		}

		GLuint getVAO()
		{
			return	mVAO;
		}

		//Set the state of whether to upload the ucolor
		void setUColor(bool bUploadUColor)
		{
			mbUploadUColor = bUploadUColor;
		}

	protected:
		std::shared_ptr<VertexArray> mpVertexArray;

		GLuint mVAO;
		GLuint mVerticeVBO, mNormalVBO, mEBO;

		bool mbDrawElements;
		bool mbUploadUColor;

		int mNumDrawVertice;
	};

	//The vertexarray holds the texture coords
	class VertexArrayTc : public VertexArray
	{
	public:
		VertexArrayTc() = delete;
		~VertexArrayTc() {}

		VertexArrayTc(const std::vector<glm::vec3> &vertices,
					  const std::vector<glm::vec3> &normals,
					  const std::vector<glm::vec2> &texcoords,
					  const std::vector<GLuint> &indices = std::vector<GLuint>())
			: VertexArray(vertices, normals, indices)
		{
			mpvTexCoord = std::make_shared<std::vector<glm::vec2>>(texcoords);
			mNumExistedAttri++;
		}

		VertexArrayTc(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					  const std::shared_ptr<std::vector<glm::vec3>> &pNormals,
					  const std::shared_ptr<std::vector<glm::vec2>> &pTexcoords,
					  const std::shared_ptr<std::vector<GLuint>> &pIndices =
					  std::make_shared<std::vector<GLuint>>())
			: VertexArray(pVertices, pNormals, pIndices), mpvTexCoord(pTexcoords)
		{
			mNumExistedAttri++;
		}

		friend class VertexArrayTcUtil;

		virtual std::string getShaderMacros()
		{
			return std::string("#define HAVE_TEXCOORD\n");
		}

		virtual std::shared_ptr<VertexArrayUtil> createUtil(const std::shared_ptr<VertexArray>
															&pVertexArray)
		{
			std::string VertexArrayTcName = typeid(VertexArrayTc).name();
			std::string currentVAName = typeid(*pVertexArray).name();
			if (VertexArrayTcName != currentVAName)
			{
				SP_CERR("The current type of pVertexArray (" + currentVAName + ") is \
						not consistent with the VertexArrayTc type (" + VertexArrayTcName
						+ ")");
				exit(-1);
			}

			std::shared_ptr<VertexArrayTc> pVertexArrayTc =
				std::dynamic_pointer_cast<VertexArrayTc>(pVertexArray);

			std::shared_ptr<VertexArrayUtil> pVertexArrayUtil;

			if (pVertexArrayTc->IsUploaded())
			{
				pVertexArrayUtil = pVertexArrayTc->mpVertexArrayUtil.lock();
			}
			else
			{
				std::shared_ptr<VertexArrayTcUtil> pVertexArrayTcUtil = 
					std::make_shared<VertexArrayTcUtil>(pVertexArrayTc);

				pVertexArrayUtil = std::static_pointer_cast<VertexArrayUtil>(pVertexArrayTcUtil);
			}
				
			return pVertexArrayUtil;
		}

	protected:
		std::shared_ptr<std::vector<glm::vec2>> mpvTexCoord;
	};

	class VertexArrayTcUtil : public VertexArrayUtil
	{
	public:
		VertexArrayTcUtil() = delete;

		//The pVertexArray must point to the VertexArrayTI
		VertexArrayTcUtil(const std::shared_ptr<VertexArrayTc> &pVertexArrayTc)
			: VertexArrayUtil(pVertexArrayTc)
		{
			
			std::vector<glm::vec2> &vTexCoord = *(pVertexArrayTc->mpvTexCoord);
			glGenBuffers(1, &mTexCoordVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mTexCoordVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoord[0])*vTexCoord.size(), &vTexCoord[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(mVAO);
			{
				glBindBuffer(GL_ARRAY_BUFFER, mTexCoordVBO);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);
				glEnableVertexAttribArray(2);
			}
			glBindVertexArray(0);

		}

		~VertexArrayTcUtil()
		{
			glDeleteVertexArrays(1, &mVAO);
			glDeleteBuffers(1, &mTexCoordVBO);
		}

	protected:
		GLuint mTexCoordVBO;
	};
}