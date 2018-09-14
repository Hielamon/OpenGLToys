#pragma once

#include "ShaderProgram.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <commonMacro.h>

namespace SP
{
	class Scene
	{
	public:
		Scene(ShaderProgram &commonShaderProgram)
			: mbUploaded(false)
		{
			setCommonShaderProgram(commonShaderProgram);
		}

		Scene() : mbUploaded(false)
		{
			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			ShaderProgram defaultShader(__currentPATH + "/Shaders/SPhoenixScene.vert", 
									  __currentPATH + "/Shaders/SPhoenixScene.frag");

			setCommonShaderProgram(defaultShader);
		}

		~Scene() {}

		void reset()
		{
			mmMeshIDToMesh.clear();
			mmMeshIDToShaderTmp.clear();

			mmMeshToLabel.clear();
			mmLabelToShader.clear();
			mmLabelToMeshes.clear();

			mbUploaded = false;
		}

		void setCommonShaderProgram(ShaderProgram &shaderProgram)
		{
			mpCommonShaderProgram = std::make_shared<ShaderProgram>(shaderProgram);
		}

		void addMesh(const std::shared_ptr<Mesh>& pMesh,
					 const std::shared_ptr<ShaderProgram> &pShaderProgramTmp = nullptr)
		{
			if (pMesh.use_count() != 0)
			{
				GLuint meshID = pMesh->getMeshID();
				mmMeshIDToMesh[meshID] = pMesh;
				mmMeshIDToShaderTmp[meshID] = pShaderProgramTmp;

				if (mbUploaded) _uploadMesh(pMesh);
			}
		}

		//Retrive the mesh by mesh ID
		std::shared_ptr<Mesh> getMesh(GLuint meshID)
		{
			if (mmMeshIDToMesh.find(meshID) != mmMeshIDToMesh.end())
			{
				return mmMeshIDToMesh[meshID];
			}
			else
			{
				return nullptr;
			}
		}

		//Retrive the all meshes in a meshID-mesh map
		std::map<GLuint, std::shared_ptr<Mesh>> getAllMeshes()
		{
			return mmMeshIDToMesh;
		}

		int getNumMesh()
		{
			return mmMeshIDToMesh.size();
		}

		//upload the scene information to the device,
		virtual void uploadToDevice()
		{
			if (mbUploaded) return;

			std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter;
			for (iter = mmMeshIDToMesh.begin(); 
				 iter != mmMeshIDToMesh.end(); iter++)
			{
				std::shared_ptr<Mesh> &pMesh = iter->second;
				assert(pMesh->getMeshID() == iter->first);

				//Upload the mesh
				_uploadMesh(pMesh);
			}

			mbUploaded = true;
		}

		virtual void draw()
		{
			if (!mbUploaded)
			{
				SP_CERR("The current scen has not been uploaded befor drawing");
				return;
			}

			std::map<std::string, std::shared_ptr<ShaderProgram>>::iterator iter;
			for (iter = mmLabelToShader.begin();
				 iter != mmLabelToShader.end(); iter++)
			{
				iter->second->useProgram();
				GLuint programID = iter->second->getProgramID();

				std::map<GLuint, std::shared_ptr<Mesh>> &mMeshIDToMesh =
					mmLabelToMeshes[iter->first];

				std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter_;
				for (iter_ = mMeshIDToMesh.begin();
					 iter_ != mMeshIDToMesh.end(); iter_++)
				{
					iter_->second->draw(programID);
				}
			}
		}

		//if the id is not valid , vMeshID will be changed, which will remove the invalid ID
		virtual void drawByMeshIDs(std::list<GLuint> &vMeshID)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current scen has not been uploaded befor drawing");
				return;
			}

