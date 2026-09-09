#include <PrimitiveType/Vector2.h>

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