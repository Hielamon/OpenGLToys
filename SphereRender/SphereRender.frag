#version 330 core

out vec4 FragColor;

in vec4 lightedColor;

void main()
{
	FragColor = lightedColor;
}