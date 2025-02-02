#define _USE_MATH_DEFINES
#include <cmath>
#include <EGSDK\Vec2.h>
#include <EGSDK\Utils\Values.h>

namespace EGSDK {
	Vec2::Vec2() : X(0.0f), Y(0.0f) {}
	Vec2::Vec2(float x, float y) : X(x), Y(y) {}

	bool Vec2::operator==(const Vec2& v) const {
		return Utils::Values::are_samef(X, v.X) && Utils::Values::are_samef(Y, v.Y);
	}
	Vec2& Vec2::operator+=(const Vec2& v) {
		X += v.X;
		Y += v.Y;
		return *this;
	}
	Vec2& Vec2::operator-=(const Vec2& v) {
		X -= v.X;
		Y -= v.Y;
		return *this;
	}
	Vec2 Vec2::operator+(const Vec2& v) const {
		return { X + v.X, Y + v.Y };
	}
	Vec2 Vec2::operator-(const Vec2& v) const {
		return { X - v.X, Y - v.Y };
	}
	Vec2 Vec2::operator*(const Vec2& scalar) const {
		return { X * scalar.X, Y * scalar.Y };
	}
	Vec2 Vec2::operator/(const Vec2& scalar) const {
		return { X / scalar.X, Y / scalar.Y };
	}
	Vec2 Vec2::operator*(float scalar) const {
		return { X * scalar, Y * scalar };
	}
	Vec2 Vec2::operator/(float scalar) const {
		return { X / scalar, Y / scalar };
	}

	Vec2 Vec2::operator-() const {
		return { -X, -Y };
	}

	Vec2 Vec2::normalize() const {
		if (isDefault())
			return *this;

		float length = std::sqrt(X * X + Y * Y);
		return { X / length, Y / length };
	}
	float Vec2::dot(const Vec2& v) const {
		return (X * v.X) + (Y * v.Y);
	}
	Vec2 Vec2::round() {
		return { std::roundf(X), std::roundf(Y) };
	}
	Vec2 Vec2::round(int decimals) {
		float power = std::powf(10.0f, static_cast<float>(decimals));
		return { std::roundf(X * power) / power, std::roundf(Y * power) / power };
	}

	bool Vec2::isDefault() const {
		return Utils::Values::are_samef(X, 0.0f) && Utils::Values::are_samef(Y, 0.0f);
	}
}