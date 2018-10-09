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
					midVert01 = glm::normalize(midVert01)*radius;
					midVert12 = glm::normalize(midVert12)*radius;
					midVert20 = glm::normalize(midVert20)*radius;

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

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, TRIANGLES);
			pVertexArray->setNormals(vVertice);
			//pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>(
				glm::vec4(color, 1.0f), glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();
		}

		int getNumVertice()
		{
			return mNumVertice;
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
}
