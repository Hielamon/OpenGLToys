#version 330 core

layout (location = 0) out vec4 FragColor;

const float PI = 3.1415926535897932384626433832795;

in vec3 Normal;
uniform samplerCube cube_map1;

void main()
{	
	vec3 normal = normalize(Normal);

	//vec2 texCoord = vec2((FragPos.x + 1.0f)*0.5f, (FragPos.y + 1.0f)*0.5f);
	//FragColor = texture(material.diffuse_maps, texCoord);
	FragColor = texture(cube_map1, normal);
	//FragColor = vec4(normal*0.5f + vec3(0.5f), 1.0f);
}