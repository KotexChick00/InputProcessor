#include <Vector4.h>

bool Vector4::IsTheSame(const Vector4& other) const {
	return x == other.x && y == other.y && z == other.z && w == other.w;
}

bool Vector4::IsTheSame(const Vector4& left, const Vector4& right) const {
	return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}

bool Vector4::NotTheSame(const Vector4& other) const {
	return !IsTheSame(other);
}

bool Vector4::NotTheSame(const Vector4& left, const Vector4& right) const {
	return !IsTheSame(left, right);
}
