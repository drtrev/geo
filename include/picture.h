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

#ifndef PICTURE
#define PICTURE

#include <string>
#include "textures.h"
#include <vector>

class Graphics; class Outverbose; class Textures;

class Picture {
  private:
    int width;
    int height;
    float x, y, z, targetX, targetY, targetZ, speed;
    float angleX, angleY, angleZ, targetAngleY, speedRot;
    float pivotX, pivotY, pivotZ;
    float scaleX, scaleY, scaleZ;
    float offsetX, offsetY, offsetZ;
    bool increasingOffset, increasingZoom;
    int direction;
    int texId;
    int loaded;

    bool active, requested;

    Graphics* graphics;
    Outverbose* out;
    Textures* textures;

  public:
    Picture();

    void init(Outverbose&, Graphics&, Textures&);
    void load(std::string);
    void incrementLoaded();
    void setActive(bool);
    bool getActive();
    void setRequested(bool);
    bool getRequested();
    int getWidth();
    int getHeight();
    float getX();
    float getY();
    float getZ();
    float getAngleY();
    void setX(float);
    void setY(float);
    void setZ(float);
    void setAngleY(float);
    void setSpeedRot(float);

    void setTargetX(float);
    void setTargetZ(float);
    void setTargetAngleY(float);

    void setDirection(int);
    void loopAngles();

    void move(double);

    void draw(int);

};

#endif
