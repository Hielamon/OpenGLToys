#include <SPhoenix/Core.h>
#include <SPhoenix/Manipulator.h>
#include <SPhoenix/BBoxMesh.h>
#include <SPhoenix/AxisMesh.h>
#include <SPhoenix/SceneAssimpLoader.h>
#include <SPhoenix/Sphere.h>
#include <thread>

using namespace SP;

void TestThread1(std::string sceneFullPath, std::string skyboxFolder)
{
	//std::shared_ptr<FasterScene> pFasterScene = std::make_shared<FasterScene>();
	//std::shared_ptr<Scene> pScene = std::static_pointer_cast<Scene>(pFasterScene);
	std::shared_ptr<Scene> pScene = std::make_shared<Scene>();

	bool showModel = !false;
	if (showModel)
	{
		SceneAssimpLoader loader;

		if (!loader.loadScene(sceneFullPath, pScene))
		{
			SP_CERR("The scene (" + sceneFullPath + ") is failed to load");
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

			vertices[0] = glm::vec3(1.0f, 1.0f, -0.0f);
			vertices[1] = glm::vec3(1.0f, -1.0f, -0.0f);
			vertices[2] = glm::vec3(-1.0f, -1.0f, -0.0f);
			vertices[3] = glm::vec3(-1.0f, 1.0f, -0.0f);

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

	if (0)
	{
		std::shared_ptr<Sphere> pSphere =
			std::make_shared<Sphere>(1, glm::vec3(1.0f, 0.0f, 0.0f));

		pScene->addMesh(pSphere);
	}

	int width = 1440, height = 900;
	std::shared_ptr<MonitorWindow> monitor = 
		std::make_shared<MonitorWindow>("TestThread1", width, height);

	if (skyboxFolder != "")
	{
		std::shared_ptr<Scene> pSkyBoxScene = createSkyBoxScene(skyboxFolder);
		monitor->setSkyBoxScene(pSkyBoxScene);
	}

	HL_INTERVAL_START
		monitor->setScene(pScene);
		//monitor->setScene(pSkyBoxScene);
	HL_INTERVAL_ENDSTR("monitor.setScene(pScene)");

	

	std::shared_ptr<MonitorManipulator> pMonitorManip =
		std::make_shared<MonitorManipulator>(monitor);
	monitor->setManipulator(std::static_pointer_cast<ManipulatorBase>(pMonitorManip));
	
	int minWidth = width * 0.25, minHeight = height * 0.25;
	std::shared_ptr<CameraMini> pCamera =
		std::make_shared<CameraMini>(minWidth, minHeight, width - minWidth, 0);
									
	std::shared_ptr<Camera> pDefaultCamera = monitor->getDefaultCamera();
	glm::vec3 eye, center, up;
	pDefaultCamera->getCameraPose(eye, center, up);
	glm::vec3 offsetDir = glm::normalize(center - eye);
	float fovy, aspect, zNear, zFar;
	pDefaultCamera->getFrustum(fovy, aspect, zNear, zFar);
	float zNearAspect = 10.0f;
	eye += offsetDir * zNear * zNearAspect;
	eye -= up * 0.3f * zNear * zNearAspect;
	center -= up * 0.3f * zNear * zNearAspect;
	//eye = -eye;
	pCamera->setViewMatrix(eye, center, up);
	monitor->addCamera(pCamera);
	/*glm::mat4 axisM = glm::inverse(pScene->getTopModelMatrix());
	std::shared_ptr<AxisMesh> axismesh = std::make_shared<AxisMesh>(5.0f, axisM);
	cam.addMeshToScene(axismesh);*/

	monitor->run();
}

int main(int argc, char *argv[])
{
	//..\..\3DModels\Drone0001\Drone166.lws
	//std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\nanosuit\\nanosuit.obj";
	std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\Drone0003\\drone.obj";
	//std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\Hall0003\\hall\\hall.obj";
	//std::string fileFullPath = "D:\\Funny-Works\\PlayWorks\\OpenGL\\3DModels\\citydetail-obj\\citydetail.obj";
	std::string skyboxFolder = "";

	if (argc == 2)
	{
		fileFullPath = argv[1];
	}
	else if(argc > 2)
	{
		skyboxFolder = argv[2];
	}

	TestThread1(fileFullPath, skyboxFolder);
	return 0;
}