#pragma once
#include <EGSDK\Vec3.h>
#include <EGSDK\Vec4.h>
#include <EGSDK\Exports.h>

namespace EGSDK {
	struct EGameSDK_API alignas(16) Mtx34 {
		Vec4 Row1;
		Vec4 Row2;
		Vec4 Row3;

        Mtx34();
        Mtx34(const Vec4& row1, const Vec4& row2, const Vec4& row3);

		bool operator==(const Mtx34& m) const;
		Mtx34& operator+=(const Mtx34& m);
		Mtx34& operator-=(const Mtx34& m);
		Mtx34 operator+(const Mtx34& m) const;
		Mtx34 operator-(const Mtx34& m) const;
		Mtx34 operator*(const Mtx34& scalar) const;
		Mtx34 operator/(const Mtx34& scalar) const;
		Mtx34 operator*(const Vec4& scalar) const;
		Mtx34 operator/(const Vec4& scalar) const;

		Vec3 GetXAxis() const;
		Vec3 GetYAxis() const;
		Vec3 GetZAxis() const;
		Vec3 GetPosition() const;

		Mtx34 normalize();
	};
}