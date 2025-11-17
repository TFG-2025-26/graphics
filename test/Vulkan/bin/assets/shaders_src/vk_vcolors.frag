// vk_vcolors.frag
#version 450

// Entrada desde el VS
layout(location = 0) in vec4 vCol;

// Salida al framebuffer (equivalente a SV_TARGET)
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vCol;
}