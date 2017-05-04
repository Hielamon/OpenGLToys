#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normalVec;

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

	float diff =min(1.0f, max(dot(norm, lightDir), 0.0f));
	float ambient = 0.2;
	vec3 result = indicatedColor.xyz * (diff + ambient);
	lightedColor = vec4(result, 1.0f);
}