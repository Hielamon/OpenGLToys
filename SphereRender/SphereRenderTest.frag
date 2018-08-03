#version 330 core

out vec4 color;

in vec4 lightedColor;

void main()
{
	color = lightedColor * vec4(1.0f, 0.0f, 0.0f, 1.0f);
}