#pragma once

#include <SPhoenix/Shader.h>
#include <SPhoenix/Geometry.h>

namespace SP
{
	class Scene
	{
	public:
		Scene(ShaderCodes &shaderCodes)
		{
			mpModelMatrix =std::make_shared<glm::mat4>(1.0f);
			mpShaderCodes = std::make_shared<ShaderCodes>(shaderCodes);
		}
		~Scene() {}

		void addGeometry(Geometry &geometry)
		{
			mvpGeometry.push_back(std::make_shared<Geometry>(geometry));
		}

		void setModelMatrix(glm::mat4 &modelMatrix)
		{
			*mpModelMatrix = modelMatrix;
		}

		glm::mat4 getModelMatrix()
		{
			return *mpModelMatrix;
		}

	protected:
		Scene(){}

		std::shared_ptr<ShaderCodes> mpShaderCodes;
		std::vector<std::shared_ptr<Geometry>> mvpGeometry;
		std::shared_ptr<glm::mat4> mpModelMatrix;
	};

	class SceneUtil : public Scene
	{
	public:
		SceneUtil(Scene &scene)
			: Scene(scene)
		{
			mpShaderUtil = std::make_shared<ShaderUtil>(*mpShaderCodes);

			std::for_each(mvpGeometry.begin(), mvpGeometry.end(),
						  [&](std::shared_ptr<Geometry> &pGeom) {
				mvpGeometryUtil.push_back(std::make_shared<GeometryUtil>(*pGeom));
			}
			);

			GLuint programID = getProgramID();
			mmodelLoc = glGetUniformLocation(programID, "model");
		}

		~SceneUtil() {}

		void show()
		{
			glUniformMatrix4fv(mmodelLoc, 1, GL_FALSE, glm::value_ptr(*mpModelMatrix));

			mpShaderUtil->useProgram();
			std::for_each(mvpGeometryUtil.begin(), mvpGeometryUtil.end(),
						  [](std::shared_ptr<GeometryUtil> &pGeom) {pGeom->show(); }
			);
		}

		GLuint getProgramID()
		{
			return mpShaderUtil->getProgramID();
		}

	private:
		std::shared_ptr<ShaderUtil> mpShaderUtil;
		std::vector<std::shared_ptr<GeometryUtil>> mvpGeometryUtil;

		GLint mmodelLoc;
	};
}

