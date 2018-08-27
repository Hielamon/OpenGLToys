#pragma once

#include "utils.h"
#include "VertexArray.h"
#include "Material.h"

namespace SP
{
	class MeshUtil;

	class MeshGlobal
	{
	public:
		~MeshGlobal() {}

		static MeshGlobal &getInstance()
		{
			static MeshGlobal GMesh;
			return GMesh;
		}

		GLuint64 totalMeshN;
	private:
		MeshGlobal() : totalMeshN(0) {}

	};

	class Mesh
	{
	public:
		Mesh(const std::shared_ptr<VertexArray> &pVertexArray,
			 const std::shared_ptr<Material> &pMaterial,
			 const std::vector<glm::mat4> &vInstanceMMatrix =
			 std::vector<glm::mat4>())
			: mpVertexArray(pVertexArray), mpMaterial(pMaterial),
			mInstanceN(vInstanceMMatrix.size()), mvInstanceMMatrix(vInstanceMMatrix)
		{
			setVertexArray(pVertexArray);

			mMeshID = MeshGlobal::getInstance().totalMeshN;
			MeshGlobal::getInstance().totalMeshN++;
		}

		Mesh(const std::shared_ptr<VertexArray> &pVertexArray,
			 const std::vector<glm::mat4> &vInstanceMMatrix =
			 std::vector<glm::mat4>())
			: mInstanceN(vInstanceMMatrix.size()),
			mvInstanceMMatrix(vInstanceMMatrix)
		{
			setVertexArray(pVertexArray);

			mMeshID = MeshGlobal::getInstance().totalMeshN;
			MeshGlobal::getInstance().totalMeshN++;
		}
		
		~Mesh() {}

		friend class MeshUtil;

		void reset()
		{
			mpMaterial.reset();
			mpVertexArray.reset();
			mInstanceN = 0;
			mvInstanceMMatrix.clear();
		}

		void reset(const std::shared_ptr<VertexArray> &pVertexArray,
				   const std::shared_ptr<Material> &pMaterial,
				   const std::vector<glm::mat4> &vInstanceMMatrix = 
				   std::vector<glm::mat4>())
		{
			setVertexArray(pVertexArray);

			mpMaterial = pMaterial;
			mvInstanceMMatrix = vInstanceMMatrix;
			mInstanceN = mvInstanceMMatrix.size();
			assert(mInstanceN == 0);
		}

		void setVertexArray(const std::shared_ptr<VertexArray> &pVertexArray)
		{
			if (pVertexArray.use_count() == 0)
			{
				//mpVertexArrayUtil cannot be empty
				SP_CERR("The vertexArray is empty, this situation is not permitted");
				exit(-1);
			}
			mMMatrixAttri = pVertexArray->getNumExistedAttri();
			mpVertexArray = pVertexArray;
		}

		void addInstance(const glm::mat4 &instanceMMatrix = glm::mat4())
		{
			mvInstanceMMatrix.push_back(instanceMMatrix);
			mInstanceN++;
		}

		virtual std::string getShaderMacros()
		{
			std::string macros;
			if (mpMaterial.use_count() != 0 &&
				mpMaterial->getAllTexturesNum() != 0)
			{
				macros += mpMaterial->getShaderMacros();
			}

			if (mpVertexArray.use_count() != 0)
			{
				macros += mpVertexArray->getShaderMacros();
				std::stringstream ioStr;
				ioStr << "#define MMATRIX_ATTR " << mMMatrixAttri << "\n";
				macros += ioStr.str();
			}

			return macros;
		}

		virtual std::shared_ptr<MeshUtil> createUtil(const std::shared_ptr<Mesh>
														 &pMesh)
		{
			std::shared_ptr<MeshUtil> pMeshUtil;
			if (pMesh->IsUploaded())
			{
				pMeshUtil = pMesh->mpMeshUtil.lock();
			}
			else
			{
				pMeshUtil = std::make_shared<MeshUtil>(pMesh);
			}

			return pMeshUtil;
		}
		//Get the total bounding box of all instances 
		BBox getBoundingBox()
		{
			BBox resBBox, oriBBox = mpVertexArray->getBoundingBox();
			
			for (size_t i = 0; i < mInstanceN; i++)
			{
				BBox currBBox = TransformBBox(mvInstanceMMatrix[i], oriBBox);
				resBBox += currBBox;
			}

			return resBBox;
		}

