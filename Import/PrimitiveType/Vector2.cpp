#include <PrimitiveType/Vector2.h>
#include <cmath>

bool Vector2::IsTheSame(const Vector2& other) const {
	return x == other.x && y == other.y;
}

bool Vector2::NotTheSame(const Vector2& other) const {
	return !IsTheSame(other);
}

bool Vector2::IsTheSame(const Vector2& left, const Vector2& right) const {
	return left.IsTheSame(right);
}

bool Vector2::NotTheSame(const Vector2& left, const Vector2& right) const {
	return left.NotTheSame(right);
}

Vector2 Vector2::Add(const Vector2& other) const {
	return Vector2(x + other.x, y + other.y);
}

Vector2 Vector2::Add(const Vector2& left, const Vector2& right) const {
	return left.Add(right);
}

Vector2 Vector2::Subtract(const Vector2& other) const {
	return Vector2(x - other.x, y - other.y);
}

Vector2 Vector2::Subtract(const Vector2& left, const Vector2& right) const {
	return left.Subtract(right);
}

Vector2 Vector2::ScalarMultiply(const float scalar, const Vector2& thisVector) const {
	return Vector2(scalar * thisVector.x, scalar * thisVector.y);
}

float Vector2::Dot(const Vector2& other) const {
	return x * other.x + y * other.y;
}

float Vector2::Dot(const Vector2& left, const Vector2& right) const {
	return left.Dot(right);
}

float Vector2::Magnitude() const {
	return std::sqrt(x * x + y * y);
}

float Vector2::Magnitude(const Vector2& vector) const {
	return vector.Magnitude();
}

Vector2 Vector2::Normalize() const {
	float mag = Magnitude();
	if (mag == 0.0f) {
		return Vector2(0.0f, 0.0f);
	}
	return Vector2(x / mag, y / mag);
}


Vector2 Vector2::Normalize(const Vector2& vector) const {
	return vector.Normalize();
}