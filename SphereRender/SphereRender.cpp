#include <SPhoenix/core.h>
#include <SPhoenix/Manipulator.h>
#include <cmath>
//#include <assimp/>

using namespace SP;

#define SLICE_Y 4000
#define SLICE_Y_2 2000
#define SLICE_X 8000


void createTexturedSphere(std::vector<GLfloat> &vertices,
						  //std::vector<GLfloat> &normals,
						  std::vector<GLfloat> &colors,
						  const std::string &imgName)
{
	int width, height, channels;
	unsigned char* image = SOIL_load_image(imgName.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

	int numTmp = SLICE_Y * SLICE_X * 2 * 3;
	float rpp = M_PI / SLICE_Y;
	std::vector<GLfloat> verticesTmp(numTmp), /*normalsTmp(numTmp),*/ colorsTmp(numTmp);
	for (int i = 0; i < SLICE_Y; i++)
	{
		for (int j = 0; j < SLICE_X; j++)
		{
			float dcoord_x =  j;
			float dcoord_y = SLICE_Y_2 - i;

			float theta = dcoord_x * rpp;
			float phi = dcoord_y * rpp;

			int idx = SLICE_X * i + j;
			idx *= 3;

			// Vector in 3D space
			verticesTmp[idx] = cos(phi)*sin(theta);
			verticesTmp[idx + 1] = sin(phi);
			verticesTmp[idx + 2] = cos(phi)*cos(theta);

			/*normalsTmp[idx] = verticesTmp[idx];
			normalsTmp[idx + 1] = verticesTmp[idx + 1];
			normalsTmp[idx + 2] = verticesTmp[idx + 2];*/

			/*colorsTmp[idx] = verticesTmp[idx];
			colorsTmp[idx + 1] = verticesTmp[idx + 1];
			colorsTmp[idx + 2] = verticesTmp[idx + 2];*/

			colorsTmp[idx] = image[idx] / 255.0f;
			colorsTmp[idx + 1] = image[idx + 1] / 255.0f;
			colorsTmp[idx + 2] = image[idx + 2] / 255.0f;
		}
	}

	numTmp = (SLICE_Y - 1) * SLICE_X * 6 * 3;
	vertices.resize(numTmp);
	//normals.resize(numTmp);
	colors.resize(numTmp);
	
	for (int i = 0; i < (SLICE_Y - 1); i++)
	{
		for (int j = 0; j < SLICE_X; j++)
		{
			int idx = (SLICE_X * i + j) * 3 * 6;
			int idx_orgin = (SLICE_X * i + j) * 3;
			int idx_right = ((j + 1) % SLICE_X + SLICE_X * i) * 3;
			int idx_down = (j + SLICE_X * (i + 1)) * 3;
			int j_tmp = j == 0 ? SLICE_X - 1 : j - 1;
			int idx_ltdn = (j_tmp + SLICE_X * (i + 1)) * 3;

			int idx_array[6] = { idx_orgin, idx_right, idx_down, idx_orgin, idx_down, idx_ltdn };

			for (size_t k = 0; k < 6; k++)
			{
				int &idx_pt = idx_array[k];
				int idx_tmp = idx + 3 * k;
				vertices[idx_tmp] = verticesTmp[idx_pt];
				vertices[idx_tmp + 1] = verticesTmp[idx_pt + 1];
				vertices[idx_tmp + 2] = verticesTmp[idx_pt + 2];

				/*normals[idx_tmp] = normalsTmp[idx_pt];
				normals[idx_tmp + 1] = normalsTmp[idx_pt + 1];
				normals[idx_tmp + 2] = normalsTmp[idx_pt + 2];*/

				colors[idx_tmp] = colorsTmp[idx_pt];
				colors[idx_tmp + 1] = colorsTmp[idx_pt + 1];
				colors[idx_tmp + 2] = colorsTmp[idx_pt + 2];
			}
		}
	}
}


int main(int argc, char *argv[])
{
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> colors;


	ShaderCodes shaderCodes("SphereRender.vert", "SphereRender.frag");

	Scene scene(shaderCodes);
	//scene.addGeometry(triangle);

	createTexturedSphere(vertices, /*normals,*/ colors, "result.jpg");
	Geometry sphere(vertices, colors);

	scene.addGeometry(sphere);
	

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
	scene.setModelMatrix(translate);

	Camera cam(1280, 720, "SphereRender");

	Manipulator manip;
	cam.setKeyCallback(manip.getKeyCallBack(&cam));
	cam.setScrollCallback(manip.getScrollCallBack(&cam));

	cam.addScene(scene);
	cam.run();
	

	return 0;
}