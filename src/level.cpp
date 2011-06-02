/* This file is part of Level - A tool for viewing 2D map data using Motlab (a
 * heterogeneous collaborative visualization library).
 *
 * Copyright (C) 2009 Trevor Dodds <trev@comp.leeds.ac.uk>
 *
 * Level is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * Level is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Level.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "level.h"
#include <cmath>
//#ifdef _MSC_VER
//#include <regex.hpp>
//#else
//#include <boost/regex.hpp>
//#endif
//#include <cmath>
//#include <fstream>
#include "graphics.h"
#include "input.h"
#include <iostream>
//#include "outverbose.h"
//#include <string>
#include <stdlib.h>

#ifdef _MSC_VER
  #include <stdio.h>
  #define snprintf sprintf_s
#endif

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using geo::Vector;
using geo::Plane;
using geo::Ray;
using std::sqrt;
using std::acos;
using namespace LevelNames;

Level::Level()
{
  top = new BlockArray;
  testallocs = 1; // for testing
  testfree = 0;
  maxdepth = 1;

  fillArray(top, 0);

  cout << "Sizeof BlockArray: " << sizeof(BlockArray) << endl;

  // if state is 2, make a child array
}

Level::~Level()
{
  freeArray(top);
  cout << "Allocated: " << testallocs << ", freed: " << testfree << endl;
}

void Level::fillArray(BlockArray* blocks, int depth)
{
  Block* block;

  // randomize blocks
  for (int i = 0; i < BLOCKARRAY_WIDTH; ++i) {
    for (int j = 0; j < BLOCKARRAY_HEIGHT; ++j) {
      for (int k = 0; k < BLOCKARRAY_DEPTH; ++k) {

        block = &blocks->b[i][j][k];
        block->state = rand() % 40;

        // hack floor
        if (j == 0) block->state = BLOCK_SOLID;
        else{
          if (block->state == BLOCK_CHILDREN) {
            if (depth < maxdepth) {
              block->children = new BlockArray;
              testallocs++;
              fillArray(block->children, depth + 1);
            }else{
              block->state = BLOCK_SOLID;
            }
          }else block->state = BLOCK_EMPTY;
        }

      }
    }
  }

}

void Level::freeArray(BlockArray* blocks)
{
  Block* block;

  for (int i = 0; i < BLOCKARRAY_WIDTH; ++i) {
    for (int j = 0; j < BLOCKARRAY_HEIGHT; ++j) {
      for (int k = 0; k < BLOCKARRAY_DEPTH; ++k) {

        block = &blocks->b[i][j][k];

        if (block->state == BLOCK_CHILDREN) {
          // free up kids
          freeArray(block->children);
        }

      }
    }
  }

  delete blocks;
  testfree++;
}

bool Level::createBlock(Vector worldPos)
// round of player position and put a block there if poss
{
  // find block we're in
  // top level

  std::vector <Wall> walls;
  Vector blockpos;
  
  blockpos = worldPos / 10;

  BlockArray* blocks = top;

  int x, y, z;
  x = (int) blockpos.x, y = (int) blockpos.y, z = (int) blockpos.z;

  if (x < 0 || y < 0 || z < 0 ||
      x >= BLOCKARRAY_WIDTH || y >= BLOCKARRAY_HEIGHT ||
      z >= BLOCKARRAY_DEPTH)
  {
    //cerr << "creation out of bounds!" << endl;
  }else{
    if (blocks->b[x][y][z].state == BLOCK_EMPTY) {
      // create a child
      blocks->b[x][y][z].children = new BlockArray;
      testallocs++;
      blocks->b[x][y][z].state = BLOCK_CHILDREN;
    }
    if (blocks->b[x][y][z].state == BLOCK_CHILDREN) {

      bool output = false;
      //if (rand()%50 < 2) output = true;

      // transform original x y and z to children's block coords
      //Vector torigin = parentorigin;
      //torigin.x += x * scale;
      //torigin.y += y * scale;
      //torigin.z += z * scale;

      float scale = 10.0;
      Vector tpos;
      tpos.x = worldPos.x - x * scale;
      tpos.y = worldPos.y - y * scale;
      tpos.z = worldPos.z - z * scale;
      scale /= 10.0;
      Vector blockpos = tpos / scale;

      int tx, ty, tz; // transformed vars, so we can still use x, y and z (below)
      tx = (int) blockpos.x, ty = (int) blockpos.y, tz = (int) blockpos.z;

      if (output) cout << "tx: " << tx << ", ty: " << ty << ", tz: " << tz << endl;

      // now we've got the transformed coords,
      // are they actually within this big block?
      // if not then it'll be detected as out of bounds

      BlockArray* b2 = blocks->b[x][y][z].children;

      if (tx < 0 || ty < 0 || tz < 0
        || tx > BLOCKARRAY_WIDTH || ty > BLOCKARRAY_HEIGHT || tz > BLOCKARRAY_DEPTH) {
        cerr << "Creation of subblocks out of bounds" << endl;
      }else{
        if (b2->b[tx][ty][tz].state == BLOCK_EMPTY) {
          b2->b[tx][ty][tz].state = BLOCK_SOLID;
          return true; // created
        }
      }

    }

  }

  return false; // not created
}

void Level::draw(Vector playerpos, Vector playerrot)
{
  graphics->drawLevel(top, props.pos, props.rot, playerpos, playerrot, collisionWalls);
}

void Level::addSlopedWalls(std::vector <Wall> &walls, Vector blockpos, float half)
// add slopes around block
{
  // TODO make once, then just translate the walls into place
  // e.g. walls.push_back(slopedWall[i].translate(blockpos));
  // or something

  // inside main block
  // front
  const float oneoverroot2 = 0.7071067811;
  Wall wall;
  wall.corner1.x = blockpos.x - half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z + half;
  wall.corner2.x = blockpos.x + half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z + half;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z - half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z - half;
  wall.normal.x = 0; wall.normal.y = oneoverroot2; wall.normal.z = oneoverroot2;
  walls.push_back(wall);

  // right
  wall.corner1.x = blockpos.x + half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z + half;
  wall.corner2.x = blockpos.x + half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z - half;
  wall.corner3.x = blockpos.x - half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z - half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z + half;
  wall.normal.x = oneoverroot2; wall.normal.y = oneoverroot2; wall.normal.z = 0;
  walls.push_back(wall);

  // back
  wall.corner1.x = blockpos.x + half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z - half;
  wall.corner2.x = blockpos.x - half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z - half;
  wall.corner3.x = blockpos.x - half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z + half;
  wall.corner4.x = blockpos.x + half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z + half;
  wall.normal.x = 0; wall.normal.y = oneoverroot2; wall.normal.z = -oneoverroot2;
  walls.push_back(wall);

  // left
  wall.corner1.x = blockpos.x - half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z - half;
  wall.corner2.x = blockpos.x - half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z + half;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z + half;
  wall.corner4.x = blockpos.x + half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z - half;
  wall.normal.x = -oneoverroot2; wall.normal.y = oneoverroot2; wall.normal.z = 0;
  walls.push_back(wall);

  // outside main block
  // front
  /*const float oneoverroot2 = 0.7071067811;
  Wall wall;
  wall.corner1.x = blockpos.x - half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z + half * 3;
  wall.corner2.x = blockpos.x + half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z + half * 3;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z + half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z + half;
  wall.normal.x = 0; wall.normal.y = oneoverroot2; wall.normal.z = oneoverroot2;
  walls.push_back(wall);

  // right
  wall.corner1.x = blockpos.x + half * 3;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z + half;
  wall.corner2.x = blockpos.x + half * 3;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z - half;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z - half;
  wall.corner4.x = blockpos.x + half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z + half;
  wall.normal.x = oneoverroot2; wall.normal.y = oneoverroot2; wall.normal.z = 0;
  walls.push_back(wall);

  // back
  wall.corner1.x = blockpos.x + half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z - half * 3;
  wall.corner2.x = blockpos.x - half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z - half * 3;
  wall.corner3.x = blockpos.x - half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z - half;
  wall.corner4.x = blockpos.x + half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z - half;
  wall.normal.x = 0; wall.normal.y = oneoverroot2; wall.normal.z = -oneoverroot2;
  walls.push_back(wall);

  // left
  wall.corner1.x = blockpos.x - half * 3;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z - half;
  wall.corner2.x = blockpos.x - half * 3;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z + half;
  wall.corner3.x = blockpos.x - half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z + half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z - half;
  wall.normal.x = -oneoverroot2; wall.normal.y = oneoverroot2; wall.normal.z = 0;
  walls.push_back(wall);*/

}

