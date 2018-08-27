#pragma once
#include "utils.h"
#include "BBox.h"

namespace SP
{
	class VertexArrayUtil;
	class VertexArrayTcUtil;

	enum PrimitiveType
	{
		POINTS, LINES, LINE_STRIP, TRIANGLES
	};

	//some global variables for vertex array:
	//such as the map between the PrimitiveType and the GL model
	class VAGlobal
	{
	public:
		~VAGlobal() {}

		static VAGlobal& getInstance()
		{
			static VAGlobal vaglobal;
			return vaglobal;
		}

		//The map between our primitive type and the opengl mode
		std::map<PrimitiveType, GLenum> PTypeModeMap;

	private:
		VAGlobal()
		{
			PTypeModeMap = {
				{ POINTS, GL_POINTS },
				{ LINES, GL_LINES },
				{ LINE_STRIP, GL_LINE_STRIP },
				{ TRIANGLES, GL_TRIANGLES }
			};
		}
	};

	//The basic vertexarray, just holds the vertices(, indices) and a global object color
	class VertexArray
	{
	public:
		~VertexArray() {}
		
		VertexArray(const std::vector<glm::vec3> &vertices, 
					const std::vector<GLuint> &indices = std::vector<GLuint>(),
					const PrimitiveType &pType = TRIANGLES,
					const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f))
			: mNumExistedAttri(1), mPrimitiveType(pType)
		{
			mpvVertice = std::make_shared<std::vector<glm::vec3>>(vertices);
			mpvIndice = std::make_shared<std::vector<GLuint>>(indices);
			mColor = color;
		}

