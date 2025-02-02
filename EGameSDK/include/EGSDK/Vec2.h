#pragma once
#include <EGSDK\Exports.h>

namespace EGSDK {
	struct EGameSDK_API Vec2 {
		float X;
		float Y;

		Vec2();
		Vec2(float x, float y);

		bool operator==(const Vec2& v) const;
		Vec2& operator+=(const Vec2& v);
		Vec2& operator-=(const Vec2& v);
		Vec2 operator+(const Vec2& v) const;
		Vec2 operator-(const Vec2& v) const;
		Vec2 operator*(const Vec2& scalar) const;
		Vec2 operator/(const Vec2& scalar) const;
		Vec2 operator*(float scalar) const;
		Vec2 operator/(float scalar) const;

		Vec2 operator-() const;

		Vec2 normalize() const;
		float dot(const Vec2& v) const;
		Vec2 round();
		Vec2 round(int decimals);

		bool isDefault() const;
	};
}