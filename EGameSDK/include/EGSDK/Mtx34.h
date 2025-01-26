#pragma once
#include <EGSDK\Vec3.h>
#include <EGSDK\Exports.h>

namespace EGSDK {
	struct EGameSDK_API Mtx34 {
		Vec3 XAxis;
		Vec3 YAxis;
		Vec3 ZAxis;
		Vec3 Position;

        Mtx34();
        Mtx34(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis, const Vec3& position);

		bool operator==(const Mtx34& m) const;
		Mtx34& operator+=(const Mtx34& m);
		Mtx34& operator-=(const Mtx34& m);
		Mtx34 operator+(const Mtx34& m) const;
		Mtx34 operator-(const Mtx34& m) const;
		Mtx34 operator*(const Mtx34& scalar) const;
		Mtx34 operator/(const Mtx34& scalar) const;
		Mtx34 operator*(const Vec3& scalar) const;
		Mtx34 operator/(const Vec3& scalar) const;

        Vec3 TransformPosition(const Vec3& v) const;
        void Normalize();

        Mtx34 TransposeRotation() const;
	};
}