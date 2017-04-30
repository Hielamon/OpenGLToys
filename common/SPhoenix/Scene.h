#pragma once

#include <SPhoenix/Shader.h>
#include <SPhoenix/Geometry.h>

namespace SP
{
	class Scene
	{
	public:
		Scene(ShaderCodes &shaderCodes, Geometry &geometry)
		{
			mpModelMatrix =std::make_shared<glm::mat4>(1.0f);
			mpShaderCodes = std::make_shared<ShaderCodes>(shaderCodes);
			mpGeometry = std::make_shared<Geometry>(geometry);
		}
		~Scene() {}

		void setModelMatrix(glm::mat4 &modelMatrix)
		{
			*mpModelMatrix = modelMatrix;
		}

	protected:
		Scene(){}

		std::shared_ptr<ShaderCodes> mpShaderCodes;
		std::shared_ptr<Geometry> mpGeometry;
		std::shared_ptr<glm::mat4> mpModelMatrix;
	};

	class SceneUtil : public Scene
	{
	public:
		SceneUtil(Scene &scene)
			: Scene(scene)
		{
			mpShaderUtil = std::make_shared<ShaderUtil>(*mpShaderCodes);
			mpGeometryUtil = std::make_shared<GeometryUtil>(*mpGeometry);
			GLuint programID = getProgramID();
			mmodelLoc = glGetUniformLocation(programID, "model");
		}

		~SceneUtil() {}

		void show()
		{
			glUniformMatrix4fv(mmodelLoc, 1, GL_FALSE, glm::value_ptr(*mpModelMatrix));

			mpShaderUtil->useProgram();
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			mpGeometryUtil->show();
		}

		GLuint getProgramID()
		{
			return mpShaderUtil->getProgramID();
		}

	private:
		std::shared_ptr<ShaderUtil> mpShaderUtil;
		std::shared_ptr<GeometryUtil> mpGeometryUtil;

		GLint mmodelLoc;
	};
}

