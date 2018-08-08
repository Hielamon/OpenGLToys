#pragma once

#include "Shader.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace SP
{
	class Model;

	class ModelLoaderFromAssimp
	{
	public:
		ModelLoaderFromAssimp(const std::string &path, Model &model)
			: mModel(model)
		{
			mModel.mbLoadSuccess = false;
			Assimp::Importer import;
			const aiScene *scene = import.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				SP_CERR("ERROR::ASSIMP::" + std::string(import.GetErrorString()));
				//exit(-1);
				return;
			}

			mDirectory = path.substr(0, path.find_last_of("/\\"));

			mTypeMap = {
				{ aiTextureType_AMBIENT, Tex_AMBIENT },
				{ aiTextureType_DIFFUSE, Tex_DIFFUSE },
				{ aiTextureType_SPECULAR, Tex_SPECULAR },
				{ aiTextureType_NORMALS, Tex_NORMALS }
			};

			_loadToModel(scene);
			mModel.mbLoadSuccess = true;
		}
		~ModelLoaderFromAssimp() {}

	private:
		Model &mModel;
		std::string mDirectory;

		//The map between the aiTextureType and TextureType of SPhoenix library
		std::map<aiTextureType, TextureType> mTypeMap;

		//The map of the texture and its path
		std::map<std::string, std::shared_ptr<Texture>> mpTextureLoaded;

	private:
		void _loadToModel(const aiScene *scene)
		{
			mModel._reallocateModel(scene->mNumMeshes);

			//Loading all meshes in the scene
			for (size_t i = 0; i < scene->mNumMeshes; i++)
			{
				aiMesh *mesh = scene->mMeshes[i];
				std::shared_ptr<VertexArray> pVertexArray = _loadVertexArray(mesh);
				
				aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<std::shared_ptr<Texture>> pTextures = _loadTextures(material);

				mModel.mvpMesh[i] = std::make_shared<Mesh>(pVertexArray, pTextures);
			}

			//Traveling the scene tree for retrieving the instanceN vector and model matrix array
			glm::mat4 sceneMMatrix;
			_processNode(scene->mRootNode, sceneMMatrix);
		}

		std::shared_ptr<VertexArray> _loadVertexArray(const aiMesh *mesh)
		{
			//Loading the vertex array information
			std::shared_ptr<std::vector<glm::vec3>> pvVertice = std::make_shared<std::vector<glm::vec3>>();
			std::shared_ptr<std::vector<glm::vec3>> pvNormal = std::make_shared<std::vector<glm::vec3>>();
			std::shared_ptr<std::vector<glm::vec2>> pvTexCoord = std::make_shared<std::vector<glm::vec2>>();
			std::shared_ptr<std::vector<GLuint>> pvIndice = std::make_shared<std::vector<GLuint>>();

			for (size_t i = 0; i < mesh->mNumVertices; i++)
			{
				pvVertice->push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
				pvNormal->push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));

			}

			if (mesh->mTextureCoords[0])
			{
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					pvTexCoord->push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
				}
			}

			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace &face = mesh->mFaces[i];
				for (size_t j = 0; j < face.mNumIndices; j++)
				{
					pvIndice->push_back(face.mIndices[i]);
				}
			}

			return std::make_shared<VertexArray>(pvVertice, pvNormal, pvTexCoord, pvIndice);
		}

		std::vector<std::shared_ptr<Texture>> _loadTextures(const aiMaterial *material)
		{
			std::vector<std::shared_ptr<Texture>> vpTexture;
			for (auto &typePair : mTypeMap)
			{
				for (size_t i = 0; i < material->GetTextureCount(typePair.first); i++)
				{
					aiString filename;
					material->GetTexture(typePair.first, i, &filename);
					std::string path = mDirectory + "/" + std::string(filename.C_Str());
					if (mpTextureLoaded.find(path) == mpTextureLoaded.end())
					{
						std::shared_ptr<Texture> tex = std::make_shared<Texture>(path, typePair.second);
						mpTextureLoaded[path] = tex;
						vpTexture.push_back(tex);
					}
				}
			}

		}

		void _processNode(aiNode *node, const glm::mat4 &topMMatrix/*, const aiScene *scene*/)
		{
			aiMatrix4x4 &T = node->mTransformation;
			glm::mat4 relMMatrix(T.a1, T.b1, T.c1, T.d1,
								 T.a2, T.b2, T.c2, T.d2,
								 T.a3, T.b3, T.c3, T.d3,
								 T.a4, T.b4, T.c4, T.d4);
			glm::mat4 curMMatrix = relMMatrix * topMMatrix;
			for (size_t i = 0; i < node->mNumMeshes; i++)
			{
				mModel.mvInstanceN[node->mMeshes[i]]++;
				mModel.mvvModelMatrix[node->mMeshes[i]].push_back(curMMatrix);
			}

			for (size_t i = 0; i < node->mNumChildren; i++)
			{
				_processNode(node->mChildren[i], curMMatrix);
			}
		}

	};

	class Model
	{
	public:
		Model(const std::vector<std::shared_ptr<Mesh>> &pMeshes,
			  const std::vector<int> &instanceNs,
			  const std::vector<std::vector<glm::mat4>> &modelMatrixes)
			: mvpMesh(pMeshes), mvInstanceN(instanceNs), mvvModelMatrix(modelMatrixes) {}

		Model() {}
		
		~Model() {}

		friend class ModelLoaderFromAssimp;

		bool loadByAssimp(const std::string &fileFullPath)
		{
			modelPath = fileFullPath;
			ModelLoaderFromAssimp loader(modelPath, *this);
			return mbLoadSuccess;
		}

	protected:
		std::vector<std::shared_ptr<Mesh>> mvpMesh;
		std::vector<int> mvInstanceN;
		std::vector<std::vector<glm::mat4>> mvvModelMatrix;

		std::string modelPath;
		bool mbLoadSuccess;

		void _reallocateModel(int numMeshes)
		{
			mvpMesh.resize(numMeshes);
			mvInstanceN.resize(numMeshes, 0);
			mvvModelMatrix.resize(numMeshes);
		}

	};

}
