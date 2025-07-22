#version 330 core

// Object material
struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shininess;
};

uniform Material material;  // object material

// Directional light
struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	bool enabled;
};

// Positional light
struct PosLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	bool enabled;
};

// Spotlight
struct SpotLight {
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;

	bool enabled;
};

void calcComponents(vec3 lightDir, vec3 normal, vec3 viewDir,
                    vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular,
                    out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(- lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// combine results
	ambient  = lightAmbient * material.ambient;
	diffuse  = lightDiffuse * diff * material.diffuse;
	specular = lightSpecular * spec * material.specular;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = - light.direction;
	vec3 ambient, diffuse, specular;

	calcComponents(lightDir, normal, viewDir,
	               light.ambient, light.diffuse, light.specular,
	               ambient, diffuse, specular);

	return ambient + diffuse + specular;
}

float calcAttenuation(float dist, float kc, float kl, float kq)
{
	return 1.0 / (kc + kl * dist + kq * (dist * dist));
}

vec3 calcPosLight(PosLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 ambient, diffuse, specular;

	calcComponents(lightDir, normal, viewDir,
	               light.ambient, light.diffuse, light.specular,
	               ambient, diffuse, specular);

	// attenuation
	float attenuation = calcAttenuation(distance(light.position, fragPos),
		light.constant, light.linear, light.quadratic);

	return attenuation * (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 ambient, diffuse, specular;

	calcComponents(lightDir, normal, viewDir,
	               light.ambient, light.diffuse, light.specular,
	               ambient, diffuse, specular);

	// attenuation
	float attenuation = calcAttenuation(distance(light.position, fragPos),
		light.constant, light.linear, light.quadratic);

	// spotlight intensity
	float theta = dot(lightDir, normalize(-light.direction)); 
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	return attenuation * (ambient + intensity * (diffuse + specular));
}


const int NR_DIR_LIGHTS = 2;
const int NR_POS_LIGHTS = 4;
const int NR_SPOT_LIGHTS = 4;

uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform PosLight posLights[NR_POS_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

in vec3 FragPos;     // fragment position in world coordinates
in vec3 Normal;      // normal vector
out vec4 FragColor;  // fixed-name color variable used by OpenGL

void main()
{
	// properties
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(- FragPos);
	vec3 result = vec3(0);

	// phase 1: Directional lighting
	for (int i = 0; i < NR_DIR_LIGHTS; i++)
		if (dirLights[i].enabled)
			result += calcDirLight(dirLights[i], norm, viewDir);

	// phase 2: Point lights
	for (int i = 0; i < NR_POS_LIGHTS; i++)
		if (posLights[i].enabled)
			result += calcPosLight(posLights[i], norm, FragPos, viewDir);

	// phase 3: Spot light
	for (int i = 0; i < NR_SPOT_LIGHTS; i++)
		if (spotLights[i].enabled)
			result += calcSpotLight(spotLights[i], norm, FragPos, viewDir);

	FragColor = vec4(result, 1.0);
}
