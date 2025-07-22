#version 330 core

in vec4 vertexColor;  // color assigned to this vertex
out vec4 FragColor;  // fixed-name color variable used by OpenGL

void main()
{
	FragColor = vertexColor;
}
