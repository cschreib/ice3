#ifndef COLOR_HPP
#define COLOR_HPP

#include <lxgui/utils.hpp>
#include <iosfwd>

class color
{
public :

    typedef float chanel;

    color();
    color(chanel nr, chanel ng, chanel nb, chanel na = 1.0f);

    chanel r, g, b, a;
    //chanel b, g, r, a;

    void saturate();

    void operator *= (const color& c);

    bool operator == (const color& c) const;
    bool operator != (const color& c) const;

    static const color WHITE;
    static const color BLACK;
    static const color RED;
    static const color GREEN;
    static const color BLUE;
    static const color GREY;
};

color operator * (float coef, const color& c);
color operator * (const color& c, float coef);
color operator * (const color& c1, const color& c2);
color operator + (const color& c1, const color& c2);
color operator - (const color& c1, const color& c2);

std::ostream& operator << (std::ostream& mStream, const color& mColor);

#endif
