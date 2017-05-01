#include <SPhoenix/core.h>
#include <SPhoenix/Manipulator.h>
//#include <assimp/>

using namespace SP;

bool LoadObjFile(const std::string &obj_filename, std::vector<GLfloat> &vertices,
				 std::vector<GLfloat> &normals)
{
	std::ifstream OBJStream(obj_filename, std::ios::in);
	if (!OBJStream.is_open())
	{
		SP_CERR("Failed to open the OBJ file: " + obj_filename);
		return false;
	}

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_texcoords;
	std::string texture_name;
	std::vector< glm::vec3 > temp_normals;

	std::string line_header, Line;
	bool start_face = false;
	while (OBJStream >> line_header)
	{
		if (line_header == "v")
		{
			glm::vec3 vertex;
			OBJStream >> vertex[0] >> vertex[1] >> vertex[2];
			temp_vertices.push_back(vertex);
		}
		else if (line_header == "vt")
		{
			glm::vec2 texcoord;
			OBJStream >> texcoord[0] >> texcoord[1];
			temp_texcoords.push_back(texcoord);
		}
		else if (line_header == "vn")
		{
			glm::vec3 normal;
			OBJStream >> normal[0] >> normal[1] >> normal[2];
			temp_normals.push_back(normal);
		}
		else if (line_header == "usemtl")
		{
			OBJStream >> texture_name;
		}
		else if (line_header == "s")
		{
			if (start_face == true)
			{
				SP_CERR("the flag 's' in a OBJ file shouldn't occur more than onece");
				return false;
			}
			start_face = true;
		}
		else if (line_header == "f")
		{
			if (start_face == false)
			{
				SP_CERR("the 's' flag need before start the face indices");
				return false;
			}
			for (size_t i = 0; i < 3; i++)
			{
				std::string indices_str, temp_str;
				OBJStream >> indices_str;
				for (size_t j = 0, p = 0; j < indices_str.size(); j++)
				{
					if (indices_str[j] == '\/')
					{
						if (!temp_str.empty())
						{
							switch (p)
							{
							case 0:vertexIndices.push_back(atoi(temp_str.c_str()) - 1); break;
							case 1:uvIndices.push_back(atoi(temp_str.c_str()) - 1); break;
							case 2:normalIndices.push_back(atoi(temp_str.c_str()) - 1); break;
							default:
								break;
							}
							temp_str.clear();
						}
						p++;
					}
					else
					{
						temp_str += indices_str[j];
					}
				}
				if (!temp_str.empty())
				{
					normalIndices.push_back(atoi(temp_str.c_str()) - 1);
				}
			}
		}
		else
		{
			std::getline(OBJStream, Line);
		}


	}

	int npoint = vertexIndices.size();
	if (npoint == 0 || npoint % 3 != 0)
	{
		SP_CERR("The OBJ file have no point indices");
		return false;
	}
	if (uvIndices.size() != 0 && uvIndices.size() != npoint)
	{
		SP_CERR("The OBJ file with unequal uvIndices.size() != npoint");
		return false;
	}
	if (normalIndices.size() != 0 && normalIndices.size() != npoint)
	{
		SP_CERR("The OBJ file with unequal normalIndices.size() != npoint");
		return false;
	}

	std::vector< GLfloat> texcoords;
	for (auto vertex_index : vertexIndices)
	{
		if (vertex_index < 0 || vertex_index >= temp_vertices.size())
		{
			SP_CERR("The vertex_index is out the valid range");
			return false;
		}
		vertices.push_back(temp_vertices[vertex_index][0]);
		vertices.push_back(temp_vertices[vertex_index][1]);
		vertices.push_back(temp_vertices[vertex_index][2]);
	}

	if (uvIndices.size() != 0 && !texture_name.empty())
	{

		for (auto texcoord_index : uvIndices)
		{
			if (texcoord_index < 0 || texcoord_index >= temp_texcoords.size())
			{
				SP_CERR("The texcoord_index is out the valid range");
				return false;
			}
			texcoords.push_back(temp_texcoords[texcoord_index][0]);
			texcoords.push_back(temp_texcoords[texcoord_index][1]);
		}
	}

	if (normalIndices.size() != 0)
	{

		for (auto normal_index : normalIndices)
		{
			if (normal_index < 0 || normal_index >= temp_normals.size())
			{
				SP_CERR("The normal_index is out the valid range");
				return false;
			}
			normals.push_back(temp_normals[normal_index][0]);
			normals.push_back(temp_normals[normal_index][1]);
			normals.push_back(temp_normals[normal_index][2]);
		}
	}

	OBJStream.close();

	return true;
}


int main(int argc, char *argv[])
{
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	glm::vec4 color(1.0f, 0.5f, 0.6f, 1.0f);

	LoadObjFile("wt_teapot.obj", vertices, normals);

	ShaderCodes shaderCodes("SphereRender.vert", "SphereRender.frag");
	Geometry geometry(vertices, normals, color);
	vertices = {
		-1.5f, -1.5f, 0.0f,
		1.5f, -1.5f, 0.0f,
		1.5f,  1.5f, 0.0f,

		-1.5f, -1.5f, 0.0f,
		1.5f,  1.5f, 0.0f,
		-1.5f,  1.5f, 0.0f
	};

	normals =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	color = glm::vec4(0.0f, 0.5f, 0.6f, 1.0f);
	Geometry triangle(vertices, normals, color);

	Scene scene(shaderCodes);
	scene.addGeometry(geometry);
	scene.addGeometry(triangle);
	

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
	scene.setModelMatrix(translate);

	Camera cam(1280, 720, "SphereRender");

	Manipulator manip;
	cam.setKeyCallback(manip.getKeyCallBack(&cam));
	cam.setScrollCallback(manip.getScrollCallBack(&cam));

	cam.addScene(scene);
	cam.run();
	

	return 0;
}