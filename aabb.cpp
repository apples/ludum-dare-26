#include "aabb.hpp"

AABB::AABB()
    : left  (0.0)
    , right (0.0)
    , bottom(0.0)
    , top   (0.0)
{}

AABB::AABB(const AABB& in)
    : left  (in.left)
    , right (in.right)
    , bottom(in.bottom)
    , top   (in.top)
{}

AABB::AABB(double a, double b, double c, double d)
    : left  (a)
    , right (b)
    , bottom(c)
    , top   (d)
{}

AABB& AABB::operator+=(const AABB& in)
{
    left   += in.left;
    right  += in.right;
    bottom += in.bottom;
    top    += in.top;
    return *this;
}

AABB& AABB::operator-=(const AABB& in)
{
    left   -= in.left;
    right  -= in.right;
    bottom -= in.bottom;
    top    -= in.top;
    return *this;
}

AABB AABB::operator+(const AABB& in) const
{
    return AABB(left  +in.left
              , right +in.right
              , bottom+in.bottom
              , top   +in.top);
}

AABB AABB::operator-(const AABB& in) const
{
    return AABB(left  -in.left
              , right -in.right
              , bottom-in.bottom
              , top   -in.top);
}

AABB& AABB::operator+=(const Coord& in)
{
    left   += in.x;
    right  += in.x;
    bottom += in.y;
    top    += in.y;
    return *this;
}

AABB& AABB::operator-=(const Coord& in)
{
    left   -= in.x;
    right  -= in.x;
    bottom -= in.y;
    top    -= in.y;
    return *this;
}

AABB AABB::operator+(const Coord& in) const
{
    return AABB(left  +in.x
              , right +in.x
              , bottom+in.y
              , top   +in.y);
}

AABB AABB::operator-(const Coord& in) const
{
    return AABB(left  -in.x
              , right -in.x
              , bottom-in.y
              , top   -in.y);
}

bool AABB::intersects(const AABB& in) const
{
    return (left   <= in.right
         && right  > in.left
         && bottom <= in.top
         && top    > in.bottom);
}
