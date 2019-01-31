#version 330 core

layout (location = 0) out vec4 FragColor;

const float PI = 3.1415926535897932384626433832795;


struct Material
{
	sampler2D diffuse_maps;
};

uniform Material material;

in vec2 FragPos;
uniform samplerCube cube_map1;

void main()
{	
	float theta = FragPos.x * PI;
	float phi = (0.5f - FragPos.y * 0.5f) * PI;
	vec3 normal = vec3(sin(phi) * sin(theta), cos(phi), -sin(phi) * cos(theta));
	normal = normalize(normal);

	vec2 texCoord = vec2((FragPos.x + 1.0f)*0.5f, (FragPos.y + 1.0f)*0.5f);
	//FragColor = texture(material.diffuse_maps, texCoord);
	FragColor = texture(cube_map1, normal);
	//FragColor = vec4(normal*0.5f + vec3(0.5f), 1.0f);
}