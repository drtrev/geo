/* This file is part of Map - A tool for viewing 2D map data using Motlab (a
 * heterogeneous collaborative visualization library).
 *
 * Copyright (C) 2009 Trevor Dodds <trev@comp.leeds.ac.uk>
 *
 * Map is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * Map is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Map.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LEVEL_H
#define LEVEL_H

#include <cstdio> // for NULL
#include "pawn.h"
#include "types.h"
#include <vector>

namespace LevelNames {
  struct BlockArray;

  enum BlockState { BLOCK_EMPTY, BLOCK_SOLID, BLOCK_CHILDREN };

  struct Block {
    unsigned short int state;
    BlockArray* children;

    Block() : state(0), children(NULL) {}
  };

  #define BLOCKARRAY_WIDTH 10
  #define BLOCKARRAY_HEIGHT 10
  #define BLOCKARRAY_DEPTH 10

  struct BlockArray {
    Block b[BLOCKARRAY_WIDTH][BLOCKARRAY_HEIGHT][BLOCKARRAY_DEPTH]; // keep things simple for now, no dynamic alloc
  };

  struct Wall {
    geo::Vector corner1, corner2, corner3, corner4, normal;
  };

  struct CollisionInfo {
    geo::Plane plane;
    geo::Ray ray;
    float raylength;
    Wall wall;
  };
}

class Graphics; class Outverbose;

/**
 * The Level class.
 *
 * Note this is also a Pawn, so can move around!
 */
class Level : public Pawn {
  private:
    LevelNames::BlockArray *top;
    int maxdepth;
    int testallocs, testfree;

    std::vector <LevelNames::Wall> collisionWalls; // for displaying them

  protected:
    void addWalls(std::vector <LevelNames::Wall> &walls, geo::Vector blockpos, float scale, float pawnRadius);
    void getWalls(std::vector <LevelNames::Wall> &walls, geo::Vector origpos, geo::Vector parentorigin, int x, int y, int z,
                  LevelNames::BlockArray *blocks, float scale, float pawnRadius);
    std::vector <LevelNames::Wall> getWalls(Props &props);

    bool checkHitPlane(LevelNames::CollisionInfo col, geo::Vector &collisionPoint, float &collisionDistance,
                       bool limitToWall = true);

  public:
    Level(); /**< Constructor. */
    ~Level(); /**< Destructor. */

    void fillArray(LevelNames::BlockArray*, int depth);
    void freeArray(LevelNames::BlockArray*);

    bool createBlock(geo::Vector playerPos);

    void draw(geo::Vector playerpos, geo::Vector playerrot); /**< Render the map. \param thumb set to true if this is the thumbnail view, false otherwise. */

    int checkCollision(Props &props);
};

#endif