void Level::addWalls(std::vector <Wall> &walls, Vector blockpos, float scale, float pawnRadius)
{
  float half = scale / 2.0;
  blockpos += half;

  half += pawnRadius; // expand walls by the radius of the object we're checking against

  // for small blocks:
  if (scale > 0.09 && scale < 0.11) {
    addSlopedWalls(walls, blockpos, half);
    return;
  }

  // front
  Wall wall;
  wall.corner1.x = blockpos.x - half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z + half;
  wall.corner2.x = blockpos.x + half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z + half;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z + half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z + half;
  wall.normal.x = 0; wall.normal.y = 0; wall.normal.z = 1;
  walls.push_back(wall);

  // right
  wall.corner1.x = blockpos.x + half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z + half;
  wall.corner2.x = blockpos.x + half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z - half;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z - half;
  wall.corner4.x = blockpos.x + half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z + half;
  wall.normal.x = 1; wall.normal.y = 0; wall.normal.z = 0;
  walls.push_back(wall);

  // back
  wall.corner1.x = blockpos.x + half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z - half;
  wall.corner2.x = blockpos.x - half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z - half;
  wall.corner3.x = blockpos.x - half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z - half;
  wall.corner4.x = blockpos.x + half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z - half;
  wall.normal.x = 0; wall.normal.y = 0; wall.normal.z = -1;
  walls.push_back(wall);

  // left
  wall.corner1.x = blockpos.x - half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z - half;
  wall.corner2.x = blockpos.x - half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z + half;
  wall.corner3.x = blockpos.x - half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z + half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z - half;
  wall.normal.x = -1; wall.normal.y = 0; wall.normal.z = 0;
  walls.push_back(wall);

  // top
  wall.corner1.x = blockpos.x - half;
  wall.corner1.y = blockpos.y + half;
  wall.corner1.z = blockpos.z + half;
  wall.corner2.x = blockpos.x + half;
  wall.corner2.y = blockpos.y + half;
  wall.corner2.z = blockpos.z + half;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y + half;
  wall.corner3.z = blockpos.z - half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y + half;
  wall.corner4.z = blockpos.z - half;
  wall.normal.x = 0; wall.normal.y = 1; wall.normal.z = 0;
  walls.push_back(wall);

  // bottom
  wall.corner1.x = blockpos.x - half;
  wall.corner1.y = blockpos.y - half;
  wall.corner1.z = blockpos.z - half;
  wall.corner2.x = blockpos.x + half;
  wall.corner2.y = blockpos.y - half;
  wall.corner2.z = blockpos.z - half;
  wall.corner3.x = blockpos.x + half;
  wall.corner3.y = blockpos.y - half;
  wall.corner3.z = blockpos.z + half;
  wall.corner4.x = blockpos.x - half;
  wall.corner4.y = blockpos.y - half;
  wall.corner4.z = blockpos.z + half;
  wall.normal.x = 0; wall.normal.y = -1; wall.normal.z = 0;
  walls.push_back(wall);
}

