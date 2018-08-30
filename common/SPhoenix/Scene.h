#pragma once

#include "Shader.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <commonMacro.h>

namespace SP
{
	class Scene;
	class SceneUtil;

	class Scene
	{
	public:
		Scene(ShaderCodes &commonShaderCodes)
		{
			setCommonShaderCodes(commonShaderCodes);
		}

		Scene()
		{
			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			ShaderCodes defaultShader(__currentPATH + "/Shaders/SPhoenixScene.vert", 
									  __currentPATH + "/Shaders/SPhoenixScene.frag");

			/*ShaderCodes defaultShader(__currentPATH + "/Shaders/SPhoenixScene-MeshID.vert",
									  __currentPATH + "/Shaders/SPhoenixScene-MeshID.frag");*/

			setCommonShaderCodes(defaultShader);
		}

		~Scene() {}

		friend class SceneUtil;

		void addMesh(const std::shared_ptr<Mesh>& pMesh,
					 const std::shared_ptr<ShaderCodes> &pShaderCodes = nullptr)
		{
			mvpMesh.push_back(pMesh);
			mvpShaderCoders.push_back(pShaderCodes);
		}

		std::shared_ptr<Mesh> getMesh(GLuint index)
		{
			if (mvpMesh.size() > index)
			{
				return mvpMesh[index];
			}
			else
			{
				return nullptr;
			}
		}

		int getNumMesh()
		{
			return mvpMesh.size();
		}

		void setCommonShaderCodes(ShaderCodes &shaderCodes)
		{
			mpCommonShaderCodes = std::make_shared<ShaderCodes>(shaderCodes);
		}

		void setTopModelMatrix(const glm::mat4 &modelmatrix)
		{
			mTopModelMatrix = modelmatrix;
		}

		glm::mat4 getTopModelMatrix()
		{
			return mTopModelMatrix;
		}

		//Get the total bounding box of all meshes
		BBox getBoundingBox()
		{
			BBox resBBox;
			for (size_t i = 0; i < mvpMesh.size(); i++)
			{
				resBBox += mvpMesh[i]->getBoundingBox();
			}
			return resBBox;
		}

		std::vector<BBox> getAllMeshBBoxes()
		{
			std::vector<BBox> vBBox;
			for (size_t i = 0; i < mvpMesh.size(); i++)
			{
				vBBox.push_back(mvpMesh[i]->getBoundingBox());
			}

			return vBBox;
		}

		std::vector<BBox> getAllInstanceBBoxes()
		{
			std::vector<BBox> vBBox;
			for (size_t i = 0; i < mvpMesh.size(); i++)
			{
				std::vector<BBox> vBBoxTmp = mvpMesh[i]->getAllBoundingBoxes();
				vBBox.insert(vBBox.end(), vBBoxTmp.begin(), vBBoxTmp.end());
			}

			return vBBox;
		}

		//For indicating whether the VertexArray has been uploaded to the GPU memory
		void setSceneUtil(const std::shared_ptr<SceneUtil> &pSceneUtil)
		{
			mpSceneUtil = pSceneUtil;
		}

		//Clearing the state of uploading to the GPU memory
		void resetSceneUtil()
		{
			mpSceneUtil.reset();
		}

		//Get if the VertexArray has been uploaded by accessing the mpVertexArrayUtil's state
		bool IsUploaded()
		{
			std::shared_ptr<SceneUtil> pSceneUtil = mpSceneUtil.lock();
			return pSceneUtil.use_count() != 0;
		}

	protected:
		std::shared_ptr<ShaderCodes> mpCommonShaderCodes;

		//std::vector<std::shared_ptr<Geometry>> mvpGeometry;
		std::vector<std::shared_ptr<Mesh>> mvpMesh;
		//Which holds the same size with mvpMesh, if the corresponding
		//pShaderCoders is empty, the commonShaderCodes will be applied
		std::vector<std::shared_ptr<ShaderCodes>> mvpShaderCoders;

