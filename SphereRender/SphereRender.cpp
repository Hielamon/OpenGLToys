#include <SPhoenix/core.h>
#include <SPhoenix/Manipulator.h>
#include <SPhoenix/BBoxMesh.h>
#include <SPhoenix/AxisMesh.h>
#include <SPhoenix/SceneAssimpLoader.h>

using namespace SP;


int main(int argc, char *argv[])
{
	std::shared_ptr<Scene> pScene = std::make_shared<Scene>();
	//..\..\3DModels\Drone0001\Drone166.lws
	//std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\GLRelatedCodes\\Freedom3D\\LearnOpenGL\\LoadModel\\nanosuit\\nanosuit.obj";
	std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\nanosuit\\nanosuit.obj";
	//std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\Hall0003\\hall\\hall.obj";

	if (argc == 2)
	{
		fileFullPath = argv[1];
	}

	bool showModel = !false;
	if (showModel)
	{
		SceneAssimpLoader loader;

		if (!loader.loadScene(fileFullPath, pScene))
		{
			SP_CERR("The scene (" + fileFullPath + ") is failed to load");
			exit(-1);
		}

		/*fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\Drone0001\\Drone166.lws";

		if (!loader.loadScene(fileFullPath, pScene))
		{
			SP_CERR("The scene (" + fileFullPath + ") is failed to load");
			exit(-1);
		}*/
	}


	/*std::vector<BBox> vBBox = pScene->getAllMeshBBoxes();
	for (size_t i = 0; i < 2; i++)
	{
		std::shared_ptr<BBoxMesh> pBBoxMesh = std::make_shared<BBoxMesh>(
			vBBox[i], glm::vec3(1.0f, 0.0f, float(i + 1) / vBBox.size()));
		pScene->addMesh(pBBoxMesh);
	}*/

	if (!showModel)
	{
		std::vector<glm::vec3> vertices(4), normals(4);
		std::vector<glm::vec4> colors(4);
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

			colors[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			colors[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			colors[2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			colors[3] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

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
		std::shared_ptr<VertexArray> pVA = std::make_shared<VertexArray>(vertices, indices);
		//std::shared_ptr<VertexArrayNTc> pVANTc = std::make_shared<VertexArrayNTc>(vertices, normals, texcoords, indices);
		//std::shared_ptr<VertexArrayC> pVANTc = std::make_shared<VertexArrayC>(vertices, colors, indices);
		//pVA->setColors(colors);
		pVA->setTexCoords(texcoords);

		pVA->addInstance();
		
		std::shared_ptr<Texture> pTex = std::make_shared<Texture>("./awesomeface.png", TextureType::Tex_DIFFUSE);
		std::shared_ptr<Material> pMatrial = std::make_shared<Material>();
		pMatrial->addTexture(pTex);
		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA, pMatrial);
		pScene->addMesh(pMesh);
	}

	CameraFBO cam(1080, 720, "SPhoenixTest");
	//cam.setViewMatrix(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.9f)); // translate it down so it's at the center of the scene
	//model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
	//pScene->setTopModelMatrix(model);
	HL_INTERVAL_START
	cam.setScene(pScene);
	HL_INTERVAL_ENDSTR("cam.setScene(pScene)");

	std::shared_ptr<Manipulator> pManip = std::make_shared<Manipulator>(&cam);
	cam.setManipulator(std::static_pointer_cast<ManipulatorBase>(pManip));

	/*glm::mat4 axisM = glm::inverse(pScene->getTopModelMatrix());
	std::shared_ptr<AxisMesh> axismesh = std::make_shared<AxisMesh>(5.0f, axisM);
	cam.addMeshToScene(axismesh);*/

	cam.run();
	return 0;
}