#pragma once

#include "ShaderProgram.h"
#include "Mesh.h"

#include <commonMacro.h>

#include <direct.h>
#include <io.h>
#include <TraverFolder.h>

#include <ppl.h>

using namespace concurrency;

namespace SP
{
	//The scene for common 3D rendering, use the SPhoenixScene shader
	class Scene
	{
	public:
		struct ID2MeshMap
		{
			std::map<GLuint, std::shared_ptr<Mesh>> mMeshID2Mesh;
		};

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
				mmMeshIDToVisible[meshID] = true;

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

			//std::cout << "Shader ID count = " << mmLabelToMeshes.size() << std::endl;

			mbUploaded = true;
		}

		virtual void drawOld()
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
				int uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");

				for (iter_ = mMeshIDToMesh.begin();
					 iter_ != mMeshIDToMesh.end(); iter_++)
				{
					glUniform1ui(uMeshIDLoc, iter_->second->getMeshID());
					iter_->second->draw(programID);
				}
			}
		}

		virtual void drawOld2()
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

				int uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");

				//For active same type materials only once
				std::map<std::shared_ptr<Material>, std::vector<ID2MeshMap>>
					&mMaterialIndex = mmLabelToMaterialIndex[iter->first];
				
				std::map<std::shared_ptr<Material>, std::vector<ID2MeshMap>>::iterator iterM;

				bool vbTexAndColor[8] =
				{
					false, false, /**/true, false, /**/
					false, true, /**/true, true/**/
				};

				for (iterM = mMaterialIndex.begin();
					 iterM != mMaterialIndex.end(); iterM++)
				{

					for (size_t i = 0, j = 0; i < 4; i++, j += 2)
					{
						std::map<GLuint, std::shared_ptr<Mesh>> &mMeshIDToMesh =
							iterM->second[i].mMeshID2Mesh;

						if (mMeshIDToMesh.size() > 0)
						{
							iterM->first->active(programID, vbTexAndColor[j],
												 vbTexAndColor[j + 1]);

							//draw meshes
							std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter_;

							for (iter_ = mMeshIDToMesh.begin();
								 iter_ != mMeshIDToMesh.end(); iter_++)
							{
								glUniform1ui(uMeshIDLoc, iter_->second->getMeshID());
								iter_->second->drawOnlyInScene(programID);
							}
						}
					}
				}
			}
		}

		//The inputs is aimed for early pruning
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

				int uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");

				//For active same type materials only once
				std::map<std::shared_ptr<Material>, std::vector<ID2MeshMap>>
					&mMaterialIndex = mmLabelToMaterialIndex[iter->first];

				std::map<std::shared_ptr<Material>, std::vector<ID2MeshMap>>::iterator iterM;

				bool vbTexAndColor[8] =
				{
					false, false, /**/true, false, /**/
					false, true, /**/true, true /**/
				};

				for (iterM = mMaterialIndex.begin();
					 iterM != mMaterialIndex.end(); iterM++)
				{

					for (size_t i = 0, j = 0; i < 4; i++, j += 2)
					{
						std::map<GLuint, std::shared_ptr<Mesh>> &mMeshIDToMesh =
							iterM->second[i].mMeshID2Mesh;

						/*std::vector<std::shared_ptr<Mesh>> vVisibleMesh;
						vVisibleMesh.reserve(mMeshIDToMesh.size());*/
						int numVisible = 0;
						std::for_each(mMeshIDToMesh.begin(), mMeshIDToMesh.end(),
									  [&](std::pair<const GLuint,
										  std::shared_ptr<Mesh>>&pair) 
						{
							if (pair.second->getVisible())
								numVisible++;
								//vVisibleMesh.push_back(pair.second);
						});

						//if (vVisibleMesh.size() > 0)
						//if (mMeshIDToMesh.size() > 0)
						if(numVisible > 0)
						{
							iterM->first->active(programID, vbTexAndColor[j],
												 vbTexAndColor[j + 1]);

							std::for_each(mMeshIDToMesh.begin(), mMeshIDToMesh.end(),
										  [&](std::pair<const GLuint,
											  std::shared_ptr<Mesh>>&pair)
							{
								if (/*pair.second->getInFrustum()*/
									pair.second->getVisible())
								{
									glUniform1ui(uMeshIDLoc, pair.second->getMeshID());
									pair.second->drawOnlyInScene(programID);
									/*if (pair.second->getInFrustum())
									{
										std::cout << "Mesh " << pair.first << "totally in the frustum" << std::endl;
									}*/
								}
							});

							/*std::map<GLuint, std::shared_ptr<Mesh>>::iterator iter_;
							for (iter_ = mMeshIDToMesh.begin();
								 iter_ != mMeshIDToMesh.end(); iter_++)
							{
								//if(mmMeshIDToVisible[iter_->first])
								if(iter_->second->getVisible())
								{
									glUniform1ui(uMeshIDLoc, iter_->second->getMeshID());
									iter_->second->drawOnlyInScene(programID);
								}
							}*/

							/*std::for_each(vVisibleMesh.begin(), vVisibleMesh.end(),
										  [&](const std::shared_ptr<Mesh> &mesh)
							{
								glUniform1ui(uMeshIDLoc, mesh->getMeshID());
								mesh->drawOnlyInScene(programID);
							});*/
						}
					}
				}
			}
		}

		virtual void filterVisible(const glm::mat4 &projMatrix,
								   const glm::mat4 &viewMatrix,
								   float zNear, float zFar)
		{
			//HL_INTERVAL_START;
			{
				/*Concurrency::parallel_for_each std::for_each*/
				glm::mat3 R = glm::mat3(viewMatrix);
				glm::vec3 t = glm::vec3(viewMatrix[3]);

				/*std::*/parallel_for_each(mmMeshIDToMesh.begin(),
										   mmMeshIDToMesh.end(),
										   [&](std::pair<const GLuint,
											   std::shared_ptr<Mesh>>&pair)
				{
					std::vector<glm::vec3> vViewPt(BOUNDINT_BOX_POINT_NUM);
					BBox viewBBox;

					const std::vector<glm::vec3> &vBBoxPt =
						pair.second->getBBoxPoints();

					for (size_t i = 0; i < BOUNDINT_BOX_POINT_NUM; i++)
					{
						vViewPt[i] = R*vBBoxPt[i] + t;
					}

					viewBBox.computeBBox(vViewPt);
					glm::vec3 minVertex = viewBBox.getMinVertex();
					glm::vec3 maxVertex = viewBBox.getMaxVertex();

					if (minVertex.z < -zFar) minVertex.z = -zFar;
					if (maxVertex.z > -zNear) maxVertex.z = -zNear;

					//z section pruning
					if (minVertex.z <= maxVertex.z)
					{
						BBox viewBBoxZ(minVertex, maxVertex);
						std::vector<glm::vec4> vBBoxPtZ =
							viewBBoxZ.getHomoBBoxVertices();

						//mmMeshIDToVisible[pair.first] = true;
						for (size_t i = 0; i < BOUNDINT_BOX_POINT_NUM; i++)
						{
							glm::vec4 projPt = projMatrix * vBBoxPtZ[i];
							float wInv = 1.0f / projPt.w;
							vViewPt[i] = glm::vec3(projPt * wInv);
						}

						glm::vec3 frustumMin(-1.0f, -1.0f, -1.0f);
						glm::vec3 frustumMax(1.0f, 1.0f, 1.0f);
						BBox frustumBBox(frustumMin, frustumMax);

						viewBBoxZ.computeBBox(vViewPt);
						glm::vec3 viewBBoxZMin = viewBBoxZ.getMinVertex();
						glm::vec3 viewBBoxZMax = viewBBoxZ.getMaxVertex();

						//Get the overlap Bounding Box
						BBox overlapBBox = frustumBBox & viewBBoxZ;
						bool visible = overlapBBox.isValid();

						if (visible)
						{
							//Test whether the frustum include the viewBBoxZ
							bool bInFrustum = overlapBBox == viewBBoxZ;
							pair.second->setInFrustum(bInFrustum);
						}
						pair.second->setVisible(visible);
					}
					else {
						//mmMeshIDToVisible[pair.first] = false;
						pair.second->setVisible(false);
					}
				}
				);
			}
			//HL_INTERVAL_ENDSTR("Pruning");
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

	protected:
		//Which is just a common shaderprogram template, for
		//the every real shader program we need to add some 
		//necessary macros to the copy of this shaderprogram,
		//And create the new shader program.
		std::shared_ptr<ShaderProgram> mpCommonShaderProgram;

		//The map for access the mesh easily by meshID
		std::map<GLuint, std::shared_ptr<Mesh>> mmMeshIDToMesh;
		std::map<GLuint, bool> mmMeshIDToVisible;

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

		//For active same type materials only once
		std::map<std::string, std::map<std::shared_ptr<Material>, std::vector<ID2MeshMap>>> mmLabelToMaterialIndex;

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

				//For active same type materials only once
				mmLabelToMaterialIndex[label] = std::map<std::shared_ptr<Material>, std::vector<ID2MeshMap>>();

				// Using uniform buffers & Bind the UMatrices
				// uniform block index to 1. This is still ugly setting.
				GLuint programID = pShaderProgram->getProgramID();
				GLuint ViewUBOIndex = glGetUniformBlockIndex(programID, "ViewUBO");
				glUniformBlockBinding(programID, ViewUBOIndex, VIEWUBO_BINDING_POINT);
			}

			mmMeshIDToMesh[meshID] = pMesh;
			mmMeshToLabel[pMesh] = label;
			mmLabelToMeshes[label][meshID] = pMesh;

			//For active same type materials only once
			std::shared_ptr<Material> pMaterial = pMesh->getMaterial();
			std::map<std::shared_ptr<Material>, std::vector<ID2MeshMap>> &mMaterialIndex
				= mmLabelToMaterialIndex[label];
			if (mMaterialIndex.find(pMaterial) == mMaterialIndex.end())
			{
				mMaterialIndex[pMaterial] = std::vector<ID2MeshMap>(4);
			}
			bool hasTexCoord = pMesh->getHasTexCoord();
			bool hasVertexColor = pMesh->getHasVertexColor();
			int vIndex = hasTexCoord * 1 + hasVertexColor * 2;
			mMaterialIndex[pMaterial][vIndex].mMeshID2Mesh[meshID] = pMesh;
		}
	};

	//The scene for the 2D graphic, use the SPhoenixScene-2DGraphic shader
	class TwoDScene : public Scene
	{
	public:
		TwoDScene()
		{
			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			ShaderProgram defaultShader(__currentPATH + "/Shaders/SPhoenixScene-2DGraphic.vert",
										__currentPATH + "/Shaders/SPhoenixScene-2DGraphic.frag");

			setCommonShaderProgram(defaultShader);
		}

		~TwoDScene() {}

		virtual void draw()
		{
			Scene::draw();
		}
	};

	//The scene for the Skybox graphic, use the SPhoenixScene-SkyBox shader
	class SkyBoxScene : public Scene
	{
	public:
		SkyBoxScene(std::vector<std::shared_ptr<Texture>> vpTexture)
		{
			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			ShaderProgram defaultShader(__currentPATH + "/Shaders/SPhoenixScene-SkyBox.vert",
										__currentPATH + "/Shaders/SPhoenixScene-SkyBox.frag");

			setCommonShaderProgram(defaultShader);

			std::vector<glm::vec3> vertices(8);
			std::vector<GLuint> indices;
			{
				//Get the six points of a space box
				//           5      6
				//            *******   ^ y
				//        4  *   7 **   | 
				//          ******* *   |
				//          *	  *	*    
				//          *	  *	*   
				//          *  0  * *1  
				//          *	  **    
				//          *******      ------->x
				//           3     2
				vertices[0] = glm::vec3(-0.5f, -0.5f, -0.5f);
				vertices[1] = glm::vec3(0.5f, -0.5f, -0.5f);
				vertices[2] = glm::vec3(0.5f, -0.5f, 0.5f);
				vertices[3] = glm::vec3(-0.5f, -0.5f, 0.5f);
				vertices[4] = glm::vec3(-0.5f, 0.5f, 0.5f);
				vertices[5] = glm::vec3(-0.5f, 0.5f, -0.5f);
				vertices[6] = glm::vec3(0.5f, 0.5f, -0.5f);
				vertices[7] = glm::vec3(0.5f, 0.5f, 0.5f);

				indices =
				{
					//Look inside
					//Bottom
					0, 2, 1, 0, 3, 2,
					//Top
					5, 6, 7, 5, 7, 4,
					//Right
					7, 6, 1, 7, 1, 2,
					//Left
					4, 0, 5, 4, 3, 0,
					//Back
					5, 1, 6, 5, 0, 1,
					//Front
					4, 7, 2, 4, 2, 3,
				};
			}
			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vertices, indices, PrimitiveType::TRIANGLES);
			pVA->addInstance();

			std::shared_ptr<MaterialCube> pMatrialCube = std::make_shared<MaterialCube>();
			pMatrialCube->setCubeTextures(vpTexture);
			mpSkyBoxMesh = std::make_shared<Mesh>(pVA, pMatrialCube);
			addMesh(mpSkyBoxMesh);
		}

		~SkyBoxScene() {}

		virtual void uploadToDevice()
		{
			Scene::uploadToDevice();

			mpSkyBoxShader = mmLabelToShader[mmMeshToLabel[mpSkyBoxMesh]];
		}

		virtual void draw()
		{
			if (!mbUploaded)
			{
				SP_CERR("The current scene has not been uploaded befor drawing");
				return;
			}

			mpSkyBoxShader->useProgram();
			mpSkyBoxMesh->draw(mpSkyBoxShader->getProgramID());
		}
	private:
		std::shared_ptr<Mesh> mpSkyBoxMesh;
		std::shared_ptr<ShaderProgram> mpSkyBoxShader;
	};

	class TextScene : public Scene
	{
	public:
		TextScene()
		{
			std::string __currentPATH = __FILE__;
			__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
			ShaderProgram defaultShader(__currentPATH + "/Shaders/SPhoenixScene-Text.vert",
										__currentPATH + "/Shaders/SPhoenixScene-Text.frag");

			setCommonShaderProgram(defaultShader);

			initGlyphSet(24);
		}

		~TextScene() {}

		/*static const std::shared_ptr<TextScene> getpInstance()
		{
			static TextScene *rpTextScene = new TextScene();
			static std::shared_ptr<TextScene> pTextScene(rpTextScene);
			return pTextScene;
		}*/

		void reset()
		{
			Scene::reset();
			mmCharToMesh.clear();
			mmMeshToChar.clear();
			mpMaterialText.reset();
		}

		void initGlyphSet(int frontSize)
		{
			reset();

			mpMaterialText = std::make_shared<MaterialText>(frontSize);
			if (!mpMaterialText->isValidText())
				return;

			for (size_t i = 0; i < TEXT_CHAR_NUM; i++)
			{
				char c = char(i);
				glm::vec2 glyphSize = mpMaterialText->getGlyphSize(c);
				float width = glyphSize.x, height = glyphSize.y;
				std::vector<glm::vec3> vertices = 
				{
					{ 0.0f, 0.0f, -1.0f },
					{ width, 0.0f, -1.0f },
					{ width, height, -1.0f },

					{ 0.0f, 0.0f, -1.0f },
					{ width, height, -1.0f },
					{ 0.0f, height, -1.0f }
				};

				glm::vec2 lb, rb, rt, lt;
				mpMaterialText->getTexCoord(c, lb, rb, rt, lt);

				std::vector<glm::vec2> texcoords = 
				{
					lb, rb, rt,
					lb, rt, lt
				};

				std::shared_ptr<VertexArray> pVA =
					std::make_shared<VertexArray>(vertices);
				pVA->setTexCoords(texcoords);

				std::shared_ptr<Mesh> pMesh =
					std::make_shared<Mesh>(pVA,/* std::static_pointer_cast<Material>*/
										   (mpMaterialText));

				addMesh(pMesh);
				mmCharToMesh[c] = pMesh;
				mmMeshToChar[pMesh] = c;
			}
		}

		//Draw the text and return the end advance point
		glm::vec2 setText(std::string text, glm::vec2 originPt,
				  float scale = 1.0f, glm::vec4 color = glm::vec4(1.0f))
		{
			if (mpMaterialText.use_count() == 0 ||
				!mpMaterialText->isValidText())
			{
				SP_CERR("The text material is not valid");
				return glm::vec2(0.0f);
			}

			//Clear previous instances of the char meshes
			std::for_each(mmMeshToChar.begin(), mmMeshToChar.end(),
						  [&](std::pair<const std::shared_ptr<Mesh>, char> &pair) 
			{
				pair.first->clearAllInstance();
			});

			//Set the color
			mpMaterialText->setCharColor(color);

			//Set the char mesh instance model matrix
			glm::vec2 cOriginPt = originPt;
			for (size_t i = 0; i < text.size(); i++)
			{
				char &c = text[i];

				glm::vec2 bearing = mpMaterialText->getGlyphBearing(c);
				glm::vec2 cSize = mpMaterialText->getGlyphSize(c);
				GLuint advance = mpMaterialText->getGlyphAdvance(c);

				std::shared_ptr<Mesh> pCharMesh = mmCharToMesh[c];
				glm::mat4 charMMatrix(1.0f);
				//Move and scale the original char bitmap
				float tx = bearing.x * scale + cOriginPt.x;
				float ty = -(cSize.y - bearing.y) * scale + cOriginPt.y;
				charMMatrix[0][0] = scale;
				charMMatrix[1][1] = scale;
				charMMatrix[3][0] = tx;
				charMMatrix[3][1] = ty;

				pCharMesh->addInstance(charMMatrix);

				advance *= scale;
				cOriginPt.x += advance;
			}

			return cOriginPt;
		}

		virtual void uploadToDevice()
		{
			if (mpMaterialText.use_count() == 0 ||
				!mpMaterialText->isValidText())
			{
				SP_CERR("The text material is not valid for uploadToDevice");
				return;
			}

			Scene::uploadToDevice();
		}

		virtual void filterVisible(const glm::mat4 &projMatrix,
								   const glm::mat4 &viewMatrix,
								   float zNear, float zFar)
		{
			//Do nothing
		}

		virtual void draw()
		{
			if (!mbUploaded)
			{
				SP_CERR("The current TextScene has not been uploaded befor drawing");
				return;
			}

			std::map<std::string, std::shared_ptr<ShaderProgram>>::iterator iter;
			//Only one shader for the text scene
			iter = mmLabelToShader.begin();
			{
				iter->second->useProgram();
				GLuint programID = iter->second->getProgramID();

				mpMaterialText->active(programID);

				int uCharIDLoc = glGetUniformLocation(programID, "uCharID");

				std::map<std::shared_ptr<Mesh>, char>::iterator iterMesh;

				for(iterMesh = mmMeshToChar.begin(); 
					iterMesh != mmMeshToChar.end();
					iterMesh++)
				{
					if (iterMesh->first->getNumInstance() > 0)
					{
						glUniform1ui(uCharIDLoc, iterMesh->second);
						iterMesh->first->drawOnlyInScene(programID);
					}
				}
			}
		}
	private:
		std::map<char, std::shared_ptr<Mesh>> mmCharToMesh;
		std::map<std::shared_ptr<Mesh>, char> mmMeshToChar;
		std::shared_ptr<MaterialText> mpMaterialText;
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

	inline std::shared_ptr<Scene> createSkyBoxScene(const std::string &cubeMapFolder)
	{
		if (_access(cubeMapFolder.c_str(), 0) == -1)
		{
			SP_CERR("The cubeMapFolder is not a valid folder");
			return nullptr;
		}

		//Load the textures from the cubeMapFolders
		std::vector<std::shared_ptr<Texture>> vpTexture(6, nullptr);
		{
			TraverFolder tf;
			tf.setFolderPath(cubeMapFolder);
			std::vector<std::string> vFileNames;
			tf.getFileFullPath(vFileNames);

			for (size_t i = 0; i < vFileNames.size(); i++)
			{
				if (vFileNames[i].find("right.") != std::string::npos)
				{
					vpTexture[0] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("left.") != std::string::npos)
				{
					vpTexture[1] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("top.") != std::string::npos)
				{
					vpTexture[2] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("bottom.") != std::string::npos)
				{
					vpTexture[3] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("front.") != std::string::npos)
				{
					vpTexture[4] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("back.") != std::string::npos)
				{
					vpTexture[5] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}

				if (vFileNames[i].find("_rt.") != std::string::npos)
				{
					vpTexture[0] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("_lf.") != std::string::npos)
				{
					vpTexture[1] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("_up.") != std::string::npos)
				{
					vpTexture[2] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
					vpTexture[2]->antiClockWise90();
				}
				else if (vFileNames[i].find("_dn.") != std::string::npos)
				{
					vpTexture[3] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
					vpTexture[3]->clockWise90();
				}
				else if (vFileNames[i].find("_bk.") != std::string::npos)
				{
					vpTexture[4] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
				else if (vFileNames[i].find("_ft.") != std::string::npos)
				{
					vpTexture[5] = std::make_shared<Texture>(vFileNames[i], Tex_CUBE);
				}
			}

			for (size_t i = 0; i < 6; i++)
			{
				if (vpTexture[i].use_count() == 0)
				{
					SP_CERR("The " << i << "th face texture is empty, the skybox is invalid");
					return nullptr;
				}
			}

		}

		std::shared_ptr<Scene> pSkyBoxScene = std::make_shared<SkyBoxScene>(vpTexture);

		//std::string __currentPATH = __FILE__;
		//__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
		//ShaderProgram defaultShader(__currentPATH + "/Shaders/SPhoenixScene-SkyBox.vert",
		//							__currentPATH + "/Shaders/SPhoenixScene-SkyBox.frag");
		//pSkyBoxScene->setCommonShaderProgram(defaultShader);

		//std::vector<glm::vec3> vertices(8);
		//std::vector<GLuint> indices;
		//{
		//	//Get the six points of a space box
		//	//           5      6
		//	//            *******   ^ y
		//	//        4  *   7 **   | 
		//	//          ******* *   |
		//	//          *	  *	*    
		//	//          *	  *	*   
		//	//          *  0  * *1  
		//	//          *	  **    
		//	//          *******      ------->x
		//	//           3     2
		//	vertices[0] = glm::vec3(-0.5f, -0.5f, -0.5f);
		//	vertices[1] = glm::vec3(0.5f, -0.5f, -0.5f);
		//	vertices[2] = glm::vec3(0.5f, -0.5f, 0.5f);
		//	vertices[3] = glm::vec3(-0.5f, -0.5f, 0.5f);
		//	vertices[4] = glm::vec3(-0.5f, 0.5f, 0.5f);
		//	vertices[5] = glm::vec3(-0.5f, 0.5f, -0.5f);
		//	vertices[6] = glm::vec3(0.5f, 0.5f, -0.5f);
		//	vertices[7] = glm::vec3(0.5f, 0.5f, 0.5f);

		//	indices =
		//	{
		//		//Look inside
		//		//Bottom
		//		0, 2, 1, 0, 3, 2,
		//		//Top
		//		5, 6, 7, 5, 7, 4,
		//		//Right
		//		7, 6, 1, 7, 1, 2,
		//		//Left
		//		4, 0, 5, 4, 3, 0,
		//		//Back
		//		5, 1, 6, 5, 0, 1,
		//		//Front
		//		4, 7, 2, 4, 2, 3,
		//	};
		//}
		//std::shared_ptr<VertexArray> pVA =
		//	std::make_shared<VertexArray>(vertices, indices, PrimitiveType::TRIANGLES);
		//pVA->addInstance();

		//std::shared_ptr<MaterialCube> pMatrialCube = std::make_shared<MaterialCube>();
		//pMatrialCube->setCubeTextures(vpTexture);
		//std::shared_ptr<Mesh> pSkyBoxMesh = std::make_shared<Mesh>(pVA, pMatrialCube);
		//pSkyBoxScene->addMesh(pSkyBoxMesh);
		return pSkyBoxScene;
	}

}