		//TODO: whether set the top model matrix
		glm::mat4 mTopModelMatrix;

		//For indicating whether the Scene has been uploaded to the GPU memory
		std::weak_ptr<SceneUtil> mpSceneUtil;
	};

	class SceneUtil
	{
	public:
		SceneUtil(const std::shared_ptr<Scene> &pScene)
			: mpScene(pScene)
		{
			std::stringstream ioStr;
			for (size_t i = 0; i < mpScene->mvpMesh.size(); i++)
			{
				std::shared_ptr<Mesh> &pMesh = mpScene->mvpMesh[i];
				ioStr.str("");

				std::shared_ptr<ShaderCodes> &pShaderCodes =
					mpScene->mvpShaderCoders[i].use_count() == 0 ?
					mpScene->mpCommonShaderCodes : mpScene->mvpShaderCoders[i];

				std::string macros = pMesh->getShaderMacros();
				ioStr << macros << "//" << pShaderCodes;
				std::string label = ioStr.str();

				std::shared_ptr<MeshUtil> pMeshUtil = std::make_shared<MeshUtil>(pMesh);
				
				pMesh->setMeshUtil(pMeshUtil);
				if (mmLabelToShader.find(label) == mmLabelToShader.end())
				{
					std::shared_ptr<ShaderCodes> pShaderCodes_ =
						std::make_shared<ShaderCodes>(*pShaderCodes);
					pShaderCodes_->addMacros(macros);

					mmLabelToShader[label] = std::make_shared<ShaderUtil>(pShaderCodes_);
					mmLabelToMeshes[label] = std::map<GLuint, std::shared_ptr<MeshUtil>>();

					// Using uniform buffers & Bind the UMatrices uniform block 
					// index to 1
					GLuint programID = mmLabelToShader[label]->getProgramID();
					GLuint ViewUBOIndex = glGetUniformBlockIndex(programID, "ViewUBO");
					glUniformBlockBinding(programID, ViewUBOIndex, VIEWUBO_BINDING_POINT);
				}
				
				GLuint currMeshID = pMeshUtil->getMeshID();
				mmLabelToMeshes[label][currMeshID] = pMeshUtil;
				mmMeshIDToMesh[currMeshID] = pMeshUtil;

				mmMeshToLabel[pMeshUtil] = label;
			}
		}

		~SceneUtil() {}

		void reset()
		{
			mmLabelToShader.clear();
			mmLabelToMeshes.clear();
			mmMeshToLabel.clear();
			mmMeshIDToMesh.clear();
		}

		virtual void draw()
		{
			std::map<std::string, std::shared_ptr<ShaderUtil>>::iterator iter;
			for (iter = mmLabelToShader.begin();
				 iter != mmLabelToShader.end(); iter++)
			{
				iter->second->useProgram();
				GLint programID = iter->second->getProgramID();
				GLint tMMatrixLoc = glGetUniformLocation(programID, "topMMatrix");
				glUniformMatrix4fv(tMMatrixLoc, 1, GL_FALSE,
								   glm::value_ptr(mpScene->mTopModelMatrix));

				std::map<GLuint, std::shared_ptr<MeshUtil>> &vpMeshUtil = 
					mmLabelToMeshes[iter->first];

				std::map<GLuint, std::shared_ptr<MeshUtil>>::iterator iter;
				for (iter = vpMeshUtil.begin(); iter != vpMeshUtil.end(); iter++)
				{
					iter->second->draw();
				}
			}
		}

		//if the id is not valid , vMeshID will be changed, which will remove the invalid ID
		virtual void drawByMeshIDs(std::list<GLuint> &vMeshID)
		{
			std::list<GLuint>::iterator iter;
			for (iter = vMeshID.begin(); iter != vMeshID.end(); )
			{
				if (mmMeshIDToMesh.find(*iter) == mmMeshIDToMesh.end())
				{
					iter = vMeshID.erase(iter);
					continue;
				}

				std::shared_ptr<MeshUtil> &pMeshUtil = mmMeshIDToMesh[*iter];
				std::string &label = mmMeshToLabel[pMeshUtil];
				std::shared_ptr<ShaderUtil> &pShaderUtil = mmLabelToShader[label];

				pShaderUtil->useProgram();
				GLint programID = pShaderUtil->getProgramID();
				GLint tMMatrixLoc = glGetUniformLocation(programID, "topMMatrix");
				glUniformMatrix4fv(tMMatrixLoc, 1, GL_FALSE,
								   glm::value_ptr(mpScene->mTopModelMatrix));
				pMeshUtil->draw();
				iter++;
			}
		}

