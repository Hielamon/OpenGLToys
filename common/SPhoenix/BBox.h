#pragma once

#include "Utils.h"
#include <vector>

namespace SP
{
	//The class of bounding box
	class BBox
	{
	public:
		BBox(const float &x1, const float &y1, const float &z1,
			const float &x2, const float &y2, const float &z2)
		{
			mMinVertex.x = x1 <= x2 ? x1 : x2;
			mMinVertex.y = y1 <= y2 ? y1 : y2;
			mMinVertex.z = z1 <= z2 ? z1 : z2;
			mMaxVertex.x = x1 > x2 ? x1 : x2;
			mMaxVertex.y = y1 > y2 ? y1 : y2;
			mMaxVertex.z = z1 > z2 ? z1 : z2;
		}

		BBox(const glm::vec3 &vertex1, const glm::vec3 &vertex2)
		{
			mMinVertex.x = vertex1.x <= vertex2.x ? vertex1.x : vertex2.x;
			mMinVertex.y = vertex1.y <= vertex2.y ? vertex1.y : vertex2.y;
			mMinVertex.z = vertex1.z <= vertex2.z ? vertex1.z : vertex2.z;
			mMaxVertex.x = vertex1.x > vertex2.x ? vertex1.x : vertex2.x;
			mMaxVertex.y = vertex1.y > vertex2.y ? vertex1.y : vertex2.y;
			mMaxVertex.z = vertex1.z > vertex2.z ? vertex1.z : vertex2.z;
		}
		
		BBox(const std::vector<glm::vec3> &vertices)
		{
			if (vertices.empty()) return;

			mMinVertex = vertices[0];
			mMaxVertex = vertices[0];

			for (size_t i = 1; i < vertices.size(); i++)
			{
				const glm::vec3 &vertex = vertices[i];
				if (vertex.x < mMinVertex.x) mMinVertex.x = vertex.x;
				if (vertex.y < mMinVertex.y) mMinVertex.y = vertex.y;
				if (vertex.z < mMinVertex.z) mMinVertex.z = vertex.z;
				if (vertex.x > mMaxVertex.x) mMaxVertex.x = vertex.x;
				if (vertex.y > mMaxVertex.y) mMaxVertex.y = vertex.y;
				if (vertex.z > mMaxVertex.z) mMaxVertex.z = vertex.z;
			}
		}

		BBox()
		{
			mMinVertex = glm::vec3(std::numeric_limits<float>::max());
			mMaxVertex = glm::vec3(std::numeric_limits<float>::lowest());
		}

		~BBox() {}

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
		std::vector<glm::vec3> getBBoxVertices()
		{
			std::vector<glm::vec3> vertices(8);
			vertices[0] = mMinVertex;
			vertices[1] = glm::vec3(mMaxVertex.x, mMinVertex.y, mMinVertex.z);
			vertices[2] = glm::vec3(mMaxVertex.x, mMinVertex.y, mMaxVertex.z);
			vertices[3] = glm::vec3(mMinVertex.x, mMinVertex.y, mMaxVertex.z);
			vertices[4] = glm::vec3(mMinVertex.x, mMaxVertex.y, mMaxVertex.z);
			vertices[5] = glm::vec3(mMinVertex.x, mMaxVertex.y, mMinVertex.z);
			vertices[6] = glm::vec3(mMaxVertex.x, mMaxVertex.y, mMinVertex.z);
			vertices[7] = mMaxVertex;
			return vertices;
		}

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
		std::vector<glm::vec3> getBBoxVertices() const
		{
			std::vector<glm::vec3> vertices(8);
			vertices[0] = mMinVertex;
			vertices[1] = glm::vec3(mMaxVertex.x, mMinVertex.y, mMinVertex.z);
			vertices[2] = glm::vec3(mMaxVertex.x, mMinVertex.y, mMaxVertex.z);
			vertices[3] = glm::vec3(mMinVertex.x, mMinVertex.y, mMaxVertex.z);
			vertices[4] = glm::vec3(mMinVertex.x, mMaxVertex.y, mMaxVertex.z);
			vertices[5] = glm::vec3(mMinVertex.x, mMaxVertex.y, mMinVertex.z);
			vertices[6] = glm::vec3(mMaxVertex.x, mMaxVertex.y, mMinVertex.z);
			vertices[7] = mMaxVertex;
			return vertices;
		}

		glm::vec3 getMinVertex()
		{
			return mMinVertex;
		}

		glm::vec3 getMaxVertex()
		{
			return mMaxVertex;
		}

		BBox operator +(const BBox &b)
		{
			{
				const glm::vec3 &vertex = b.mMinVertex;
				if (vertex.x < mMinVertex.x) mMinVertex.x = vertex.x;
				if (vertex.y < mMinVertex.y) mMinVertex.y = vertex.y;
				if (vertex.z < mMinVertex.z) mMinVertex.z = vertex.z;
			}
			{
				const glm::vec3 &vertex = b.mMaxVertex;
				if (vertex.x > mMaxVertex.x) mMaxVertex.x = vertex.x;
				if (vertex.y > mMaxVertex.y) mMaxVertex.y = vertex.y;
				if (vertex.z > mMaxVertex.z) mMaxVertex.z = vertex.z;
			}
		}

		BBox& operator +=(const BBox &b)
		{
			{
				const glm::vec3 &vertex = b.mMinVertex;
				if (vertex.x < mMinVertex.x) mMinVertex.x = vertex.x;
				if (vertex.y < mMinVertex.y) mMinVertex.y = vertex.y;
				if (vertex.z < mMinVertex.z) mMinVertex.z = vertex.z;
			}
			{
				const glm::vec3 &vertex = b.mMaxVertex;
				if (vertex.x > mMaxVertex.x) mMaxVertex.x = vertex.x;
				if (vertex.y > mMaxVertex.y) mMaxVertex.y = vertex.y;
				if (vertex.z > mMaxVertex.z) mMaxVertex.z = vertex.z;
			}

			return *this;
		}

	private:
		glm::vec3 mMinVertex, mMaxVertex;
	};

	inline BBox TransformBBox(const glm::mat4 &T, BBox &box)
	{
		std::vector<glm::vec3> vBBoxVertex = box.getBBoxVertices();
		for (size_t i = 0; i < vBBoxVertex.size(); i++)
		{
			glm::vec4 hVertex = glm::vec4(vBBoxVertex[i], 1.0f);
			hVertex = T * hVertex;
			float iw = 1.0 / hVertex.w;
			hVertex *= iw;
			vBBoxVertex[i] = glm::vec3(hVertex);
		}

		return BBox(vBBoxVertex);
	}
}
