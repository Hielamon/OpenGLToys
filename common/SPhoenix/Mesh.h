#pragma once

#include "Utils.h"
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
			 const std::shared_ptr<Material> &pMaterial = 
			 std::make_shared<Material>(),
			 const std::vector<glm::mat4> &vInstanceMMatrix =
			 std::vector<glm::mat4>())
			: mInstanceN(vInstanceMMatrix.size()), mvInstanceMMatrix(vInstanceMMatrix)
		{
			setVertexArray(pVertexArray);
			setMaterial(pMaterial);

			MeshGlobal::getInstance().totalMeshN++;
			mMeshID = MeshGlobal::getInstance().totalMeshN;
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

		void setMaterial(const std::shared_ptr<Material> &pMaterial)
		{
			if (pMaterial.use_count() == 0)
			{
				SP_CERR("The Material is empty, this situation is not permitted");
				exit(-1);
			}
			mpMaterial = pMaterial;
		}

		void addInstance(const glm::mat4 &instanceMMatrix = glm::mat4())
		{
			mvInstanceMMatrix.push_back(instanceMMatrix);
			mInstanceN++;
		}

		virtual std::string getShaderMacros()
		{
			std::string macros = "";

			if (mpMaterial.use_count() != 0)
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
			MeshGlobal::getInstance().totalMeshN++;
			mMeshID = MeshGlobal::getInstance().totalMeshN;
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
		MeshUtil() = delete;

		MeshUtil(const std::shared_ptr<Mesh> &pMesh)
		: mpMesh(pMesh), mMMatrixVBO(FAILED_RETURN)
		{
			if (mpMesh->mInstanceN <= 0) return;

			//Uploading the mpMaterial, if not upload
			setMaterialUtil(mpMesh->mpMaterial);

			//Uploading the vpVertexArray, if not upload
			setVertexArrayUtil(mpMesh->mpVertexArray);

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
			std::string macros = "";

			if (mpMaterialUtil.use_count() != 0)
			{
				macros += mpMaterialUtil->getShaderMacros();
			}

			if (mpVertexArrayUtil.use_count() != 0)
			{
				macros += mpVertexArrayUtil->getShaderMacros();
				std::stringstream ioStr;
				ioStr << "#define MMATRIX_ATTR " << mpMesh->mMMatrixAttri << "\n";
				macros += ioStr.str();
			}
			return macros;
		}

		void setVertexArrayUtil(const std::shared_ptr<VertexArray> &pVertexArray)
		{
			if (pVertexArray.use_count() != 0)
			{
				mpVertexArrayUtil = pVertexArray->createUtil(pVertexArray);
				if (!pVertexArray->IsUploaded())
				{
					//Aiming to avoid the duplicate upload the vertex array
					pVertexArray->setVertexArrayUtil(mpVertexArrayUtil);
				}
			}
			else
			{
				//mpVertexArrayUtil cannot be empty
				SP_CERR("The vertexArray is empty, this situation is not permitted");
				exit(-1);
			}
		}

		void setMaterialUtil(const std::shared_ptr<Material> &pMaterial)
		{
			if (pMaterial.use_count() != 0)
			{
				mpMaterialUtil = pMaterial->createUtil(pMaterial);
				if (!pMaterial->IsUploaded())
				{
					//Aiming to avoid the duplicate upload the material
					pMaterial->setMaterialUtil(mpMaterialUtil);
				}
			}
			else
			{
				//mpVertexArrayUtil cannot be empty
				SP_CERR("The Material is empty, this situation is not permitted");
				exit(-1);
			}
		}

		virtual void draw()
		{
			//Bind the textures
			mpMaterialUtil->activeMaterial(mpVertexArrayUtil->hasTexCoord(), 
										   mpVertexArrayUtil->hasVertexColor());

			//Bind the vertexarray and draw
			{
				GLint programID;
				glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
				GLint uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");
				glUniform1ui(uMeshIDLoc, getMeshID());

				mpVertexArrayUtil->drawInstanced(mpMesh->mInstanceN);
			}
		}

	protected:
		std::shared_ptr<Mesh> mpMesh;

		std::shared_ptr<MaterialUtil> mpMaterialUtil;
		std::shared_ptr<VertexArrayUtil> mpVertexArrayUtil;

		//This vertex buffer object consuming 4 attributes
		//due to the maximum amount of data allowed as a vertex
		//attribute is equal to a vec4, but the model matrix is
		//a mat4 which is basically 4 vec4s
		GLuint mMMatrixVBO;
	};

	//The MeshColorIDUtil is used for showing the colored id mesh scene
	//Which only can instanced from a MeshUtil
	/*class MeshColorIDUtil : public MeshUtil
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
				int ID = getMeshID();
				glm::vec3 uIDColor;
				for (size_t i = 0; i < 3 && ID > 0; i++)
				{
					uIDColor[i] = (ID % Nc) / float(Nc);
					ID = (ID - ID % Nc) / Nc;
				}

				glUniform3f(uIDColorLoc, uIDColor.r, uIDColor.g, uIDColor.b);

				GLint uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");
				glUniform1ui(uMeshIDLoc, getMeshID());
				
				mpVertexArrayUtil->drawInstanced(getInstanceN());
			}
		}
	};

	class MeshSelectedUtil : public MeshUtil
	{
	public:
		MeshSelectedUtil() = delete;

		MeshSelectedUtil(const std::shared_ptr<MeshUtil> &pMeshUtil)
			: MeshUtil(*pMeshUtil) {}

		virtual void draw()
		{
			//Bind the vertexarray and draw
			if (mpVertexArrayUtil.use_count() != 0)
			{
				GLint programID;
				glGetIntegerv(GL_CURRENT_PROGRAM, &programID);

				GLint uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");
				glUniform1ui(uMeshIDLoc, getMeshID());

				mpVertexArrayUtil->drawInstanced(getInstanceN());
			}
		}

	};*/
}