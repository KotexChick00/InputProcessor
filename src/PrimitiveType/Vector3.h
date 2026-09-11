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

private:
	float x, y, z;
};