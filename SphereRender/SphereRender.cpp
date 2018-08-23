#include <SPhoenix/core.h>
#include <SPhoenix/Manipulator.h>
#include <SPhoenix/BBoxMesh.h>
//#include <assimp/>

using namespace SP;

int main(int argc, char *argv[])
{
	std::shared_ptr<Scene> pScene = std::make_shared<Scene>();
	
	//std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\GLRelatedCodes\\Freedom3D\\LearnOpenGL\\LoadModel\\nanosuit\\nanosuit.obj";
	//std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\nanosuit\\nanosuit.obj";
	std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\4nlnidkes4g0-C\\The-City\\The-City.obj";
	{
		SceneAssimpLoader loader(fileFullPath, pScene);
		if (!loader.IsLoadSuccess())
		{
			SP_CERR("The scene (" + fileFullPath + ") is failed to load");
			exit(-1);
		}
	}

	/*std::vector<BBox> vBBox = pScene->getAllMeshBBoxes();
	for (size_t i = 0; i < 2; i++)
	{
		std::shared_ptr<BBoxMesh> pBBoxMesh = std::make_shared<BBoxMesh>(
			vBBox[i], glm::vec3(1.0f, 0.0f, float(i + 1) / vBBox.size()));
		pScene->addMesh(pBBoxMesh);
	}*/

	/*{
		std::vector<glm::vec3> vertices(4), normals(4);
		std::vector<glm::vec2> texcoords(4);
		std::vector<GLuint> indices;
		{

			vertices[0] = glm::vec3(1.0f, 1.0f, -1.0f);
			vertices[1] = glm::vec3(1.0f, -1.0f, -1.0f);
			vertices[2] = glm::vec3(-1.0f, -1.0f, -1.0f);
			vertices[3] = glm::vec3(-1.0f, 1.0f, -1.0f);

			normals[0] = glm::vec3(0.0f, 0.0f, 1.0f);
			normals[1] = glm::vec3(0.0f, 0.0f, 1.0f);
			normals[2] = glm::vec3(0.0f, 0.0f, 1.0f);
			normals[3] = glm::vec3(0.0f, 0.0f, 1.0f);

			texcoords[0] = glm::vec2(1.0f, 1.0f);
			texcoords[1] = glm::vec2(1.0f, 0.0f);
			texcoords[2] = glm::vec2(0.0f, 0.0f);
			texcoords[3] = glm::vec2(0.0f, 1.0f);

			indices =
			{
				0, 2, 1,
				0, 3, 2
			};
		}
		std::vector<glm::mat4> mmatrixs(1, glm::mat4());
		std::shared_ptr<VertexArrayNTc> pVANTc = std::make_shared<VertexArrayNTc>(vertices, normals, texcoords, indices);
		
		std::shared_ptr<VertexArray> pVA = std::static_pointer_cast<VertexArray>(pVANTc);
		std::shared_ptr<Texture> pTex = std::make_shared<Texture>("./awesomeface.png", TextureType::Tex_DIFFUSE);
		std::shared_ptr<Material> pMatrial = std::make_shared<Material>();
		pMatrial->addTexture(pTex);
		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA, pMatrial, mmatrixs);
		pScene->addMesh(pMesh);
	}*/

	Camera cam(1280, 720, "SPhoenixTest");
	//cam.setViewMatrix(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.9f)); // translate it down so it's at the center of the scene
	//model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
	//pScene->setTopModelMatrix(model);
	cam.setScene(pScene);

	std::shared_ptr<Manipulator> pManip = std::make_shared<Manipulator>(&cam);
	cam.setManipulator(std::static_pointer_cast<ManipulatorBase>(pManip));
	cam.run();
	return 0;
}