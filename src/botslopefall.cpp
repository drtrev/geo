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
    cout << block << endl;
  }

  if (block == BLOCK_CHILDREN) {
    if (currentScale > 9.9) check1.pos = check10.pos;
    else if (currentScale > 0.9) check01.pos = check1.pos;
    currentScale /= 10.0; // go through smaller blocks
    cout << "currentScale: " << currentScale << endl;
  }else{
    if (currentScale > 9.9) incBlock(check10);
    else if (currentScale > 9.9) incBlock(check1);
    else incBlock(check01);
  }

}

void Botslopefall::incBlock(Check &check)
{
  check.pos.x += currentScale;
  if (check.pos.x >= BLOCKARRAY_WIDTH * currentScale) { // we're in world coords
    check.pos.x = 0;
    check.pos.z += currentScale;
    if (check.pos.z >= BLOCKARRAY_DEPTH * currentScale) {
      check.pos.z = 0;
      check.pos.y += currentScale;
      if (check.pos.y >= BLOCKARRAY_HEIGHT * currentScale) {
        check.pos.y = 0;
        cout << "completed cycle" << endl;
        if (currentScale > 9.9) {
          currentScale = 10.0; // in case of rounding error
        }else currentScale *= 10;
        cout << "currentScale: " << currentScale << endl;
      }
    }
  }
}

