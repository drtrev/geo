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
  limitMax.x = BLOCKARRAY_WIDTH;
  limitMax.y = BLOCKARRAY_HEIGHT;
  limitMax.z = BLOCKARRAY_DEPTH;

  // make smaller limits
  limitSmall.x = 2;
  limitSmall.y = 2;
  limitSmall.z = 2;
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
  Block* block;

  // if top or mid level then don't recurse
  if (currentScale > 9.9) block = level.getBlock(check10.pos * check1.scale, 0);
  else if (currentScale > 0.9) block = level.getBlock(check10.pos + check1.pos * check1.scale, 1);
  else{
    block = level.getBlock(check10.pos + check1.pos * check1.scale + check01.pos * check1.scale, 2);

    if (block != NULL && block->state == BLOCK_SOLID) {
      // for now, just make all small blocks slopes to see if it works
      // Use IDHACK_MAKESLOPE and send to server.
      // get server to receive it and send on to client
      // get client to process it too!
      
      // TODO if this goes too fast then it will send makeslope twice. Currently server and client can handle this
      cout << "Sending makeslope for: " << check01.pos.x << "," << check01.pos.y << "," << check01.pos.z << endl;
      unit.flag = UNIT_POSITION;
      unit.position.id = IDHACK_MAKESLOPE;
      unit.position.x = check01.pos.x;
      unit.position.y = check01.pos.y;
      unit.position.z = check01.pos.z;

      net.addUnit(unit, client);
    }
  }

  if (block != NULL) {
    if (check1.pos.x == 0 && check1.pos.y == 1 && check1.pos.z == 0) {
      cout << "At 0, 1, 0: " << block << endl;
    }

    if (block->state == BLOCK_CHILDREN) {
      if (currentScale > 9.9) check1.pos = check10.pos;
      else if (currentScale > 0.9) check01.pos = check1.pos;
      currentScale /= 10.0; // go through smaller blocks
      //cout << "currentScale: " << currentScale << endl;
    }else{
      // This is bad code, don't look
      bool scaledUp = false;
      if (currentScale > 9.9) scaledUp = incBlock(check10);
      else if (currentScale > 9.9) scaledUp = incBlock(check1);
      else scaledUp = incBlock(check01);

      // if block cycle was complete and we've scaled up again, then we need to inc again
      if (scaledUp) {
        if (currentScale > 9.9) scaledUp = incBlock(check10);
        else if (currentScale > 9.9) scaledUp = incBlock(check1);
        else scaledUp = incBlock(check01);

        if (scaledUp) {
          if (currentScale > 9.9) scaledUp = incBlock(check10);
          else if (currentScale > 9.9) scaledUp = incBlock(check1);
          else scaledUp = incBlock(check01);
        }
      }
    }

  }

}

bool Botslopefall::incBlock(Check &check)
{
  geo::Vector currentLimit = limitMax;
  if (currentScale > 9.9) currentLimit = limitSmall;

  check.pos.x += currentScale;
  if (check.pos.x >= currentLimit.x * currentScale) { // we're in world coords
    check.pos.x = 0;
    check.pos.z += currentScale;
    if (check.pos.z >= currentLimit.z * currentScale) {
      check.pos.z = 0;
      check.pos.y += currentScale;
      if (check.pos.y >= currentLimit.y * currentScale) {
        check.pos.y = 0;
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

