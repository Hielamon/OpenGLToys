#pragma once

#include <SPhoenix/utils.h>

namespace SP
{

	class Geometry
	{
	public:
		Geometry(const std::vector<GLfloat> &vertices,
				 const std::vector<GLfloat> &normals,
				 glm::vec4 color)
		{
			mpVertices = std::make_shared<std::vector<GLfloat>>(vertices);
			mpNormals = std::make_shared<std::vector<GLfloat>>(normals);
			mpColor = std::make_shared<glm::vec4>(color);
		}

		~Geometry() {}

	protected:
		Geometry() {}

		std::shared_ptr<std::vector<GLfloat>> mpVertices;
		std::shared_ptr<std::vector<GLfloat>> mpNormals;
		std::shared_ptr<glm::vec4> mpColor;
	};

	class GeometryUtil : public Geometry
	{
	public:
		GeometryUtil(Geometry &geom)
			: Geometry(geom)
		{
			std::vector<GLfloat> &vertices = *mpVertices;
			std::vector<GLfloat> &normals = *mpNormals;
			GLuint pointsNum = vertices.size();
			assert(pointsNum == normals.size() && pointsNum % 3 == 0);

			mprimmitiveNum = mpVertices->size() / 3;

			mvVBO.resize(2);
			glGenBuffers(1, &mvVBO[0]);
			glBindBuffer(GL_ARRAY_BUFFER, mvVBO[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*pointsNum, &vertices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glGenBuffers(1, &mvVBO[1]);
			glBindBuffer(GL_ARRAY_BUFFER, mvVBO[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0])*pointsNum, &normals[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glGenVertexArrays(1, &mVAO);
			{
				glBindVertexArray(mVAO);
				glBindBuffer(GL_ARRAY_BUFFER, mvVBO[0]);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ARRAY_BUFFER, mvVBO[1]);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(1);
			}
			glBindVertexArray(0);
		}

		~GeometryUtil()
		{
			glDeleteVertexArrays(1, &mVAO);
			glDeleteBuffers(2, &mvVBO[0]);
		}

		void show()
		{
			GLint programID;
			glGetIntegerv(GL_CURRENT_PROGRAM, &programID);

			GLint colorLoc = glGetUniformLocation(programID, "indicatedColor");
			glUniform4fv(colorLoc, 1, glm::value_ptr(*mpColor));

			glBindVertexArray(mVAO);
			glDrawArrays(GL_TRIANGLES, 0, mprimmitiveNum);
			glBindVertexArray(0);
		}

	private:
		GLuint mVAO;
		std::vector<GLuint> mvVBO;
		GLuint mprimmitiveNum;
	};
}



