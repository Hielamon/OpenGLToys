#pragma once

#include <SPhoenix/Shader.h>
#include <SPhoenix/Geometry.h>

namespace SP
{
	class SceneUtil;

	class Scene
	{
	public:
		Scene(ShaderCodes &shaderCodes, Geometry &geometry)
			: mshaderCodes(shaderCodes), mgeometry(geometry)
		{
			mmodelMatrix = glm::mat4(1.0f);
		}
		~Scene() {}

		void setModelMatrix(glm::mat4 &modelMatrix)
		{
			mmodelMatrix = modelMatrix;
		}

		friend class SceneUtil;
	private:
		ShaderCodes mshaderCodes;
		Geometry mgeometry;
		glm::mat4 mmodelMatrix;
	};

	class SceneUtil
	{
	public:
		SceneUtil(Scene &scene)
			: mshaderUtil(new ShaderUtil(scene.mshaderCodes)),
			mgeometryUtil(new GeometryUtil(scene.mgeometry)),
			mmodelMatrix(scene.mmodelMatrix)
		{
			GLuint programID = getProgramID();
			mmodelLoc = glGetUniformLocation(programID, "model");
		}

		~SceneUtil() {}

		void show()
		{
			glUniformMatrix4fv(mmodelLoc, 1, GL_FALSE, glm::value_ptr(mmodelMatrix));

			mshaderUtil->useProgram();
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			mgeometryUtil->show();
		}

		GLuint getProgramID()
		{
			return mshaderUtil->getProgramID();
		}

	private:

		std::shared_ptr<ShaderUtil> mshaderUtil;
		std::shared_ptr<GeometryUtil> mgeometryUtil;

		GLint mmodelLoc;
		glm::mat4 mmodelMatrix;
	};
}

