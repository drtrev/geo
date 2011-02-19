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

#ifndef GRAPHICS_CURSES
#define GRAPHICS_CURSES

#include "graphics.h"
#include <vector>

class Outverbose;

class GraphicsCurses : public Graphics {
  private:

  public:
    bool init(Outverbose&, WindowInfo, const char*, int);
    void kill();

    void drawStart();
    void drawThumbStart();
    void drawMap(std::vector < std::vector < std::vector <double> > >&, std::vector <MapPolygon>&, double, double, double);
    void drawText(GraphicsInfo);
    void drawViewbox(double posX, double posY, double scale, double thumbOffsetX, double thumbOffsetY, double thumbScale, float r, float g, float b, float a);
    void drawStop();
    void drawThumbStop();

    void refresh();

    float getWidth();
};

#endif