		void addMeshUtil(const std::shared_ptr<MeshUtil>& pMeshUtil,
						 const std::shared_ptr<ShaderCodes> &pShaderCodes = nullptr)
		{
			const std::shared_ptr<ShaderCodes> &pShaderCodesUsed =
				pShaderCodes.use_count() == 0 ?
				mpScene->mpCommonShaderCodes : pShaderCodes;

			std::stringstream ioStr;
			std::string macros = pMeshUtil->getShaderMacros();
			ioStr << macros << "//" << pShaderCodes;
			std::string label = ioStr.str();

			if (mmLabelToShader.find(label) == mmLabelToShader.end())
			{
				std::shared_ptr<ShaderCodes> pShaderCodes_ =
					std::make_shared<ShaderCodes>(*pShaderCodesUsed);
				pShaderCodes_->addMacros(macros);

				mmLabelToShader[label] = std::make_shared<ShaderUtil>(pShaderCodes_);
				mmLabelToMeshes[label] = std::map<GLuint, std::shared_ptr<MeshUtil>>();

				// Using uniform buffers & Bind the UMatrices uniform block 
				// index to 1
				GLuint programID = mmLabelToShader[label]->getProgramID();
				GLuint ViewUBOIndex = glGetUniformBlockIndex(programID, "ViewUBO");
				glUniformBlockBinding(programID, ViewUBOIndex, VIEWUBO_BINDING_POINT);
			}

			GLuint currMeshID = pMeshUtil->getMeshID();
			mmLabelToMeshes[label][currMeshID] = pMeshUtil;
			mmMeshIDToMesh[currMeshID] = pMeshUtil;

			mmMeshToLabel[pMeshUtil] = label;
		}

		std::vector<std::shared_ptr<MeshUtil>> getMeshUtils()
		{
			std::vector<std::shared_ptr<MeshUtil>> vMeshUtil(mmMeshToLabel.size());

			std::map<std::shared_ptr<MeshUtil>, std::string>::iterator iter;
			int i = 0;
			for (iter = mmMeshToLabel.begin(); iter != mmMeshToLabel.end(); iter++, i++)
			{
				vMeshUtil[i] = iter->first;
			}
			return vMeshUtil;
		}

	protected:
		SceneUtil() {}

		std::shared_ptr<Scene> mpScene;
		
		std::map<std::shared_ptr<MeshUtil>, std::string> mmMeshToLabel;
		std::map<GLuint, std::shared_ptr<MeshUtil>> mmMeshIDToMesh;
		std::map<std::string, std::shared_ptr<ShaderUtil>> mmLabelToShader;
		std::map<std::string, std::map<GLuint, std::shared_ptr<MeshUtil>>> mmLabelToMeshes;
	};
	
