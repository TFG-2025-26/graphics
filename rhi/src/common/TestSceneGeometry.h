#ifndef TEST_SCENE_GEOMETRY_H_
#define TEST_SCENE_GEOMETRY_H_

#pragma once
#include <cstdint>

struct TestVertex {
	float position[3];
	float color[4];
};

namespace TestSceneGeometry {

	// Array de vértices GPU
	extern const TestVertex gVertices[];

	// Offsets y counts para saber qué dibujar
	constexpr std::uint32_t kTriangleVertexCount = 3;
	constexpr std::uint32_t kAxesVertexCount = 6;

	constexpr std::uint32_t kTriangleStart = 0;
	constexpr std::uint32_t kAxesStart = kTriangleStart + kTriangleVertexCount;

	constexpr std::uint32_t kTotalVertexCount = kTriangleVertexCount + kAxesVertexCount;
}


#endif // TEST_SCENE_GEOMETRY_H_