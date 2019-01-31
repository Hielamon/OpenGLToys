#pragma once

#include "Mesh.h"

namespace SP
{
	class SpherePlane : public Mesh
	{
	public:
		SpherePlane(float radius, glm::vec3 color)
		{
			//icosahedron parameters a, b for the rectangle height and width
			float a = std::sqrtf(50.0f - 10.0f * std::sqrtf(5.0f)) * 0.2 * radius;
			float b = 0.5f * (std::sqrtf(5.0f) + 1.0f) * a;
			float a_2 = a * 0.5f, b_2 = b * 0.5f;
			std::vector<glm::vec3> vVertice =
			{
				//Rectangle 1
				{  a_2,  b_2, 0.0f },
				{  a_2, -b_2, 0.0f },
				{ -a_2, -b_2, 0.0f },
				{ -a_2,  b_2, 0.0f },

				//Rectangle 2
				{  b_2, 0.0f, -a_2 },
				{ b_2, 0.0f,  a_2 },
				{ -b_2, 0.0f,  a_2 },
				{ -b_2, 0.0f, -a_2 },


				//Rectangle 2
				{ 0.0f,  a_2, -b_2 },
				{ 0.0f, -a_2, -b_2 },
				{ 0.0f, -a_2,  b_2 },
				{ 0.0f,  a_2,  b_2 }

			};

			std::vector<glm::vec4> vColor =
			{
				//Rectangle 1
				{ 1.0f,  0.0f, 0.0f, 1.0f },
				{ 1.0f, -0.0f, 0.0f, 1.0f },
				{ 1.0f, -0.0f, 0.0f, 1.0f },
				{ 1.0f,  0.0f, 0.0f, 1.0f },

				//Rectangle 2
				{ 0.0f, 1.0f, -0.0f, 1.0f },
				{ 0.0f, 1.0f,  0.0f, 1.0f },
				{ -0.0f, 1.0f,  0.0f, 1.0f },
				{ -0.0f, 1.0f, -0.0f, 1.0f },


				//Rectangle 2
				{ 0.0f,  0.0f,  1.0f, 1.0f },
				{ 0.0f, -0.0f,  1.0f, 1.0f },
				{ 0.0f, -0.0f,  1.0f, 1.0f },
				{ 0.0f,  0.0f,  1.0f, 1.0f }
			};

			//std::vector<GLuint> vIndice =
			//{
			//	0, 5, 4,/*  */0, 4, 8,/*  */0, 8, 3,/*  */0, 3, 11,/*  */0, 11, 5,
			//	/////////////////////////////////////////////////////////////////
			//	1, 4, 5,/*  */9, 8, 4,/*  */7, 3, 8,/*  */6, 11, 3,/*  */10, 5, 11,
			//	4, 1, 9,/*  */8, 9, 7,/*  */3, 7, 6,/*  */11, 6, 10,/*  */5, 10, 1,
			//	/////////////////////////////////////////////////////////////////
			//	2, 9, 1,/*  */2, 7, 9,/*  */2, 6, 7,/*  */2, 10, 6,/*  */2, 1, 10,
			//};

			std::vector<GLuint> vIndice =
			{
				0, 2, 1,/*  */0, 3, 2,
				/////////////////////////////////////////////////////////////////
				4, 6, 5,/*  */4, 7, 6,
				/////////////////////////////////////////////////////////////////
				8, 10, 9,/*  */8, 11, 10
			};

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, TRIANGLES);
			//pVertexArray->setNormals(vVertice);
			pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>(
				glm::vec4(color, 1.0f), glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();
		}

	};

