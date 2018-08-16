#pragma once

#include "Shader.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

			setCommonShaderCodes(defaultShader);
		}

		~Scene() {}

		friend class SceneAssimpLoader;
		friend class SceneUtil;

		/*void addGeometry(Geometry &geometry)
		{
			mvpGeometry.push_back(std::make_shared<Geometry>(geometry));
		}*/

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

	protected:
		std::shared_ptr<ShaderCodes> mpCommonShaderCodes;

		//std::vector<std::shared_ptr<Geometry>> mvpGeometry;
		std::vector<std::shared_ptr<Mesh>> mvpMesh;
		//Which holds the same size with mvpMesh, if the corresponding
		//pShaderCoders is empty, the commonShaderCodes will be applied
		std::vector<std::shared_ptr<ShaderCodes>> mvpShaderCoders;

		//TODO: whether set the top model matrix
		glm::mat4 mTopModelMatrix;
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
				if (mmLabelToShader.find(label) == mmLabelToShader.end())
				{
					std::shared_ptr<ShaderCodes> pShaderCodes_ =
						std::make_shared<ShaderCodes>(*pShaderCodes);
					pShaderCodes_->addMacros(macros);

					mmLabelToShader[label] = std::make_shared<ShaderUtil>(pShaderCodes_);
					mmLabelToMeshes[label] = std::vector<std::shared_ptr<MeshUtil>>();

					// Using uniform buffers & Bind the UMatrices uniform block 
					// index to 1
					GLuint programID = mmLabelToShader[label]->getProgramID();
					GLuint ViewUBOIndex = glGetUniformBlockIndex(programID, "ViewUBO");
					glUniformBlockBinding(programID, ViewUBOIndex, VIEWUBO_BINDING_POINT);
				}

				mmLabelToMeshes[label].push_back(pMeshUtil);
			}
		}

		~SceneUtil() {}

		void draw()
		{
			std::map<std::string, std::shared_ptr<ShaderUtil>>::iterator iter;
			for (iter = mmLabelToShader.begin();
				 iter != mmLabelToShader.end(); iter++)
			{
				iter->second->useProgram();
				std::vector<std::shared_ptr<MeshUtil>> &vpMeshUtil = 
					mmLabelToMeshes[iter->first];

				for (size_t i = 0; i < vpMeshUtil.size(); i++)
				{
					vpMeshUtil[i]->draw();
				}
			}
		}

	private:
		std::shared_ptr<Scene> mpScene;
		
		std::map<std::string, std::shared_ptr<ShaderUtil>> mmLabelToShader;
		std::map<std::string, std::vector<std::shared_ptr<MeshUtil>>> mmLabelToMeshes;
	};

	class SceneAssimpLoader
	{
	public:
		SceneAssimpLoader(const std::string &path, std::shared_ptr<Scene> &pScene)
			: mpScene(pScene)
		{
			mbLoadSuccess = false;
			mExistedMeshNum = mpScene->mvpMesh.size();
			//mExistedMaterialNum = mScene.mvpMaterial.size();

			Assimp::Importer import;
			const aiScene *aiscene = import.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

			if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode)
			{
				SP_CERR("ERROR::ASSIMP::" + std::string(import.GetErrorString()));
				//exit(-1);
				return;
			}

			mDirectory = path.substr(0, path.find_last_of("/\\"));

			mTypeMap = TextureGlobal::getInstance().aiTypeMap;

			_loadToScene(aiscene);
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
			_processNode(aiscene->mRootNode, mpScene->mTopModelMatrix);
		}

		std::shared_ptr<VertexArray> _loadVertexArray(const aiMesh *aimesh)
		{
			//Loading the vertex array information
			std::shared_ptr<std::vector<glm::vec3>> pvVertice = std::make_shared<std::vector<glm::vec3>>();
			std::shared_ptr<std::vector<glm::vec3>> pvNormal = std::make_shared<std::vector<glm::vec3>>();
			std::shared_ptr<std::vector<glm::vec2>> pvTexCoord = std::make_shared<std::vector<glm::vec2>>();
			std::shared_ptr<std::vector<GLuint>> pvIndice = std::make_shared<std::vector<GLuint>>();

			for (size_t i = 0; i < aimesh->mNumVertices; i++)
			{
				pvVertice->push_back(glm::vec3(aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z));
				pvNormal->push_back(glm::vec3(aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z));

			}

			if (aimesh->mTextureCoords[0])
			{
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

			std::shared_ptr<VertexArrayTc> pVertexArrayTc =
				std::make_shared<VertexArrayTc>(pvVertice, pvNormal, pvTexCoord, pvIndice);

			return std::static_pointer_cast<VertexArray>(pVertexArrayTc);
		}

		std::shared_ptr<Material>  _loadMaterial(const aiMaterial *aimaterial)
		{
			std::shared_ptr<Material> pMaterial;
			for (auto &typePair : mTypeMap)
			{
				for (size_t i = 0; i < aimaterial->GetTextureCount(typePair.first); i++)
				{
					aiString filename;
					aimaterial->GetTexture(typePair.first, i, &filename);
					std::string path = mDirectory + "/" + std::string(filename.C_Str());
					std::shared_ptr<Texture> tex;
					if (mmpPathTextureLoaded.find(path) == mmpPathTextureLoaded.end())
					{
						tex = std::make_shared<Texture>(path, typePair.second);
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

			glm::mat4 curMMatrix = relMMatrix * topMMatrix;
			for (size_t i = 0; i < ainode->mNumMeshes; i++)
			{
				mpScene->mvpMesh[i + mExistedMeshNum]->addInstance(curMMatrix);
			}

			for (size_t i = 0; i < ainode->mNumChildren; i++)
			{
				_processNode(ainode->mChildren[i], curMMatrix);
			}
		}
	};
}

