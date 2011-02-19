#ifndef TYPES_H
#define TYPES_H

#include <ostream>

namespace geo {

struct Vector {
  float x;
  float y;
  float z;

  Vector() : x(0), y(0), z(0) {}
  Vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

  // TODO make relevant params const, see TODO file!
  Vector operator+(Vector &v)
  {
    return Vector( x + v.x, y + v.y, z + v.z);
  }

  Vector operator+(float f)
  {
    return Vector( x + f, y + f, z + f);
  }

  Vector& operator+=(Vector &v)
  {
    return *this = *this + v;
  }

  Vector& operator+=(float f)
  {
    return *this = *this + f;
  }

  Vector operator-(Vector &v)
  {
    return Vector( x - v.x, y - v.y, z - v.z);
  }

  Vector operator-(float f)
  {
    return Vector( x - f, y - f, z - f);
  }

  Vector operator-()
  {
    return Vector( -x, -y, -z );
  }

  Vector& operator-=(Vector &v)
  {
    return *this = *this - v;
  }

  Vector& operator-=(float f)
  {
    return *this = *this - f;
  }

  // dot product
  float dot(Vector &v)
  {
    return x*v.x + y*v.y + z*v.z;
  }

  Vector operator*(Vector &v)
  {
    return Vector( x * v.x, y * v.y, z * v.z );
  }

  Vector operator*(float f)
  {
    return Vector( x * f, y * f, z * f);
  }

  Vector& operator*=(Vector &v)
  {
    return *this = *this * v;
  }

  Vector& operator*=(float f)
  {
    return *this = *this * f;
  }

  Vector operator/(Vector &v)
  {
    return Vector( x / v.x, y / v.y, z / v.z );
  }

  Vector operator/(float f)
  {
    return Vector( x / f, y / f, z / f);
  }

  Vector& operator/=(Vector &v)
  {
    return *this = *this / v;
  }

  Vector& operator/=(float f)
  {
    return *this = *this / f;
  }

  bool operator==(Vector &v)
  {
    if (x == v.x && y == v.y && z == v.z) return true;
    return false;
  }

  bool operator!=(Vector &v)
  {
    if (x != v.x || y != v.y || z != v.z) return true;
    return false;
  }

  //declaring this as a friend gives access to private members,
  //which is not needed in a struct (all public)
  //friend ostream& operator<<(ostream &os, Vector &v);

};

// need inline to avoid multiple definitions TODO check!
inline Vector operator+(float f, Vector &v)
{
  return v + f;
}

inline Vector operator-(float f, Vector &v)
{
  return -v + f;
}

inline Vector operator*(float f, Vector &v)
{
  return v * f;
}

inline std::ostream& operator<<(std::ostream &os, const Vector &v)
{
  os << "x: " << v.x << ", y: " << v.y << ", z: " << v.z;
  return os;
}

struct Plane {
  Vector point;
  Vector normal;
};

struct Ray {
  Vector start;
  Vector direction;
};

}

#endif
