#version 330 core

//#define MMATRIX_ATTR 3, this macro must be added

layout (location = 0) in vec3 aVertice;
layout (location = 1) in vec3 aNormal;

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
layout (location = 2) in vec2 aTexCoord;
#elif defined(HAVE_COLOR)
layout (location = 2) in vec3 aColor;
#else //defined(UNIFORM_COLOR)
uniform vec3 uColor;
#endif

layout (location = MMATRIX_ATTR) in mat4 aModelMatrix;

layout (std140) uniform ViewUBO
{
	mat4 projMatrix; // 16 * 4
	mat4 viewMatrix; // 16 * 4
	vec3 viewPos;    // 16
};

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
out vec2 TexCoord;
#else
out vec3 ObjectColor;
#endif

out vec3 Normal;
out vec3 FragPos;
out vec3 ViewPos;

void main()
{
	vec4 worldPos = aModelMatrix * vec4(aVertice, 1.0f);
	gl_Position = projMatrix * viewMatrix * worldPos;

	Normal = mat3(transpose(inverse(aModelMatrix)))*aNormal;

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
	TexCoord = aTexCoord;
#elif defined(HAVE_COLOR)
	ObjectColor = aColor;
#else //defined(UNIFORM_COLOR)
	ObjectColor = uColor;
#endif

	FragPos = vec3(worldPos);
	ViewPos = viewPos;
}

