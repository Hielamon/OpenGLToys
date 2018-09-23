#version 330 core
layout (location = 0) in vec3 aVertice;

layout (std140) uniform ViewUBO
{
	mat4 projMatrix; // 16 * 4
	mat4 viewMatrix; // 16 * 4
	vec3 viewPos;    // 16
};

out vec3 TexCoord3D;

void main()
{
	TexCoord3D = aVertice;
	vec3 point = mat3(viewMatrix) * aVertice;
	vec4 position = projMatrix * vec4(point, 1.0f);
	gl_Position = vec4(position.x, position.y, position.w, position.w);
	//gl_Position = projMatrix * viewMatrix * vec4(aVertice, 1.0f);
}

