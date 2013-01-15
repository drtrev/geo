#ifndef BOTSLOPEFALL_H
#define BOTSLOPEFALL_H

#include "bot.h"
#include "types.h"

class Botslopefall : public Bot {
  private:
    struct Check {
      geo::Vector pos;
      int scale;
    } check10, check1, check01;
    float currentScale;
    geo::Vector limitMax;
    geo::Vector limitSmall;

    geo::Vector localToWorld();

  public:
    Botslopefall();
    void go();
    void checkNextBlock();
    bool incBlock(Check &check);
};

#endif
