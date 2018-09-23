#version 330 core

//#define MMATRIX_ATTR 3, this macro must be added
//#define NORMAL_ATTR 1, if HAVE_NORMAL, this is necessary
//#define TEXCOORD_ATTR 2, if HAVE_TEXCOORD and HAVE_TEXTURE, this is necessary
//#define COLOR_ATTR 2, if HAVE_COLOR but not HAVE_TEXCOORD, this is necessary

layout (location = 0) in vec3 aVertice;

#if defined(HAVE_COLOR)
layout (location = COLOR_ATTR) in vec4 aColor;
#endif

layout (location = MMATRIX_ATTR) in mat4 aModelMatrix;

layout (std140) uniform ViewUBO
{
	mat4 projMatrix; // 16 * 4
	mat4 viewMatrix; // 16 * 4
	vec3 viewPos;    // 16
};

//uniform mat4 topMMatrix;
#if defined(HAVE_COLOR)
out vec4 VertexColor;
#endif

void main()
{
	vec4 worldPos = aModelMatrix * vec4(aVertice, 1.0f);
	//vec4 worldPos = topMMatrix * aModelMatrix * vec4(aVertice, 1.0f);
	gl_Position = /*projMatrix * viewMatrix **/ worldPos;

#if defined(HAVE_COLOR)
	VertexColor = aColor;
#endif
}

