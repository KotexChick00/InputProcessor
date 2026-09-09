#pragma once
class Vector4
{
public:
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	bool IsTheSame(const Vector4& other) const;
	bool IsTheSame(const Vector4& left, const Vector4& right) const;
	bool NotTheSame(const Vector4& other) const;
	bool NotTheSame(const Vector4& left, const Vector4& right) const;

private:
	float x, y, z, w;
};