		//Get the all bounding boxes of all instances 
		std::vector<BBox> getAllBoundingBoxes()
		{
			std::vector<BBox> vBBox;
			BBox oriBBox = mpVertexArray->getBoundingBox();
			for (size_t i = 0; i < mInstanceN; i++)
			{
				BBox currBBox = TransformBBox(mvInstanceMMatrix[i], oriBBox);
				vBBox.push_back(currBBox);
			}

			return vBBox;
		}

		//For indicating whether the VertexArray has been uploaded to the GPU memory
		void setMeshUtil(const std::shared_ptr<MeshUtil> &pMeshUtil)
		{
			mpMeshUtil = pMeshUtil;
		}

		//Clearing the state of uploading to the GPU memory
		void resetMeshUtil()
		{
			mpMeshUtil.reset();
		}

		//Get if the VertexArray has been uploaded by accessing the mpVertexArrayUtil's state
		bool IsUploaded()
		{
			std::shared_ptr<MeshUtil> pMeshUtil = mpMeshUtil.lock();
			return pMeshUtil.use_count() != 0;
		}

	protected:
		Mesh() : mInstanceN(0), mMMatrixAttri(-1)
		{
			mMeshID = MeshGlobal::getInstance().totalMeshN;
			MeshGlobal::getInstance().totalMeshN++;
		}

		std::shared_ptr<Material> mpMaterial;
		std::shared_ptr<VertexArray> mpVertexArray;

		int mInstanceN;
		std::vector<glm::mat4> mvInstanceMMatrix;
		int mMMatrixAttri;

		GLuint mMeshID;

		//For indicating whether the Mesh has been uploaded to the GPU memory
		std::weak_ptr<MeshUtil> mpMeshUtil;
	};

	class MeshUtil
	{
	public:
		MeshUtil(const std::shared_ptr<Mesh> &pMesh)
		: mpMesh(pMesh), mbUploadUColor(false), mMMatrixVBO(FAILED_RETURN)
		{
			if (mpMesh->mInstanceN <= 0) return;

			//Uploading the mpMaterial, if not upload
			if (mpMesh->mpMaterial.use_count() != 0 &&
				mpMesh->mpMaterial->getAllTexturesNum() != 0)
			{
				mpMaterialUtil = mpMesh->mpMaterial->createUtil(mpMesh->mpMaterial);
				if (!mpMesh->mpMaterial->IsUploaded())
				{
					//Aiming to avoid the duplicate upload the material
					mpMesh->mpMaterial->setMaterialUtil(mpMaterialUtil);
				}
			}

			//Uploading the vpVertexArray, if not upload
			if (mpMesh->mpVertexArray.use_count() != 0)
			{
				mpVertexArrayUtil = mpMesh->mpVertexArray->createUtil(mpMesh->mpVertexArray);
				if (!mpMesh->mpVertexArray->IsUploaded())
				{
					//Aiming to avoid the duplicate upload the vertex array
					mpMesh->mpVertexArray->setVertexArrayUtil(mpVertexArrayUtil);
				}
				
				//For deciding whether to setup the uniform color
				if ((mpMesh->mpMaterial.use_count() == 0 ||
					 mpMesh->mpMaterial->getAllTexturesNum() == 0) &&
					mpMesh->mpVertexArray->getShaderMacros().find(
					"#define HAVE_COLORS\n") == std::string::npos)
				{
					mbUploadUColor = true;
				}
			}
			else
			{
				//mpVertexArrayUtil cannot be empty
				SP_CERR("The vertexArray is empty, this situation is not permitted");
				exit(-1);
			}

			//Uploading the model matrixs of all instances
			if (mpMesh->mInstanceN > 0)
			{
				glGenBuffers(1, &mMMatrixVBO);
				glBindBuffer(GL_ARRAY_BUFFER, mMMatrixVBO);
				glBufferData(GL_ARRAY_BUFFER, mpMesh->mInstanceN * sizeof(glm::mat4),
							 &(mpMesh->mvInstanceMMatrix[0]), GL_STATIC_DRAW);

				GLuint VAO = mpVertexArrayUtil->getVAO();
				int baseIdx = mpMesh->mMMatrixAttri;
				GLsizei vec4Size = sizeof(glm::vec4);
				GLsizei stride = 4 * vec4Size;

				glBindVertexArray(VAO);
				for (size_t i = 0; i < 4; i++)
				{
					GLuint index = baseIdx + i;
					glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, stride, (void *)(i * vec4Size));
					glEnableVertexAttribArray(index);
					glVertexAttribDivisor(index, 1);
				}
				glBindVertexArray(0);
			}
		}

