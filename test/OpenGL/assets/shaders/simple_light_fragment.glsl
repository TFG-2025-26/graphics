#version 330 core

// Light components
const vec4 ambient = vec4(.25, .25, .25, 1);
const vec4 diffuse = vec4(.6, .6, .6, 1);
const vec4 specular = vec4(0, 0.2, 0, 1);

// Exponent of the specular component
const int specularExponent = 32;

uniform vec4 lightDir; // light direction (must be normalized)
uniform vec4 color;    // uniform color for all vertices

in vec3 FragPos;     // fragment position in view coordinates
in vec3 Normal;      // normal vector (must be normalized)
out vec4 FragColor;  // fixed-name color variable used by OpenGL

void main()
{
	// Normalize the normal vector since the interpolation
	// of unitary vectors is not necessarily unitary
	vec3 normal = normalize(Normal);

	// Diffuse component
	float diff_factor = max(dot(normal, - lightDir.xyz), 0.0);

	// Specular component (with Phong reflection coefficient)
	vec3 viewDir = normalize(- FragPos); // view direction (from the fragment to the camera)
	vec3 reflectDir = reflect(lightDir.xyz, normal);   // reflected light direction

	float spec_factor = pow(max(dot(viewDir, reflectDir), 0.0), specularExponent);

	// All together
	FragColor = color * (ambient + diff_factor * diffuse + spec_factor * specular);
}
