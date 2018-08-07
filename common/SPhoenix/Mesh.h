#pragma once

#include "utils.h"
#include "VertexArray.h"
#include "Texture.h"

namespace SP
{
	class Mesh
	{
	public:
		Mesh(const std::shared_ptr<VertexArray> &pVertexArray,
			 const std::vector<std::shared_ptr<Texture>> &pTextures)
			: mpVertexArray(pVertexArray), mvpTexture(pTextures) {}
		
		Mesh() {}
		~Mesh() {}

		void reset()
		{
			mvpTexture.clear();
			mpVertexArray.reset();
		}

		void reset(const std::shared_ptr<VertexArray> &pVertexArray,
				   const std::vector<std::shared_ptr<Texture>> &pTextures)
		{
			mvpTexture = pTextures;
			mpVertexArray = pVertexArray;
		}

	protected:
		std::vector<std::shared_ptr<Texture>> mvpTexture;
		std::shared_ptr<VertexArray> mpVertexArray;
	};

}