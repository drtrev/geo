#include "botslopefall.h"

using namespace LevelNames;

using std::cout;
using std::endl;

Botslopefall::Botslopefall()
{
  check10.scale = 10;
  check1.scale = 1;
  check01.scale = 0.1;
  currentScale = 10;

  // note that it must be < limit
  limit.x = BLOCKARRAY_WIDTH;
  limit.y = BLOCKARRAY_HEIGHT;
  limit.z = BLOCKARRAY_DEPTH;

  // make smaller limits
  limit.y = 2;
  limit.x = 2;
  limit.z = 2;
}

void Botslopefall::go()
{
  while(client.getConnected()) {
    networkloop();
    checkNextBlock();
    usleep(10000);
  }
}

void Botslopefall::checkNextBlock()
{
  int block = 0;

  // if top or mid level then don't recurse
  if (currentScale > 9.9) block = level.getBlock(check10.pos, 0);
  else if (currentScale > 0.9) block = level.getBlock(check1.pos, 1);
  else{
    block = level.getBlock(check01.pos, 2);

    if (block == BLOCK_SOLID) {
      // for now, just make all small blocks slopes to see if it works
      // TODO XXX working here, use IDHACK_MAKESLOPE and send to server.
      // get server to receive it and send on to client
      // get client to process it too!
    }
  }

  if (check1.pos.x == 0 && check1.pos.y == 1 && check1.pos.z == 0) {
    cout << "At 0, 1, 0: " << block << endl;
  }

  if (block == BLOCK_CHILDREN) {
    if (currentScale > 9.9) check1.pos = check10.pos;
    else if (currentScale > 0.9) check01.pos = check1.pos;
    currentScale /= 10.0; // go through smaller blocks
    //cout << "currentScale: " << currentScale << endl;
  }else{
    bool scaledUp = false;
    if (currentScale > 9.9) scaledUp = incBlock(check10);
    else if (currentScale > 9.9) scaledUp = incBlock(check1);
    else scaledUp = incBlock(check01);

    // if block cycle was complete and we've scaled up again, then we need to inc again
    if (scaledUp) {
      if (currentScale > 9.9) incBlock(check10);
      else if (currentScale > 9.9) incBlock(check1);
      else incBlock(check01);
    }
  }

}

bool Botslopefall::incBlock(Check &check)
{
  check.pos.x += currentScale;
  if (check.pos.x >= limit.x * currentScale) { // we're in world coords
    check.pos.x = 0;
    check.pos.z += currentScale;
    if (check.pos.z >= limit.z * currentScale) {
      check.pos.z = 0;
      check.pos.y += currentScale;
      if (check.pos.y >= limit.y * currentScale) {
        check.pos.y = 0;
        //cout << "completed cycle" << endl;
        if (currentScale > 9.9) {
          cout << "completed everything" << endl;
          currentScale = 10.0; // in case of rounding error
        }else{
          currentScale *= 10;
          //cout << "currentScale: " << currentScale << endl;
          return true;
        }
      }
    }
  }

  return false;
}

