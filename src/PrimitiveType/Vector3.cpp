#include <PrimitiveType/Vector3.h>
#include <cmath>

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

Vector3 Vector3::Add(const Vector3& other) const {
	return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::Add(const Vector3& left, const Vector3& right) const {
	return left.Add(right);
}

Vector3 Vector3::Subtract(const Vector3& other) const {
	return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::Subtract(const Vector3& left, const Vector3& right) const {
	return left.Subtract(right);
}

Vector3 Vector3::ScalarMultiply(const float scalar, const Vector3& thisVector) const {
	return Vector3(scalar * thisVector.x, scalar * thisVector.y, scalar * thisVector.z);
}

float Vector3::Dot(const Vector3& other) const {
	return x * other.x + y * other.y + z * other.z;
}

float Vector3::Dot(const Vector3& left, const Vector3& right) const {
	return left.Dot(right);
}

float Vector3::Magnitude() const {
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	return std::sqrt(x2 + y2 + z2);
}

float Vector3::Magnitude(const Vector3& vector) const {
	return vector.Magnitude();
}


Vector3 Vector3::Cross(const Vector3& other) const {
	return Vector3(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

Vector3 Vector3::Cross(const Vector3& left, const Vector3& right) const {
	return left.Cross(right);
}

Vector3 Vector3::Normalize() const {
	float mag = Magnitude();
	if (mag == 0.0f) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	return Vector3(x / mag, y / mag, z / mag);
}

Vector3 Vector3::Normalize(const Vector3& vector) const {
	return vector.Normalize();
}
