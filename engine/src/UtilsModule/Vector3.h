#pragma once

#ifndef VECTOR_3_H_
#define VECTOR_3_H_

#include "defs.h"

#include <ostream>

namespace flux_utils {
    class Vector3
    {
    private:
        float x;
        float y;
        float z;
    public:
        FLUX_API Vector3();
        FLUX_API Vector3(float X, float Y, float Z);

        FLUX_API float getX() const;
        FLUX_API float getY() const;
        FLUX_API float getZ() const;

        FLUX_API void setX(float X);
        FLUX_API void setY(float Y);
        FLUX_API void setZ(float Z);

        FLUX_API void setVector(float X, float Y, float Z);

        FLUX_API float magnitude();
        FLUX_API Vector3 normalized();
        FLUX_API Vector3 cross(const Vector3 other);

        FLUX_API Vector3 operator*(const float n) const;
        FLUX_API float operator*(const Vector3 other) const;
        FLUX_API Vector3 operator+(const Vector3 other) const;
        FLUX_API Vector3 operator+=(const Vector3 other);
        FLUX_API Vector3 operator-(const Vector3 other) const;
        FLUX_API Vector3 operator-=(const Vector3 other);

        FLUX_API friend std::ostream& operator<<(std::ostream& os, const Vector3& vector);
    };
}


#endif // VECTOR_3_H_