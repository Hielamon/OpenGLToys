#pragma once

#include "Mesh.h"

namespace SP
{
	class BBoxMesh : public Mesh
	{
	public:
		BBoxMesh(const BBox &bbox, const glm::vec3 &color = glm::vec3(1.0f, 0.0f, 0.0f))
		{
			std::vector<glm::vec3> vVertice = bbox.getBBoxVertices();
			std::vector<GLuint> vIndice = {
				0, 1, 1, 2, 2, 3, 3, 0,
				4, 5, 5, 6, 6, 7, 7, 4,
				0, 5, 1, 6, 2, 7, 3, 4
			};

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vIndice, LINES);

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>(
				glm::vec4(color, 1.0f), glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();
		}

		~BBoxMesh() {}
	};
}
