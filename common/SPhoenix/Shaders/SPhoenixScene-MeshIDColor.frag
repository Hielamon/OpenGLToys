#version 330 core

out vec4 FragColor;

uniform vec3 uIDColor;

#if defined(HAVE_NORMAL)
in vec3 Normal;
#endif

in vec3 FragPos;
in vec3 ViewPos;

void main()
{
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 result = vec3(0.0f, 0.0f, 0.0f);
#if defined(HAVE_NORMAL)
	float ambientStrength = 0.2f;

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(ViewPos - FragPos);
	//vec3 lightDir = normalize(light.position - FragPos);
	float diffuseStrength = max(dot(normal, lightDir), 0.0f);
	//float diffuseStrength = 0.0f;

	//Blinn-Phong specular
	vec3 viewDir = normalize(ViewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float shininess = 32;
	float specularStrength = pow(max(dot(halfwayDir, normal), 0.0f), shininess);
#else
	float ambientStrength = 0.2f;
	float diffuseStrength = 0.8f;
	float specularStrength = 0.0f;
#endif	

	FragColor = vec4((ambientStrength + diffuseStrength + specularStrength)*uIDColor*lightColor, 1.0f);
	//FragColor = vec4(uIDColor, 1.0f);
}