#version 330 core

layout (location = 0) in vec3 aPos;  // input vertex in local coordinates

uniform mat4 modelView;  // model-view matrix
uniform mat4 projection; // projection

void main()
{
	gl_Position = projection * modelView * vec4(aPos, 1.0);
}
