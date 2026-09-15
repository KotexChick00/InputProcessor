#pragma once

class Vector3 {
public:
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	float GetX() const { return x; }
	float GetY() const { return y; }
	float GetZ() const { return z; }
	bool IsTheSame(const Vector3& other) const;
	bool IsTheSame(const Vector3& left, const Vector3& right) const;
	bool NotTheSame(const Vector3& other) const;
	bool NotTheSame(const Vector3& left, const Vector3& right) const;

	Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	Vector3 Add(const Vector3& other) const;
	Vector3 Add(const Vector3& left, const Vector3& right) const;
	Vector3 Subtract(const Vector3& other) const;
	Vector3 Subtract(const Vector3& left, const Vector3& right) const;
	Vector3 ScalarMultiply(const float scalar, const Vector3& thisVector) const;
	float Dot(const Vector3& other) const;
	float Dot(const Vector3& left, const Vector3& right) const;
	float Magnitude() const;
	float Magnitude(const Vector3& vector) const;
	Vector3 Cross(const Vector3& other) const;
	Vector3 Cross(const Vector3& left, const Vector3& right) const;
	Vector3 Normalize() const;
	Vector3 Normalize(const Vector3& vector) const;

private:
	float x, y, z;
};