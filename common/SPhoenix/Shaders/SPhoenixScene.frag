#version 330 core

out vec4 FragColor;

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
in vec2 TexCoord;
#else
in vec3 ObjectColor;
#endif

#if defined(HAVE_NORMAL)
in vec3 Normal;
#endif

struct Light
{
	vec3 position;
	vec3 lightColor;
};

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
struct Material
{
#if defined(AMBIENT_TEXTURE)
	sampler2DArray ambient_maps;
#endif
#if defined(DIFFUSE_TEXTURE)
	sampler2DArray diffuse_maps;
#endif
#if defined(SPECULAR_TEXTURE)
	sampler2DArray specular_maps;
#endif
};
#endif

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
uniform Material material;
#endif
uniform Light light;

in vec3 FragPos;
in vec3 ViewPos;

void main()
{	
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 result = vec3(0.0f, 0.0f, 0.0f);
#if defined(HAVE_NORMAL)
	float ambientStrength = 0.2f;

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(ViewPos - FragPos);
	//vec3 lightDir = normalize(light.position - FragPos);
	float diffuseStrength = max(dot(normal, lightDir), 0.0f);
	//float diffuseStrength = 0.0f;

	//Blinn-Phong specular
	vec3 viewDir = normalize(ViewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float shininess = 32;
	float specularStrength = pow(max(dot(halfwayDir, normal), 0.0f), shininess);
#else
	float ambientStrength = 0.2f;
	float diffuseStrength = 0.8f;
	float specularStrength = 0.0f;
#endif

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
	vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
	vec3 ambient = diffuse;
	vec3 specular = vec3(0.0f, 0.0f, 0.0f);

#if defined(DIFFUSE_TEXTURE)
	diffuse = vec3(1.0f, 1.0f, 1.0f);
	ivec3 diffuse_size = textureSize(material.diffuse_maps, 0);
	int diffuse_layers = diffuse_size.z;
	for(int i = 0; i < diffuse_layers; i++)
	{
		diffuse *= vec3(texture(material.diffuse_maps, vec3(TexCoord, float(i))));
	}
	ambient = diffuse;
#endif

#if defined(AMBIENT_TEXTURE)
	ambient = vec3(1.0f, 1.0f, 1.0f);
	ivec3 ambient_size = textureSize(material.ambient_maps, 0);
	int ambient_layers = ambient_size.z;
	for(int i = 0; i < ambient_layers; i++)
	{
		ambient *= vec3(texture(material.ambient_maps, vec3(TexCoord, float(i))));
	}
#endif

#if defined(SPECULAR_TEXTURE)
	specular = vec3(1.0f, 1.0f, 1.0f);
	ivec3 specular_size = textureSize(material.specular_maps, 0);
	int specular_layers = specular_size.z;
	for(int i = 0; i < specular_layers; i++)
	{
		specular *= vec3(texture(material.specular_maps, vec3(TexCoord, float(i))));
	}
#endif
	result = (ambientStrength * ambient + diffuseStrength * diffuse) * lightColor;
	result += specularStrength * specular * lightColor;
#else
	result = (ambientStrength + diffuseStrength + specularStrength) * ObjectColor * lightColor;
#endif
	//result = vec3(1.0f, 1.0f, 1.0f);
	FragColor = vec4(result, 1.0f);
}