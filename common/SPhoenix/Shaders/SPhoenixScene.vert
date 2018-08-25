#version 330 core

//#define MMATRIX_ATTR 3, this macro must be added
//#define NORMAL_ATTR 1, if HAVE_NORMAL, this is necessary
//#define TEXCOORD_ATTR 2, if HAVE_TEXCOORD and HAVE_TEXTURE, this is necessary
//#define COLOR_ATTR 2, if HAVE_COLOR but not HAVE_TEXCOORD, this is necessary

layout (location = 0) in vec3 aVertice;

#if defined(HAVE_NORMAL)
layout (location = NORMAL_ATTR) in vec3 aNormal;
#endif

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
layout (location = TEXCOORD_ATTR) in vec2 aTexCoord;
#elif defined(HAVE_COLOR)
layout (location = COLOR_ATTR) in vec3 aColor;
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

uniform mat4 topMMatrix;

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
out vec2 TexCoord;
#else
out vec3 ObjectColor;
#endif

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
	
#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
	TexCoord = vec2(aTexCoord.x, 1.0f - aTexCoord.y);
#elif defined(HAVE_COLOR)
	ObjectColor = aColor;
#else //defined(UNIFORM_COLOR)
	ObjectColor = uColor;
#endif

	FragPos = vec3(worldPos);
	ViewPos = viewPos;
}

