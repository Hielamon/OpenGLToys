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

		friend class SceneAssimpLoader;
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

	class SceneAssimpLoader
	{
	public:
		SceneAssimpLoader(const std::string &path, std::shared_ptr<Scene> &pScene, bool bFlipUV = false)
			: mpScene(pScene)
		{
			mbLoadSuccess = false;
			mExistedMeshNum = mpScene->mvpMesh.size();
			//mExistedMaterialNum = mScene.mvpMaterial.size();

			Assimp::Importer import;
			unsigned int flag = aiProcess_Triangulate;
			if (bFlipUV) flag |= aiProcess_FlipUVs;

			HL_INTERVAL_START;
			const aiScene *aiscene = import.ReadFile(path.c_str(), flag);
			HL_INTERVAL_ENDSTR("import.ReadFile");

			if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode)
			{
				SP_CERR("ERROR::ASSIMP::" + std::string(import.GetErrorString()));
				//exit(-1);
				return;
			}

			mDirectory = path.substr(0, path.find_last_of("/\\"));

			mTypeMap = TextureGlobal::getInstance().aiTypeMap;

			HL_INTERVAL_START;
			_loadToScene(aiscene);
			HL_INTERVAL_ENDSTR("_loadToScene(aiscene)");
			mbLoadSuccess = true;
		}
		~SceneAssimpLoader() {}

		bool IsLoadSuccess()
		{
			return mbLoadSuccess;
		}

	private:
		std::shared_ptr<Scene> &mpScene;
		std::string mDirectory;
		int mExistedMeshNum;
		//int mExistedMaterialNum;
		bool mbLoadSuccess;

		//The map between the aiTextureType and TextureType of SPhoenix library
		std::map<aiTextureType, TextureType> mTypeMap;

		//The map of the texture and its path
		std::map<std::string, std::shared_ptr<Texture>> mmpPathTextureLoaded;

	private:
		void _loadToScene(const aiScene *aiscene)
		{
			//Loading all materials in the scene
			std::vector<std::shared_ptr<Material>> vpMaterial(aiscene->mNumMaterials);
			for (size_t i = 0; i < aiscene->mNumMaterials; i++)
			{
				aiMaterial *aimaterial = aiscene->mMaterials[i];
				vpMaterial[i] = _loadMaterial(aimaterial);
			}

			//Loading all meshes in the scene
			for (size_t i = 0; i < aiscene->mNumMeshes; i++)
			{
				aiMesh *aimesh = aiscene->mMeshes[i];
				std::shared_ptr<VertexArray> pVertexArray = _loadVertexArray(aimesh);
				std::shared_ptr<Material> pMaterial = vpMaterial[aimesh->mMaterialIndex];
				
				//Add the model to mScene, but the instanceN are still zero
				mpScene->addMesh(std::make_shared<Mesh>(pVertexArray, pMaterial));
			}

			//Traveling the scene tree for retrieving the instanceN vector and model matrix array
			glm::mat4 initModelMatrix;
			_processNode(aiscene->mRootNode, initModelMatrix);
		}

		std::shared_ptr<VertexArray> _loadVertexArray(const aiMesh *aimesh)
		{
			//Loading the vertex array information
			std::shared_ptr<std::vector<glm::vec3>> pvVertice = std::make_shared<std::vector<glm::vec3>>();
			std::shared_ptr<std::vector<glm::vec3>> pvNormal = std::make_shared<std::vector<glm::vec3>>();
			std::shared_ptr<std::vector<glm::vec3>> pvColor = std::make_shared<std::vector<glm::vec3>>();
			std::shared_ptr<std::vector<glm::vec2>> pvTexCoord = std::make_shared<std::vector<glm::vec2>>();
			std::shared_ptr<std::vector<GLuint>> pvIndice = std::make_shared<std::vector<GLuint>>();

			pvVertice->reserve(aimesh->mNumVertices);
			for (size_t i = 0; i < aimesh->mNumVertices; i++)
			{
				pvVertice->push_back(glm::vec3(aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z));
			}

			if (aimesh->mNormals)
			{
				pvNormal->reserve(aimesh->mNumVertices);
				for (size_t i = 0; i < aimesh->mNumVertices; i++)
				{
					pvNormal->push_back(glm::vec3(aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z));
				}
			}

			if (aimesh->mColors[0])
			{
				pvColor->reserve(aimesh->mNumVertices);
				for (size_t i = 0; i < aimesh->mNumVertices; i++)
				{
					pvColor->push_back(glm::vec3(aimesh->mColors[0][i].r, aimesh->mColors[0][i].g, aimesh->mColors[0][i].b));
				}
			}

			if (aimesh->mTextureCoords[0])
			{
				pvTexCoord->reserve(aimesh->mNumVertices);
				for (size_t i = 0; i < aimesh->mNumVertices; i++)
				{
					pvTexCoord->push_back(glm::vec2(aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y));
				}
			}

			for (size_t i = 0; i < aimesh->mNumFaces; i++)
			{
				aiFace &face = aimesh->mFaces[i];
				for (size_t j = 0; j < face.mNumIndices; j++)
				{
					pvIndice->push_back(face.mIndices[j]);
				}
			}

			std::shared_ptr<VertexArray> pVertexArray;

			if (pvTexCoord->size() == aimesh->mNumVertices)
			{
				if (pvNormal->size() == aimesh->mNumVertices)
				{
					std::shared_ptr<VertexArrayNTc> pVertexArrayNTc =
						std::make_shared<VertexArrayNTc>(pvVertice, pvNormal, pvTexCoord, pvIndice);

					pVertexArray = std::static_pointer_cast<VertexArray>(pVertexArrayNTc);
				}
				else
				{
					std::shared_ptr<VertexArrayTc> pVertexArrayTc =
						std::make_shared<VertexArrayTc>(pvVertice, pvTexCoord, pvIndice);

					pVertexArray = std::static_pointer_cast<VertexArray>(pVertexArrayTc);
				}
			}
			else if(pvColor->size() == aimesh->mNumVertices)
			{
				if (pvNormal->size() == aimesh->mNumVertices)
				{
					std::shared_ptr<VertexArrayNC> pVertexArrayNC =
						std::make_shared<VertexArrayNC>(pvVertice, pvNormal, pvColor, pvIndice);

					pVertexArray = std::static_pointer_cast<VertexArray>(pVertexArrayNC);
				}
				else
				{
					std::shared_ptr<VertexArrayC> pVertexArrayC =
						std::make_shared<VertexArrayC>(pvVertice, pvColor, pvIndice);

					pVertexArray = std::static_pointer_cast<VertexArray>(pVertexArrayC);
				}
			}
			else
			{
				if (pvNormal->size() == aimesh->mNumVertices)
				{
					std::shared_ptr<VertexArrayN> pVertexArrayN =
						std::make_shared<VertexArrayN>(pvVertice, pvNormal, pvIndice);

					pVertexArray = std::static_pointer_cast<VertexArray>(pVertexArrayN);
				}
				else
				{
					pVertexArray = std::make_shared<VertexArray>(pvVertice, pvIndice);
				}
			}

			return pVertexArray;
		}

		std::shared_ptr<Material>  _loadMaterial(const aiMaterial *aimaterial)
		{
			std::shared_ptr<Material> pMaterial;
			for (auto &typePair : mTypeMap)
			{
				GLuint textureNum = aimaterial->GetTextureCount(typePair.first);
				for (size_t i = 0; i < textureNum; i++)
				{
					aiString filename;
					aimaterial->GetTexture(typePair.first, i, &filename);
					std::string path = filename.C_Str();
					if (path.find(':') != 1)
					{
						//The file name is not a absolute path
						path = mDirectory + "/" + path;
					}

					std::shared_ptr<Texture> tex;
					if (mmpPathTextureLoaded.find(path) == mmpPathTextureLoaded.end())
					{
						tex = std::make_shared<Texture>(path, typePair.second);
						if (!tex->IsValid()) continue;
						mmpPathTextureLoaded[path] = tex;
					}
					else
					{
						tex = mmpPathTextureLoaded[path];
					}

					if (pMaterial.use_count() == 0)
					{
						pMaterial = std::make_shared<Material>();
					}

					pMaterial->addTexture(tex);
				}
			}
			return pMaterial;
		}

		void _processNode(aiNode *ainode, const glm::mat4 &topMMatrix)
		{
			aiMatrix4x4 &T = ainode->mTransformation;
			glm::mat4 relMMatrix(T.a1, T.b1, T.c1, T.d1,
								 T.a2, T.b2, T.c2, T.d2,
								 T.a3, T.b3, T.c3, T.d3,
								 T.a4, T.b4, T.c4, T.d4);

			glm::mat4 curMMatrix = topMMatrix * relMMatrix;
			for (size_t i = 0; i < ainode->mNumMeshes; i++)
			{
				mpScene->mvpMesh[ainode->mMeshes[i] + mExistedMeshNum]->addInstance(curMMatrix);
			}

			for (size_t i = 0; i < ainode->mNumChildren; i++)
			{
				_processNode(ainode->mChildren[i], curMMatrix);
			}
		}
	};
}

