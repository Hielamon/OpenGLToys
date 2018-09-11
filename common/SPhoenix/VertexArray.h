#pragma once
#include "Utils.h"
#include "BBox.h"

namespace SP
{
	class VertexArray;

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

	//The class hold the array information for the vertex, such as:
	//1. vertices (position) : vec3
	//2. normals             : vec3
	//3. texture coordinates : vec2
	//4. vertex-colors       : vec4
	class VertexArray
	{
	public:
		VertexArray() = delete;

		VertexArray(const std::vector<glm::vec3> &vertices,
					const std::vector<GLuint> &indices = std::vector<GLuint>(),
					const PrimitiveType &pType = TRIANGLES)
			: mPrimitiveType(pType), mOriginBBox(vertices), mbUploaded(false), mNumInstance(0)
		{
			mpvVertice = std::make_shared<std::vector<glm::vec3>>(vertices);
			mpvInstanceMMatrix = std::make_shared<std::vector<glm::mat4>>();

			if (!indices.empty())
			{
				mpvIndice = std::make_shared<std::vector<GLuint>>(indices);
			}

			std::map<PrimitiveType, GLenum> &typeMap = VAGlobal::getInstance().PTypeModeMap;
			mDrawMode = typeMap[mPrimitiveType];
		}

		VertexArray(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
					const std::shared_ptr<std::vector<GLuint>> &pIndices = nullptr,
					const PrimitiveType &pType = TRIANGLES)
			: mpvVertice(pVertices), mpvIndice(pIndices),
			mPrimitiveType(pType), mOriginBBox(*pVertices), mbUploaded(false), mNumInstance(0)
		{
			std::map<PrimitiveType, GLenum> &typeMap = VAGlobal::getInstance().PTypeModeMap;
			mDrawMode = typeMap[mPrimitiveType];

			mpvInstanceMMatrix = std::make_shared<std::vector<glm::mat4>>();
		}

		~VertexArray()
		{
			clearUploaded();
		}

		//If the input is not has the same size with the vertice array
		//the corresponding array(normals, texcoord or colors) will not be changed
		void setNormals(const std::vector<glm::vec3> &normals)
		{
			if (normals.size() == mpvVertice->size())
			{
				mpvNormal = std::make_shared<std::vector<glm::vec3>>(normals);
			}
			else
			{
				SP_CERR("The size of normals is not consistent with the vertices");
			}
		}

		//If the input is not has the same size with the vertice array
		//the corresponding array(normals, texcoord or colors) will not be changed
		void setNormals(const std::shared_ptr<std::vector<glm::vec3>> &pNormals)
		{
			if (pNormals.use_count() != 0 &&
				pNormals->size() == mpvVertice->size())
			{
				mpvNormal = pNormals;
			}
			else
			{
				SP_CERR("The size of normals is not consistent with the vertices");
			}
		}

		//If the input is not has the same size with the vertice array
		//the corresponding array(normals, texcoord or colors) will not be changed
		void setTexCoords(const std::vector<glm::vec2> &texcoords)
		{
			if (texcoords.size() == mpvVertice->size())
			{
				mpvTexCoord = std::make_shared<std::vector<glm::vec2>>(texcoords);
			}
			else
			{
				SP_CERR("The size of texcoords is not consistent with the vertices");
			}
		}

		//If the input is not has the same size with the vertice array
		//the corresponding array(normals, texcoord or colors) will not be changed
		void setTexCoords(const std::shared_ptr<std::vector<glm::vec2>> &pTexcoords)
		{
			if (pTexcoords.use_count() != 0 &&
				pTexcoords->size() == mpvVertice->size())
			{
				mpvTexCoord = pTexcoords;
			}
			else
			{
				SP_CERR("The size of texcoords is not consistent with the vertices");
			}
		}

		//If the input is not has the same size with the vertice array
		//the corresponding array(normals, texcoord or colors) will not be changed
		void setColors(const std::vector<glm::vec4> &colors)
		{
			if (colors.size() == mpvVertice->size())
			{
				mpvColor = std::make_shared<std::vector<glm::vec4>>(colors);
			}
			else
			{
				SP_CERR("The size of colors is not consistent with the vertices");
			}
		}

		//If the input is not has the same size with the vertice array
		//the corresponding array(normals, texcoord or colors) will not be changed
		void setColors(const std::shared_ptr<std::vector<glm::vec4>> &pColors)
		{
			if (pColors.use_count() != 0 && 
				pColors->size() == mpvVertice->size())
			{
				mpvColor = pColors;
			}
			else
			{
				SP_CERR("The size of colors is not consistent with the vertices");
			}
		}

		//Check whether the class has the texcoord
		bool hasTexCoord()
		{
			return mpvTexCoord.use_count() != 0;
		}

		//Check whether the class has the colors for each vertice
		bool hasVertexColor()
		{
			return mpvColor.use_count() != 0;
		}

