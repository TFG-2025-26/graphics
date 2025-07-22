#version 330 core

layout (location = 0) in vec3 aPos;  // input vertex in local coordinates
layout (location = 3) in vec3 aNormal;  // input vertex in local coordinates

uniform mat4 modelView;  // model-view matrix
uniform mat4 projection; // projection

out vec3 FragPos;  // for the fragment shader
out vec3 Normal;

void main()
{
	gl_Position = projection * modelView * vec4(aPos, 1.0);
	FragPos = vec3(modelView * vec4(aPos, 1.0));
	Normal = (modelView * vec4(aNormal, 0.0)).xyz; // we assume uniform scale
}
