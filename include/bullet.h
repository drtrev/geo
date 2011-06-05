#ifndef BULLET_H
#define BULLET_H

#include "pawn.h"

class Bullet : public Pawn {
  private:
    bool active;

  protected:

  public:
    Bullet();
    ~Bullet();

    void setActive(bool);
    bool getActive();
};

#endif
