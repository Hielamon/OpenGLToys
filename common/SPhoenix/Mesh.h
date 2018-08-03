#pragma once

#include "utils.h"

namespace SP
{
	struct Texture {
		GLuint id;
		std::string type;
		std::string path;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<glm::vec3> &vertices,
			 const std::vector<glm::vec3> &normals,
			 const std::vector<glm::vec2> &texcoords, 
			 const std::vector<std::shared_ptr<Texture>> &texturePs)
			: mvpTexture(texturePs)
		{
			mpvVertice = std::make_shared<std::vector<glm::vec3>>(vertices);
			mpvNormal = std::make_shared<std::vector<glm::vec3>>(normals);
			mpvTexCoord = std::make_shared<std::vector<glm::vec2>>(normals);
		}

		Mesh(const std::shared_ptr<std::vector<glm::vec3>> &pVertices,
			 const std::shared_ptr<std::vector<glm::vec3>> &pNormals,
			 const std::shared_ptr<std::vector<glm::vec2>> &pTexcoords,
			 const std::vector<std::shared_ptr<Texture>> &pTextures)
			: mpvVertice(pVertices), mpvNormal(pNormals), mpvTexCoord(pTexcoords), mvpTexture(pTextures) {}

		Mesh() {}
		~Mesh() {}

	protected:
		std::shared_ptr<std::vector<glm::vec3>> mpvVertice;
		std::shared_ptr<std::vector<glm::vec3>> mpvNormal;
		std::shared_ptr<std::vector<glm::vec2>> mpvTexCoord;
		std::vector<std::shared_ptr<Texture>> mvpTexture;
	};

}