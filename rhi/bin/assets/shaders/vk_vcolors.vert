// vk_vcolors.vert
#version 450

// Entrada de vértices (equivalente a VSInput)
layout(location = 0) in vec3 inPos;   // POSITION
layout(location = 1) in vec4 inCol;   // COLOR

// Salida hacia el fragment shader (equivalente a PSInput.col)
layout(location = 0) out vec4 vCol;

// UBO con la matriz MVP (equivalente a cbuffer MVP : register(b0))
layout(set = 0, binding = 0) uniform MVP
{
    mat4 modelViewProj;
};

void main()
{
    gl_Position = modelViewProj * vec4(inPos, 1.0);
    vCol = inCol;
}