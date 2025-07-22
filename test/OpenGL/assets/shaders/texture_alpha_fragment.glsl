#version 330 core

in vec4 vertexColor;  // color assigned to this vertex
in vec2 texCoord;

uniform sampler2D ourTexture;  // chosen texture
uniform bool modulate;  // whether to modulate with the vertex color

out vec4 FragColor;  // fixed-name color variable used by OpenGL

void main()
{
	if (modulate)
		FragColor = texture(ourTexture, texCoord) * vertexColor;
	else
		FragColor = texture(ourTexture, texCoord);

	if (FragColor.a  == 0.0)  // alpha test (only difference with texture_fragment.glsl)
		discard;
}
