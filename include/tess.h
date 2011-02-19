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

// polygon tessellator
#ifndef TESS_H
#define TESS_H

#ifdef _MSC_VER
  #include <windows.h>
#endif
#include <GL/glu.h>

// stolen from: http://www.flipcode.com/archives/Polygon_Tessellation_In_OpenGL.shtml

class Tess {

  private:

    GLUtesselator *tobj; // the tessellation object

  public:

#ifdef _MSC_VER
    int Init(GLvoid);
    int Set_Winding_Rule(GLenum winding_rule);
    int Render_Contour(GLdouble obj_data[][6], int num_vertices);
    int Begin_Polygon(GLvoid);
    int End_Polygon(GLvoid);
    int Begin_Contour(GLvoid);
    int End_Contour(GLvoid);
    int End(GLvoid);
#else
    int Init();
    int Set_Winding_Rule(GLenum winding_rule);
    int Render_Contour(GLdouble obj_data[][6], int num_vertices);
    int Begin_Polygon();
    int End_Polygon();
    int Begin_Contour();
    int End_Contour();
    int End();
#endif

};

#endif
