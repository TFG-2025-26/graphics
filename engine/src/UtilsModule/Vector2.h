#pragma once

#ifndef VECTOR_2_H_
#define VECTOR_2_H_

namespace flux_utils {
	class Vector2
	{
	private:
		float x;
		float y;
	public:
		Vector2();
		Vector2(float X, float Y);

		float getX() const;
		float getY() const;

		void setX(float X);
		void setY(float Y);

		void setVector(float X, float Y);

		float magnitude();
		Vector2 normalized();

		Vector2 operator*(const float n) const;
		float operator*(const Vector2 other) const;
		Vector2 operator+(const Vector2 other) const;
		Vector2 operator-(const Vector2 other) const;
	};
}


#endif // VECTOR_2_H_