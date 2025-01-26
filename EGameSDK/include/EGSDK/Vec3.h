#pragma once
#include <EGSDK\Exports.h>

namespace EGSDK {
	struct EGameSDK_API Vec3 {
		float X;
		float Y;
		float Z;

		Vec3();
		Vec3(float x, float y, float z);

		bool operator==(const Vec3& v) const;
		Vec3& operator+=(const Vec3& v);
		Vec3& operator-=(const Vec3& v);
		Vec3 operator+(const Vec3& v) const;
		Vec3 operator-(const Vec3& v) const;
		Vec3 operator*(const Vec3& scalar) const;
		Vec3 operator/(const Vec3& scalar) const;
		Vec3 operator*(float scalar) const;
		Vec3 operator/(float scalar) const;

		Vec3 normalize() const;
		Vec3 cross(const Vec3& v) const;
		float dot(const Vec3& v) const;
		Vec3 round();
		Vec3 round(int decimals);

		bool isDefault() const;
	};
}