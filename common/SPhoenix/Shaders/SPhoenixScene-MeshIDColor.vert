#version 330 core

//#define MMATRIX_ATTR 3, this macro must be added

layout (location = 0) in vec3 aVertice;

#if defined(HAVE_NORMAL)
layout (location = NORMAL_ATTR) in vec3 aNormal;
#endif

layout (location = MMATRIX_ATTR) in mat4 aModelMatrix;

layout (std140) uniform ViewUBO
{
	mat4 projMatrix; // 16 * 4
	mat4 viewMatrix; // 16 * 4
	vec3 viewPos;    // 16
};

uniform mat4 topMMatrix;

#if defined(HAVE_NORMAL)
out vec3 Normal;
#endif

out vec3 FragPos;
out vec3 ViewPos;

void main()
{
	vec4 worldPos = topMMatrix * aModelMatrix * vec4(aVertice, 1.0f);
	gl_Position = projMatrix * viewMatrix * worldPos;

#if defined(HAVE_NORMAL)
	Normal = mat3(transpose(inverse(aModelMatrix)))*aNormal;
#endif

	FragPos = vec3(worldPos);
	ViewPos = viewPos;
}

