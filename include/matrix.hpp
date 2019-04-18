#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <lxgui/utils.hpp>
#include "vector3.hpp"
#include "quaternion.hpp"
#include <initializer_list>
#include <iosfwd>

class matrix
{
public :

    matrix();
    matrix(std::initializer_list<float> mList);
    explicit matrix(float* mat);

    inline float& operator () (size_t i, size_t j)
    {
        return data[i+j*4];
    }

    inline float operator () (size_t i, size_t j) const
    {
        return data[i+j*4];
    }

    inline float& operator () (size_t i)
    {
        return data[i];
    }

    inline float operator () (size_t i) const
    {
        return data[i];
    }

    void make_translation(const vector3f& dx);
    void make_scale(const vector3f& scale);
    void make_rotation(const quaternion& rot);
    void make_transform(const vector3f& dx, const vector3f& scale, const quaternion& rot);

    void transpose();
    void invert();

    static matrix translation(const vector3f& dx);
    static matrix scale(const vector3f& scale);
    static matrix rotation(const quaternion& rot);
    static matrix transform(const vector3f& dx, const vector3f& scale, const quaternion& rot);

    static matrix transposed(const matrix& m);
    static matrix inverse(const matrix& m);

    matrix operator + (const matrix& m);
    matrix operator - (const matrix& m);
    matrix operator * (const matrix& m);
    vector3f operator * (const vector3f& v);

    float data[16];

    static const matrix IDENTITY;
};

std::ostream& operator << (std::ostream& o, const matrix& m);

#endif