			std::list<GLuint>::iterator iter;
			for (iter = vMeshID.begin(); iter != vMeshID.end(); )
			{
				if (mmMeshIDToMesh.find(*iter) == mmMeshIDToMesh.end())
				{
					iter = vMeshID.erase(iter);
					continue;
				}

				std::shared_ptr<Mesh> &pMesh = mmMeshIDToMesh[*iter];
				std::string &label = mmMeshToLabel[pMesh];
				std::shared_ptr<ShaderProgram> &pShaderProgram = 
					mmLabelToShader[label];

				pShaderProgram->useProgram();
				GLint programID = pShaderProgram->getProgramID();
				pMesh->draw(programID);
				iter++;
			}
		}

		//Get the total bounding box of all meshes
		BBox getTotalBBox()
		{
			BBox result;

			std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter;
			for (iter = mmMeshIDToMesh.begin(); 
				 iter != mmMeshIDToMesh.end(); iter++)
			{
				result += iter->second->getTotalBBox();
			}

			return result;
		}

		//Transform the scene, which will transform all meshes in the scene
		void transformMesh(const glm::mat4 &T)
		{
			std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter;
			for (iter = mmMeshIDToMesh.begin();
				 iter != mmMeshIDToMesh.end(); iter++)
			{
				std::shared_ptr<Mesh> &pMesh = iter->second;
				assert(pMesh->getMeshID() == iter->first);
				//transform the mesh
				pMesh->transformMesh(T);
			}
		}

	protected:
		//Which is just a common shaderprogram template, for
		//the every real shader program we need to add some 
		//necessary macros to the copy of this shaderprogram,
		//And create the new shader program.
		std::shared_ptr<ShaderProgram> mpCommonShaderProgram;

		//The map for access the mesh easily by meshID
		std::map<GLuint, std::shared_ptr<Mesh>> mmMeshIDToMesh;

		//The map for access the shaderProgram template easily by meshID
		//If the corresponding shader program is nullptr, the default
		//common shader program will be viewed as the shaderProgram Template.
		//This map always keep the same keyvalue(meshID) with the mmMeshIDToMesh
		std::map<GLuint, std::shared_ptr<ShaderProgram>> mmMeshIDToShaderTmp;

		//The label is constructed by the macros + // + shaderprogram address
		//These maps can be use for real draw function, and guarantee minimum
		//switch between the different shader programs
		std::map<std::shared_ptr<Mesh>, std::string> mmMeshToLabel;
		std::map<std::string, std::shared_ptr<ShaderProgram>> mmLabelToShader;
		std::map<std::string, std::map<GLuint, std::shared_ptr<Mesh>>> mmLabelToMeshes;

		bool mbUploaded;

	private:
		void _uploadMesh(const std::shared_ptr<Mesh> &pMesh)
		{
			GLuint meshID = pMesh->getMeshID();
			pMesh->uploadToDevice();

			//Get the template shaderprogram
			std::shared_ptr<ShaderProgram> &pShaderProgramTmp =
				mmMeshIDToShaderTmp[meshID].use_count() == 0 ?
				mpCommonShaderProgram : mmMeshIDToShaderTmp[meshID];

			std::string macros = pMesh->getShaderMacros();
			std::stringstream ioStr;
			ioStr << macros << "//" << pShaderProgramTmp;
			std::string label = ioStr.str();

			if (mmLabelToShader.find(label) == mmLabelToShader.end())
			{
				std::shared_ptr<ShaderProgram> pShaderProgram =
					std::make_shared<ShaderProgram>(*pShaderProgramTmp);
				pShaderProgram->addMacros(macros);
				//Create the program, including compiling and linking the shader
				pShaderProgram->createProgram();

				mmLabelToShader[label] = pShaderProgram;
				mmLabelToMeshes[label] = std::map<GLuint, std::shared_ptr<Mesh>>();

				// Using uniform buffers & Bind the UMatrices
				// uniform block index to 1. This is still ugly setting.
				GLuint programID = pShaderProgram->getProgramID();
				GLuint ViewUBOIndex = glGetUniformBlockIndex(programID, "ViewUBO");
				glUniformBlockBinding(programID, ViewUBOIndex, VIEWUBO_BINDING_POINT);
			}

			mmMeshIDToMesh[meshID] = pMesh;
			mmMeshToLabel[pMesh] = label;
			mmLabelToMeshes[label][meshID] = pMesh;
		}
	};
	
	//SceneColorID will copy the existed Scene and change the material of
	//all existed meshes, meanwhile it will reset the origin meshes. So
	//we need to re-upload this class by call uploadToDevice again
	class SceneColorID : public Scene
	{
	public:
		SceneColorID() = delete;
		SceneColorID(const std::shared_ptr<Scene> &pScene)
			: Scene(*pScene)
		{
			reset();

			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			std::shared_ptr<ShaderProgram> pDefaultShaderTmp =
				std::make_shared<ShaderProgram>(__currentPATH + "/Shaders/SPhoenixScene.vert",
											  __currentPATH + "/Shaders/SPhoenixScene.frag");

			std::string undefColor = "#undef HAVE_COLOR\n";
			pDefaultShaderTmp->addMacros(undefColor);

			std::map<GLuint, std::shared_ptr<Mesh>> mExistedMeshes =
				pScene->getAllMeshes();

			std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter;
			for (iter = mExistedMeshes.begin();
				 iter != mExistedMeshes.end(); iter++)
			{
				std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(*(iter->second));

				//Compute the ID Color
				double totalColor = MeshGlobal::getInstance().totalMeshCount + 1;
				int Nc = std::ceil(std::pow(totalColor, 1.0 / 3));
				int ID = iter->first;
				glm::vec4 uIDColor(1.0f);
				for (size_t i = 0; i < 3 && ID > 0; i++)
				{
					uIDColor[i] = (ID % Nc) / float(Nc);
					ID = (ID - ID % Nc) / Nc;
				}

				std::shared_ptr<Material> pMaterial = std::make_shared<Material>(uIDColor);
				pMesh->setMaterial(pMaterial);
				addMesh(pMesh, pDefaultShaderTmp);
			}
		}

		~SceneColorID() {}
	private:

	};

	class SceneSelected : public Scene
	{
	public:
		SceneSelected() = delete;
		SceneSelected(const std::shared_ptr<Scene> &pScene)
			: Scene(*pScene)
		{
			reset();

			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			std::shared_ptr<ShaderProgram> pDefaultShaderTmp =
				std::make_shared<ShaderProgram>(__currentPATH + "/Shaders/SPhoenixScene.vert",
												__currentPATH + "/Shaders/SPhoenixScene.frag");

			std::string undefColor = "#undef HAVE_COLOR\n";
			pDefaultShaderTmp->addMacros(undefColor);

			std::map<GLuint, std::shared_ptr<Mesh>> mExistedMeshes =
				pScene->getAllMeshes();

			std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter;
			for (iter = mExistedMeshes.begin();
				 iter != mExistedMeshes.end(); iter++)
			{
				std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(*(iter->second));

				std::shared_ptr<Material> pMaterial = std::make_shared<Material>();
				pMesh->setMaterial(pMaterial);
				addMesh(pMesh, pDefaultShaderTmp);
			}
		}

		~SceneSelected() {}

		virtual void drawByMeshIDs(std::list<GLuint> &vMeshID)
		{
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			Scene::drawByMeshIDs(vMeshID);
			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}

	private:
	};

}