void Level::getWalls(std::vector <Wall> &walls, Vector origpos, Vector parentorigin, int x, int y, int z, BlockArray *blocks, float scale, float pawnRadius)
{
  if (x < 0 || y < 0 || z < 0 ||
      x >= BLOCKARRAY_WIDTH || y >= BLOCKARRAY_HEIGHT ||
      z >= BLOCKARRAY_DEPTH)
  {
    //cerr << "collision out of bounds!" << endl;
  }else{
    if (blocks->b[x][y][z].state == BLOCK_SOLID) {
      // make walls
      Vector wallpos;
      wallpos.x = x * scale; wallpos.y = y * scale; wallpos.z = z * scale;
      wallpos += parentorigin;
      // TODO optimise this so it adds one long wall when blocks are adjacent
      addWalls(walls, wallpos, scale, pawnRadius);
    }

    if (blocks->b[x][y][z].state == BLOCK_CHILDREN) {

      bool output = false;
      //if (rand()%50 < 2) output = true;

      if (output) cout << "origpos.x: " << origpos.x << ", origpos.y: " << origpos.y << ", origpos.z: " << origpos.z << endl;
      if (output) cout << "y: " << y << endl;
      // transform original x y and z to children's block coords
      Vector torigin = parentorigin;
      torigin.x += x * scale;
      torigin.y += y * scale;
      torigin.z += z * scale;

      Vector tpos;
      tpos.x = origpos.x - x * scale;
      tpos.y = origpos.y - y * scale;
      tpos.z = origpos.z - z * scale;
      scale /= 10.0;
      if (output) cout << "origpos.x: " << origpos.x << ", origpos.y: " << origpos.y << ", origpos.z: " << origpos.z << endl;
      Vector blockpos = tpos / scale;
      if (output) cout << "origpos.x: " << origpos.x << ", origpos.y: " << origpos.y << ", origpos.z: " << origpos.z << endl;
      if (output) cout << "blockpos.x: " << blockpos.x << ", blockpos.y: " << blockpos.y << ", blockpos.z: " << blockpos.z << endl;

      int tx, ty, tz; // transformed vars, so we can still use x, y and z (below)
      tx = (int) blockpos.x, ty = (int) blockpos.y, tz = (int) blockpos.z;

      if (output) cout << "tx: " << tx << ", ty: " << ty << ", tz: " << tz << endl;

      // now we've got the transformed coords,
      // are they actually within this big block?
      // if not then it'll be detected as out of bounds

      //for (int checkz = tz - 1; checkz < tz + 2; checkz++)
       // for (int checky = ty - 1; checky < ty + 2; checky++)
        //  for (int checkx = tx - 1; checkx < tx + 2; checkx++)
        // TODO change this, only search more when at small blocks, add slopes for those
      for (int checkz = tz - 2; checkz < tz + 3; checkz++)
        for (int checky = ty - 2; checky < ty + 3; checky++)
          for (int checkx = tx - 2; checkx < tx + 3; checkx++)
            getWalls(walls, origpos, torigin, checkx, checky, checkz, blocks->b[x][y][z].children, scale, pawnRadius);

    }

  }
}

