#include "matrix.hpp"
#include <iostream>

const matrix matrix::IDENTITY = {
    1.0f,0.0f,0.0f,0.0f,
    0.0f,1.0f,0.0f,0.0f,
    0.0f,0.0f,1.0f,0.0f,
    0.0f,0.0f,0.0f,1.0f
};

matrix::matrix()
{
}

matrix::matrix(std::initializer_list<float> mList)
{
    int i = 0;
    for (const float* p = mList.begin(); p != mList.end() && i < 16; ++p)
    {
        i = p - mList.begin();
        (*this)(i/4, i%4) = *p;
    }
}

matrix::matrix(float* mat)
{
    for (int i = 0; i < 16; ++i)
        (*this)(i/4, i%4) = mat[i];
}

void matrix::make_translation(const vector3f& dx)
{
    (*this)(0,0) = 1.0f; (*this)(0,1) = 0.0f; (*this)(0,2) = 0.0f; (*this)(0,3) = dx.x;
    (*this)(1,0) = 0.0f; (*this)(1,1) = 1.0f; (*this)(1,2) = 0.0f; (*this)(1,3) = dx.y;
    (*this)(2,0) = 0.0f; (*this)(2,1) = 0.0f; (*this)(2,2) = 1.0f; (*this)(2,3) = dx.z;
    (*this)(3,0) = 0.0f; (*this)(3,1) = 0.0f; (*this)(3,2) = 0.0f; (*this)(3,3) = 1.0f;
}

void matrix::make_scale(const vector3f& scale)
{
    (*this)(0,0) = scale.x; (*this)(0,1) = 0.0f;    (*this)(0,2) = 0.0f;    (*this)(0,3) = 0.0f;
    (*this)(1,0) = 0.0f;    (*this)(1,1) = scale.y; (*this)(1,2) = 0.0f;    (*this)(1,3) = 0.0f;
    (*this)(2,0) = 0.0f;    (*this)(2,1) = 0.0f;    (*this)(2,2) = scale.z; (*this)(2,3) = 0.0f;
    (*this)(3,0) = 0.0f;    (*this)(3,1) = 0.0f;    (*this)(3,2) = 0.0f;    (*this)(3,3) = 1.0f;
}

void matrix::make_rotation(const quaternion& rot)
{
    float fTx  = 2.0f*rot.x;
    float fTy  = 2.0f*rot.y;
    float fTz  = 2.0f*rot.z;
    float fTwx = fTx*rot.w;
    float fTwy = fTy*rot.w;
    float fTwz = fTz*rot.w;
    float fTxx = fTx*rot.x;
    float fTxy = fTy*rot.x;
    float fTxz = fTz*rot.x;
    float fTyy = fTy*rot.y;
    float fTyz = fTz*rot.y;
    float fTzz = fTz*rot.z;

    (*this)(0,0) = 1.0f - (fTyy+fTzz);
    (*this)(0,1) = fTxy - fTwz;
    (*this)(0,2) = fTxz + fTwy;
    (*this)(1,0) = fTxy + fTwz;
    (*this)(1,1) = 1.0f - (fTxx+fTzz);
    (*this)(1,2) = fTyz - fTwx;
    (*this)(2,0) = fTxz - fTwy;
    (*this)(2,1) = fTyz + fTwx;
    (*this)(2,2) = 1.0f - (fTxx+fTyy);

    (*this)(3,0) = (*this)(3,1) = (*this)(3,2) =
    (*this)(0,3) = (*this)(1,3) = (*this)(2,3) = 0.0f;
    (*this)(3,3) = 1.0f;
}

void matrix::make_transform(const vector3f& dx, const vector3f& scale, const quaternion& rot)
{
    make_scale(scale);
    *this = rotation(rot)*(*this);
    (*this)(0,3) = dx.x; (*this)(1,3) = dx.y; (*this)(2,3) = dx.z;
}

void matrix::transpose()
{
    float fTemp;

    for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
    {
        if (i != j)
        {
            fTemp = (*this)(i,j);
            (*this)(i,j) = (*this)(j,i);
            (*this)(j,i) = fTemp;
        }
    }
}

void matrix::invert()
{
    *this = inverse(*this);
}

matrix matrix::translation(const vector3f& dx)
{
    matrix m; m.make_translation(dx);
    return m;
}

matrix matrix::scale(const vector3f& scale)
{
    matrix m; m.make_scale(scale);
    return m;
}

matrix matrix::rotation(const quaternion& rot)
{
    matrix m; m.make_rotation(rot);
    return m;
}

matrix matrix::transform(const vector3f& dx, const vector3f& scale, const quaternion& rot)
{
    matrix m; m.make_transform(dx, scale, rot);
    return m;
}

