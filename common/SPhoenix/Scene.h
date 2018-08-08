#pragma once

#include "Shader.h"
#include "Geometry.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace SP
{
	class Scene;

	class SceneAssimpLoader
	{
	public:
		SceneAssimpLoader(const std::string &path, Scene &scene)
			: mScene(scene)
		{
			mScene.mbLoadSuccess = false;
			Assimp::Importer import;
			const aiScene *aiscene = import.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

			if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode)
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

			_loadToScene(aiscene);
			mScene.mbLoadSuccess = true;
		}
		~SceneAssimpLoader() {}

	private:
		Scene &mScene;
		std::string mDirectory;

		//The map between the aiTextureType and TextureType of SPhoenix library
		std::map<aiTextureType, TextureType> mTypeMap;

		//The map of the texture and its path
		std::map<std::string, std::shared_ptr<Texture>> mpTextureLoaded;

	private:
		void _loadToScene(const aiScene *aiscene)
		{
			//Loading all meshes in the scene
			for (size_t i = 0; i < aiscene->mNumMeshes; i++)
			{
				aiMesh *aimesh = aiscene->mMeshes[i];
				std::shared_ptr<VertexArray> pVertexArray = _loadVertexArray(aimesh);

				aiMaterial *aimaterial = aiscene->mMaterials[aimesh->mMaterialIndex];
				std::vector<std::shared_ptr<Texture>> pTextures = _loadTextures(aimaterial);

				mScene.mvpMesh[i] = std::make_shared<Mesh>(pVertexArray, pTextures);
			}

			//Traveling the scene tree for retrieving the instanceN vector and model matrix array
			_processNode(aiscene->mRootNode, mScene.mTopModelMatrix);
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
					pvIndice->push_back(face.mIndices[i]);
				}
			}

			return std::make_shared<VertexArray>(pvVertice, pvNormal, pvTexCoord, pvIndice);
		}

		std::vector<std::shared_ptr<Texture>> _loadTextures(const aiMaterial *aimaterial)
		{
			std::vector<std::shared_ptr<Texture>> vpTexture;
			for (auto &typePair : mTypeMap)
			{
				for (size_t i = 0; i < aimaterial->GetTextureCount(typePair.first); i++)
				{
					aiString filename;
					aimaterial->GetTexture(typePair.first, i, &filename);
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
				mScene.mvInstanceN[ainode->mMeshes[i]]++;
				mScene.mvvModelMatrix[ainode->mMeshes[i]].push_back(curMMatrix);
			}

			for (size_t i = 0; i < ainode->mNumChildren; i++)
			{
				_processNode(ainode->mChildren[i], curMMatrix);
			}
		}
	};

	class Scene
	{
	public:
		Scene(ShaderCodes &shaderCodes)
		{
			setShaderCodes(shaderCodes);
		}
		Scene() = delete;
		~Scene() {}

		friend class SceneAssimpLoader;

		void addGeometry(Geometry &geometry)
		{
			mvpGeometry.push_back(std::make_shared<Geometry>(geometry));
		}

		void setShaderCodes(ShaderCodes &shaderCodes)
		{
			mpShaderCodes = std::make_shared<ShaderCodes>(shaderCodes);
		}

	public:
		glm::mat4 mTopModelMatrix;

	protected:
		std::shared_ptr<ShaderCodes> mpShaderCodes;
		std::vector<std::shared_ptr<Geometry>> mvpGeometry;

		std::vector<std::shared_ptr<Mesh>> mvpMesh;
		std::vector<int> mvInstanceN;
		std::vector<std::vector<glm::mat4>> mvvModelMatrix;

		std::string modelPath;
		bool mbLoadSuccess;
	};

	class SceneUtil : public Scene
	{
	public:
		SceneUtil(Scene &scene)
			: Scene(scene)
		{
			mpShaderUtil = std::make_shared<ShaderUtil>(*mpShaderCodes);

			std::for_each(mvpGeometry.begin(), mvpGeometry.end(),
						  [&](std::shared_ptr<Geometry> &pGeom) {
				mvpGeometryUtil.push_back(std::make_shared<GeometryUtil>(*pGeom));
			}
			);

			GLuint programID = getProgramID();
			mmodelLoc = glGetUniformLocation(programID, "model");
		}

		~SceneUtil() {}

		void show()
		{
			glUniformMatrix4fv(mmodelLoc, 1, GL_FALSE, glm::value_ptr(mModelMatrix));

			mpShaderUtil->useProgram();
			std::for_each(mvpGeometryUtil.begin(), mvpGeometryUtil.end(),
						  [](std::shared_ptr<GeometryUtil> &pGeom) {pGeom->show(); }
			);
		}

		GLuint getProgramID()
		{
			return mpShaderUtil->getProgramID();
		}

	private:
		std::shared_ptr<ShaderUtil> mpShaderUtil;
		std::vector<std::shared_ptr<GeometryUtil>> mvpGeometryUtil;

		GLint mmodelLoc;
	};
}

