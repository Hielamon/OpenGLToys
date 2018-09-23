#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;

#if defined(HAVE_COLOR)
in vec4 VertexColor;
#endif


//#if (defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)) || !defined(HAVE_COLOR)
struct Material
{
#if !defined(HAVE_COLOR)
	vec4 uAmbient;
	vec4 uDiffuse;
	vec4 uSpecular;
#endif

	float uShininess;
};

uniform Material material;
//#endif
uniform uint uMeshID;

void main()
{	
	vec3 result = vec3(0.0f, 0.0f, 0.0f);

	float ambientStrength = 0.2f;
	float diffuseStrength = 0.8f;
	float specularStrength = 0.0f;

	vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
	vec3 ambient = diffuse;
	vec3 specular = vec3(0.0f, 0.0f, 0.0f);

#if !defined(HAVE_COLOR)
	diffuse = vec3(material.uDiffuse);
	ambient = vec3(material.uAmbient);
	specular = vec3(material.uSpecular);
#else
//Use the vertex color
	diffuse = vec3(VertexColor);
	ambient = diffuse;
	specular = diffuse;
#endif

	result = (ambientStrength * ambient + diffuseStrength * diffuse);
	result += specularStrength * specular;
	//result = vec3(1.0f, 1.0f, 1.0f);
	FragColor = vec4(result/*.z, result.y, result.x*/, 1.0f);
	MeshID = uMeshID;
}