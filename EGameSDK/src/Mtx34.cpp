#include <EGSDK\Mtx34.h>

namespace EGSDK {
    Mtx34::Mtx34() : XAxis(1.0f, 0.0f, 0.0f), YAxis(0.0f, 1.0f, 0.0f), ZAxis(0.0f, 0.0f, 1.0f), Position(0.0f, 0.0f, 0.0f) {}
    Mtx34::Mtx34(const Vec3& XAxis, const Vec3& YAxis, const Vec3& ZAxis, const Vec3& Pos) : XAxis(XAxis), YAxis(YAxis), ZAxis(ZAxis), Position(Pos) {}

	bool Mtx34::operator==(const Mtx34& m) const {
		return XAxis == m.XAxis && YAxis == m.YAxis && ZAxis == m.ZAxis && Position == m.Position;
	}
	Mtx34& Mtx34::operator+=(const Mtx34& m) {
		XAxis += m.XAxis;
		YAxis += m.YAxis;
		ZAxis += m.ZAxis;
		Position += m.Position;
		return *this;
	}
	Mtx34& Mtx34::operator-=(const Mtx34& m) {
		XAxis -= m.XAxis;
		YAxis -= m.YAxis;
		ZAxis -= m.ZAxis;
		Position -= m.Position;
		return *this;
	}
	Mtx34 Mtx34::operator+(const Mtx34& m) const {
		return { XAxis + m.XAxis, YAxis + m.YAxis, ZAxis + m.ZAxis, Position + m.Position };
	}
	Mtx34 Mtx34::operator-(const Mtx34& m) const {
		return { XAxis - m.XAxis, YAxis - m.YAxis, ZAxis - m.ZAxis, Position - m.Position };
	}
	Mtx34 Mtx34::operator*(const Mtx34& scalar) const {
		return { XAxis * scalar.XAxis, YAxis * scalar.YAxis, ZAxis * scalar.ZAxis, Position * scalar.Position };
	}
	Mtx34 Mtx34::operator/(const Mtx34& scalar) const {
		return { XAxis / scalar.XAxis, YAxis / scalar.YAxis, ZAxis / scalar.ZAxis, Position / scalar.Position };
	}
	Mtx34 Mtx34::operator*(const Vec3& scalar) const {
		return { XAxis * scalar, YAxis * scalar, ZAxis * scalar, Position * scalar };
	}
	Mtx34 Mtx34::operator/(const Vec3& scalar) const {
		return { XAxis / scalar, YAxis / scalar, ZAxis / scalar, Position / scalar };
	}

    Vec3 Mtx34::TransformPosition(const Vec3& v) const {
        return {
            XAxis.X * v.X + YAxis.X * v.Y + ZAxis.X * v.Z + Position.X,
            XAxis.Y * v.X + YAxis.Y * v.Y + ZAxis.Y * v.Z + Position.Y,
            XAxis.Z * v.X + YAxis.Z * v.Y + ZAxis.Z * v.Z + Position.Z
        };
    }
    void Mtx34::Normalize() {
        XAxis = XAxis.normalize();
        YAxis = YAxis.normalize();
        ZAxis = ZAxis.normalize();
    }

    Mtx34 Mtx34::TransposeRotation() const {
        return Mtx34(
            Vec3(XAxis.X, YAxis.X, ZAxis.X),
            Vec3(XAxis.Y, YAxis.Y, ZAxis.Y),
            Vec3(XAxis.Z, YAxis.Z, ZAxis.Z),
            Position
        );
    }
}