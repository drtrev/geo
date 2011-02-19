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
 *
 * $Id$
 */

#include "map.h"
//#ifdef _MSC_VER
//#include <regex.hpp>
//#else
//#include <boost/regex.hpp>
//#endif
#include <cmath>
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
using std::vector;

Map::Map()
{
  accelX = 0, accelY = 0;
  oldAccelX = 0, oldAccelY = 0;
  friction = 1400; // 100 Hz, added in sync to power and friction
  speedX = 0, speedY = 0;
  minSpeed = 5;
  x = 0, y = 0;
  zoom = 0.01;
  power = 200000; // 120000;

  //highestSync = 0; // temporary for error checking
}

Map::~Map()
{
}

//#define BUFFER_SIZE 10000

void Map::tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters) // delimiters default arg set to " " in header
  // stolen from http://www.oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void Map::init(Outverbose &o, Graphics &g, int p)
{
  out = &o;
  graphics = &g;
  polygonMax = p;
}

void Map::load(string filename)
{
  // load GML data here

  // The following provides some data as an example...

  MapPolygon mp;
  mp.colour = 4; // land

  vector <double> point(2);

  double coords[] = { 0, 0, 100, 0, 100, 100, 0, 100, -1 };

  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      for (int i = 0; coords[i] != -1; i+=2) {
        point[0] = coords[i] + x * 100;
        point[1] = coords[i+1] + y * 100;
        mp.outer.push_back(point);
      }
      polygon.push_back(mp);
      mp.outer.clear();
      mp.inner.clear();
      mp.colour++;
      if (mp.colour > 9) mp.colour = 1;
    }
  }

  // generate one polygon manually
  // make it with a hollow middle (inner polygon cut out)
  /*
  point.push_back(0);
  point.push_back(0);
  mp.outer.push_back(point);
  point[0] = 10000;
  point[1] = 0;
  mp.outer.push_back(point);
  point[0] = 10000;
  point[1] = 10000;
  mp.outer.push_back(point);
  point[0] = 0;
  point[1] = 10000;
  mp.outer.push_back(point);

  point[0] = 3000;
  point[1] = 3000;
  mp.inner.push_back(point);
  point[0] = 7000;
  point[1] = 3000;
  mp.inner.push_back(point);
  point[0] = 7000;
  point[1] = 7000;
  mp.inner.push_back(point);
  point[0] = 3000;
  point[1] = 7000;
  mp.inner.push_back(point);

  polygon.push_back(mp);
  */

}

void Map::draw(bool thumb)
{
  if (!thumb) {
    // draw the word zoom
    GraphicsInfo g = graphics->defaultInfo();
    g.x = -10, g.y = -10;
    g.scaleX = 0.5, g.scaleY = 0.5;
    g.text = "Zoom: ";
    char c[10];
#ifdef _MSC_VER
    sprintf(c, "Zoom: X");
#else
    if (snprintf(c, 10, "%.2f", zoom) > 9) {
      c[9] = '\0';
    }
#endif
    g.text += c;
    graphics->drawText(g);
  }

  float tzoom = zoom, tx = x, ty = y;
  if (thumb) {
    tzoom = 0.2;
    tx = -500;
    ty = -500;
  }

  graphics->drawMap(linestrip, polygon, tzoom, tx, ty);

  if (thumb) {

    graphics->drawViewbox(0, 0, 1, 0, 0, 1, 1, 1, 1, 1); // border for thumbnail

  }

  //graphics->drawThumbMap(linestrip, polygon, 0.25, -1480, 0);
}

void Map::input(int in, double sync)
  // called by server
{
  //if (sync > highestSync) {
    //highestSync = sync;
    //cerr << "highestSync: " << sync << endl;
  //}
  if (sync > 0.05) {
    sync = 0.05;
  }
  if (in & KEYS_RIGHT) accelX -= power * sync;
  if (in & KEYS_LEFT) accelX += power * sync;
  if (in & KEYS_FORWARD) accelY -= power * sync;
  if (in & KEYS_BACKWARD) accelY += power * sync;
#ifdef _MSC_VER
  if (in & KEYS_UP) zoom += sync / 10.0;
  if (in & KEYS_DOWN) {
    zoom -= sync / 10.0;
    if (zoom < 0.01) zoom = 0.01;
  }
#else
  if (in & KEYS_UP) zoom += sync; /// 10.0;
  if (in & KEYS_DOWN) {
    zoom -= sync;// / 10.0;
    if (zoom < 0.01) zoom = 0.01;
  }
#endif
  //if (in & KEYS_ZOOM_IN || in & KEYS_ZOOM_OUT) {
    //cerr << "x: " << x << ", y: " << y << endl;
    //cerr << "zoom: " << zoom << endl;
  //}
}

void Map::move(double sync)
{
  // create a dead zone, because the slow movement as we stop can cause a jump into a new text coord
  if (accelX == oldAccelX && fabs(speedX) < minSpeed) speedX = 0;
  if (accelY == oldAccelY && fabs(speedY) < minSpeed) speedY = 0;

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

  x += (speedX/zoom) * sync + 0.5 * accelX * sync * sync;
  y += (speedY/zoom) * sync + 0.5 * accelY * sync * sync;

  // update velocity
  // v = u + at
  speedX = speedX + accelX * sync;
  speedY = speedY + accelY * sync;

  // friction
  // this is a percentage of speed, i.e. the faster you move the more air resistance
  // this should really take into account time, see http://en.wikipedia.org/wiki/Drag_(physics)
  // This works here with sync, but friction is assumed constant over that time period,
  // so if it is running significantly slower then it will stop noticably quicker
  // Think integral approximations: need small time intervals to be more precise.
  accelX = -friction * speedX * sync;
  accelY = -friction * speedY * sync;

  oldAccelX = accelX; // may be modified in input
  oldAccelY = accelY;

  // could have boundary check here, see player.cpp
}

void Map::setX(float nx)
{
  x = nx;
}

void Map::setY(float ny)
{
  y = ny;
}

void Map::setZoom(float nz)
{
  zoom = nz;
}

float Map::getX() const
{
  return x;
}

float Map::getY() const
{
  return y;
}

float Map::getZoom() const
{
  return zoom;
}

