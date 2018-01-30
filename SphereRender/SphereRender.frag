#version 330 core

out vec4 color;

in vec4 lightedColor;
void main()
{
	color = lightedColor;
}