// TODO just pass props.pos?
std::vector <Wall> Level::getWalls(Props &props)
{
  // find block we're in
  // top level

  std::vector <Wall> walls;
  Vector blockpos;
  
  blockpos = props.pos / 10;

  // or look ahead:
  //Vector move = props.pos - props.oldPos;
  //move *= 4;
  //blockpos = props.pos + move;
  //blockpos /= 10;

  int x, y, z;
  x = (int) blockpos.x, y = (int) blockpos.y, z = (int) blockpos.z;

  Vector origin;

  for (int checkz = z - 1; checkz < z + 2; checkz++)
    for (int checky = y - 1; checky < y + 2; checky++)
      for (int checkx = x - 1; checkx < x + 2; checkx++)
        getWalls(walls, props.pos, origin, checkx, checky, checkz, top, 10, props.radius);

  return walls;
}

// taken from shoot
/*bool Pawn::checkHit2(float planeNormal[], float pointOnPlane[], float rayStart[], float rayDirection[], float distanceMoved, Wall wall, float collisionPoint[], float &collisionDistance, bool limitToWall)
{
  // t= (planeNormal dot (pointOnPlane - rayStart)) / (planeNormal dot rayDirection) 
  // where t is point of collision

  // planeNormal dot rayDirection
  float normalDotRay = dotProduct(planeNormal, rayDirection);
  
  //cout << "normalDotRay: " << normalDotRay << endl;
  //cout << "rayDirection: " << rayDirection[0] << ", " << rayDirection[1] << ", " << rayDirection[2] << endl;
  //cout << "planeNormal: " << planeNormal[0] << ", " << planeNormal[1] << ", " << planeNormal[2] << endl;
  //cout << "pointOnPlane: " << pointOnPlane[0] << ", " << pointOnPlane[1] << ", " << pointOnPlane[2] << endl;
  //cout << "normalDotRay: " << normalDotRay << endl;
  if (normalDotRay == 0.0 || normalDotRay == -0.0) return false; // point is moving parallel to plane
  
  // pointOnPlane - rayStart
  float pointMinusRay[3];
  pointMinusRay[0] = pointOnPlane[0] - rayStart[0];
  pointMinusRay[1] = pointOnPlane[1] - rayStart[1];
  pointMinusRay[2] = pointOnPlane[2] - rayStart[2];

  float t = dotProduct(planeNormal, pointMinusRay) / normalDotRay;

  // CollisionPointOnRay = Raystart + t * Raydirection
  //float collisionPoint[3];
  collisionPoint[0] = rayStart[0] + t * rayDirection[0];
  collisionPoint[1] = rayStart[1] + t * rayDirection[1];
  collisionPoint[2] = rayStart[2] + t * rayDirection[2];

  float wallXMin = 0.0, wallXMax = 0.0, wallYMin = 0.0, wallYMax = 0.0, wallZMin = 0.0, wallZMax = 0.0;

  if (wall.getX1() <= wall.getX2()) wallXMin = wall.getX1(), wallXMax = wall.getX2();
  else wallXMax = wall.getX1(), wallXMin = wall.getX2();

  if (wall.getZ1() <= wall.getZ2()) wallZMin = wall.getZ1(), wallZMax = wall.getZ2();
  else wallZMax = wall.getZ1(), wallZMin = wall.getZ2();

  // move plane to account for size of player (don't want to be able to go right into wall)
  wallXMin += planeNormal[0], wallXMax += planeNormal[0];
  wallZMin += planeNormal[2], wallZMax += planeNormal[2];

  // wall y1 must be lower than wall y3
  wallYMin = wall.getY1();
  wallYMax = wall.getY3();

  //cout << "collisionPoint[0]: " << collisionPoint[0] << " -- wallXMin: " << wallXMin << " -- wallXMax: " << wallXMax << endl;
  
  // extend the collision area by a boundary (due to gaps between walls from plane increase)
  // even with a boundary of 0 this is needed to account for point of
  // collision being off the wall
  if (limitToWall) {
    float boundary = 0.1;

    if (collisionPoint[0] < wallXMin - boundary || collisionPoint[0] > wallXMax + boundary
        //|| collisionPoint[1] < wallYMin - boundary || collisionPoint[1] > wallYMax + boundary
        || collisionPoint[2] < wallZMin - boundary || collisionPoint[2] > wallZMax + boundary)
      return false; // collided outside of wall
  }
  

  //cout << "t: " << t << endl;
  // if t is negative intersection takes place before the starting point and so there is no collision
  //if (t >= 0.0 && t < distanceMoved)cb
  if (t > -0.01 && t < distanceMoved + 0.01){
    collisionDistance = t;
    return true;
  }else return false;
}*/

