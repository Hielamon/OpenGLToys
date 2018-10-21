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
			 std::make_shared<Material>(),
			 const glm::mat4 &relMMatrix = glm::mat4(1.0f))
			: mbUploaded(false), mRelMMatrix(relMMatrix),
			mbAccessible(true), mbVisible(true), mbInFrustum(false)
		{
			reset(pVertexArray, pMaterial);

			MeshGlobal::getInstance().totalMeshCount++;
			mMeshID = MeshGlobal::getInstance().totalMeshCount;
		}

		Mesh(const Mesh &mesh)
		{
			mbUploaded = mesh.mbUploaded;
			mRelMMatrix = mesh.mRelMMatrix;
			mbAccessible = mesh.mbAccessible;
			mbVisible = mesh.mbVisible;
			mbInFrustum = mesh.mbInFrustum;

			reset(mesh.mpVertexArray, mesh.mpMaterial);

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
			mbHasTexCoord = mpVertexArray->hasTexCoord();
			mbHasVertexColor = mpVertexArray->hasVertexColor();

			if (mbUploaded) mpVertexArray->uploadToDevice();

			//Update the variables related to the bounding box
			updateBBox();
		}

		std::shared_ptr<VertexArray> getVertexArray()
		{
			return mpVertexArray;
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

		//For active same type materials only once
		std::shared_ptr<Material> getMaterial()
		{
			return mpMaterial;
		}
		
		//For active same type materials only once
		bool getHasTexCoord()
		{
			return mbHasTexCoord;
		}
		
		//For active same type materials only once
		bool getHasVertexColor()
		{
			return mbHasVertexColor;
		}

		/**************************************************************************/
		/*****************************Instance operation***************************/

		void setInstanceMMatrix(const glm::mat4 &instanceMMatrix,
								GLuint instanceID)
		{
			mpVertexArray->setInstanceMMatrix(instanceMMatrix, instanceID);

			//Update the variables related to the bounding box
			updateBBox();
		}

		glm::mat4 getInstanceMMatrix(GLuint instanceID)
		{
			return mpVertexArray->getInstanceMMatrix(instanceID);
		}

		void setRelInstanceMMatrix(const glm::mat4 &relInstanceMMatrix,
								GLuint instanceID)
		{
			mpVertexArray->setRelInstanceMMatrix(relInstanceMMatrix, mRelMMatrix,
												 instanceID);

			//Update the variables related to the bounding box
			updateBBox();
		}

		glm::mat4 getRelInstanceMMatrix(GLuint instanceID)
		{
			return mpVertexArray->getRelInstanceMMatrix(instanceID);
		}

		void setRelMMatrix(const glm::mat4 &relMMatrix)
		{
			mRelMMatrix = relMMatrix;
			int numInstance = mpVertexArray->getNumInstance();
			for (size_t i = 0; i < numInstance; i++)
			{
				glm::mat4 instanceMMatrix = mRelMMatrix *
					mpVertexArray->getRelInstanceMMatrix(i);

				mpVertexArray->setInstanceMMatrix(instanceMMatrix, i);
			}

			//Update the variables related to the bounding box
			updateBBox();
		}

		//Means to transform all instances of the mesh with T matrix
		void transformMesh(const glm::mat4 &T)
		{
			mpVertexArray->transformAllInstances(T);

			//Update the variables related to the bounding box
			updateBBox();
		}

		int getNumInstance()
		{
			return mpVertexArray->getNumInstance();
		}

		void clearAllInstance()
		{
			mpVertexArray->clearAllInstance();
		}

		/*****************************Instance operation***************************/
		/**************************************************************************/


		const BBox& getTotalBBox()
		{
			return mBBox;
		}

		const std::vector<glm::vec3> &getBBoxPoints()
		{
			return mvBBoxVertice;
		}

		GLuint getMeshID()
		{
			return mMeshID;
		}

		void addInstance(const glm::mat4 &instanceMMatrix = glm::mat4())
		{
			mpVertexArray->addInstance(instanceMMatrix);
			//mpVertexArray->addRelInstance(instanceMMatrix, mRelMMatrix);

			//Update the variables related to the bounding box
			updateBBox();
		}

		//Access the accessible variable
		bool getAccessible()
		{
			return mbAccessible;
		}

		void setAccessible(bool bAccessible)
		{
			mbAccessible = bAccessible;
		}

		//Access the visible variable
		bool getVisible()
		{
			return mbVisible;
		}

		void setVisible(bool bVisible)
		{
			mbVisible = bVisible;
		}

		//Access the InFrustum variable
		bool getInFrustum()
		{
			return mbInFrustum;
		}

		void setInFrustum(bool bInFrustum)
		{
			mbInFrustum = bInFrustum;
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
			if (mbUploaded) return;
			mpMaterial->uploadToDevice();
			mpVertexArray->uploadToDevice();
			mbUploaded = true;
		}

		virtual void drawOnlyInScene(const GLuint &programID)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current mesh has not been uploaded befor drawing");
				return;
			}

			mpVertexArray->draw(programID);
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
		Mesh() : mbUploaded(false), mRelMMatrix(glm::mat4(1.0f)),
			mbAccessible(true), mbVisible(true), mbInFrustum(false)
		{
			MeshGlobal::getInstance().totalMeshCount++;
			mMeshID = MeshGlobal::getInstance().totalMeshCount;
		}

		void updateBBox()
		{
			mBBox = mpVertexArray->getTotalBBox();
			mBBox.padding(BOUNDING_BOX_PADDING);
			mvBBoxVertice = mBBox.getBBoxVertices();
		}

		//The mpMaterial must be assign to in the constructor
		//or the inherited class' constructor
		std::shared_ptr<Material> mpMaterial;

		//The mpVertexArray must be assign to in the constructor
		//or the inherited class' constructor
		std::shared_ptr<VertexArray> mpVertexArray;
		bool mbHasTexCoord, mbHasVertexColor;

		bool mbUploaded;

		glm::mat4 mRelMMatrix;

		BBox mBBox;
		std::vector<glm::vec3> mvBBoxVertice;

		//The top switch for determining whether to render the mesh
		bool mbAccessible;
		//Indicate whether the mesh can be viewed in the current frustum
		bool mbVisible;
		//Indicate whether the mesh is totally included in the current frustum
		bool mbInFrustum;

		GLuint mMeshID;
	};
}