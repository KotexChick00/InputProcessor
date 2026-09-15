#pragma once

class Vector2 {
public:
	Vector2() : x(0.0f), y(0.0f) {}
	Vector2(float x, float y) : x(x), y(y) {}
	float GetX() const { return x; }
	float GetY() const { return y; }

	bool IsTheSame(const Vector2& other) const;
	bool IsTheSame(const Vector2& left, const Vector2& right) const;
	bool NotTheSame(const Vector2& other) const;
	bool NotTheSame(const Vector2& left, const Vector2& right) const;

	Vector2 operator+(const Vector2& other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator-(const Vector2& other) const {
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 Add(const Vector2& other) const;
	Vector2 Add(const Vector2& left, const Vector2& right) const;
	Vector2 Subtract(const Vector2& other) const;
	Vector2 Subtract(const Vector2& left, const Vector2& right) const;
	Vector2 ScalarMultiply(const float scalar, const Vector2& thisVector) const;
	float Dot(const Vector2& other) const;
	float Dot(const Vector2& left, const Vector2& right) const;
	float Magnitude() const;
	float Magnitude(const Vector2& vector) const;
	Vector2 Normalize() const;
	Vector2 Normalize(const Vector2& vector) const;

private:
	float x, y;
};