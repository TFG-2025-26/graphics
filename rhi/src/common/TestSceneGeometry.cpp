#include "TestSceneGeometry.h"

namespace TestSceneGeometry {

    const TestVertex gVertices[] = {
        // Triángulo (cian)
        {{  0.0f,  300.0f,   0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
        {{ 260.0f, -150.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
        {{-260.0f, -150.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }},

        // Eje X (rojo)
        {{  0.0f,   0.0f,   0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }},
        {{400.0f,   0.0f,   0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }},

        // Eje Y (verde)
        {{  0.0f,   0.0f,   0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }},
        {{  0.0f, 400.0f,   0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }},

        // Eje Z (azul)
        {{  0.0f,   0.0f,   0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }},
        {{  0.0f,   0.0f, 400.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }},
    };

    static_assert(
        kTotalVertexCount == sizeof(gVertices) / sizeof(TestVertex),
        "kTtotalVertexCount debe coincidir con el tamaño de gVertices"
    );
}