bool Level::checkHitPlane(CollisionInfo col, Vector &collisionPoint, float &collisionDistance, bool limitToWall)
{
  bool hit = true; // assume we'll hit the plane, then set to false in certain circumstances

  // t= (planeNormal dot (pointOnPlane - rayStart)) / (planeNormal dot rayDirection) 
  // where t is point of collision

  // planeNormal dot rayDirection
  float normalDotRay = col.plane.normal.dot(col.ray.direction);
  
  if (normalDotRay == 0.0 || normalDotRay == -0.0) return false; // point is moving parallel to plane

  // plane.point - ray.start
  Vector pointMinusRay = col.plane.point - col.ray.start;

  // TODO optimise - why so many vars?
  float t = col.plane.normal.dot(pointMinusRay) / normalDotRay;

  // CollisionPointOnRay = ray.start + t * ray.direction
  //float collisionPoint[3];
  Vector v = t * col.ray.direction;
  //collisionPoint = col.ray.start + (t * col.ray.direction);
  collisionPoint = col.ray.start + v;

  // TODO optimise for g++ using <? and >?
  // see http://gcc.gnu.org/onlinedocs/gcc-3.3.6/gcc/Min-and-Max.html
  float wallXMin = std::min(col.wall.corner1.x, col.wall.corner2.x);
  wallXMin = std::min(col.wall.corner3.x, wallXMin);
  float wallXMax = std::max(col.wall.corner1.x, col.wall.corner2.x);
  wallXMax = std::max(col.wall.corner3.x, wallXMax);
  float wallYMin = std::min(col.wall.corner1.y, col.wall.corner2.y);
  wallYMin = std::min(col.wall.corner3.y, wallYMin);
  float wallYMax = std::max(col.wall.corner1.y, col.wall.corner2.y);
  wallYMax = std::max(col.wall.corner3.y, wallYMax);
  float wallZMin = std::min(col.wall.corner1.z, col.wall.corner2.z);
  wallZMin = std::min(col.wall.corner3.z, wallZMin);
  float wallZMax = std::max(col.wall.corner1.z, col.wall.corner2.z);
  wallZMax = std::max(col.wall.corner3.z, wallZMax);


  // move plane to account for size of player (don't want to be able to go right into wall)
  //wallXMin += planeNormal[0], wallXMax += planeNormal[0];
  //wallZMin += planeNormal[2], wallZMax += planeNormal[2];

  // extend the collision area by a boundary (due to gaps between walls from plane increase)
  // even with a boundary of 0 this is needed to account for point of
  // collision being off the wall
  if (limitToWall) {
    float boundary = 0.001;

    if (collisionPoint.x < wallXMin - boundary || collisionPoint.x > wallXMax + boundary
        || collisionPoint.y < wallYMin - boundary || collisionPoint.y > wallYMax + boundary
        || collisionPoint.z < wallZMin - boundary || collisionPoint.z > wallZMax + boundary)
    {
      hit = false;
      // note at this stage we may not even be within raylength (and t > 0)
      //cout << "wxmin: " << wallXMin << ", max: " << wallXMax << ", ymin: " << wallYMin << ", ymax: " << wallYMax;
      //cout << ", zmin: " << wallZMin << ", zmax: " << wallZMax << endl;
      //cout << collisionPoint << endl;
    }
  }
  
  // (collisionDistance could be -ve, hit behind ray start)
  collisionDistance = t;

  // with raylength of 0, don't care about distance
  if (col.raylength > 0.0) {

    // if t is negative intersection takes place before the starting point and so there is no collision
    float tolerance = 0.005; // TODO could be lower?

    if (t < -tolerance || t > col.raylength + tolerance) {
      hit = false;
    }

  }

  return hit;
}

