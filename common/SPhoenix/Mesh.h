#pragma once

#include "utils.h"
#include "VertexArray.h"
#include "Material.h"

namespace SP
{
	class MeshUtil;

	class Mesh
	{
	public:
		Mesh() = delete;

		Mesh(const std::shared_ptr<VertexArray> &pVertexArray,
			 const std::shared_ptr<Material> &pMaterial,
			 const std::vector<glm::mat4> &vInstanceMMatrix =
			 std::vector<glm::mat4>())
			: mpVertexArray(pVertexArray), mpMaterial(pMaterial),
			mInstanceN(vInstanceMMatrix.size()), mvInstanceMMatrix(vInstanceMMatrix)
		{
			if (pVertexArray.use_count() == 0)
			{
				//mpVertexArrayUtil cannot be empty
				SP_CERR("The vertexArray is empty, this situation is not permitted");
				exit(-1);
			}
			mMMatrixAttri = pVertexArray->getNumExistedAttri();
		}

		Mesh(const std::shared_ptr<VertexArray> &pVertexArray,
			 const std::vector<glm::mat4> &vInstanceMMatrix =
			 std::vector<glm::mat4>())
			: mpVertexArray(pVertexArray),
			mInstanceN(vInstanceMMatrix.size()),
			mvInstanceMMatrix(vInstanceMMatrix)
		{
			if (pVertexArray.use_count() == 0)
			{
				//mpVertexArrayUtil cannot be empty
				SP_CERR("The vertexArray is empty, this situation is not permitted");
				exit(-1);
			}
			mMMatrixAttri = pVertexArray->getNumExistedAttri();
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
			if (pVertexArray.use_count() == 0)
			{
				//mpVertexArrayUtil cannot be empty
				SP_CERR("The vertexArray is empty, this situation is not permitted");
				exit(-1);
			}
			mMMatrixAttri = pVertexArray->getNumExistedAttri();

			mpMaterial = pMaterial;
			mpVertexArray = pVertexArray;
			mvInstanceMMatrix = vInstanceMMatrix;
			mInstanceN = mvInstanceMMatrix.size();
			assert(mInstanceN == 0);
		}

		void addInstance(const glm::mat4 &instanceMMatrix)
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
		std::shared_ptr<Material> mpMaterial;
		std::shared_ptr<VertexArray> mpVertexArray;

		int mInstanceN;
		std::vector<glm::mat4> mvInstanceMMatrix;
		int mMMatrixAttri;

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
					mpMesh->mpVertexArray->getShaderMacros() ==
					"#define HAVE_TEXCOORD\n")
				{
					mpVertexArrayUtil->setUColor(true);
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

		void draw()
		{
			//Bind the textures
			if (mpMaterialUtil.use_count() != 0)
			{
				mpMaterialUtil->activeMaterial();
			}

			//Bind the vertexarray and draw
			if (mpVertexArrayUtil.use_count() != 0)
			{
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

}