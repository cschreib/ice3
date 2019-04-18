#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <lxgui/utils.hpp>
#include <iosfwd>
#include <cmath>
#include <limits>

struct vector3i;

struct vector3f
{
    enum constraint
    {
        CONSTRAINT_NONE,
        CONSTRAINT_X,
        CONSTRAINT_Y,
        CONSTRAINT_Z,
        CONSTRAINT_XY,
        CONSTRAINT_YZ,
        CONSTRAINT_ZX
    };

    vector3f() {}

    vector3f(float fX, float fY, float fZ) : x(fX), y(fY), z(fZ) {}

    explicit vector3f(const vector3i& v);

    explicit vector3f(float fValue) : x(fValue), y(fValue), z(fValue) {}

    bool is_null() const {
        static const float EPSILON = std::numeric_limits<float>::epsilon();
        return fabs(x) < EPSILON && fabs(y) < EPSILON && fabs(z) < EPSILON;
    }

    float get_norm() const {
        return sqrt(x*x + y*y + z*z);
    }

    float get_length() const {
        return sqrt(x*x + y*y + z*z);
    }

    float get_norm_squared() const {
        return x*x + y*y + z*z;
    }

    float get_length_squared() const {
        return x*x + y*y + z*z;
    }

    vector3f get_unit() const {
        float fNorm = sqrt(x*x + y*y + z*z);
        return vector3f(x/fNorm, y/fNorm, z/fNorm);
    }

    void normalize() {
        float fNorm = sqrt(x*x + y*y + z*z);
        x /= fNorm; y /= fNorm; z /= fNorm;
    }

    void rotate(float fAngle, const vector3f& mAxis);

    void scale_up(const vector3f& mScale) {
        x *= mScale.x; y *= mScale.y; z *= mScale.z;
    }

    void scale_down(const vector3f& mScale) {
        x *= mScale.x; y *= mScale.y; z *= mScale.z;
    }

    static vector3f scale_up(const vector3f& mV, const vector3f& mScale) {
        return vector3f(mV.x*mScale.x, mV.y*mScale.y, mV.z*mScale.z);
    }

    static vector3f scale_down(const vector3f& mV, const vector3f& mScale) {
        return vector3f(mV.x/mScale.x, mV.y/mScale.y, mV.z/mScale.z);
    }

    vector3f operator + () const {
        return *this;
    }

    vector3f operator - () const {
        return vector3f(-x, -y, -z);
    }

    vector3f operator * (float fValue) const {
        return vector3f(x*fValue, y*fValue, z*fValue);
    }

    vector3f operator / (float fValue) const {
        return vector3f(x/fValue, y/fValue, z/fValue);
    }

    void operator *= (float fValue) {
        x *= fValue; y *= fValue; z *= fValue;
    }

    void operator /= (float fValue) {
        x /= fValue; y /= fValue; z /= fValue;
    }

    vector3f operator + (const vector3f& mVec) const {
        return vector3f(x + mVec.x, y + mVec.y, z + mVec.z);
    }

    vector3f operator - (const vector3f& mVec) const {
        return vector3f(x - mVec.x, y - mVec.y, z - mVec.z);
    }

    float operator * (const vector3f& mVec) const {
        return x*mVec.x + y*mVec.y + z*mVec.z;
    }

    vector3f operator ^ (const vector3f& mVec) const {
        return vector3f( y*mVec.z - z*mVec.y, z*mVec.x - x*mVec.z, x*mVec.y - y*mVec.x);
    }

    void operator += (const vector3f& mVec) {
        x += mVec.x; y += mVec.y; z += mVec.z;
    }

    void operator -= (const vector3f& mVec) {
        x -= mVec.x; y -= mVec.y; z -= mVec.z;
    }

    bool operator == (const vector3f& mVec) const {
        return x == mVec.x && y == mVec.y && z == mVec.z;
    }

    bool operator != (const vector3f& mVec) const {
        return x != mVec.x || y != mVec.y || z != mVec.z;
    }

    /// (0, 0, 0)
    static const vector3f ZERO;
    /// (1, 1, 1)
    static const vector3f UNIT;
    /// (1, 0, 0)
    static const vector3f UNIT_X;
    /// (0, 1, 0)
    static const vector3f UNIT_Y;
    /// (0, 0, 1)
    static const vector3f UNIT_Z;
    /// (1/sqrt(2), 1/sqrt(2), 0)
    static const vector3f UNIT_XY;
    /// (1/sqrt(2), 0, 1/sqrt(2))
    static const vector3f UNIT_XZ;
    /// (0, 1/sqrt(2), 1/sqrt(2))
    static const vector3f UNIT_YZ;
    /// (1/sqrt(3), 1/sqrt(3), 1/sqrt(3))
    static const vector3f UNIT_XYZ;

    float x, y, z;
};

inline vector3f operator * (float fLeft, const vector3f& mRight) {
    return vector3f(mRight.x*fLeft, mRight.y*fLeft, mRight.z*fLeft);
}

struct vector3i
{
    vector3i() {}

    vector3i(int iX, int iY, int iZ) : x(iX), y(iY), z(iZ) {}

    explicit vector3i(const vector3f& v) : x(v.x), y(v.y), z(v.z) {}

    explicit vector3i(int iValue) : x(iValue), y(iValue), z(iValue) {}

    int get_norm_squared() const {
        return x*x + y*y + z*z;
    }

    bool is_null() const {
        return x == 0 && y == 0 && z == 0;
    }

    vector3i operator + () const {
        return *this;
    }

    vector3i operator - () const {
        return vector3i(-x, -y, -z);
    }

    vector3i operator + (const vector3i& mVec) const  {
        return vector3i(x + mVec.x, y + mVec.y, z + mVec.z);
    }

    vector3i operator - (const vector3i& mVec) const  {
        return vector3i(x - mVec.x, y - mVec.y, z - mVec.z);
    }

    int operator * (const vector3i& mVec) const {
        return x*mVec.x + y*mVec.y + z*mVec.z;
    }

    void operator += (const vector3i& mVec) {
        x += mVec.x; y += mVec.y; z += mVec.z;
    }

    void operator -= (const vector3i& mVec) {
        x -= mVec.x; y -= mVec.y; z -= mVec.z;
    }

    bool operator == (const vector3i& mVec) const {
        return x == mVec.x && y == mVec.y && z == mVec.z;
    }

    bool operator != (const vector3i& mVec) const {
        return x != mVec.x || y != mVec.y || z != mVec.z;
    }

    /// (0, 0, 0)
    static const vector3i ZERO;
    /// (1, 1, 1)
    static const vector3i UNIT;
    /// (1, 0, 0)
    static const vector3i UNIT_X;
    /// (0, 1, 0)
    static const vector3i UNIT_Y;
    /// (0, 0, 1)
    static const vector3i UNIT_Z;
    /// (1, 1, 0)
    static const vector3i UNIT_XY;
    /// (1, 0, 1)
    static const vector3i UNIT_XZ;
    /// (0, 1, 1)
    static const vector3i UNIT_YZ;

    int x, y, z;
};

inline vector3f operator * (float fLeft, const vector3i& mRight) {
    return vector3f(mRight.x*fLeft, mRight.y*fLeft, mRight.z*fLeft);
}

inline vector3f::vector3f(const vector3i& v) : x(v.x), y(v.y), z(v.z) {}

std::ostream& operator << (std::ostream& s, const vector3f& v);
std::ostream& operator << (std::ostream& s, const vector3i& v);

#endif
