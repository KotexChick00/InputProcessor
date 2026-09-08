#pragma once

class Vector3 {
public:
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	bool IsTheSame(const Vector3& other) const;
	bool IsTheSame(const Vector3& left, const Vector3& right) const;
	bool NotTheSame(const Vector3& other) const;
	bool NotTheSame(const Vector3& left, const Vector3& right) const;

private:
	float x, y, z;
};