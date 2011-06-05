/* This file is part of Pawn - A tool for viewing 2D map data using Motlab (a
 * heterogeneous collaborative visualization library).
 *
 * Copyright (C) 2009 Trevor Dodds <trev@comp.leeds.ac.uk>
 *
 * Pawn is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * Pawn is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Pawn.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "input.h"
#include "level.h"
#include "math.h" // TODO cmath or nothing?
#include "pawn.h"
//#ifdef _MSC_VER
//#include <regex.hpp>
//#else
//#include <boost/regex.hpp>
//#endif

#include<iostream>

/*#include <cmath>
#include <fstream>
#include "graphics.h"
#include "input.h"
#include <iostream>
#include "outverbose.h"
#include <string>
#include <stdlib.h>

#ifdef _MSC_VER
  #include <stdio.h>
  #define snprintf sprintf_s
#endif

using std::string;
using std::cerr;
using std::endl;
using std::vector;*/

using geo::rad;

Pawn::Pawn()
{
  synclimit = 0.03; // prevent too high sync
}

Pawn::~Pawn()
{
}

void Pawn::init(Outverbose &o, Graphics &g)
{
  out = &o;
  graphics = &g;
}

/*void Pawn::draw()
{
  //graphics->drawBlocks(blocks, x, y, zoom);
}*/

void Pawn::input(int in, geo::Vector levelrot, double sync)
  // called by server
{
  if (sync > synclimit) sync = synclimit;

  float amount = props.power * sync;

  geo::Vector v;

  if (in & KEYS_RIGHT) v.x += amount;
  if (in & KEYS_LEFT) v.x -= amount;
  if (in & KEYS_FORWARD) v.z -= amount;
  if (in & KEYS_BACKWARD) v.z += amount;
  if (in & KEYS_UP || in & KEYS_JUMP) v.y += amount;
  if (in & KEYS_DOWN) v.y -= amount;
  //std::cout << "amount: " << amount << std::endl;
  //v.y -= 9.8 * sync; // actually friction is way too high
  v.y -= 1000 * sync;

  // TODO make rotation function, or steal from OpenGL
  // rotate around y
  v.rotY(rad(props.rot.y)); // rotation is applied locally and sent
  v.rotZ(rad(props.rot.z)); // for fun
  // rotate with respect to level
  v.rotX(rad(levelrot.x));
  v.rotY(rad(levelrot.y));
  v.rotZ(rad(levelrot.z));
  /*geo::Vector v2;
  float radY = props.rot.y / 180.0 * 3.14159;
  float sinRadY = sin(radY);
  float cosRadY = cos(radY);
  v2.x = v.x * cosRadY - v.z * sinRadY;
  v2.y = v.y;
  v2.z = v.x * sinRadY + v.z * cosRadY;
  props.accel += v2;*/
  props.accel += v;

  /*if (in & KEYS_RIGHT) props.accel.x += amount;
  if (in & KEYS_LEFT) props.accel.x -= amount;
  if (in & KEYS_FORWARD) props.accel.z -= amount;
  if (in & KEYS_BACKWARD) props.accel.z += amount;
  if (in & KEYS_UP) props.accel.y += amount;
  if (in & KEYS_DOWN) props.accel.y -= amount;*/
}

void Pawn::setMove(geo::Vector v, geo::Vector levelrot)
// set an initial movement vector, e.g. for a bullet
// not called setSpeed, because that would imply caller is doing the calcs
{
  v.rotX(rad(props.rot.x));
  v.rotY(rad(props.rot.y));
  v.rotZ(rad(props.rot.z));
  // rotate with respect to level
  v.rotX(rad(levelrot.x));
  v.rotY(rad(levelrot.y));
  v.rotZ(rad(levelrot.z));
  props.speed = v;
}

void Pawn::moveSimple(Level &level, double sync)
// just use speed
{
  // if there's a sync limit, client goes slower than server
  // so can't use this.
  //if (sync > synclimit) sync = synclimit;
  props.oldPos = props.pos;

  // TODO can I optimise this, i.e. pos += speed ...;
  props.pos.x += props.speed.x * sync;
  props.pos.y += props.speed.y * sync;
  props.pos.z += props.speed.z * sync;
}

void Pawn::move(Level &level, double sync)
{
  // TODO optimise this using the operator overloads in types.h
  // i.e. props.pos += props.speed ...

  if (sync > synclimit) sync = synclimit;

  // this is left over from curses implementation, but I'm ok with a dead zone
  // create a dead zone, because the slow movement as we stop can cause a jump into a new text coord
  if (props.accel.x == props.oldAccel.x && fabs(props.speed.x) < props.minSpeed) props.speed.x = 0;
  if (props.accel.y == props.oldAccel.y && fabs(props.speed.y) < props.minSpeed) props.speed.y = 0;
  if (props.accel.z == props.oldAccel.z && fabs(props.speed.z) < props.minSpeed) props.speed.z = 0;

  // this distance should be the integral of the velocity function, which can be
  // found on that drag site
  // hmm... acceleration's not constant. Need to find velocity function and then
  // can get approximate acceleration between last frame using a = (v - u) / t
  // Ok think I've got this now - basically calculating friction at tiny intervals
  // is the same as working out the integral (an approximation)
  //
  // s = ut + 0.5 * att
  // s is displacement. s equals ut plus a half a t-squared
  // u is initial velocity, t is time, using 0 as time started accelerating

  props.oldPos = props.pos;

  props.pos.x += props.speed.x * sync + 0.5 * props.accel.x * sync * sync;
  props.pos.y += props.speed.y * sync + 0.5 * props.accel.y * sync * sync;
  props.pos.z += props.speed.z * sync + 0.5 * props.accel.z * sync * sync;

  // update velocity
  // v = u + at
  props.speed.x = props.speed.x + props.accel.x * sync;
  props.speed.y = props.speed.y + props.accel.y * sync;
  props.speed.z = props.speed.z + props.accel.z * sync;

  // friction
  // this is a percentage of speed, i.e. the faster you props the more air resistance
  // this should really take into account time, see http://en.wikipedia.org/wiki/Drag_(physics)
  // This works here with sync, but friction is assumed constant over that time period,
  // so if it is running significantly slower then it will stop noticably quicker
  // Think integral approximations: need small time intervals to be more precise.
  props.accel.x = -props.friction * props.speed.x * sync;
  props.accel.y = -props.friction * props.speed.y * sync;
  props.accel.z = -props.friction * props.speed.z * sync;

  props.oldAccel = props.accel; // may be modified in input
  /*props.oldAccelX = props.accel.x; // may be modified in input
  props.oldAccelY = props.accel.y;
  props.oldAccelZ = props.accel.z;*/

  // could have boundary check here, see player.cpp

  // collision check
  level.checkCollision(props);
}

void Pawn::turn(geo::Vector rot)
{
  props.rot += rot;
}

void Pawn::setPos(geo::Vector pos)
{
  props.pos = pos;
}

geo::Vector Pawn::getPos()
{
  return props.pos;
}

void Pawn::setRot(geo::Vector rot)
{
  props.rot = rot;
}

geo::Vector Pawn::getRot()
{
  return props.rot;
}

void Pawn::setX(float nx)
{
  props.pos.x = nx;
}

void Pawn::setY(float ny)
{
  props.pos.y = ny;
}

void Pawn::setZ(float nz)
{
  props.pos.z = nz;
}

float Pawn::getX() const
{
  return props.pos.x;
}

float Pawn::getY() const
{
  return props.pos.y;
}

float Pawn::getZ() const
{
  return props.pos.z;
}

