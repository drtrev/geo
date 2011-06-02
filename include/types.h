#ifndef TYPES_H
#define TYPES_H

#include <cmath>
#include <ostream>

#define IDHACK_PLAYERPOS_MIN 0
#define IDHACK_PLAYERPOS_MAX 100
#define IDHACK_PLAYERROT_MIN 100
#define IDHACK_PLAYERROT_MAX 200
#define IDHACK_BULLETPOS_MIN 200
#define IDHACK_BULLETPOS_MAX 300
#define IDHACK_BULLETROT_MIN 300
#define IDHACK_BULLETROT_MAX 400
#define IDHACK_CREATE 400
#define IDHACK_LEVELROT 401
#define BULLETS_MAX 100 // should be consistent with IDHACKs above

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

  Vector& rotX(float radians)
  {
    float cosAngle = cos(radians), sinAngle = sin(radians);
    Vector n;
    n.x = x;
    n.y = cosAngle * y + sinAngle * z;
    n.z = -sinAngle * y + cosAngle * z;

    return *this = n;
  }

  Vector& rotY(float radians)
  {
    float cosAngle = cos(radians), sinAngle = sin(radians);
    Vector n;
    n.x = cosAngle * x -sinAngle * z;
    n.y = y;
    n.z = sinAngle * x + cosAngle * z;

    return *this = n;
  }

  Vector& rotZ(float radians)
  {
    float cosAngle = cos(radians), sinAngle = sin(radians);
    Vector n;
    n.x = cosAngle * x + sinAngle * y;
    n.y = -sinAngle * x + cosAngle * y;
    n.z = z;

    return *this = n;
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

const double PI = 3.14159265;

inline float rad(float f)
{
  return f / 180.0 * PI;
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
