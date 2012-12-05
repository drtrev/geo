#ifndef PAWN_H
#define PAWN_H

#include "types.h"

class Graphics; class Level; class Outverbose;

class Pawn {
  private:

  protected:
    Outverbose* out;
    Graphics* graphics;

    struct Props {
      geo::Vector pos;
      geo::Vector oldPos;
      geo::Vector accel;
      geo::Vector oldAccel;
      geo::Vector speed;

      geo::Vector rot; // rotation

      float power, friction, minSpeed, radius;

      Props() : power(8000), friction(1400), minSpeed(0.001), radius(0.3) {}
    };

    Props props; // pawn properties

    float synclimit;

  public:
    Pawn();
    ~Pawn();

    void init(Outverbose &o);
    void init(Outverbose &o, Graphics &g);

    //virtual void draw() = 0;

    void input(int in, geo::Vector levelrot, double sync);

    void setMove(geo::Vector v, geo::Vector levelrot);
    void moveSimple(Level &level, double sync);
    void move(Level &level, double sync);
    void turn(geo::Vector rot);

    void setPos(geo::Vector pos);
    geo::Vector getPos();

    void setRot(geo::Vector rot);
    geo::Vector getRot();

    void setX(float x);
    void setY(float x);
    void setZ(float x);
    float getX() const;
    float getY() const;
    float getZ() const;
    float getAccelY() const;
};

#endif
