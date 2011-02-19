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

#include "graphicscurses.h"
#include "outverbose.h"

bool GraphicsCurses::init(Outverbose &o, WindowInfo w, const char* font, int fontsize)
// font and fontsize not used by graphicscurses
{
  initShared(o);

  // window info not used - just use current terminal

  return true; // success
}

void GraphicsCurses::kill()
{
}

void GraphicsCurses::drawStart()
{
}

void GraphicsCurses::drawThumbStart()
{
}

void GraphicsCurses::drawMap(std::vector < std::vector < std::vector <double> > > &linestrip, std::vector <MapPolygon> &polygon, double zoom, double offsetX, double offsetY)
{
}

void GraphicsCurses::drawText(GraphicsInfo g)
{
  /*if (!g.visible) {
    for (int i = 0; i < (int) g.text.size(); i++) g.text[i] = ' ';
  }

  // should specify coords here
  out->add(g.text);*/
}

void GraphicsCurses::drawViewbox(double posX, double posY, double scale, double thumbOffsetX, double thumbOffsetY, double thumbScale, float r, float g, float b, float a)
{
}

void GraphicsCurses::drawStop()
{
}

void GraphicsCurses::drawThumbStop()
{
}

void GraphicsCurses::refresh()
{
  // done in clientcontrol and servercontrol
  //out->refreshScreen();
}

float GraphicsCurses::getWidth()
{
  return out->getWidth();
}