	class IcoSphere : public Mesh
	{
	public:
		IcoSphere(float radius,  glm::vec3 color, int subdivison = 0)
		{
			//icosahedron parameters a, b for the rectangle height and width
			float a = std::sqrtf(50.0f - 10.0f * std::sqrtf(5.0f)) * 0.2 * radius;
			float b = 0.5f * (std::sqrtf(5.0f) + 1.0f) * a;
			float a_2 = a * 0.5f, b_2 = b * 0.5f;
			std::vector<glm::vec3> vIcoVertice =
			{
				//Rectangle 1
				{ a_2,  b_2, 0.0f },
				{ a_2, -b_2, 0.0f },
				{ -a_2, -b_2, 0.0f },
				{ -a_2,  b_2, 0.0f },

				//Rectangle 2
				{ b_2, 0.0f, -a_2 },
				{ b_2, 0.0f,  a_2 },
				{ -b_2, 0.0f,  a_2 },
				{ -b_2, 0.0f, -a_2 },


				//Rectangle 2
				{ 0.0f,  a_2, -b_2 },
				{ 0.0f, -a_2, -b_2 },
				{ 0.0f, -a_2,  b_2 },
				{ 0.0f,  a_2,  b_2 }

			};

			std::vector<glm::vec4> vIcoColor =
			{
				//Rectangle 1
				{ 1.0f,  0.0f, 0.0f, 1.0f },
				{ 1.0f, -0.0f, 0.0f, 1.0f },
				{ 1.0f, -0.0f, 0.0f, 1.0f },
				{ 1.0f,  0.0f, 0.0f, 1.0f },

				//Rectangle 2
				{ 0.0f, 1.0f, -0.0f, 1.0f },
				{ 0.0f, 1.0f,  0.0f, 1.0f },
				{ -0.0f, 1.0f,  0.0f, 1.0f },
				{ -0.0f, 1.0f, -0.0f, 1.0f },


				//Rectangle 2
				{ 0.0f,  0.0f,  1.0f, 1.0f },
				{ 0.0f, -0.0f,  1.0f, 1.0f },
				{ 0.0f, -0.0f,  1.0f, 1.0f },
				{ 0.0f,  0.0f,  1.0f, 1.0f }
			};

			std::vector<GLuint> vIcoIndice =
			{
				0, 5, 4,/*  */0, 4, 8,/*  */0, 8, 3,/*  */0, 3, 11,/*  */0, 11, 5,
				/////////////////////////////////////////////////////////////////
				1, 4, 5,/*  */9, 8, 4,/*  */7, 3, 8,/*  */6, 11, 3,/*  */10, 5, 11,
				4, 1, 9,/*  */8, 9, 7,/*  */3, 7, 6,/*  */11, 6, 10,/*  */5, 10, 1,
				/////////////////////////////////////////////////////////////////
				2, 9, 1,/*  */2, 7, 9,/*  */2, 6, 7,/*  */2, 10, 6,/*  */2, 1, 10,
			};

			std::vector<glm::vec3> vVertice = vIcoVertice;

			std::vector<GLuint> vIndice = vIcoIndice, vIndiceTmp = vIcoIndice;

			for (size_t i = 0; i < subdivison; i++)
			{
				vIndice.clear();
				int numFace = vIndiceTmp.size() / 3;
				for (size_t j = 0, idx = 0; j < numFace; j++, idx += 3)
				{
					GLuint indice0 = vIndiceTmp[idx];
					GLuint indice1 = vIndiceTmp[idx + 1];
					GLuint indice2 = vIndiceTmp[idx + 2];
					
					glm::vec3 midVert01 = 0.5f*(vVertice[indice0] + vVertice[indice1]);
					glm::vec3 midVert12 = 0.5f*(vVertice[indice1] + vVertice[indice2]);
					glm::vec3 midVert20 = 0.5f*(vVertice[indice2] + vVertice[indice0]);
					/*midVert01 = glm::normalize(midVert01)*radius;
					midVert12 = glm::normalize(midVert12)*radius;
					midVert20 = glm::normalize(midVert20)*radius;*/

					GLuint indice01 = vVertice.size();
					vVertice.push_back(midVert01);
					GLuint indice12 = vVertice.size();
					vVertice.push_back(midVert12);
					GLuint indice20 = vVertice.size();
					vVertice.push_back(midVert20);

					std::vector<GLuint> vIndiceSub =
					{
						indice0, indice01, indice20,
						indice01, indice1, indice12,
						indice12, indice2, indice20,
						indice12, indice20, indice01
					};

					vIndice.insert(vIndice.end(), vIndiceSub.begin(), vIndiceSub.end());
				}
				vIndiceTmp = vIndice;
			}

			std::vector<glm::vec4> vColor;
			for (size_t i = 0; i < vVertice.size(); i++)
			{
				glm::vec3 &vert = vVertice[i];

				vert = glm::normalize(vert);
				glm::vec3 color = 0.5f * (vert + glm::vec3(1.0f));
				//glm::vec3 color =  vert;
				vColor.push_back(glm::vec4(color, 1.0f));
				vert = vert * radius;
			}

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, TRIANGLES);
			pVertexArray->setNormals(vVertice);
			