		//Add the instance to model matrix array, if this VertexArray has been uploaded,
		//Then this function will also upload the new model matrix array
		void addInstance(const glm::mat4 &instanceMMatrix = glm::mat4())
		{
			mpvInstanceMMatrix->push_back(instanceMMatrix);
			mNumInstance++;
			
			//If the array of model matrix has been uploaded to the device
			//we will update the model matrix buffer 
			if (mbUploaded)
			{
				_uploadArrayBuffer(GL_ARRAY_BUFFER, mMMatrixVBO, mpvInstanceMMatrix);
			}
		}

		//The the instance model matrix in instanceID, if this VertexArray has been
		//uploaded, then this function will also upload the device model matrix 
		//attribute memory
		void setInstanceMMatrix(const glm::mat4 &instanceMMatrix, 
								GLuint instanceID)
		{
			if (mpvInstanceMMatrix->size() > instanceID)
			{
				glm::mat4 &modelMatrix = (*mpvInstanceMMatrix)[instanceID];
				modelMatrix = instanceMMatrix;

				if (mbUploaded)
				{
					glBindBuffer(GL_ARRAY_BUFFER, mMMatrixVBO);
					glBufferSubData(GL_ARRAY_BUFFER, instanceID * sizeof(glm::mat4),
									sizeof(glm::mat4), &modelMatrix);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}
			else
			{
				SP_CERR("The instance ID beyond the mpvInstanceMMatrix size");
			}
		}

		//Get the instance model matrix in instanceID
		glm::mat4 getInstanceMMatrix(GLuint instanceID)
		{
			glm::mat4 result;
			if (mpvInstanceMMatrix->size() > instanceID)
			{
				result = (*mpvInstanceMMatrix)[instanceID];
			}
			else
			{
				SP_CERR("The instance ID beyond the mpvInstanceMMatrix size");
				exit(-1);
			}

			return result;
		}

		//Get the number of instances
		GLuint getNumInstance()
		{
			return mpvInstanceMMatrix->size();
		}

		BBox getTotalBBox()
		{
			BBox result;
			for (size_t i = 0; i < mpvInstanceMMatrix->size(); i++)
			{
				result += TransformBBox((*mpvInstanceMMatrix)[i],
										mOriginBBox);
			}
			return result;
		}

		BBox getOriginBBox()
		{
			return mOriginBBox;
		}

		//Get the macro for the shader codes, which defines some necessary variable 
		//Such as vertex array attribute index for the shader codes
		virtual std::string getShaderMacros()
		{
			std::string macros = "";
			std::stringstream ioStr;

			//The vertice array hold the index 0
			GLuint attriIndex = 1;

			if (mpvNormal.use_count() != 0)
			{
				ioStr.str("");
				ioStr << "#define HAVE_NORMAL\n#define NORMAL_ATTR " << attriIndex << "\n";
				macros += ioStr.str();
				attriIndex++;
			}

			if (mpvTexCoord.use_count() != 0)
			{
				ioStr.str("");
				ioStr << "#define HAVE_TEXCOORD\n#define TEXCOORD_ATTR " << attriIndex << "\n";
				macros += ioStr.str();
				attriIndex++;
			}

			if (mpvColor.use_count() != 0)
			{
				ioStr.str("");
				ioStr << "#define HAVE_COLOR\n#define COLOR_ATTR " << attriIndex << "\n";
				macros += ioStr.str();
				attriIndex++;
			}

			ioStr.str("");
			ioStr << "#define MMATRIX_ATTR " << attriIndex << "\n";
			macros += ioStr.str();
			attriIndex += 4;//The model matrix array must occupy 4 attribute

			return macros;
		}

		//upload the vetex array information to the device
		//If the mbUploaded is true, this function will do nothing
		virtual void uploadToDevice()
		{
			if (mbUploaded) return;

			mNumDrawVertice = mpvVertice->size();

			_uploadArrayBuffer(GL_ARRAY_BUFFER, mVerticeVBO, mpvVertice);
			_uploadArrayBuffer(GL_ARRAY_BUFFER, mNormalVBO, mpvNormal);
			_uploadArrayBuffer(GL_ARRAY_BUFFER, mTexCoordVBO, mpvTexCoord);
			_uploadArrayBuffer(GL_ARRAY_BUFFER, mColorVBO, mpvColor);

			//upload the model matrix of all instances
			_uploadArrayBuffer(GL_ARRAY_BUFFER, mMMatrixVBO, mpvInstanceMMatrix);

			_uploadArrayBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO, mpvIndice);

			mbDrawElements = mpvIndice.use_count() == 0 ? false : true;
			if (mbDrawElements) mNumDrawVertice = mpvIndice->size();

			_setupVAO();

			mbUploaded = true;
		}

