#pragma once
#include <EGSDK\Exports.h>

namespace EGSDK {
	struct EGameSDK_API Vector3 {
		float X;
		float Y;
		float Z;

		Vector3();
		Vector3(float x, float y, float z);

		bool operator==(const Vector3& v) const;
		Vector3& operator+=(const Vector3& v);
		Vector3& operator-=(const Vector3& v);
		Vector3 operator+(const Vector3& v) const;
		Vector3 operator-(const Vector3& v) const;
		Vector3 operator*(float scalar) const;
		Vector3 operator/(float scalar) const;

		Vector3 normalize();
		Vector3 cross(const Vector3& v) const;
		Vector3 round();
		Vector3 round(int decimals);

		bool isDefault() const;
	};
}