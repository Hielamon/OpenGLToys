#pragma once

#include "Mesh.h"

namespace SP
{
	class AxisMesh : public Mesh
	{
	public:
		AxisMesh(float scale = 1.0f, const glm::mat4 &m = glm::mat4())
		{
			std::vector<glm::vec3> vVertice(6), vColor(6);

			vVertice[0] = glm::vec3(0.0f, 0.0f, 0.0f);
			vVertice[1] = glm::vec3(scale, 0.0f, 0.0f);
			vVertice[2] = glm::vec3(0.0f, 0.0f, 0.0f);
			vVertice[3] = glm::vec3(0.0f, scale, 0.0f);
			vVertice[4] = glm::vec3(0.0f, 0.0f, 0.0f);
			vVertice[5] = glm::vec3(0.0f, 0.0f, scale);

			vColor[0] = glm::vec3(1.0f, 0.0f, 0.0f);
			vColor[1] = glm::vec3(1.0f, 0.0f, 0.0f);
			vColor[2] = glm::vec3(0.0f, 1.0f, 0.0f);
			vColor[3] = glm::vec3(0.0f, 1.0f, 0.0f);
			vColor[4] = glm::vec3(0.0f, 0.0f, 1.0f);
			vColor[5] = glm::vec3(0.0f, 0.0f, 1.0f);

			std::shared_ptr<VertexArrayC> pVertexArrayC = std::make_shared<VertexArrayC>(
				vVertice, vColor, std::vector<GLuint>(), LINES);

			setVertexArray(std::static_pointer_cast<VertexArray>(pVertexArrayC));
			addInstance(m);
		}

		~AxisMesh() {}
	};
}
