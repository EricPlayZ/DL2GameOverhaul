#define _USE_MATH_DEFINES
#include <cmath>
#include <EGSDK\Vec3.h>
#include <EGSDK\Utils\Values.h>

namespace EGSDK {
	Vec3::Vec3() : X(0.0f), Y(0.0f), Z(0.0f) {}
	Vec3::Vec3(float x, float y, float z) : X(x), Y(y), Z(z) {}

	bool Vec3::operator==(const Vec3& v) const {
		return Utils::Values::are_samef(X, v.X) && Utils::Values::are_samef(Y, v.Y) && Utils::Values::are_samef(Z, v.Z);
	}
	Vec3& Vec3::operator+=(const Vec3& v) {
		X += v.X;
		Y += v.Y;
		Z += v.Z;
		return *this;
	}
	Vec3& Vec3::operator-=(const Vec3& v) {
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;
		return *this;
	}
	Vec3 Vec3::operator+(const Vec3& v) const {
		return { X + v.X, Y + v.Y, Z + v.Z };
	}
	Vec3 Vec3::operator-(const Vec3& v) const {
		return { X - v.X, Y - v.Y, Z - v.Z };
	}
	Vec3 Vec3::operator*(const Vec3& scalar) const {
		return { X * scalar.X, Y * scalar.Y, Z * scalar.Z };
	}
	Vec3 Vec3::operator/(const Vec3& scalar) const {
		return { X / scalar.X, Y / scalar.Y, Z / scalar.Z };
	}
	Vec3 Vec3::operator*(float scalar) const {
		return { X * scalar, Y * scalar, Z * scalar };
	}
	Vec3 Vec3::operator/(float scalar) const {
		return { X / scalar, Y / scalar, Z / scalar };
	}

	Vec3 Vec3::operator-() const {
		return { -X, -Y, -Z };
	}

	Vec3 Vec3::normalize() const {
		if (isDefault())
			return *this;

		float length = std::sqrt(X * X + Y * Y + Z * Z);
		return { X / length, Y / length, Z / length };
	}
	Vec3 Vec3::cross(const Vec3& v) const {
		return {
			Y * v.Z - Z * v.Y,
			Z * v.X - X * v.Z,
			X * v.Y - Y * v.X
		};
	}
	float Vec3::dot(const Vec3& v) const {
		return (X * v.X) + (Y * v.Y) + (Z * v.Z);
	}
	Vec3 Vec3::round() {
		return { std::roundf(X), std::roundf(Y), std::roundf(Z) };
	}
	Vec3 Vec3::round(int decimals) {
		float power = std::powf(10.0f, static_cast<float>(decimals));
		return { std::roundf(X * power) / power, std::roundf(Y * power) / power, std::roundf(Z * power) / power };
	}

	bool Vec3::isDefault() const {
		return Utils::Values::are_samef(X, 0.0f) && Utils::Values::are_samef(Y, 0.0f) && Utils::Values::are_samef(Z, 0.0f);
	}
}