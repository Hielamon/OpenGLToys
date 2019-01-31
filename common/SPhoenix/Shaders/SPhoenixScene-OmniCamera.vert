#version 330 core
layout (location = 0) in vec3 aVertice;

out vec2 FragPos;

void main()
{
	vec4 worldPos = /*aModelMatrix * */vec4(aVertice, 1.0f);
	gl_Position = worldPos;
	FragPos = vec2(aVertice);
}