	class SceneColorIDUtil : public SceneUtil
	{
	public:
		SceneColorIDUtil() = delete;
		SceneColorIDUtil(const std::shared_ptr<SceneUtil> &pSceneUtil)
			: SceneUtil(*pSceneUtil)
		{
			reset();

			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			std::shared_ptr<ShaderCodes> pDefaultShader =
				std::make_shared<ShaderCodes>(__currentPATH + "/Shaders/SPhoenixScene.vert",
											  __currentPATH + "/Shaders/SPhoenixScene.frag");

			std::string undefColor = "#undef HAVE_COLOR\n";
			pDefaultShader->addMacros(undefColor);

			std::vector<std::shared_ptr<MeshUtil>> vExistedMesh = pSceneUtil->getMeshUtils();
			for (size_t i = 0; i < vExistedMesh.size(); i++)
			{
				std::shared_ptr<MeshUtil> pMeshUtil =
					std::make_shared<MeshUtil>(*(vExistedMesh[i]));

				//Compute the ID Color
				double totalColor = MeshGlobal::getInstance().totalMeshN + 1;
				int Nc = std::ceil(std::pow(totalColor, 1.0 / 3));
				int ID = pMeshUtil->getMeshID();
				glm::vec4 uIDColor(1.0f);
				for (size_t i = 0; i < 3 && ID > 0; i++)
				{
					uIDColor[i] = (ID % Nc) / float(Nc);
					ID = (ID - ID % Nc) / Nc;
				}

				//uIDColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

				std::shared_ptr<Material> pMaterial = std::make_shared<Material>(uIDColor);
				pMeshUtil->setMaterialUtil(pMaterial);

				addMeshUtil(pMeshUtil, pDefaultShader);
			}
		}

		~SceneColorIDUtil() {}
	private:

	};

	/*class SceneSelectedUtil : public SceneUtil
	{
	public:
		SceneSelectedUtil() = delete;
		SceneSelectedUtil(const std::shared_ptr<SceneUtil> &pSceneUtil)
			: SceneUtil(*pSceneUtil)
		{
			reset();

			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			std::shared_ptr<ShaderCodes> pDefaultShader =
				std::make_shared<ShaderCodes>(__currentPATH + "/Shaders/SPhoenixScene.vert",
											  __currentPATH + "/Shaders/SPhoenixScene.frag");

			std::string undefColor = "#undef HAVE_COLOR\n";
			pDefaultShader->addMacros(undefColor);

			std::vector<std::shared_ptr<MeshUtil>> vExistedMesh = pSceneUtil->getMeshUtils();
			for (size_t i = 0; i < vExistedMesh.size(); i++)
			{
				std::shared_ptr<MeshUtil> pMeshUtil =
					std::make_shared<MeshUtil>(*(vExistedMesh[i]));

				std::shared_ptr<Material> pMaterial = std::make_shared<Material>();
				pMeshUtil->setMaterialUtil(pMaterial);

				addMeshUtil(pMeshUtil, pDefaultShader);
			}
		}

		~SceneSelectedUtil() {}

		virtual void drawByMeshIDs(std::list<GLuint> &vMeshID)
		{
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			SceneUtil::drawByMeshIDs(vMeshID);
			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}

	private:
	};*/

	class SceneSelectedUtil : public SceneUtil
	{
	public:
		SceneSelectedUtil() = delete;
		SceneSelectedUtil(const std::shared_ptr<SceneUtil> &pSceneUtil)
			: SceneUtil(*pSceneUtil)
		{
			reset();

			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			std::shared_ptr<ShaderCodes> pDefaultShader =
				std::make_shared<ShaderCodes>(__currentPATH + "/Shaders/SPhoenixScene.vert",
											  __currentPATH + "/Shaders/SPhoenixScene.frag");

			std::string undefColor = "#undef HAVE_COLOR\n";
			pDefaultShader->addMacros(undefColor);

			std::vector<std::shared_ptr<MeshUtil>> vExistedMesh = pSceneUtil->getMeshUtils();
			for (size_t i = 0; i < vExistedMesh.size(); i++)
			{
				std::shared_ptr<MeshUtil> pMeshUtil =
					std::make_shared<MeshUtil>(*(vExistedMesh[i]));

				std::shared_ptr<Material> pMaterial = std::make_shared<Material>();
				pMeshUtil->setMaterialUtil(pMaterial);

				addMeshUtil(pMeshUtil, pDefaultShader);
			}
		}

		~SceneSelectedUtil() {}

		virtual void drawByMeshIDs(std::list<GLuint> &vMeshID)
		{
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			SceneUtil::drawByMeshIDs(vMeshID);
			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}

	private:
	};

}

