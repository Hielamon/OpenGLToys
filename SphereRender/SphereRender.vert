#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;

uniform mat4 model;

out vec4 lightedColor;
uniform vec4 indicatedColor;


void main()
{
	vec4 mpos = model * vec4(position, 1.0f);
	gl_Position = projection * view * mpos;
	mat3 R = mat3(view);
	vec3 T = view[3].xyz;
	R = transpose(R);
	vec3 lightPos = - (R * T);
	vec3 lightDir = normalize(lightPos - mpos.xyz);
	vec3 norm = normalize(normalVec);

	//float diff =abs(dot(norm, lightDir));
	float diff =max(dot(norm, lightDir), 0.0f);
	float ambient = 0.3;
	vec3 result = indicatedColor.xyz * min(diff + ambient, 1.0f);
	lightedColor = vec4(result, 1.0f);
}