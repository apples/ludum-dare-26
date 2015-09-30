#include "coord.hpp"

Coord::Coord()
    : x(0.0)
    , y(0.0)
{}

Coord::Coord(const Coord& in)
    : x(in.x)
    , y(in.y)
{}

Coord::Coord(double a, double b)
    : x(a)
    , y(b)
{}

Coord& Coord::operator+=(const Coord& in)
{
    x += in.x;
    y += in.y;
    return *this;
}

Coord& Coord::operator-=(const Coord& in)
{
    x -= in.x;
    y -= in.y;
    return *this;
}

Coord& Coord::operator*=(const Coord& in)
{
    x *= in.x;
    y *= in.y;
    return *this;
}

Coord& Coord::operator/=(const Coord& in)
{
    x /= in.x;
    y /= in.y;
    return *this;
}

Coord Coord::operator+(const Coord& in) const
{
    return Coord(x+in.x, y+in.y);
}

Coord Coord::operator-(const Coord& in) const
{
    return Coord(x-in.x, y-in.y);
}

Coord Coord::operator*(const Coord& in) const
{
    return Coord(x*in.x, y*in.y);
}

Coord Coord::operator/(const Coord& in) const
{
    return Coord(x/in.x, y/in.y);
}

Coord& Coord::operator*=(double in)
{
    x *= in;
    y *= in;
    return *this;
}

Coord& Coord::operator/=(double in)
{
    x /= in;
    y /= in;
    return *this;
}

Coord Coord::operator*(double in) const
{
    return Coord(x*in, y*in);
}

Coord Coord::operator/(double in) const
{
    return Coord(x/in, y/in);
}
