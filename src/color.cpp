#include "color.hpp"
#include <iostream>

const color color::WHITE(1.0f, 1.0f, 1.0f);
const color color::BLACK(0.0f, 0.0f, 0.0f);
const color color::RED(1.0f, 0.0f, 0.0f);
const color color::GREEN(0.0f, 1.0f, 0.0f);
const color color::BLUE(0.0f, 0.0f, 1.0f);
const color color::GREY(0.5f, 0.5f, 0.5f);

color::color()
{
}

color::color(chanel nr, chanel ng, chanel nb, chanel na) :
    r(nr), g(ng), b(nb), a(na)
{
}

template<typename T>
inline void clamp(T& v, T min, T max)
{
    v = (v < min ? min : (v > max ? max : v));
}

void color::saturate()
{
    clamp(r, 0.0f, 1.0f);
    clamp(g, 0.0f, 1.0f);
    clamp(b, 0.0f, 1.0f);
    clamp(a, 0.0f, 1.0f);
}

void color::operator *= (const color& c)
{
    r *= c.r; g *= c.g; b *= c.b; a *= c.a;
}

bool color::operator == (const color& c) const
{
    return r == c.r && g == c.g && b == c.b && a == c.a;
}

bool color::operator != (const color& c) const
{
    return r != c.r || g != c.g || b != c.b || a != c.a;
}

color operator * (float coef, const color& c)
{
    return color(coef*c.r, coef*c.g, coef*c.b, coef*c.a);
}

color operator * (const color& c, float coef)
{
    return color(coef*c.r, coef*c.g, coef*c.b, coef*c.a);
}

color operator * (const color& c1, const color& c2)
{
    return color(c1.r*c2.r, c1.g*c2.g, c1.b*c2.b, c1.a*c2.a);
}

color operator + (const color& c1, const color& c2)
{
    return color(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a);
}

color operator - (const color& c1, const color& c2)
{
    return color(c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a);
}

std::ostream& operator << (std::ostream& mStream, const color& mColor)
{
    return mStream << (uint)255*mColor.r << ", " << (uint)255*mColor.g << ", "
                   << (uint)255*mColor.b << ", " << (uint)255*mColor.a;
}
