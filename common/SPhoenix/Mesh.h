#pragma once

#include "Utils.h"
#include "VertexArray.h"
#include "Material.h"

namespace SP
{
	class MeshGlobal
	{
	public:
		~MeshGlobal() {}

		static MeshGlobal &getInstance()
		{
			static MeshGlobal GMesh;
			return GMesh;
		}

		GLuint totalMeshCount;
	private:
		MeshGlobal() : totalMeshCount(0) {}

	};

	//The mesh class holds the vertexarray-material pair,
	//For the inherited classes of the mesh,
	//They must set the material and the vertex array explicite in
	//the constructor function.
	class Mesh
	{
	public:
		Mesh(const std::shared_ptr<VertexArray> &pVertexArray,
			 const std::shared_ptr<Material> &pMaterial =
			 std::make_shared<Material>()/*,
			 const glm::mat4 &modelMatrix = glm::mat4()*/)
			: mbUploaded(false)/*, mModelMatrix(modelMatrix)*/
		{
			reset(pVertexArray, pMaterial);

			MeshGlobal::getInstance().totalMeshCount++;
			mMeshID = MeshGlobal::getInstance().totalMeshCount;
		}
		
		~Mesh() {}

		void reset(const std::shared_ptr<VertexArray> &pVertexArray,
				   const std::shared_ptr<Material> &pMaterial)
		{
			mbUploaded = false;
			setVertexArray(pVertexArray);
			setMaterial(pMaterial);
		}

		void setVertexArray(const std::shared_ptr<VertexArray> &pVertexArray)
		{
			if (pVertexArray.use_count() == 0)
			{
				SP_CERR("The VertexArray is empty, this situation is not permitted");
				exit(-1);
			}

			mpVertexArray = pVertexArray;
			if (mbUploaded)
			{
				mpVertexArray->uploadToDevice();
				mbHasTexCoord = mpVertexArray->hasTexCoord();
				mbHasVertexColor = mpVertexArray->hasVertexColor();
			}
		}

		void setMaterial(const std::shared_ptr<Material> &pMaterial)
		{
			if (pMaterial.use_count() == 0)
			{
				SP_CERR("The Material is empty, this situation is not permitted");
				exit(-1);
			}

			mpMaterial = pMaterial;
			if (mbUploaded) mpMaterial->uploadToDevice();
		}

		void setInstanceMMatrix(const glm::mat4 &instanceMMatrix,
								GLuint instanceID)
		{
			mpVertexArray->setInstanceMMatrix(instanceMMatrix, instanceID);
		}

		glm::mat4 getInstanceMMatrix(GLuint instanceID)
		{
			return mpVertexArray->getInstanceMMatrix(instanceID);
		}

		BBox getTotalBBox()
		{
			return mpVertexArray->getTotalBBox();
		}

		GLuint getMeshID()
		{
			return mMeshID;
		}

		void addInstance(const glm::mat4 &instanceMMatrix = glm::mat4())
		{
			mpVertexArray->addInstance(instanceMMatrix);
		}

		virtual std::string getShaderMacros()
		{
			std::string macros = "";

			macros += mpMaterial->getShaderMacros();
			macros += mpVertexArray->getShaderMacros();

			return macros;
		}

		//upload the mesh information to the device
		virtual void uploadToDevice()
		{
			mpMaterial->uploadToDevice();
			mpVertexArray->uploadToDevice();
			mbHasTexCoord = mpVertexArray->hasTexCoord();
			mbHasVertexColor = mpVertexArray->hasVertexColor();
			mbUploaded = true;
		}

		virtual void draw(const GLuint &programID)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current mesh has not been uploaded befor drawing");
				return;
			}

			int uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");
			glUniform1ui(uMeshIDLoc, mMeshID);

			mpMaterial->active(programID, mbHasTexCoord, mbHasVertexColor);
			mpVertexArray->draw(programID);
		}

	protected:
		//For the inherited classes of the mesh£¬
		//They must set the material and the vertex array explicite in
		//the constructor function
		Mesh() {}

		//The mpMaterial must be assign to in the constructor
		//or the inherited class' constructor
		std::shared_ptr<Material> mpMaterial;

		//The mpVertexArray must be assign to in the constructor
		//or the inherited class' constructor
		std::shared_ptr<VertexArray> mpVertexArray;
		bool mbHasTexCoord, mbHasVertexColor;

		bool mbUploaded;

		//glm::mat4 mModelMatrix;

		GLuint mMeshID;
	};
}