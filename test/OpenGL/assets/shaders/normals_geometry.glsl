#version 330 core

// Based on https://learnopengl.com/Advanced-OpenGL/Geometry-Shader

layout (triangles) in;  // takes triangle primitives (like GL_TRIANGLES)
layout (line_strip, max_vertices = 6) out;  // procues line primitives

uniform mat4 projection; // projection

in vec3 Normal[];

const float MAGNITUDE = 10.0;

void GenerateLine(int index)
{
	// Generate the two vertices of the normal segment

	gl_Position = projection * gl_in[index].gl_Position;
	EmitVertex();

	gl_Position = projection * (gl_in[index].gl_Position
			+ vec4(Normal[index], 0.0) * MAGNITUDE);
	EmitVertex();

	EndPrimitive();
}

void main()
{
	// Normals for each vertex
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
}
