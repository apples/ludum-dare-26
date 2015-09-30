#ifndef AABB_H
#define AABB_H

#include "coord.hpp"

class AABB
{
public:
    AABB();
    AABB(const AABB& in);
    AABB(double a, double b, double c, double d);

    AABB& operator+=(const AABB& in);
    AABB& operator-=(const AABB& in);

    AABB operator+(const AABB& in) const;
    AABB operator-(const AABB& in) const;

    AABB& operator+=(const Coord& in);
    AABB& operator-=(const Coord& in);

    AABB operator+(const Coord& in) const;
    AABB operator-(const Coord& in) const;

    bool intersects(const AABB& in) const;

    double left, right, top, bottom;
};

#endif // AABB_H
