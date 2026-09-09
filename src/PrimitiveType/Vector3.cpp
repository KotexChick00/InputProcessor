#include <PrimitiveType/Vector3.h>

bool Vector3::IsTheSame(const Vector3& other) const {
	return x == other.x && y == other.y && z == other.z;
}

bool Vector3::NotTheSame(const Vector3& other) const {
	return !IsTheSame(other);
}

bool Vector3::IsTheSame(const Vector3& left, const Vector3& right) const {
	return left.IsTheSame(right);
}

bool Vector3::NotTheSame(const Vector3& left, const Vector3& right) const {
	return left.NotTheSame(right);
}