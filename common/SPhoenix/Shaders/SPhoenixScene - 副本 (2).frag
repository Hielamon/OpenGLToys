#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
in vec2 TexCoord;
#elif defined(HAVE_COLOR)
in vec4 VertexColor;
#endif

#if defined(HAVE_NORMAL)
in vec3 Normal;
#endif

struct Light
{
	vec3 position;
	vec3 lightColor;
};

//#if (defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)) || !defined(HAVE_COLOR)
struct Material
{
#if (defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)) || !defined(HAVE_COLOR)

#if defined(AMBIENT_TEXTURE) && defined(HAVE_TEXCOORD)
	sampler2DArray ambient_maps;
#elif !defined(DIFFUSE_TEXTURE)
	vec4 uAmbient;
#endif

#if defined(DIFFUSE_TEXTURE) && defined(HAVE_TEXCOORD)
	sampler2DArray diffuse_maps;
#else
	vec4 uDiffuse;
#endif

#if defined(SPECULAR_TEXTURE) && defined(HAVE_TEXCOORD)
	sampler2DArray specular_maps;
#else
	vec4 uSpecular;
#endif

#endif

	float uShininess;
};

uniform Material material;
//#endif

uniform Light light;
uniform uint uMeshID;

in vec3 FragPos;
in vec3 ViewPos;

void main()
{	
	vec4 lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
#if defined(HAVE_NORMAL)
	float ambientStrength = 0.2f;

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(ViewPos - FragPos);
	//vec3 lightDir = normalize(light.position - FragPos);
	float diffuseStrength = max(dot(normal, lightDir), 0.0f);

	//Blinn-Phong specular
	vec3 viewDir = normalize(ViewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specularStrength = pow(max(dot(halfwayDir, normal), 0.0f), material.uShininess);
#else
	float ambientStrength = 0.2f;
	float diffuseStrength = 0.8f;
	float specularStrength = 0.0f;
#endif

	vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 ambient = diffuse;
	vec4 specular = vec4(0.0f, 0.0f, 0.0f, 1.0f);

#if (defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)) || !defined(HAVE_COLOR)

#if defined(DIFFUSE_TEXTURE) && defined(HAVE_TEXCOORD)
	diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	ivec3 diffuse_size = textureSize(material.diffuse_maps, 0);
	int diffuse_layers = diffuse_size.z;
	for(int i = 0; i < diffuse_layers; i++)
	{
		diffuse *= /*vec3*/(texture(material.diffuse_maps, vec3(TexCoord, float(i))));
		//diffuse *= vec3(1.0f, 0.0f, 0.0f);
	}
	ambient = diffuse;
#else
	diffuse = /*vec3*/(material.uDiffuse);
#endif

#if defined(AMBIENT_TEXTURE) && defined(HAVE_TEXCOORD)
	ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	ivec3 ambient_size = textureSize(material.ambient_maps, 0);
	int ambient_layers = ambient_size.z;
	for(int i = 0; i < ambient_layers; i++)
	{
		ambient *= /*vec3*/(texture(material.ambient_maps, vec3(TexCoord, float(i))));
	}
#elif !defined(DIFFUSE_TEXTURE) || !defined(HAVE_TEXCOORD)
	ambient = /*vec3*/(material.uAmbient);
#endif

#if defined(SPECULAR_TEXTURE) && defined(HAVE_TEXCOORD)
	specular = /*vec3*/(1.0f, 1.0f, 1.0f, 1.0f);
	ivec3 specular_size = textureSize(material.specular_maps, 0);
	int specular_layers = specular_size.z;
	for(int i = 0; i < specular_layers; i++)
	{
		specular *= /*vec3*/(texture(material.specular_maps, vec3(TexCoord, float(i))));
	}
#else
	specular = /*vec3*/(material.uSpecular);
#endif

#else
//Use the vertex color
	diffuse = /*vec3*/(VertexColor);
	ambient = diffuse;
	//specular = diffuse;
#endif

	result = (ambientStrength * ambient + diffuseStrength * diffuse) * lightColor;
	result += specularStrength * specular * lightColor;
	//result = vec3(1.0f, 1.0f, 1.0f);
	FragColor = result;
	//FragColor = vec4(result/*.z, result.y, result.x*/, 1.0f);
	MeshID = uMeshID;
}