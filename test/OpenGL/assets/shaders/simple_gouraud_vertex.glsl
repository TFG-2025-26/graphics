#version 330 core

layout (location = 0) in vec3 aPos;  // input vertex in local coordinates
layout (location = 3) in vec3 aNormal;  // input vertex in local coordinates

// Light components
const vec4 ambient = vec4(.25, .25, .25, 1);
const vec4 diffuse = vec4(.6, .6, .6, 1);
const vec4 specular = vec4(0, 0.2, 0, 1);

// Exponent of the specular component
const int specularExponent = 32;

uniform vec4 lightDir;   // light direction (must be normalized)
uniform vec4 color;      // uniform color for all vertices
uniform mat4 modelView;  // model-view matrix
uniform mat4 projection; // projection

out vec4 Color;

void main()
{
	gl_Position = projection * modelView * vec4(aPos, 1.0);

	vec3 normal = (modelView * vec4(aNormal, 0.0)).xyz; // we assume uniform scale
	vec3 FragPos = vec3(modelView * vec4(aPos, 1.0));

	// Diffuse component
	float diff_factor = max(dot(normal, - lightDir.xyz), 0.0);

	// Specular component (with Phong reflection coefficient)
	vec3 viewDir = normalize(- FragPos); // view direction (from the fragment to the camera)
	vec3 reflectDir = reflect(lightDir.xyz, normal);   // reflected light direction

	float spec_factor = pow(max(dot(viewDir, reflectDir), 0.0), specularExponent);

	// All together
	Color = color * (ambient + diff_factor * diffuse + spec_factor * specular);
}