			//pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>(
				glm::vec4(color, 1.0f), glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();

			mNumFace = vIndice.size() / 3;
			mNumVertice = vVertice.size();
		}

		int getNumVertice()
		{
			return mNumVertice;
		}

		int getNumFace()
		{
			return mNumFace;
		}

		virtual void drawOnlyInScene(const GLuint &programID)
		{
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			Mesh::drawOnlyInScene(programID);

			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}

		virtual void draw(const GLuint &programID)
		{
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			Mesh::draw(programID);

			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}

	private:
		int mNumFace;
		int mNumVertice;

		/*virtual void drawOnlyInScene(const GLuint &programID)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current mesh has not been uploaded befor drawing");
				return;
			}
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			mpVertexArray->draw(programID);
			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}*/
	};

	class UVSphere : public Mesh
	{
	public:
		UVSphere(float radius, glm::vec3 color, int subdivison = 0)
		{
			int numLongitude = 1 << (subdivison + 2);
			int numLatitude = (1 << (subdivison + 1)) - 1;

			int numVertice = 2 + numLatitude * numLongitude;
			int numFace = numLatitude * numLongitude * 2;

			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numFace*3);
			{
				vVertice[0] = { 0.0f,  radius, 0.0f };
				int numInterval = 1 << subdivison;
				float angleInterval = glm::half_pi<float>() /  numInterval;

				//Add the first row triangles
				{
					float fai = angleInterval;
					float y = cos(fai) * radius;
					float xz = sin(fai) * radius;
					vVertice[1] = { xz, y, 0 };
					int vertIdx = 2, indiceIdx = 0;

					for (int j = 1; j < numLongitude; j++)
					{
						float theta = j * angleInterval;
						float x = cos(theta) * xz;
						float z = -sin(theta) * xz;
						vVertice[vertIdx] = { x, y, z };
						vIndice[indiceIdx] = vertIdx;
						vIndice[indiceIdx + 1] = 0;
						vIndice[indiceIdx + 2] = vertIdx - 1;

						vertIdx++;
						indiceIdx += 3;
					}

					vIndice[indiceIdx] = 1;
					vIndice[indiceIdx + 1] = 0;
					vIndice[indiceIdx + 2] = vertIdx - 1;
				}

				//Add the middle rows triangles
				{
					int vertIdx = numLongitude + 1;
					int indiceIdx = numLongitude * 3;

					for (int i = 2; i <= numLatitude; i++)
					{
						float fai = i * angleInterval;
						float y = cos(fai) * radius;
						float xz = sin(fai) * radius;
						vVertice[vertIdx] = { xz, y, 0 };
						int vertX0Idx = vertIdx;

						vertIdx++;

						for (int j = 1; j < numLongitude; j++)
						{
							float theta = j * angleInterval;
							float x = cos(theta) * xz;
							float z = -sin(theta) * xz;
							vVertice[vertIdx] = { x, y, z };

							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - numLongitude - 1;
							vIndice[indiceIdx + 2] = vertIdx - 1;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numLongitude;
							vIndice[indiceIdx + 5] = vertIdx - numLongitude - 1;

							vertIdx++;
							indiceIdx += 6;
						}

						vIndice[indiceIdx] = vertX0Idx;
						vIndice[indiceIdx + 1] = vertX0Idx - 1;
						vIndice[indiceIdx + 2] = vertIdx - 1;
						vIndice[indiceIdx + 3] = vertX0Idx;
						vIndice[indiceIdx + 4] = vertX0Idx - numLongitude;
						vIndice[indiceIdx + 5] = vertX0Idx - 1;

						indiceIdx += 6;
					}
				}

				vVertice[numVertice - 1] = { 0.0f,  -radius, 0.0f };
				//Add the last row triangles
				{
					int vertIdx = numVertice - numLongitude;
					int indiceIdx = (numFace - numLongitude) * 3;

					for (int j = 1; j < numLongitude; j++)
					{
						vIndice[indiceIdx] = vertIdx;
						vIndice[indiceIdx + 1] = vertIdx - 1;
						vIndice[indiceIdx + 2] = numVertice - 1;

						vertIdx++;
						indiceIdx += 3;
					}

					vIndice[indiceIdx] = numVertice - numLongitude - 1;
					vIndice[indiceIdx + 1] = numVertice - 2;
					vIndice[indiceIdx + 2] = numVertice - 1;
				}

			}
			

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, PrimitiveType::TRIANGLES);
			pVertexArray->setNormals(vVertice);
			std::vector<glm::vec4> vColor;
			std::for_each(vVertice.begin(), vVertice.end(), [&](glm::vec3 &vert)
			{
				glm::vec3 color = 0.5f * (glm::normalize(vert) + glm::vec3(1.0f));
				//glm::vec3 color =  vert;
				vColor.push_back(glm::vec4(color, 1.0f));
			});
			//pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>(
				glm::vec4(color, 1.0f), glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();

			mNumFace = vIndice.size() / 3;
			mNumVertice = vVertice.size();
		}

		int getNumVertice()
		{
			return mNumVertice;
		}

		int getNumFace()
		{
			return mNumFace;
		}

	private:
		int mNumFace;
		int mNumVertice;
	};

	class UVSphereLine : public Mesh
	{
	public:
		UVSphereLine(float radius, glm::vec3 color, 
					 int subdivisionLa = 0,
					 int subdivisionLo = 0)
		{
			int numLongitude = 1 << (subdivisionLo + 2);
			int numLatitude = (1 << (subdivisionLa + 1)) - 1;

			int numVertice = 2 + numLatitude * numLongitude;
			int numLine = (numLatitude + 1) * numLongitude;

			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numLine * 2);
			{
				vVertice[0] = { 0.0f,  radius, 0.0f };
				int numIntervalLa = 1 << subdivisionLa;
				int numIntervalLo = 1 << subdivisionLo;
				float angleIntervalLa = glm::half_pi<float>() / numIntervalLa;
				float angleIntervalLo = glm::half_pi<float>() / numIntervalLo;

				//Add the first row triangles
				{
					float fai = angleIntervalLa;
					float y = cos(fai) * radius;
					float xz = sin(fai) * radius;
					vVertice[1] = { xz, y, 0 };
					int vertIdx = 2, indiceIdx = 0;

					for (int j = 1; j < numLongitude; j++)
					{
						float theta = j * angleIntervalLo;
						float x = cos(theta) * xz;
						float z = -sin(theta) * xz;
						vVertice[vertIdx] = { x, y, z };
						vIndice[indiceIdx] = vertIdx;
						vIndice[indiceIdx + 1] = 0;

						vertIdx++;
						indiceIdx += 2;
					}

					vIndice[indiceIdx] = 1;
					vIndice[indiceIdx + 1] = 0;
				}

				//Add the middle rows triangles
				{
					int vertIdx = numLongitude + 1;
					int indiceIdx = numLongitude * 2;

					for (int i = 2; i <= numLatitude; i++)
					{
						float fai = i * angleIntervalLa;
						float y = cos(fai) * radius;
						float xz = sin(fai) * radius;
						vVertice[vertIdx] = { xz, y, 0 };
						int vertX0Idx = vertIdx;

						vertIdx++;

						for (int j = 1; j < numLongitude; j++)
						{
							float theta = j * angleIntervalLo;
							float x = cos(theta) * xz;
							float z = -sin(theta) * xz;
							vVertice[vertIdx] = { x, y, z };

							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - numLongitude;

							vertIdx++;
							indiceIdx += 2;
						}

						vIndice[indiceIdx] = vertX0Idx;
						vIndice[indiceIdx + 1] = vertX0Idx - numLongitude;

						indiceIdx += 2;
					}
				}

				vVertice[numVertice - 1] = { 0.0f,  -radius, 0.0f };
				//Add the last row triangles
				{
					int vertIdx = numVertice - numLongitude;
					int indiceIdx = (numLine - numLongitude) * 2;

					for (int j = 1; j < numLongitude; j++)
					{
						vIndice[indiceIdx] = vertIdx;
						vIndice[indiceIdx + 1] = numVertice - 1;

						vertIdx++;
						indiceIdx += 2;
					}

					vIndice[indiceIdx] = numVertice - numLongitude - 1;
					vIndice[indiceIdx + 1] = numVertice - 1;
				}

			}

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, PrimitiveType::LINES);
			pVertexArray->setNormals(vVertice);
			std::vector<glm::vec4> vColor;
			std::for_each(vVertice.begin(), vVertice.end(), [&](glm::vec3 &vert)
			{
				glm::vec3 color = 0.5f * (glm::normalize(vert) + glm::vec3(1.0f));
				//glm::vec3 color =  vert;
				vColor.push_back(glm::vec4(color, 1.0f));
			});
			//pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>(
				glm::vec4(color, 1.0f), glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();

			mNumFace = vIndice.size() / 3;
			mNumVertice = vVertice.size();
		}

		~UVSphereLine() {}

		int getNumVertice()
		{
			return mNumVertice;
		}

		int getNumFace()
		{
			return mNumFace;
		}

	private:
		int mNumFace;
		int mNumVertice;
	};

	class CubeSphere : public Mesh
	{
	public:
		CubeSphere(float radius, glm::vec4 color, int subdivison = 0)
		{
			//icosahedron parameters a, b for the rectangle height and width
			float a =  2.0f * std::sqrtf(3.0f) * radius / 3.0f;
			float a_2 = a * 0.5f;
			std::vector<glm::vec3> vCubeVertice =
			{
				//up rectangle
				{ a_2, a_2, a_2 },
				{ a_2, a_2, -a_2 },
				{ -a_2, a_2, -a_2 },
				{ -a_2, a_2, a_2 },

				//down rectangle
				{ a_2, -a_2, a_2 },
				{ a_2, -a_2, -a_2 },
				{ -a_2, -a_2, -a_2 },
				{ -a_2, -a_2, a_2 }
			};

			std::vector<GLuint> vCubeIndice =
			{
				0, 1, 2,/*  */0, 2, 3, //y-positive up 
				//////////////////////
				0, 7, 4,/*  */0, 3, 7, //z-positive back 
				//////////////////////
				0, 4, 5,/*  */0, 5, 1, //x-positive right
				////////////////////// 
				3, 6, 7,/*  */3, 2, 6, //x-negative left
				//////////////////////
				1, 5, 6,/*  */1, 6, 2, //z-negative front
				//////////////////////
				4, 6, 5,/*  */4, 7, 6  //y-negative down 
			};

			int numInterval = 1 << subdivison;
			int numVertice = 2 * (numInterval + 1) * (numInterval + 1) +
				(numInterval - 1) * numInterval * 4;
			int numFace = 6 * numInterval * numInterval * 2;

			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numFace * 3);
			
			{
				float aInterval = a / numInterval;

				//Add the top cube sphere part
				{
					int vertIdx = 0, indiceIdx = 0;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, a_2, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}
					
					float zStart = a_2 - aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vertIdx++;
							indiceIdx += 6;

							xStart += aInterval;
						}

						zStart -= aInterval;
					}


				}

				std::vector<GLuint> vLastUpRowVertIdx(numInterval * 4);
				{
					//Left up
					for (int i = 0, vertIdx = 0;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vLastUpRowVertIdx[i] = vertIdx;
					//Front up
					for (int i = 0, vertIdx = (numInterval + 1) * numInterval;
						 i < numInterval; i++, vertIdx++)
						vLastUpRowVertIdx[i + numInterval] = vertIdx;
					//Right up
					for (int i = 0, vertIdx = (numInterval + 1) * (numInterval + 1) - 1;
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vLastUpRowVertIdx[i + numInterval*2] = vertIdx;
					//Back up
					for (int i = 0, vertIdx = numInterval;
						 i < numInterval; i++, vertIdx--)
						vLastUpRowVertIdx[i + numInterval*3] = vertIdx;
				}

				//Add the middle cube sphere parts
				//Left-->Front-->Right-->Back
				{
					int vertIdx = (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numInterval * numInterval * 2) * 3;

					if (numInterval > 1)
					{
						float yStart = a_2 - aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 5] = vertStartIdx;
						indiceIdx += 6;
					}

					for (int i = 2; i < numInterval; i++)
					{
						float yStart = a_2 - float(i) * aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
						vIndice[indiceIdx + 2] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 5] = vertStartIdx;
						indiceIdx += 6;

						yStart -= aInterval;
					}

					if (numInterval > 1)
					{
						int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1)
							- numInterval * 4;
						for (int i = 0; i < numInterval * 4; i++)
						{
							vLastUpRowVertIdx[i] = vertIdx;
							vertIdx++;
						}
					}
				}

				//Add the bottom cube sphere part
				{
					int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numFace - numInterval * numInterval * 2) * 3;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, -a_2, -a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = -a_2 + aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, -a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, -a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vertIdx++;
							indiceIdx += 6;

							xStart += aInterval;
						}

						zStart += aInterval;
					}
				}

				std::vector<GLuint> vDownRowVertIdx(numInterval * 4);
				{
					//Left down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1);
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vDownRowVertIdx[i] = vertIdx;
					//Front down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * (numInterval + 1);
						 i < numInterval; i++, vertIdx++)
						vDownRowVertIdx[i + numInterval] = vertIdx;
					//Right down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * numInterval - 1;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vDownRowVertIdx[i + numInterval*2] = vertIdx;
					//Back down
					for (int i = 0, vertIdx = numVertice - 1;
						 i < numInterval; i++, vertIdx--)
						vDownRowVertIdx[i + numInterval*3] = vertIdx;
				}

				//Add the faces between the last up and the Down row vertices
				{
					int indiceIdx = (numFace - (numInterval + 4) * numInterval * 2) * 3;
					int i = 0;
					for (; i < numInterval * 4 - 1; i++)
					{
						vIndice[indiceIdx] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 5] = vDownRowVertIdx[i + 1];
						indiceIdx += 6;
					}

					vIndice[indiceIdx] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
					vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 5] = vDownRowVertIdx[0];
				}
				
			}

			std::vector<glm::vec4> vColor;
			std::for_each(vVertice.begin(), vVertice.end(), [&](glm::vec3 &vert)
			{
				vert = glm::normalize(vert);
				glm::vec3 vertColor = 0.5f * (vert + glm::vec3(1.0f));
				//glm::vec3 color =  vert;
				vColor.push_back(glm::vec4(vertColor, color.w));

				vert *= radius;
			});

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, TRIANGLES);
			//pVertexArray->setNormals(vVertice);
			pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial =
				std::make_shared<Material>(color, glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();

			mNumFace = vIndice.size() / 3;
			mNumVertice = vVertice.size();
		}

		int getNumVertice()
		{
			return mNumVertice;
		}

		int getNumFace()
		{
			return mNumFace;
		}

	private:
		int mNumFace;
		int mNumVertice;
	};

	class CubeLine : public Mesh
	{
	public:
		CubeLine(float radius, glm::vec4 color, int subdivison = 0)
		{
			//icosahedron parameters a, b for the rectangle height and width
			float a = 2.0f * std::sqrtf(3.0f) * radius / 3.0f;
			float a_2 = a * 0.5f;
			int numInterval = 1 << subdivison;
			int numVertice = 2 * (numInterval + 1) * (numInterval + 1) +
				(numInterval - 1) * numInterval * 4;
			int numLine = 6 * numInterval * numInterval * 4;

			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numLine * 2);

			{
				float aInterval = a / numInterval;

				//Add the top cube sphere part
				{
					int vertIdx = 0, indiceIdx = 0;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, a_2, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = a_2 - aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - 1;
							vIndice[indiceIdx + 3] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;
							vIndice[indiceIdx + 6] = vertIdx - numInterval - 1;
							vIndice[indiceIdx + 7] = vertIdx;

							vertIdx++;
							indiceIdx += 8;

							xStart += aInterval;
						}

						zStart -= aInterval;
					}

				}

				std::vector<GLuint> vLastUpRowVertIdx(numInterval * 4);
				{
					//Left up
					for (int i = 0, vertIdx = 0;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vLastUpRowVertIdx[i] = vertIdx;
					//Front up
					for (int i = 0, vertIdx = (numInterval + 1) * numInterval;
						 i < numInterval; i++, vertIdx++)
						vLastUpRowVertIdx[i + numInterval] = vertIdx;
					//Right up
					for (int i = 0, vertIdx = (numInterval + 1) * (numInterval + 1) - 1;
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vLastUpRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back up
					for (int i = 0, vertIdx = numInterval;
						 i < numInterval; i++, vertIdx--)
						vLastUpRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the middle cube sphere parts
				//Left-->Front-->Right-->Back
				{
					int vertIdx = (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numInterval * numInterval * 4) * 2;

					if (numInterval > 1)
					{
						float yStart = a_2 - aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 3] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;

							indiceIdx += 8;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 3] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;

							indiceIdx += 8;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 3] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;

							indiceIdx += 8;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 3] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;

							indiceIdx += 6;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
						vIndice[indiceIdx + 3] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 5] = vertStartIdx;
						vIndice[indiceIdx + 6] = vertStartIdx;
						vIndice[indiceIdx + 7] = vertIdx - 1;
						indiceIdx += 8;
					}

					for (int i = 2; i < numInterval; i++)
					{
						float yStart = a_2 - float(i) * aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;


							indiceIdx += 8;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;

							indiceIdx += 8;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;

							indiceIdx += 8;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;
							vIndice[indiceIdx + 6] = vertIdx;
							vIndice[indiceIdx + 7] = vertIdx - 1;

							indiceIdx += 8;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
						vIndice[indiceIdx + 2] = vertIdx - 1 - numInterval * 4;
						vIndice[indiceIdx + 3] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 4] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 5] = vertStartIdx;
						vIndice[indiceIdx + 6] = vertStartIdx;
						vIndice[indiceIdx + 7] = vertIdx - 1;
						indiceIdx += 8;

						yStart -= aInterval;
					}

					if (numInterval > 1)
					{
						int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1)
							- numInterval * 4;
						for (int i = 0; i < numInterval * 4; i++)
						{
							vLastUpRowVertIdx[i] = vertIdx;
							vertIdx++;
						}
					}
				}

				//Add the bottom cube sphere part
				{
					int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numLine - numInterval * numInterval * 4) * 2;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, -a_2, -a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = -a_2 + aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, -a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, -a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - 1;
							vIndice[indiceIdx + 3] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;
							vIndice[indiceIdx + 6] = vertIdx - numInterval - 1;
							vIndice[indiceIdx + 7] = vertIdx;

							vertIdx++;
							indiceIdx += 8;

							xStart += aInterval;
						}

						zStart += aInterval;
					}
				}

				std::vector<GLuint> vDownRowVertIdx(numInterval * 4);
				{
					//Left down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1);
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vDownRowVertIdx[i] = vertIdx;
					//Front down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * (numInterval + 1);
						 i < numInterval; i++, vertIdx++)
						vDownRowVertIdx[i + numInterval] = vertIdx;
					//Right down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * numInterval - 1;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vDownRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back down
					for (int i = 0, vertIdx = numVertice - 1;
						 i < numInterval; i++, vertIdx--)
						vDownRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the faces between the last up and the Down row vertices
				{
					int indiceIdx = (numLine - (numInterval + 4) * numInterval * 4) * 2;
					int i = 0;
					for (; i < numInterval * 4 - 1; i++)
					{
						vIndice[indiceIdx] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[i];
						vIndice[indiceIdx + 3] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 5] = vDownRowVertIdx[i + 1];
						vIndice[indiceIdx + 6] = vDownRowVertIdx[i + 1];
						vIndice[indiceIdx + 7] = vDownRowVertIdx[i];
						indiceIdx += 8;
					}

					vIndice[indiceIdx] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
					vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 5] = vDownRowVertIdx[0];

					vIndice[indiceIdx] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
					vIndice[indiceIdx + 2] = vLastUpRowVertIdx[i];
					vIndice[indiceIdx + 3] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 5] = vDownRowVertIdx[0];
					vIndice[indiceIdx + 6] = vDownRowVertIdx[0];
					vIndice[indiceIdx + 7] = vDownRowVertIdx[i];
				}

			}

			std::vector<glm::vec4> vColor;
			std::for_each(vVertice.begin(), vVertice.end(), [&](glm::vec3 &vert)
			{
				glm::vec3 vertColor = 0.5f * (glm::normalize(vert) + glm::vec3(1.0f));
				//glm::vec3 color =  vert;
				vColor.push_back(glm::vec4(vertColor, color.w));

				vert = glm::normalize(vert) * radius;
			});


			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, PrimitiveType::LINES);

			

			pVertexArray->setNormals(vVertice);
			//pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial =
				std::make_shared<Material>(color, glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();

			mNumFace = vIndice.size() / 3;
			mNumVertice = vVertice.size();
		}

		~CubeLine() {}

		int getNumVertice()
		{
			return mNumVertice;
		}

		int getNumFace()
		{
			return mNumFace;
		}

	private:
		int mNumFace;
		int mNumVertice;
	};
}