matrix matrix::transposed(const matrix& m)
{
    return {
        m(0,0), m(1,0), m(2,0), m(3,0),
        m(0,1), m(1,1), m(2,1), m(3,1),
        m(0,2), m(1,2), m(2,2), m(3,2),
        m(0,3), m(1,3), m(2,3), m(3,3)
    };
}

matrix matrix::inverse(const matrix& m)
{
    float m00 = m(0,0), m01 = m(0,1), m02 = m(0,2), m03 = m(0,3);
    float m10 = m(1,0), m11 = m(1,1), m12 = m(1,2), m13 = m(1,3);
    float m20 = m(2,0), m21 = m(2,1), m22 = m(2,2), m23 = m(2,3);
    float m30 = m(3,0), m31 = m(3,1), m32 = m(3,2), m33 = m(3,3);

    float v0 = m20*m31 - m21*m30;
    float v1 = m20*m32 - m22*m30;
    float v2 = m20*m33 - m23*m30;
    float v3 = m21*m32 - m22*m31;
    float v4 = m21*m33 - m23*m31;
    float v5 = m22*m33 - m23*m32;

    float t00 =  (v5*m11 - v4*m12 + v3*m13);
    float t10 = -(v5*m10 - v2*m12 + v1*m13);
    float t20 =  (v4*m10 - v2*m11 + v0*m13);
    float t30 = -(v3*m10 - v1*m11 + v0*m12);

    float invDet = 1.0f/(t00*m00 + t10*m01 + t20*m02 + t30*m03);

    float d00 = t00*invDet;
    float d10 = t10*invDet;
    float d20 = t20*invDet;
    float d30 = t30*invDet;

    float d01 = -(v5*m01 - v4*m02 + v3*m03)*invDet;
    float d11 =  (v5*m00 - v2*m02 + v1*m03)*invDet;
    float d21 = -(v4*m00 - v2*m01 + v0*m03)*invDet;
    float d31 =  (v3*m00 - v1*m01 + v0*m02)*invDet;

    v0 = m10*m31 - m11*m30;
    v1 = m10*m32 - m12*m30;
    v2 = m10*m33 - m13*m30;
    v3 = m11*m32 - m12*m31;
    v4 = m11*m33 - m13*m31;
    v5 = m12*m33 - m13*m32;

    float d02 =  (v5*m01 - v4*m02 + v3*m03)*invDet;
    float d12 = -(v5*m00 - v2*m02 + v1*m03)*invDet;
    float d22 =  (v4*m00 - v2*m01 + v0*m03)*invDet;
    float d32 = -(v3*m00 - v1*m01 + v0*m02)*invDet;

    v0 = m21*m10 - m20*m11;
    v1 = m22*m10 - m20*m12;
    v2 = m23*m10 - m20*m13;
    v3 = m22*m11 - m21*m12;
    v4 = m23*m11 - m21*m13;
    v5 = m23*m12 - m22*m13;

    float d03 = -(v5*m01 - v4*m02 + v3*m03)*invDet;
    float d13 =  (v5*m00 - v2*m02 + v1*m03)*invDet;
    float d23 = -(v4*m00 - v2*m01 + v0*m03)*invDet;
    float d33 =  (v3*m00 - v1*m01 + v0*m02)*invDet;

    return {
        d00, d01, d02, d03,
        d10, d11, d12, d13,
        d20, d21, d22, d23,
        d30, d31, d32, d33
    };
}

matrix matrix::operator + (const matrix& m)
{
    matrix r;

    r(0,0) = (*this)(0,0) + m(0,0);
    r(0,1) = (*this)(0,1) + m(0,1);
    r(0,2) = (*this)(0,2) + m(0,2);
    r(0,3) = (*this)(0,3) + m(0,3);

    r(1,0) = (*this)(1,0) + m(1,0);
    r(1,1) = (*this)(1,1) + m(1,1);
    r(1,2) = (*this)(1,2) + m(1,2);
    r(1,3) = (*this)(1,3) + m(1,3);

    r(2,0) = (*this)(2,0) + m(2,0);
    r(2,1) = (*this)(2,1) + m(2,1);
    r(2,2) = (*this)(2,2) + m(2,2);
    r(2,3) = (*this)(2,3) + m(2,3);

    r(3,0) = (*this)(3,0) + m(3,0);
    r(3,1) = (*this)(3,1) + m(3,1);
    r(3,2) = (*this)(3,2) + m(3,2);
    r(3,3) = (*this)(3,3) + m(3,3);

    return r;
}


