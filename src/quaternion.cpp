#include "quaternion.hpp"
#include <cmath>

const quaternion quaternion::UNIT = quaternion(1.0f, 0.0f, 0.0f, 0.0f);

quaternion::quaternion()
{
}

quaternion::quaternion(const vector3f& mAxis, float fAngle)
{
    float fHalfAngle = 0.5f*fAngle;
    float fSin = sin(fHalfAngle);
    w = cos(fHalfAngle);
    x = fSin*mAxis.x;
    y = fSin*mAxis.y;
    z = fSin*mAxis.z;
}

quaternion::quaternion(float fW, float fX, float fY, float fZ) :
    w(fW), x(fX), y(fY), z(fZ)
{
}

quaternion::~quaternion()
{
}

void quaternion::invert()
{
    float fInvNorm = 1.0f/(w*w+x*x+y*y+z*z);
    w *=  fInvNorm;
    x *= -fInvNorm;
    y *= -fInvNorm;
    z *= -fInvNorm;
}

quaternion quaternion::get_inverse() const
{
    quaternion q = *this; q.invert();
    return q;
}

float quaternion::get_yaw() const
{
    float fTx  = 2.0f*x;
    float fTy  = 2.0f*y;
    float fTz  = 2.0f*z;
    float fTwy = fTy*w;
    float fTxx = fTx*x;
    float fTxz = fTz*x;
    float fTyy = fTy*y;

    return atan2(fTxz+fTwy, 1.0f-(fTxx+fTyy));
}

float quaternion::get_pitch() const
{
    float fTx  = 2.0f*x;
    //float fTy  = 2.0f*y;
    float fTz  = 2.0f*z;
    float fTwx = fTx*w;
    float fTxx = fTx*x;
    float fTyz = fTz*y;
    float fTzz = fTz*z;

    return atan2(fTyz+fTwx, 1.0f-(fTxx+fTzz));
}

float quaternion::get_roll() const
{
    //float fTx  = 2.0f*x;
    float fTy  = 2.0f*y;
    float fTz  = 2.0f*z;
    float fTwz = fTz*w;
    float fTxy = fTy*x;
    float fTyy = fTy*y;
    float fTzz = fTz*z;

    return atan2(fTxy+fTwz, 1.0f-(fTyy+fTzz));
}

quaternion quaternion::operator + (const quaternion& q) const
{
    return quaternion(w+q.w, x+q.x, y+q.y, z+q.z);
}

quaternion quaternion::operator - (const quaternion& q) const
{
    return quaternion(w-q.w, x-q.x, y-q.y, z-q.z);
}

quaternion quaternion::operator * (const quaternion& q) const
{
    return quaternion(
        w*q.w - x*q.x - y*q.y - z*q.z,
        w*q.x + x*q.w + y*q.z - z*q.y,
        w*q.y + y*q.w + z*q.x - x*q.z,
        w*q.z + z*q.w + x*q.y - y*q.x
    );
}

quaternion quaternion::operator * (float f) const
{
    return quaternion(w*f, x*f, y*f, z*f);
}

quaternion quaternion::operator - () const
{
    return quaternion(-w, -x, -y, -z);
}

vector3f quaternion::operator * (const vector3f& v) const
{
    vector3f uv, uuv;
    vector3f qvec(x, y, z);
    uv = qvec^v;
    uuv = qvec^uv;
    uv *= 2.0f*w;
    uuv *= 2.0f;

    return v + uv + uuv;
}

quaternion operator * (float f, const quaternion& q)
{
    return quaternion(f*q.w, f*q.x, f*q.y, f*q.z);
}

bool quaternion::operator == (const quaternion& q) const
{
    return w == q.w && x == q.x && y == q.y && z == q.z;
}

bool quaternion::operator != (const quaternion& q) const
{
    return w != q.w || x != q.x || y != q.y || z != q.z;
}
