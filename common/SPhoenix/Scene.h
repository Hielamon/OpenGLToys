#pragma once

#include "Shader.h"
#include "Geometry.h"
#include <assimp\scene.h>

namespace SP
{
	class Scene
	{
	public:
		Scene(ShaderCodes &shaderCodes)
		{
			setShaderCodes(shaderCodes);
		}
		Scene() = delete;
		~Scene() {}

		void addGeometry(Geometry &geometry)
		{
			mvpGeometry.push_back(std::make_shared<Geometry>(geometry));
		}

		void setShaderCodes(ShaderCodes &shaderCodes)
		{
			mpShaderCodes = std::make_shared<ShaderCodes>(shaderCodes);
		}

	public:
		glm::mat4 mModelMatrix;

	protected:
		std::shared_ptr<ShaderCodes> mpShaderCodes;
		std::vector<std::shared_ptr<Geometry>> mvpGeometry;
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
			glUniformMatrix4fv(mmodelLoc, 1, GL_FALSE, glm::value_ptr(mModelMatrix));

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

