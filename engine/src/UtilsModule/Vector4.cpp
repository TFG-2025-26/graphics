#include "Vector4.h"
#include "FluxError.h"

#include <cmath>

flux_utils::Vector4::Vector4()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 0.0f;
}

flux_utils::Vector4::Vector4(float X, float Y, float Z, float W)
{
    //if (color) {
    //    throwFluxError(,"Parametro invalido para un color");
    //}
    x = X;
    y = Y;
    z = Z;
    w = W;
}

float flux_utils::Vector4::getX() const
{
    return x;
}

float flux_utils::Vector4::getY() const
{
    return y;
}

float flux_utils::Vector4::getZ() const
{
    return z;
}

float flux_utils::Vector4::getW() const
{
    return w;
}

void flux_utils::Vector4::setX(float X)
{
    x = X;
}

void flux_utils::Vector4::setY(float Y)
{
    y = Y;
}

void flux_utils::Vector4::setZ(float Z)
{
    z = Z;
}

void flux_utils::Vector4::setW(float W)
{
    w = W;
}

void flux_utils::Vector4::setVector(float X, float Y, float Z, float W)
{
    x = X;
    y = Y;
    z = Z;
    w = W;
}

float flux_utils::Vector4::magnitude()
{
    return sqrtf(x * x + y * y + z * z + w * w);
}

flux_utils::Vector4 flux_utils::Vector4::normalized()
{
    float m = magnitude();
    return Vector4(x / m, y / m, z / m, w / m);
}

flux_utils::Vector4 flux_utils::Vector4::operator*(const float n) const
{
    return Vector4(x * n, y * n, z * n, w * n);
}

float flux_utils::Vector4::operator*(const Vector4 other) const
{
    return (x * other.x + y * other.y + z * other.z + w * other.w);
}

flux_utils::Vector4 flux_utils::Vector4::operator+(const Vector4 other) const
{
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

flux_utils::Vector4 flux_utils::Vector4::operator-(const Vector4 other) const
{
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

bool flux_utils::Vector4::operator==(const Vector4 other) const
{
    return (x == other.x && y == other.y && z == other.z && w == other.w);
}