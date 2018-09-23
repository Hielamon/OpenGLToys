#pragma once

#include "Mesh.h"

namespace SP
{
	class Sphere : public Mesh
	{
	public:
		Sphere(float radius, glm::vec3 color)
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
				{  a_2, 0.0f, -b_2 },
				{  a_2, 0.0f,  b_2 },
				{ -a_2, 0.0f,  b_2 },
				{ -a_2, 0.0f, -b_2 },


				//Rectangle 2
				{ 0.0f,  a_2, -b_2, },
				{ 0.0f, -a_2, -b_2, },
				{ 0.0f, -a_2,  b_2, },
				{ 0.0f,  a_2,  b_2, },

			};

			std::vector<GLuint> vIndice =
			{
				0, 5, 4,/*  */0, 4, 8,/*  */0, 8, 3,/*  */0, 3, 11,/*  */0, 11, 5,
				/////////////////////////////////////////////////////////////////
				1, 4, 5,/*  */9, 8, 4,/*  */7, 3, 8,/*  */6, 11, 3,/*  */10, 5, 11,
				4, 1, 9,/*  */8, 9, 7,/*  */3, 7, 6,/*  */11, 6, 10,/*  */5, 10, 1,
				/////////////////////////////////////////////////////////////////
				2, 9, 1,/*  */2, 7, 9,/*  */2, 6, 7,/*  */2, 10, 6,/*  */2, 1, 10,
			};

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, TRIANGLES);
			pVertexArray->setNormals(vVertice);

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>(
				glm::vec4(color, 1.0f), glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();
		}
	};
}
