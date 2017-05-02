#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normalVec;
layout (location = 2) in vec3 colorVec;

uniform mat4 projection;
uniform mat4 view;

uniform mat4 model;

out vec4 colorVert;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	colorVert = vec4(colorVec, 1.0f);
}