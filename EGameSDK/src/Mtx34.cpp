#include <EGSDK\Mtx34.h>

namespace EGSDK {
    Mtx34::Mtx34() : Row1(1.0f, 0.0f, 0.0f, 0.0f), Row2(0.0f, 1.0f, 0.0f, 0.0f), Row3(0.0f, 0.0f, 1.0f, 0.0f) {}
    Mtx34::Mtx34(const Vec4& row1, const Vec4& row2, const Vec4& row3) : Row1(row1), Row2(row2), Row3(row3) {}

	bool Mtx34::operator==(const Mtx34& m) const {
		return Row1 == m.Row1 && Row2 == m.Row2 && Row3 == m.Row3;
	}
	Mtx34& Mtx34::operator+=(const Mtx34& m) {
		Row1 += m.Row1;
		Row2 += m.Row2;
		Row3 += m.Row3;
		return *this;
	}
	Mtx34& Mtx34::operator-=(const Mtx34& m) {
		Row1 -= m.Row1;
		Row2 -= m.Row2;
		Row3 -= m.Row3;
		return *this;
	}
	Mtx34 Mtx34::operator+(const Mtx34& m) const {
		return { Row1 + m.Row1, Row2 + m.Row2, Row3 + m.Row3 };
	}
	Mtx34 Mtx34::operator-(const Mtx34& m) const {
		return { Row1 - m.Row1, Row2 - m.Row2, Row3 - m.Row3 };
	}
	Mtx34 Mtx34::operator*(const Mtx34& scalar) const {
		return { Row1 * scalar.Row1, Row2 * scalar.Row2, Row3 * scalar.Row3 };
	}
	Mtx34 Mtx34::operator/(const Mtx34& scalar) const {
		return { Row1 / scalar.Row1, Row2 / scalar.Row2, Row3 / scalar.Row3 };
	}
	Mtx34 Mtx34::operator*(const Vec4& scalar) const {
		return { Row1 * scalar, Row2 * scalar, Row3 * scalar };
	}
	Mtx34 Mtx34::operator/(const Vec4& scalar) const {
		return { Row1 / scalar, Row2 / scalar, Row3 / scalar };
	}

	Vec3 Mtx34::GetXAxis() const {
		return Vec3(Row1.X, Row2.X, Row3.X);
	}
	Vec3 Mtx34::GetYAxis() const {
		return Vec3(Row1.Y, Row2.Y, Row3.Y);
	}
	Vec3 Mtx34::GetZAxis() const {
		return Vec3(Row1.Z, Row2.Z, Row3.Z);
	}
	Vec3 Mtx34::GetPosition() const {
		return Vec3(Row1.W, Row2.W, Row3.W);
	}

	Mtx34 Mtx34::normalize() {
		return { Row1.normalize(), Row2.normalize(), Row3.normalize() };
    }
}