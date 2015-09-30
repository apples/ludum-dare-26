#ifndef COORD_H
#define COORD_H

class Coord
{
public:
    Coord();
    Coord(const Coord& in);
    Coord(double a, double b);

    Coord& operator+=(const Coord& in);
    Coord& operator-=(const Coord& in);

    Coord& operator*=(const Coord& in);
    Coord& operator/=(const Coord& in);

    Coord operator+(const Coord& in) const;
    Coord operator-(const Coord& in) const;

    Coord operator*(const Coord& in) const;
    Coord operator/(const Coord& in) const;

    Coord& operator*=(double in);
    Coord& operator/=(double in);

    Coord operator*(double in) const;
    Coord operator/(double in) const;

    double x, y;
};

#endif // COORD_H
