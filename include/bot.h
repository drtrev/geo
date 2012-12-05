#ifndef BOT_H
#define BOT_H

#include "controller.h"
#include "motlab/client.h"

class Bot : public Controller {
  protected:
    Client client;
    int myId;

  public:
    Bot();
    ~Bot();

    void init(Args &args);

    void go();
    void gameover();

    void networkloop();
    void process(Unit unit);
};

#endif