		VertexArray(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					const std::shared_ptr<std::vector<GLuint>> &pIndices = 
					std::make_shared<std::vector<GLuint>>(),
					const PrimitiveType &pType = TRIANGLES,
					const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f))
			: mpvVertice(pVertices), mpvIndice(pIndices), 
			mColor(color), mNumExistedAttri(1), mPrimitiveType(pType) {}

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

		//Get the number of existed attributions
		int getNumExistedAttri()
		{
			return mNumExistedAttri;
		}

		//Get the bounding box
		BBox getBoundingBox()
		{
			return BBox(*mpvVertice);
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

	protected:
		VertexArray() {};

		std::shared_ptr<std::vector<glm::vec3>> mpvVertice;
		std::shared_ptr<std::vector<GLuint>> mpvIndice;
		glm::vec3 mColor;
		PrimitiveType mPrimitiveType;

		//The number of existed attributions;
		//For the VertexArray , there are only vertice ,
		//So that the value for the VertexArray is 1
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
			std::map<PrimitiveType, GLenum> &typeMap = VAGlobal::getInstance().PTypeModeMap;
			mDrawMode = typeMap[pVertexArray->mPrimitiveType];

			std::vector<glm::vec3> &vVertice = *(pVertexArray->mpvVertice);
			glGenBuffers(1, &mVerticeVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mVerticeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vVertice[0])*vVertice.size(), &vVertice[0], GL_STATIC_DRAW);
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
			glBindVertexArray(mVAO);
			{
				//GLuint attriIndex = pVertexArray->mNumExistedAttri - 1;
				glBindBuffer(GL_ARRAY_BUFFER, mVerticeVBO);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
			}
			glBindVertexArray(0);
		}

		~VertexArrayUtil()
		{
			glDeleteVertexArrays(1, &mVAO);
			glDeleteBuffers(1, &mVerticeVBO);
			if (mbDrawElements)
			{
				glDeleteBuffers(1, &mEBO);
			}
		}

		virtual void draw()
		{
			glBindVertexArray(mVAO);
			if (mbDrawElements)
			{
				glDrawElements(mDrawMode, mNumDrawVertice, GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(mDrawMode, 0, mNumDrawVertice);
			}
		}

		virtual void drawInstanced(int count)
		{
			if (count <= 0) return;

			glBindVertexArray(mVAO);
			if (mbDrawElements)
			{
				glDrawElementsInstanced(mDrawMode, mNumDrawVertice, GL_UNSIGNED_INT, 0, count);
			}
			else
			{
				glDrawArraysInstanced(mDrawMode, 0, mNumDrawVertice, count);
			}
		}

		GLuint getVAO()
		{
			return	mVAO;
		}

		const glm::vec3 & getUniformColor()
		{
			return mpVertexArray->mColor;
		}

	protected:
		std::shared_ptr<VertexArray> mpVertexArray;

		GLuint mVAO;
		GLuint mVerticeVBO, mNormalVBO, mEBO;
		GLenum mDrawMode;

		bool mbDrawElements;

		int mNumDrawVertice;
	};

	//The vertexarray holds normals
	class VertexArrayN : public VertexArray
	{
	public:
		~VertexArrayN() {}

		VertexArrayN(const std::vector<glm::vec3> &vertices,
					 const std::vector<glm::vec3> &normals,
					 const std::vector<GLuint> &indices = std::vector<GLuint>(),
					 const PrimitiveType &pType = TRIANGLES,
					 const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f))
			: VertexArray(vertices, indices, pType, color)
		{
			mpvNormal = std::make_shared<std::vector<glm::vec3>>(normals);
			mNumExistedAttri++;
		}

		VertexArrayN(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					 const std::shared_ptr<std::vector<glm::vec3>> &pNormals,
					 const std::shared_ptr<std::vector<GLuint>> &pIndices =
					 std::make_shared<std::vector<GLuint>>(),
					 const PrimitiveType &pType = TRIANGLES,
					 const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f))
			: VertexArray(pVertices, pIndices, pType, color), mpvNormal(pNormals)
		{
			mNumExistedAttri++;
		}

		friend class VertexArrayNUtil;

		virtual std::string getShaderMacros()
		{
			return VertexArray::getShaderMacros() + 
				std::string("#define HAVE_NORMAL\n#define NORMAL_ATTR 1\n");
		}

		virtual std::shared_ptr<VertexArrayUtil> createUtil(const std::shared_ptr<VertexArray>
															&pVertexArray)
		{
			std::string VertexArrayNName = typeid(VertexArrayN).name();
			std::string currentVAName = typeid(*pVertexArray).name();
			if (VertexArrayNName != currentVAName)
			{
				SP_CERR("The current type of pVertexArray (" + currentVAName + ") is \
						not consistent with the VertexArrayN type (" + VertexArrayNName
						+ ")");
				exit(-1);
			}

			std::shared_ptr<VertexArrayN> pVertexArrayN =
				std::dynamic_pointer_cast<VertexArrayN>(pVertexArray);

			std::shared_ptr<VertexArrayUtil> pVertexArrayUtil;
			if (pVertexArrayN->IsUploaded())
			{
				pVertexArrayUtil = pVertexArrayN->mpVertexArrayUtil.lock();
			}
			else
			{
				std::shared_ptr<VertexArrayNUtil> pVertexArrayNUtil =
					std::make_shared<VertexArrayNUtil>(pVertexArrayN);

				pVertexArrayUtil = std::static_pointer_cast<VertexArrayUtil>(pVertexArrayNUtil);
			}

			return pVertexArrayUtil;
		}

	protected:
		VertexArrayN() {};

		std::shared_ptr<std::vector<glm::vec3>> mpvNormal;
	};

	class VertexArrayNUtil : public VertexArrayUtil
	{
	public:
		VertexArrayNUtil() = delete;

		VertexArrayNUtil(const std::shared_ptr<VertexArrayN> &pVertexArrayN)
			: VertexArrayUtil(std::static_pointer_cast<VertexArray>(pVertexArrayN))
		{
			std::vector<glm::vec3> &vNormal = *(pVertexArrayN->mpvNormal);
			glGenBuffers(1, &mNormalVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vNormal[0])*vNormal.size(), &vNormal[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(mVAO);
			{
				//GLuint attriIndex = pVertexArrayN->mNumExistedAttri - 1;
				glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
				glEnableVertexAttribArray(1);
			}
			glBindVertexArray(0);
		}

		~VertexArrayNUtil()
		{
			glDeleteBuffers(1, &mNormalVBO);
		}

	protected:
		GLuint mNormalVBO;
	};

	//The vertexarray holds normals and texture coords
	class VertexArrayNTc : public VertexArrayN
	{
	public:
		~VertexArrayNTc() {}

		VertexArrayNTc(const std::vector<glm::vec3> &vertices,
					   const std::vector<glm::vec3> &normals,
					   const std::vector<glm::vec2> &texcoords,
					   const std::vector<GLuint> &indices = std::vector<GLuint>(),
					   const PrimitiveType &pType = TRIANGLES)
			: VertexArrayN(vertices, normals, indices, pType)
		{
			mpvTexCoord = std::make_shared<std::vector<glm::vec2>>(texcoords);
			mNumExistedAttri++;
		}

		VertexArrayNTc(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					   const std::shared_ptr<std::vector<glm::vec3>> &pNormals,
					   const std::shared_ptr<std::vector<glm::vec2>> &pTexcoords,
					   const std::shared_ptr<std::vector<GLuint>> &pIndices =
					   std::make_shared<std::vector<GLuint>>(),
					   const PrimitiveType &pType = TRIANGLES)
			: VertexArrayN(pVertices, pNormals, pIndices, pType), mpvTexCoord(pTexcoords)
		{
			mNumExistedAttri++;
		}

		friend class VertexArrayNTcUtil;

		virtual std::string getShaderMacros()
		{
			return VertexArrayN::getShaderMacros() + 
				std::string("#define HAVE_TEXCOORD\n#define TEXCOORD_ATTR 2\n");
		}

		virtual std::shared_ptr<VertexArrayUtil> createUtil(const std::shared_ptr<VertexArray>
															&pVertexArray)
		{
			std::string VertexArrayNTcName = typeid(VertexArrayNTc).name();
			std::string currentVAName = typeid(*pVertexArray).name();
			if (VertexArrayNTcName != currentVAName)
			{
				SP_CERR("The current type of pVertexArray (" + currentVAName + ") is \
						not consistent with the VertexArrayNTc type (" + VertexArrayNTcName
						+ ")");
				exit(-1);
			}

			std::shared_ptr<VertexArrayNTc> pVertexArrayNTc =
				std::dynamic_pointer_cast<VertexArrayNTc>(pVertexArray);

			std::shared_ptr<VertexArrayUtil> pVertexArrayUtil;

			if (pVertexArrayNTc->IsUploaded())
			{
				pVertexArrayUtil = pVertexArrayNTc->mpVertexArrayUtil.lock();
			}
			else
			{
				std::shared_ptr<VertexArrayNTcUtil> pVertexArrayNTcUtil =
					std::make_shared<VertexArrayNTcUtil>(pVertexArrayNTc);

				pVertexArrayUtil = std::static_pointer_cast<VertexArrayUtil>(pVertexArrayNTcUtil);
			}
				
			return pVertexArrayUtil;
		}

	protected:
		VertexArrayNTc() {};

		std::shared_ptr<std::vector<glm::vec2>> mpvTexCoord;
	};

	class VertexArrayNTcUtil : public VertexArrayNUtil
	{
	public:
		VertexArrayNTcUtil() = delete;

		//The pVertexArray must point to the VertexArrayTI
		VertexArrayNTcUtil(const std::shared_ptr<VertexArrayNTc> &pVertexArrayNTc)
			: VertexArrayNUtil(std::static_pointer_cast<VertexArrayN>(pVertexArrayNTc))
		{
			std::vector<glm::vec2> &vTexCoord = *(pVertexArrayNTc->mpvTexCoord);
			glGenBuffers(1, &mTexCoordVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mTexCoordVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoord[0])*vTexCoord.size(), &vTexCoord[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(mVAO);
			{
				//GLuint attriIndex = pVertexArrayNTc->mNumExistedAttri - 1;
				glBindBuffer(GL_ARRAY_BUFFER, mTexCoordVBO);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);
				glEnableVertexAttribArray(2);
			}
			glBindVertexArray(0);
		}

		~VertexArrayNTcUtil()
		{
			glDeleteBuffers(1, &mTexCoordVBO);
		}

	protected:
		GLuint mTexCoordVBO;
	};

	//The vertexarray holds normals and colors
	class VertexArrayNC : public VertexArrayN
	{
	public:
		~VertexArrayNC() {}

		VertexArrayNC(const std::vector<glm::vec3> &vertices,
					   const std::vector<glm::vec3> &normals,
					  const std::vector<glm::vec3> &colors,
					   const std::vector<GLuint> &indices = std::vector<GLuint>(),
					   const PrimitiveType &pType = TRIANGLES)
			: VertexArrayN(vertices, normals, indices, pType)
		{
			mpvColors = std::make_shared<std::vector<glm::vec3>>(colors);
			mNumExistedAttri++;
		}

		VertexArrayNC(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					   const std::shared_ptr<std::vector<glm::vec3>> &pNormals,
					  const std::shared_ptr<std::vector<glm::vec3>> &pColors,
					   const std::shared_ptr<std::vector<GLuint>> &pIndices =
					   std::make_shared<std::vector<GLuint>>(),
					   const PrimitiveType &pType = TRIANGLES)
			: VertexArrayN(pVertices, pNormals, pIndices, pType), mpvColors(pColors)
		{
			mNumExistedAttri++;
		}

		friend class VertexArrayNCUtil;

		virtual std::string getShaderMacros()
		{
			return VertexArrayN::getShaderMacros() +
				std::string("#define HAVE_COLOR\n#define COLOR_ATTR 2\n");
		}

		virtual std::shared_ptr<VertexArrayUtil> createUtil(const std::shared_ptr<VertexArray>
															&pVertexArray)
		{
			std::string VertexArrayNCName = typeid(VertexArrayNC).name();
			std::string currentVAName = typeid(*pVertexArray).name();
			if (VertexArrayNCName != currentVAName)
			{
				SP_CERR("The current type of pVertexArray (" + currentVAName + ") is \
						not consistent with the VertexArrayNC type (" + VertexArrayNCName
						+ ")");
				exit(-1);
			}

			std::shared_ptr<VertexArrayNC> pVertexArrayNC =
				std::dynamic_pointer_cast<VertexArrayNC>(pVertexArray);

			std::shared_ptr<VertexArrayUtil> pVertexArrayUtil;

			if (pVertexArrayNC->IsUploaded())
			{
				pVertexArrayUtil = pVertexArrayNC->mpVertexArrayUtil.lock();
			}
			else
			{
				std::shared_ptr<VertexArrayNCUtil> pVertexArrayNCUtil =
					std::make_shared<VertexArrayNCUtil>(pVertexArrayNC);

				pVertexArrayUtil = std::static_pointer_cast<VertexArrayUtil>(pVertexArrayNCUtil);
			}

			return pVertexArrayUtil;
		}

	protected:
		VertexArrayNC() {};

		std::shared_ptr<std::vector<glm::vec3>> mpvColors;
	};

	class VertexArrayNCUtil : public VertexArrayNUtil
	{
	public:
		VertexArrayNCUtil() = delete;

		//The pVertexArray must point to the VertexArrayTI
		VertexArrayNCUtil(const std::shared_ptr<VertexArrayNC> &pVertexArrayNC)
			: VertexArrayNUtil(std::static_pointer_cast<VertexArrayN>(pVertexArrayNC))
		{
			std::vector<glm::vec3> &vColor = *(pVertexArrayNC->mpvColors);
			glGenBuffers(1, &mColorVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vColor[0])*vColor.size(), &vColor[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(mVAO);
			{
				//GLuint attriIndex = pVertexArrayC->mNumExistedAttri - 1;
				glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
				glEnableVertexAttribArray(2);
			}
			glBindVertexArray(0);
		}

		~VertexArrayNCUtil()
		{
			glDeleteBuffers(1, &mColorVBO);
		}

	protected:
		GLuint mColorVBO;
	};

	//The vertexarray holds texture coords
	class VertexArrayTc : public VertexArray
	{
	public:
		~VertexArrayTc() {}

		VertexArrayTc(const std::vector<glm::vec3> &vertices,
					   const std::vector<glm::vec2> &texcoords,
					   const std::vector<GLuint> &indices = std::vector<GLuint>(),
					   const PrimitiveType &pType = TRIANGLES)
			: VertexArray(vertices, indices, pType)
		{
			mpvTexCoord = std::make_shared<std::vector<glm::vec2>>(texcoords);
			mNumExistedAttri++;
		}

		VertexArrayTc(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					   const std::shared_ptr<std::vector<glm::vec2>> &pTexcoords,
					   const std::shared_ptr<std::vector<GLuint>> &pIndices =
					   std::make_shared<std::vector<GLuint>>(),
					   const PrimitiveType &pType = TRIANGLES)
			: VertexArray(pVertices, pIndices, pType), mpvTexCoord(pTexcoords)
		{
			mNumExistedAttri++;
		}

		friend class VertexArrayTcUtil;

		virtual std::string getShaderMacros()
		{
			return VertexArray::getShaderMacros() +
				std::string("#define HAVE_TEXCOORD\n#define TEXCOORD_ATTR 1\n");
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
		VertexArrayTc() {};

		std::shared_ptr<std::vector<glm::vec2>> mpvTexCoord;
	};

	class VertexArrayTcUtil : public VertexArrayUtil
	{
	public:
		VertexArrayTcUtil() = delete;

		//The pVertexArray must point to the VertexArrayTI
		VertexArrayTcUtil(const std::shared_ptr<VertexArrayTc> &pVertexArrayTc)
			: VertexArrayUtil(std::static_pointer_cast<VertexArray>(pVertexArrayTc))
		{
			std::vector<glm::vec2> &vTexCoord = *(pVertexArrayTc->mpvTexCoord);
			glGenBuffers(1, &mTexCoordVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mTexCoordVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoord[0])*vTexCoord.size(), &vTexCoord[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(mVAO);
			{
				//GLuint attriIndex = pVertexArrayTc->mNumExistedAttri - 1;
				glBindBuffer(GL_ARRAY_BUFFER, mTexCoordVBO);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);
				glEnableVertexAttribArray(1);
			}
			glBindVertexArray(0);
		}

		~VertexArrayTcUtil()
		{
			glDeleteBuffers(1, &mTexCoordVBO);
		}

	protected:
		GLuint mTexCoordVBO;
	};

	//The vertexarray holds colors for per vertices
	class VertexArrayC : public VertexArray
	{
	public:
		~VertexArrayC() {}

		VertexArrayC(const std::vector<glm::vec3> &vertices,
					  const std::vector<glm::vec3> &colors,
					  const std::vector<GLuint> &indices = std::vector<GLuint>(),
					  const PrimitiveType &pType = TRIANGLES)
			: VertexArray(vertices, indices, pType)
		{
			mpvColors = std::make_shared<std::vector<glm::vec3>>(colors);
			mNumExistedAttri++;
		}

		VertexArrayC(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					  const std::shared_ptr<std::vector<glm::vec3>> &pColors,
					  const std::shared_ptr<std::vector<GLuint>> &pIndices =
					  std::make_shared<std::vector<GLuint>>(),
					  const PrimitiveType &pType = TRIANGLES)
			: VertexArray(pVertices, pIndices, pType), mpvColors(pColors)
		{
			mNumExistedAttri++;
		}

		friend class VertexArrayCUtil;

		virtual std::string getShaderMacros()
		{
			return VertexArray::getShaderMacros() +
				std::string("#define HAVE_COLOR\n#define COLOR_ATTR 1\n");
		}

		virtual std::shared_ptr<VertexArrayUtil> createUtil(const std::shared_ptr<VertexArray>
															&pVertexArray)
		{
			std::string VertexArrayCName = typeid(VertexArrayC).name();
			std::string currentVAName = typeid(*pVertexArray).name();
			if (VertexArrayCName != currentVAName)
			{
				SP_CERR("The current type of pVertexArray (" + currentVAName + ") is \
						not consistent with the VertexArrayC type (" + VertexArrayCName
						+ ")");
				exit(-1);
			}

			std::shared_ptr<VertexArrayC> pVertexArrayC =
				std::dynamic_pointer_cast<VertexArrayC>(pVertexArray);

			std::shared_ptr<VertexArrayUtil> pVertexArrayUtil;

			if (pVertexArrayC->IsUploaded())
			{
				pVertexArrayUtil = pVertexArrayC->mpVertexArrayUtil.lock();
			}
			else
			{
				std::shared_ptr<VertexArrayCUtil> pVertexArrayCUtil =
					std::make_shared<VertexArrayCUtil>(pVertexArrayC);

				pVertexArrayUtil = std::static_pointer_cast<VertexArrayUtil>(pVertexArrayCUtil);
			}

			return pVertexArrayUtil;
		}

	protected:
		VertexArrayC() {};

		std::shared_ptr<std::vector<glm::vec3>> mpvColors;
	};

	class VertexArrayCUtil : public VertexArrayUtil
	{
	public:
		VertexArrayCUtil() = delete;

		//The pVertexArray must point to the VertexArrayTI
		VertexArrayCUtil(const std::shared_ptr<VertexArrayC> &pVertexArrayC)
			: VertexArrayUtil(std::static_pointer_cast<VertexArray>(pVertexArrayC))
		{
			std::vector<glm::vec3> &vColor = *(pVertexArrayC->mpvColors);
			glGenBuffers(1, &mColorVBO);
			glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vColor[0])*vColor.size(), &vColor[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(mVAO);
			{
				//GLuint attriIndex = pVertexArrayC->mNumExistedAttri - 1;
				glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
				glEnableVertexAttribArray(1);
			}
			glBindVertexArray(0);
		}

		~VertexArrayCUtil()
		{
			glDeleteBuffers(1, &mColorVBO);
		}

	protected:
		GLuint mColorVBO;
	};
}