		~MeshUtil()
		{
			if (mMMatrixVBO != FAILED_RETURN)
			{
				glDeleteBuffers(1, &mMMatrixVBO);
			}
		}

		//Set the state of whether to upload the ucolor
		void setUColor(bool bUploadUColor)
		{
			mbUploadUColor = bUploadUColor;
		}

		GLuint getMeshID()
		{
			return mpMesh->mMeshID;
		}

		int getInstanceN()
		{
			return mpMesh->mInstanceN;
		}

		std::string getShaderMacros()
		{
			return mpMesh->getShaderMacros();
		}

		virtual void draw()
		{
			//Bind the textures
			if (mpMaterialUtil.use_count() != 0)
			{
				mpMaterialUtil->activeMaterial();
			}

			//Bind the vertexarray and draw
			if (mpVertexArrayUtil.use_count() != 0)
			{
				/*mpVertexArrayUtil->setUColor(false);
				GLint programID;
				glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
				GLint uColorLoc = glGetUniformLocation(programID, "uColor");
				double totalColor = MeshGlobal::getInstance().totalMeshN + 1;
				int Nc = std::ceil(std::pow(totalColor, 1.0 / 3));
				int ID = mpMesh->mMeshID + 1;
				glm::vec3 uColor;
				for (size_t i = 0; i < 3 && ID >0; i++)
				{
					uColor[i] = (ID % Nc) / float(Nc);
					ID = (ID - ID % Nc) / Nc;
				}


				
				glUniform3f(uColorLoc, uColor.r, uColor.g, uColor.b);*/

				if (mbUploadUColor)
				{
					GLint programID;
					glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
					GLint uColorLoc = glGetUniformLocation(programID, "uColor");
					const glm::vec3 &uColor = mpVertexArrayUtil->getUniformColor();
					glUniform3f(uColorLoc, uColor.r, uColor.g, uColor.b);
				}

				mpVertexArrayUtil->drawInstanced(mpMesh->mInstanceN);
			}
		}

	protected:
		MeshUtil() {}

		std::shared_ptr<Mesh> mpMesh;

		std::shared_ptr<MaterialUtil> mpMaterialUtil;
		std::shared_ptr<VertexArrayUtil> mpVertexArrayUtil;
		//If there is not any texture and vertex's colors for the VertexArray
		//mbUploadUColor will set to the true
		bool mbUploadUColor;

		//This vertex buffer object consuming 4 attributes
		//due to the maximum amount of data allowed as a vertex
		//attribute is equal to a vec4, but the model matrix is
		//a mat4 which is basically 4 vec4s
		GLuint mMMatrixVBO;
	};

	//The MeshColorIDUtil is used for showing the colored id mesh scene
	//Which only can instanced from a MeshUtil
	class MeshColorIDUtil : public MeshUtil
	{
	public:
		MeshColorIDUtil() = delete;
		
		MeshColorIDUtil(const std::shared_ptr<MeshUtil> &pMeshUtil)
			: MeshUtil(*pMeshUtil) {}

		virtual void draw()
		{
			//Bind the vertexarray and draw
			if (mpVertexArrayUtil.use_count() != 0)
			{
				GLint programID;
				glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
				GLint uIDColorLoc = glGetUniformLocation(programID, "uIDColor");
				double totalColor = MeshGlobal::getInstance().totalMeshN + 1;
				int Nc = std::ceil(std::pow(totalColor, 1.0 / 3));
				int ID = getMeshID() + 1;
				glm::vec3 uIDColor;
				for (size_t i = 0; i < 3 && ID >0; i++)
				{
					uIDColor[i] = (ID % Nc) / float(Nc);
					ID = (ID - ID % Nc) / Nc;
				}

				glUniform3f(uIDColorLoc, uIDColor.r, uIDColor.g, uIDColor.b);
				
				mpVertexArrayUtil->drawInstanced(getInstanceN());
			}
		}
	};
}