int Level::checkCollision(Props &props)
{
  Vector move = props.pos - props.oldPos;
  float distanceMoved = sqrt(move.x * move.x + move.y * move.y + move.z * move.z);
  if (distanceMoved > 0) {
    move /= distanceMoved; // normalise
  }

  CollisionInfo col;
  col.ray.start = props.oldPos;
  col.ray.direction = move;
  col.raylength = distanceMoved;

  std::vector <Wall> walls = getWalls(props);
  collisionWalls = walls; // for display

  //if (rand()%200 < 2) cout << "numwalls: " << walls.size() << endl;
  bool hit;
  int numHit = 0;
  Vector collisionPoint;
  float collisionDistance;

  // TODO go through all walls, find nearest collisionDistance, move based on that wall
  // then repeat a few times
  for (unsigned int i = 0; i < walls.size(); i++) {
    col.plane.point = walls[i].corner1;
    col.plane.normal = walls[i].normal;
    col.wall = walls[i];
    hit = checkHitPlane(col, collisionPoint, collisionDistance);

    if (hit) {
      // is it going against the normal? check > PI/2
      if ( acos( move.dot(walls[i].normal) ) > 1.56) {
        //cout << "hit!" << endl;
        numHit++;

        // go from new position to the wall (along the normal) to find
        // new point for sliding algorithm
        col.ray.start = props.pos;
        col.ray.direction = col.plane.normal;
        col.raylength++; // make sure it goes back far enough (distanceMoved should be *just* enough).
        
        // if this is the 4th wall we've hit, give up (reset pos)
        // Also, due to some rounding errors may not hit wall on way back?
        // just reset position and push TODO push!
        if (numHit > 3 || !checkHitPlane(col, collisionPoint, collisionDistance, false)) {
          props.pos = props.oldPos;
          break; // we've reset position, just get out of here
        }else{
          Vector v = col.plane.normal * 0.01;
          props.pos = collisionPoint + v;

          // we've now changed the new position, so repeat collision detection
          move = props.pos - props.oldPos;
          distanceMoved = sqrt(move.x * move.x + move.y * move.y + move.z * move.z);
          if (distanceMoved > 0) {
            move /= distanceMoved; // normalise
          }

          col.ray.start = props.oldPos;
          col.ray.direction = move;
          col.raylength = distanceMoved;
          i = -1; // reset loop!
        }

      } // end going against normal

    } // end if hit

  } // end for

  return 0;
}

/*int Level::checkCollisionBasic(Props &props)
{
  // basic
  // find block we're in
  // top level

  int hit = 0;
  Vector blockpos;
  
  blockpos = props.pos / 10;
  int x, y, z;
  x = blockpos.x, y = blockpos.y, z = blockpos.z;

  if (x < 0 || y < 0 || z < 0 ||
      x >= BLOCKARRAY_WIDTH || y >= BLOCKARRAY_HEIGHT ||
      z >= BLOCKARRAY_DEPTH)
  {
      hit = 0; // out of bounds
      cerr << "out of bounds!" << endl;
  }else{
    if (top->b[x][y][z].state == BLOCK_SOLID) hit = 1;
  }

  if (hit == 1) {
    cout << "hit!" << endl;
    props.pos = props.oldPos;
    Vector zero;
    props.speed = zero;
    props.accel = zero;
  }

  return hit;
}*/

