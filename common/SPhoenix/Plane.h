#pragma once
#include "Mesh.h"

namespace SP
{
	//The plane was preset on the xy-plane defaultly
	class Plane : public Mesh
	{
	public:
		Plane(float width, float height, glm::mat4 m  = glm::mat4(1.0f), int subdivision = 0)
			: mSubDivision(subdivision)
		{
			float w_2 = width * 0.5, h_2 = height * 0.5;
			int numInterval = 1 << subdivision;
			int numVertice = (numInterval + 1) * (numInterval + 1);
			int numFace = numInterval * numInterval * 2;
			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numFace * 3);

			float xInterval = width / numInterval;
			float yInterval = height / numInterval;

			float xOffset = -w_2, yOffset = -h_2;
			for (size_t i = 0, vertIdx = 0; i <= numInterval; i++)
			{
				xOffset = -w_2;
				for (size_t j = 0; j <= numInterval; j++)
				{
					vVertice[vertIdx] = { xOffset, yOffset, 0.0f };
					xOffset += xInterval;
					vertIdx++;
				}
				yOffset += yInterval;
			}

			for (size_t i = 0, vertIdx = 0, indiceIdx = 0; i < numInterval; i++)
			{
				for (size_t j = 0; j < numInterval; j++)
				{
					vIndice[indiceIdx] = vertIdx;
					vIndice[indiceIdx + 1] = vertIdx + 1;
					vIndice[indiceIdx + 2] = vertIdx + 1 + (numInterval + 1);
					vIndice[indiceIdx + 3] = vertIdx;
					vIndice[indiceIdx + 4] = vertIdx + 1 + (numInterval + 1);
					vIndice[indiceIdx + 5] = vertIdx + (numInterval + 1);

					indiceIdx += 6;
					vertIdx++;
				}
				vertIdx++;
			}

			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vVertice, vIndice,
											  PrimitiveType::TRIANGLES);

			std::vector<glm::vec3> vNormal(numVertice, glm::vec3(0.0f, 0.0f, 1.0f));
			pVA->setNormals(vNormal);
			pVA->addInstance(m);

			std::shared_ptr<Material> pMat = std::make_shared<Material>();

			setVertexArray(pVA);
			setMaterial(pMat);
		}
		~Plane() {}

		void setTexCoord(float xOffset, float yOffset, float width, float height)
		{
			int numInterval = 1 << mSubDivision;
			int numVertice = (numInterval + 1) * (numInterval + 1);
			std::vector<glm::vec2> vTexCoord(numVertice);

			float xInterval = width / numInterval;
			float yInterval = height / numInterval;

			float xOffset_ = xOffset, yOffset_ = yOffset;
			for (size_t i = 0, vertIdx = 0; i <= numInterval; i++)
			{
				xOffset_ = xOffset;
				for (size_t j = 0; j <= numInterval; j++)
				{
					vTexCoord[vertIdx] = { xOffset_, yOffset_};
					xOffset_ += xInterval;
					vertIdx++;
				}
				yOffset_ += yInterval;
			}

			mpVertexArray->setTexCoords(vTexCoord);
			setVertexArray(mpVertexArray);

			if(mbUploaded)
			{
				mpVertexArray->clearUploaded();
				mpVertexArray->uploadToDevice();
			}
		}

	private:
		int mSubDivision;
	};
}
