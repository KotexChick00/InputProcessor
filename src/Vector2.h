#pragma once

class Vector2 {
public:
	Vector2(float x, float y) : x(x), y(y) {}
	bool IsTheSame(const Vector2& other) const;
	bool IsTheSame(const Vector2& left, const Vector2& right) const;
	bool NotTheSame(const Vector2& other) const;
	bool NotTheSame(const Vector2& left, const Vector2& right) const;

private:
	float x, y;
};