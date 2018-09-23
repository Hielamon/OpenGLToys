#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;

in vec3 TexCoord3D;
uniform samplerCube skybox;

void main()
{	
	FragColor = texture(skybox, TexCoord3D);
	//FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}