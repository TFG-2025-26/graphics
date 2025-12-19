#pragma once

#ifndef VECTOR_4_H_
#define VECTOR_4_H_

#include "defs.h"
#include "Vector3.h"

namespace flux_utils {
    class Vector4
    {
    private:
        float x;
        float y;
        float z;
        float w;

    public:
        FLUX_API Vector4();
        FLUX_API Vector4(float X, float Y, float Z, float W);

        FLUX_API float getX() const;
        FLUX_API float getY() const;
        FLUX_API float getZ() const;
        FLUX_API float getW() const;

        FLUX_API void setX(float X);
        FLUX_API void setY(float Y);
        FLUX_API void setZ(float Z);
        FLUX_API void setW(float W);

        FLUX_API void setVector(float X, float Y, float Z, float W);

        FLUX_API float magnitude();
        FLUX_API Vector4 normalized();

        FLUX_API Vector4 operator*(const float n) const;
        FLUX_API float operator*(const Vector4 other) const;
        FLUX_API Vector4 operator+(const Vector4 other) const;
        FLUX_API Vector4 operator-(const Vector4 other) const;
        FLUX_API bool operator==(const Vector4 other) const;

        // En Vector4.h, dentro de class Vector4:
        FLUX_API Vector4 conjugate() const {
            return Vector4(-x, -y, -z, w);
        }
        FLUX_API float normSquared() const {
            return x * x + y * y + z * z + w * w;
        }
        FLUX_API Vector4 inverse() const {
            auto conj = conjugate();
            float n2 = normSquared();
            return Vector4(conj.x / n2, conj.y / n2, conj.z / n2, conj.w / n2);
        }
        // cuaternión × cuaternión
        FLUX_API Vector4 operator*(const Vector4& q) const {
            return Vector4(
                w * q.x + x * q.w + y * q.z - z * q.y,
                w * q.y - x * q.z + y * q.w + z * q.x,
                w * q.z + x * q.y - y * q.x + z * q.w,
                w * q.w - x * q.x - y * q.y - z * q.z
            );
        }
        FLUX_API Vector4 quatMul(const Vector4& rhs) const {
            return Vector4(
                w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
                w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
                w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
                w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
            );
        }
        // rota un Vector3 v aplicando este cuaternión: q·(v,0)·q⁻¹
        FLUX_API Vector3 rotate(const Vector3& v) const {
            Vector4 p(v.getX(), v.getY(), v.getZ(), 0.0f);
            Vector4 r = this->quatMul(p).quatMul(this->inverse());
            return Vector3(r.getX(), r.getY(), r.getZ());
        }


    };
}

#endif // VECTOR_4_H_