		//clear the uploaded buffers and reset the mbUploaded to false
		virtual void clearUploaded()
		{
			if (mbUploaded)
			{
				glDeleteBuffers(1, &mVerticeVBO);

				if (mpvNormal.use_count() != 0)
				{
					glDeleteBuffers(1, &mNormalVBO);
				}

				if (mpvTexCoord.use_count() != 0)
				{
					glDeleteBuffers(1, &mTexCoordVBO);
				}

				if (mpvColor.use_count() != 0)
				{
					glDeleteBuffers(1, &mColorVBO);
				}

				glDeleteBuffers(1, &mMMatrixVBO);

				if (mbDrawElements)
				{
					glDeleteBuffers(1, &mEBO);
				}

				glDeleteVertexArrays(1, &mVAO);

				mbUploaded = false;
			}
		}

		//draw the vertex array, we use the glDraw*Instanced function for uniform usage
		virtual void draw(const GLuint &programID)
		{
			if (mNumInstance <= 0)
			{
				SP_CERR("The current vertex array has no instance for drawing");
				return;
			}

			if (!mbUploaded)
			{
				SP_CERR("The current vertex array has not been uploaded befor drawing");
				return;
			}
			
			glBindVertexArray(mVAO);
			if (mbDrawElements)
			{
				glDrawElementsInstanced(mDrawMode, mNumDrawVertice, GL_UNSIGNED_INT, 
										0, mNumInstance);
			}
			else
			{
				glDrawArraysInstanced(mDrawMode, 0, mNumDrawVertice, mNumInstance);
			}
		}

	protected:
		std::shared_ptr<std::vector<glm::vec3>> mpvVertice;
		std::shared_ptr<std::vector<glm::vec3>> mpvNormal;
		std::shared_ptr<std::vector<glm::vec2>> mpvTexCoord;
		std::shared_ptr<std::vector<glm::vec4>> mpvColor;
		std::shared_ptr<std::vector<GLuint>> mpvIndice;
		PrimitiveType mPrimitiveType;

		//The bounding box of orgin vertex array which
		//means without any model matrix transform
		BBox mOriginBBox;

		//The mpvInstanceMMatrix must be assign to a empty vector in
		//the constructor
		std::shared_ptr<std::vector<glm::mat4>> mpvInstanceMMatrix;
		int mNumInstance;

		//****** The inner variable for device ******//
		bool mbUploaded;
		GLuint mVAO;
		GLuint mEBO;

		GLuint mVerticeVBO, mNormalVBO, mTexCoordVBO, mColorVBO;
		GLuint mMMatrixVBO;

		GLenum mDrawMode;

		bool mbDrawElements;
		int mNumDrawVertice;

	private:
		template <class T>
		void _uploadArrayBuffer(GLenum target, GLuint &buffer,
								const std::shared_ptr<std::vector<T>> &pvData)
		{
			if (pvData.use_count() == 0) return;

			if (!mbUploaded) glGenBuffers(1, &buffer);

			std::vector<T> &vData = *pvData;
			glBindBuffer(target, buffer);
			glBufferData(target, sizeof(vData[0])*vData.size(), &vData[0], GL_STATIC_DRAW);
			glBindBuffer(target, 0);
		}

		//Set up the vertex array object and set the vertex attributes
		void _setupVAO()
		{
			if (!mbUploaded) glGenVertexArrays(1, &mVAO);

			glBindVertexArray(mVAO);
			{
				GLuint attriIndex = 0;
				_setupFloatVertexAttrib<glm::vec3>(mVerticeVBO, attriIndex, 3, 0);
				attriIndex++;

				if (mpvNormal.use_count() != 0)
				{
					_setupFloatVertexAttrib<glm::vec3>(mNormalVBO, attriIndex, 3, 0);
					attriIndex++;
				}

				if (mpvTexCoord.use_count() != 0)
				{
					_setupFloatVertexAttrib<glm::vec2>(mTexCoordVBO, attriIndex, 2, 0);
					attriIndex++;
				}

				if (mpvColor.use_count() != 0)
				{
					_setupFloatVertexAttrib<glm::vec4>(mColorVBO, attriIndex, 4, 0);
					attriIndex++;
				}

				//setup the model matrix array attributes
				//which need 4 attributes for matrix
				GLsizei vec4Size = sizeof(glm::vec4);
				for (size_t i = 0; i < 4; i++)
				{
					_setupFloatVertexAttrib<glm::mat4>(mMMatrixVBO, attriIndex, 4,
													   i * vec4Size, 1);
					attriIndex++;
				}

				if (mbDrawElements) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
			}
			glBindVertexArray(0);
		}

		//The template type vecType is used for the stride
		//The divisor is defaultly set to 0, specially for the model matrix
		//array, the divisor need to be set to 1, which means that the atrribute 
		//update for each instance
		template <class vecType>
		void _setupFloatVertexAttrib(GLuint buffer, GLuint index, GLint vecn,
									 GLint offset, GLuint divisor = 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glVertexAttribPointer(index, vecn, GL_FLOAT, GL_FALSE,
								  sizeof(vecType), (GLvoid*)(offset));
			glEnableVertexAttribArray(index);
			glVertexAttribDivisor(index, divisor);
		}

	};
}