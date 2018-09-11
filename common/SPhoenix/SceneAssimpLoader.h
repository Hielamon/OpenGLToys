#pragma once

#include "Scene.h"

namespace SP
{
	//The assimp scene loader 
	class SceneAssimpLoader
	{
	public:
		SceneAssimpLoader() {}

		~SceneAssimpLoader() {}

		bool loadScene(const std::string &path, std::shared_ptr<Scene> &pScene, bool bFlipUV = false)
		{
			mpScene = pScene;

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
				return GL_FALSE;
			}

			mDirectory = path.substr(0, path.find_last_of("/\\"));

			mTypeMap = TextureGlobal::getInstance().mAitextypeToTextype;

			HL_INTERVAL_START;
			_loadToScene(aiscene);
			HL_INTERVAL_ENDSTR("_loadToScene(aiscene)");

			return GL_TRUE;
		}

	private:
		std::shared_ptr<Scene> mpScene;
		std::string mDirectory;
		//int mExistedMaterialNum;

		//The map between the aiTextureType and TextureType of SPhoenix library
		std::map<aiTextureType, TextureType> mTypeMap;

		//The vector stores the meshID of the assimp scene
		std::vector<GLuint> mvMeshID;

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
			mvMeshID.reserve(aiscene->mNumMeshes);
			for (size_t i = 0; i < aiscene->mNumMeshes; i++)
			{
				aiMesh *aimesh = aiscene->mMeshes[i];
				std::shared_ptr<VertexArray> pVertexArray = _loadVertexArray(aimesh);
				std::shared_ptr<Material> pMaterial = vpMaterial[aimesh->mMaterialIndex];

				//Add the model to mScene, but the instanceN are still zero
				std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVertexArray, pMaterial);
				mvMeshID.push_back(pMesh->getMeshID());

				mpScene->addMesh(pMesh);
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
			std::shared_ptr<std::vector<glm::vec4>> pvColor = std::make_shared<std::vector<glm::vec4>>();
			std::shared_ptr<std::vector<glm::vec2>> pvTexCoord = std::make_shared<std::vector<glm::vec2>>();
			std::shared_ptr<std::vector<GLuint>> pvIndice = std::make_shared<std::vector<GLuint>>();

			pvVertice->reserve(aimesh->mNumVertices);
			for (size_t i = 0; i < aimesh->mNumVertices; i++)
			{
				pvVertice->push_back(glm::vec3(aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z));
			}

			for (size_t i = 0; i < aimesh->mNumFaces; i++)
			{
				aiFace &face = aimesh->mFaces[i];
				for (size_t j = 0; j < face.mNumIndices; j++)
				{
					pvIndice->push_back(face.mIndices[j]);
				}
			}

			std::shared_ptr<VertexArray> pVertexArray =
				std::make_shared<VertexArray>(pvVertice, pvIndice,
											  PrimitiveType::TRIANGLES);
			if (aimesh->mNormals)
			{
				pvNormal->reserve(aimesh->mNumVertices);
				for (size_t i = 0; i < aimesh->mNumVertices; i++)
				{
					pvNormal->push_back(glm::vec3(aimesh->mNormals[i].x, 
												  aimesh->mNormals[i].y,
												  aimesh->mNormals[i].z));
				}

				pVertexArray->setNormals(pvNormal);
			}

			if (aimesh->mColors[0])
			{
				pvColor->reserve(aimesh->mNumVertices);
				for (size_t i = 0; i < aimesh->mNumVertices; i++)
				{
					pvColor->push_back(glm::vec4(aimesh->mColors[0][i].r,
												 aimesh->mColors[0][i].g,
												 aimesh->mColors[0][i].b,
												 aimesh->mColors[0][i].a));
				}
				pVertexArray->setColors(pvColor);
			}

			if (aimesh->mTextureCoords[0])
			{
				pvTexCoord->reserve(aimesh->mNumVertices);
				for (size_t i = 0; i < aimesh->mNumVertices; i++)
				{
					pvTexCoord->push_back(glm::vec2(aimesh->mTextureCoords[0][i].x,
													aimesh->mTextureCoords[0][i].y));
				}
				pVertexArray->setTexCoords(pvTexCoord);
			}
			
			return pVertexArray;
		}

		std::shared_ptr<Material>  _loadMaterial(const aiMaterial *aimaterial)
		{
			std::shared_ptr<Material> pMaterial = std::make_shared<Material>();
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
					pMaterial->addTexture(tex);
				}
			}

			//Load the diffuse, ambient and specular value
			//if these are existed
			aiColor4D diffuse(1.0f), ambient(1.0f), specular(0.0f);
			float shininess = 32;
			if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
			{
				pMaterial->setDiffuseColor(glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a));
				pMaterial->setAmbientColor(glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a));
			}
			if (aimaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS)
			{
				pMaterial->setAmbientColor(glm::vec4(ambient.r, ambient.g, ambient.b, ambient.a));
			}
			if (aimaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
			{
				pMaterial->setSpecularColor(glm::vec4(specular.r, specular.g, specular.b, specular.a));
			}
			if (aimaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
			{
				pMaterial->setShininess(shininess);
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
				GLuint meshID = mvMeshID[ainode->mMeshes[i]];
				mpScene->getMesh(meshID)->addInstance(curMMatrix);
			}

			for (size_t i = 0; i < ainode->mNumChildren; i++)
			{
				_processNode(ainode->mChildren[i], curMMatrix);
			}
		}
	};
}

