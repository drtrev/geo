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

#include "tess.h"

void vertexCallback(GLvoid *vertex);
void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],
    GLfloat weight[4], GLdouble **dataOut);

void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],
    GLfloat weight[4], GLdouble **dataOut)
{
  /*GLdouble *vertex;

  vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));
  vertex[0] = coords[0];
  vertex[1] = coords[1];
  vertex[2] = coords[2];

  for (int i = 3; i < 6; i++)
  {
    vertex[i] = weight[0] * vertex_data[0][i] +
      indent indweight[1] * vertex_data[1][i] +
      indent indweight[2] * vertex_data[2][i] +
      indent indweight[3] * vertex_data[3][i];
  }

  *dataOut = vertex;*/
}

void vertexCallback(GLvoid *vertex)
{
  GLdouble *ptr;

  ptr = (GLdouble *) vertex;
  if (*(ptr + 3) != -1) glColor3dv((GLdouble *) ptr + 3); // swapped this so it's before glVertex!!
  glVertex3dv((GLdouble *) ptr);
}


#ifdef _MSC_VER
int Tess::Init(GLvoid)
#else
int Tess::Init()
#endif
{
  // Create a new tessellation object 
  tobj = gluNewTess(); 

  // Set callback functions
#ifdef _MSC_VER
  // think for combineCallback and vertexCallback, may need
  // to declare them as __stdcall (think this is used for 'API' functions).
  // but for now just map to open gl functions (presumably these are declared
  // as stdcall)
  gluTessCallback(tobj, GLU_TESS_VERTEX, (void (__stdcall*) ( )) glVertex3dv);
  //gluTessCallback(tobj, GLU_TESS_VERTEX, (void (__stdcall*) ( )) vertexCallback);
  gluTessCallback(tobj, GLU_TESS_BEGIN, (void (__stdcall*) ( )) glBegin);
  gluTessCallback(tobj, GLU_TESS_END, (void (__stdcall*) ( )) glEnd);
  //gluTessCallback(tobj, GLU_TESS_COMBINE, (void (__stdcall*) ( )) combineCallback);
#else
  gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid (*) ( )) &vertexCallback);
  gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid (*) ( )) &glBegin);
  gluTessCallback(tobj, GLU_TESS_END, (GLvoid (*) ( )) &glEnd);
  gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid (*) ( ))&combineCallback);
#endif

  return(1);
}

int Tess::Set_Winding_Rule(GLenum winding_rule)
{
  // Set the winding rule
  gluTessProperty(tobj, GLU_TESS_WINDING_RULE, winding_rule); 

  return(1);
}

int Tess::Render_Contour(GLdouble obj_data[][6], int num_vertices)
{
  for (int x = 0; x < num_vertices; x++) //loop through the vertices
  {
    gluTessVertex(tobj, obj_data[x], obj_data[x]); //store the vertex
  }

  return(1);
}

#ifdef _MSC_VER
int Tess::Begin_Polygon(GLvoid)
#else
int Tess::Begin_Polygon()
#endif
{
  gluTessBeginPolygon(tobj, NULL);

  return(1);
}

#ifdef _MSC_VER
int Tess::End_Polygon(GLvoid)
#else
int Tess::End_Polygon()
#endif
{
  gluTessEndPolygon(tobj);

  return(1);
}

#ifdef _MSC_VER
int Tess::Begin_Contour(GLvoid)
#else
int Tess::Begin_Contour()
#endif
{
  gluTessBeginContour(tobj);

  return(1);
}

#ifdef _MSC_VER
int Tess::End_Contour(GLvoid)
#else
int Tess::End_Contour()
#endif
{
  gluTessEndContour(tobj);

  return(1);
}

#ifdef _MSC_VER
int Tess::End(GLvoid)
#else
int Tess::End()
#endif
{
  gluDeleteTess(tobj);

  return(1);
}


