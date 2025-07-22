#version 330 core

uniform vec4 color;  // uniform color for all vertices
out vec4 FragColor;  // fixed-name color variable used by OpenGL

void main()
{
	FragColor = color;
}
