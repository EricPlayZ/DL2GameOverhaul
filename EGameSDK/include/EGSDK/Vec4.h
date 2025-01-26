#pragma once
#include <EGSDK\Exports.h>

namespace EGSDK {
	struct EGameSDK_API Vec4 {
		float X;
		float Y;
		float Z;
		float W;

		Vec4();
		Vec4(float x, float y, float z, float w);

		bool operator==(const Vec4& v) const;
		Vec4& operator+=(const Vec4& v);
		Vec4& operator-=(const Vec4& v);
		Vec4 operator+(const Vec4& v) const;
		Vec4 operator-(const Vec4& v) const;
		Vec4 operator*(const Vec4& scalar) const;
		Vec4 operator/(const Vec4& scalar) const;
		Vec4 operator*(float scalar) const;
		Vec4 operator/(float scalar) const;

		Vec4 normalize() const;
		Vec4 round();
		Vec4 round(int decimals);

		bool isDefault() const;
	};
}