matrix matrix::operator - (const matrix& m)
{
    matrix r;

    r(0,0) = (*this)(0,0) - m(0,0);
    r(0,1) = (*this)(0,1) - m(0,1);
    r(0,2) = (*this)(0,2) - m(0,2);
    r(0,3) = (*this)(0,3) - m(0,3);

    r(1,0) = (*this)(1,0) - m(1,0);
    r(1,1) = (*this)(1,1) - m(1,1);
    r(1,2) = (*this)(1,2) - m(1,2);
    r(1,3) = (*this)(1,3) - m(1,3);

    r(2,0) = (*this)(2,0) - m(2,0);
    r(2,1) = (*this)(2,1) - m(2,1);
    r(2,2) = (*this)(2,2) - m(2,2);
    r(2,3) = (*this)(2,3) - m(2,3);

    r(3,0) = (*this)(3,0) - m(3,0);
    r(3,1) = (*this)(3,1) - m(3,1);
    r(3,2) = (*this)(3,2) - m(3,2);
    r(3,3) = (*this)(3,3) - m(3,3);

    return r;
}

matrix matrix::operator * (const matrix& m)
{
    matrix r;

    r(0,0) = (*this)(0,0)*m(0,0) + (*this)(0,1)*m(1,0) + (*this)(0,2)*m(2,0) + (*this)(0,3)*m(3,0);
    r(0,1) = (*this)(0,0)*m(0,1) + (*this)(0,1)*m(1,1) + (*this)(0,2)*m(2,1) + (*this)(0,3)*m(3,1);
    r(0,2) = (*this)(0,0)*m(0,2) + (*this)(0,1)*m(1,2) + (*this)(0,2)*m(2,2) + (*this)(0,3)*m(3,2);
    r(0,3) = (*this)(0,0)*m(0,3) + (*this)(0,1)*m(1,3) + (*this)(0,2)*m(2,3) + (*this)(0,3)*m(3,3);

    r(1,0) = (*this)(1,0)*m(0,0) + (*this)(1,1)*m(1,0) + (*this)(1,2)*m(2,0) + (*this)(1,3)*m(3,0);
    r(1,1) = (*this)(1,0)*m(0,1) + (*this)(1,1)*m(1,1) + (*this)(1,2)*m(2,1) + (*this)(1,3)*m(3,1);
    r(1,2) = (*this)(1,0)*m(0,2) + (*this)(1,1)*m(1,2) + (*this)(1,2)*m(2,2) + (*this)(1,3)*m(3,2);
    r(1,3) = (*this)(1,0)*m(0,3) + (*this)(1,1)*m(1,3) + (*this)(1,2)*m(2,3) + (*this)(1,3)*m(3,3);

    r(2,0) = (*this)(2,0)*m(0,0) + (*this)(2,1)*m(1,0) + (*this)(2,2)*m(2,0) + (*this)(2,3)*m(3,0);
    r(2,1) = (*this)(2,0)*m(0,1) + (*this)(2,1)*m(1,1) + (*this)(2,2)*m(2,1) + (*this)(2,3)*m(3,1);
    r(2,2) = (*this)(2,0)*m(0,2) + (*this)(2,1)*m(1,2) + (*this)(2,2)*m(2,2) + (*this)(2,3)*m(3,2);
    r(2,3) = (*this)(2,0)*m(0,3) + (*this)(2,1)*m(1,3) + (*this)(2,2)*m(2,3) + (*this)(2,3)*m(3,3);

    r(3,0) = (*this)(3,0)*m(0,0) + (*this)(3,1)*m(1,0) + (*this)(3,2)*m(2,0) + (*this)(3,3)*m(3,0);
    r(3,1) = (*this)(3,0)*m(0,1) + (*this)(3,1)*m(1,1) + (*this)(3,2)*m(2,1) + (*this)(3,3)*m(3,1);
    r(3,2) = (*this)(3,0)*m(0,2) + (*this)(3,1)*m(1,2) + (*this)(3,2)*m(2,2) + (*this)(3,3)*m(3,2);
    r(3,3) = (*this)(3,0)*m(0,3) + (*this)(3,1)*m(1,3) + (*this)(3,2)*m(2,3) + (*this)(3,3)*m(3,3);

    return r;
}

vector3f matrix::operator*(const vector3f& v)
{
    vector3f r;

    float fInvW = 1.0f/((*this)(3,0)*v.x + (*this)(3,1)*v.y + (*this)(3,2)*v.z + (*this)(3,3));

    r.x = ((*this)(0,0)*v.x + (*this)(0,1)*v.y + (*this)(0,2)*v.z + (*this)(0,3))*fInvW;
    r.y = ((*this)(1,0)*v.x + (*this)(1,1)*v.y + (*this)(1,2)*v.z + (*this)(1,3))*fInvW;
    r.z = ((*this)(2,0)*v.x + (*this)(2,1)*v.y + (*this)(2,2)*v.z + (*this)(2,3))*fInvW;

    return r;
}

std::ostream& operator << (std::ostream& o, const matrix& m)
{
    for (size_t i = 0; i < 4; ++i)
        o << "(" << m(i, 0) << ", " << m(i, 1) << ", " << m(i, 2) << ", " << m(i, 3) << ")" << (i != 3 ? "\n" : "");

    return o;
}
