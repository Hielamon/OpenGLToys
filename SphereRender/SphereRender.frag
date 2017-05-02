#version 330 core

out vec4 color;

in vec4 colorVert;

void main()
{
	color = colorVert;
}