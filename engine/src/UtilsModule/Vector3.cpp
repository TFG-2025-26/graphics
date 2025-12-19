#include "Vector3.h"

#include <cmath>

flux_utils::Vector3::Vector3()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

flux_utils::Vector3::Vector3(float X, float Y, float Z)
{
    x = X;
    y = Y;
    z = Z;
}

float flux_utils::Vector3::getX() const
{
    return x;
}

float flux_utils::Vector3::getY() const
{
    return y;
}

float flux_utils::Vector3::getZ() const
{
    return z;
}

void flux_utils::Vector3::setX(float X)
{
    x = X;
}

void flux_utils::Vector3::setY(float Y)
{
    y = Y;
}

void flux_utils::Vector3::setZ(float Z)
{
    z = Z;
}

void flux_utils::Vector3::setVector(float X, float Y, float Z)
{
    x = X;
    y = Y;
    z = Z;
}

float flux_utils::Vector3::magnitude()
{
    return sqrtf(x * x + y * y + z * z);
}

flux_utils::Vector3 flux_utils::Vector3::normalized()
{
    float m = magnitude();
    return Vector3(x / m, y / m, z / m);
}

flux_utils::Vector3 flux_utils::Vector3::cross(const Vector3 other)
{
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

flux_utils::Vector3 flux_utils::Vector3::operator*(const float n) const
{
    return Vector3(x * n, y * n, z * n);
}

float flux_utils::Vector3::operator*(const Vector3 other) const
{
    return (x * other.x + y * other.y + z * other.z);
}

flux_utils::Vector3 flux_utils::Vector3::operator+(const Vector3 other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}
flux_utils::Vector3 flux_utils::Vector3::operator+=(const Vector3 other) {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

flux_utils::Vector3 flux_utils::Vector3::operator-=(const Vector3 other) {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

flux_utils::Vector3 flux_utils::Vector3::operator-(const Vector3 other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

std::ostream& flux_utils::operator<<(std::ostream& os, const Vector3& vector)
{
    os << "(" << vector.getX() << ", " << vector.getY() << ", " << vector.getZ() << ")";
    return os;
}
