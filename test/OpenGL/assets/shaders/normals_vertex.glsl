#version 330 core

layout (location = 0) in vec3 aPos;  // input vertex in local coordinates
layout (location = 3) in vec3 aNormal;  // input vertex in local coordinates

uniform mat4 modelView;  // model-view matrix

out vec3 Normal;  // normal vector in view coordinates

void main()
{
	gl_Position = modelView * vec4(aPos, 1.0);
	Normal = (modelView * vec4(aNormal, 0.0)).xyz;

	// In the general setting, normal vectors should be applied
	// a normal matrix to put them into the view space. However,
	// our lighting shaders assume the scale of the model-view
	// matrix is uniform and normal vectors are unitary.

	// mat3 normalMatrix = mat3(transpose(inverse(modelView)));
	// Normal = normalize(normalMatrix * Normal);
}
