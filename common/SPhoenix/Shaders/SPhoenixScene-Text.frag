#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

struct Material
{
	sampler2DArray diffuse_maps;
};

uniform Material material;
uniform uint uCharID;
uniform vec4 uCharColor;

void main()
{	
	vec3 texcoord3D = vec3(TexCoord, float(uCharID));
	vec4 charMask = vec4(1.0f, 1.0f, 1.0f, texture(material.diffuse_maps, texcoord3D).r);

	FragColor = uCharColor * charMask;
	//FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}