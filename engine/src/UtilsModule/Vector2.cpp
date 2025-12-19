#include "Vector2.h"

#include <cmath>

flux_utils::Vector2::Vector2()
{
	x = 0.0f;
	y = 0.0f;
}

flux_utils::Vector2::Vector2(float X, float Y)
{
	x = X;
	y = Y;
}

float flux_utils::Vector2::getX() const
{
	return x;
}

float flux_utils::Vector2::getY() const
{
	return y;
}

void flux_utils::Vector2::setX(float X)
{
	x = X;
}

void flux_utils::Vector2::setY(float Y)
{
	y = Y;
}

void flux_utils::Vector2::setVector(float X, float Y)
{
	x = X;
	y = Y;
}

float flux_utils::Vector2::magnitude() {
	return static_cast<float>(sqrt(pow(x, 2) + pow(y, 2)));
}


flux_utils::Vector2 flux_utils::Vector2::normalized()
{
	float m = magnitude();
	return Vector2(x / m, y / m);
}

flux_utils::Vector2 flux_utils::Vector2::operator*(const float n) const
{
	return Vector2(x * n, y * n);
}

float flux_utils::Vector2::operator*(const Vector2 other) const
{
	return (x * other.x + y * other.y);
}

flux_utils::Vector2 flux_utils::Vector2::operator+(const Vector2 other) const
{
	return Vector2(x + other.x, y + other.y);
}

flux_utils::Vector2 flux_utils::Vector2::operator-(const Vector2 other) const
{
	return Vector2(x - other.x, y - other.y);
}