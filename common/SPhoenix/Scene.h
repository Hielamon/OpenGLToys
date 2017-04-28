#pragma once

#include <SPhoenix/Shader.h>
#include <memory>

namespace SP
{
	class Scene
	{
	public:
		Scene(ShaderCodes &shaderCodes)
			: mshaderCodes(shaderCodes)
		{
			mvertices = {
				// First triangle
				0.5f,  0.5f, 0.0f,  // Top Right
				0.5f, -0.5f, 0.0f,  // Bottom Right
				-0.5f,  0.5f, 0.0f,  // Top Left 
				// Second triangle
				0.5f, -0.5f, 0.0f,  // Bottom Right
				-0.5f, -0.5f, 0.0f,  // Bottom Left
				-0.5f,  0.5f, 0.0f   // Top Left
			};
		}
		~Scene() {}

		ShaderCodes &getShaderCodes()
		{
			return mshaderCodes;
		}

		std::vector<GLfloat> &getVertices()
		{
			return mvertices;
		}

	private:
		ShaderCodes mshaderCodes;
		std::vector<GLfloat> mvertices;

	};

	class SceneUtil
	{
	public:
		SceneUtil(Scene &scene)
			: mshaderUtil(new ShaderUtil(scene.getShaderCodes()))
		{
			std::vector<GLfloat> vertices = scene.getVertices();
			GLuint VBO;
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*vertices.size(), &vertices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glGenVertexArrays(1, &mVAO);
			glBindVertexArray(mVAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			glBindVertexArray(0);

		}
		~SceneUtil() {}

		void show()
		{
			mshaderUtil->useProgram();

			glBindVertexArray(mVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}

	private:
		std::shared_ptr<ShaderUtil> mshaderUtil;

		GLuint mVAO;
	};
}

