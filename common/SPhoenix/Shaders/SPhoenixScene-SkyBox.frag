#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;

in vec3 TexCoord3D;
uniform samplerCube cube_map1;

void main()
{	
	FragColor = texture(cube_map1, TexCoord3D);
	//FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}