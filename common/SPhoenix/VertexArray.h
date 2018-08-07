#pragma once
#include "utils.h"

namespace SP
{
	class VertexArray
	{
	public:
		VertexArray() {}
		~VertexArray() {}

		VertexArray(const std::vector<glm::vec3> &vertices,
			 const std::vector<glm::vec3> &normals,
			 const std::vector<glm::vec2> &texcoords,
			 const std::vector<GLuint> &indices)
		{
			mpvVertice = std::make_shared<std::vector<glm::vec3>>(vertices);
			mpvNormal = std::make_shared<std::vector<glm::vec3>>(normals);
			mpvTexCoord = std::make_shared<std::vector<glm::vec2>>(normals);
			mpvIndice = std::make_shared<std::vector<GLuint>>(indices);
		}

		VertexArray(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
			 const std::shared_ptr<std::vector<glm::vec3>> &pNormals,
			 const std::shared_ptr<std::vector<glm::vec2>> &pTexcoords,
			 const std::shared_ptr<std::vector<GLuint>> &pIndices)
			: mpvVertice(pVertices), mpvNormal(pNormals), mpvTexCoord(pTexcoords), mpvIndice(pIndices)
		{}

	protected:
		std::shared_ptr<std::vector<glm::vec3>> mpvVertice;
		std::shared_ptr<std::vector<glm::vec3>> mpvNormal;
		std::shared_ptr<std::vector<glm::vec2>> mpvTexCoord;
		std::shared_ptr<std::vector<GLuint>> mpvIndice;
	};

}