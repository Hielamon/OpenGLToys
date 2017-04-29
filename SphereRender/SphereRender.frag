#version 330 core

out vec4 color;

uniform vec4 indicatedColor;

void main()
{
	color = indicatedColor;
}