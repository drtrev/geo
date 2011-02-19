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

#include "windowgenglut.h"
#include <cstdlib>
#include <GL/glut.h>
#include "outverbose.h"

#ifdef _MSC_VER
#include <stdio.h> // for sprintf_s
#endif

// this was static because it was called from a static function i.e. reshape
//extern struct WindowGen::Frustum WindowGen::frustum;

bool WindowGlut::init(Outverbose& o, WindowInfo w)
{
  initShared(o, w);

  int argc = 0;
  char **argv = 0;
  glutInit(&argc, argv);

  // make window
  int mode = GLUT_RGB;
  if (w.doubleBuffer) mode |= GLUT_DOUBLE;
  if (w.depthBuffer) mode |= GLUT_DEPTH;
  glutInitDisplayMode(mode);

  if (!w.fullscreen) {
    glutInitWindowPosition(w.x, w.y);
    glutInitWindowSize(w.width, w.height);
    wid = glutCreateWindow(w.title.c_str());
    *out << VERBOSE_LOUD << "x,y: " << w.x << "," << w.y << ". width,height: " << w.width << "," << w.height;
    *out << " title: " << w.title << " wid: " << wid << '\n';
  }else{
    char str[20];
    // TODO make fullcreen using glutGet(GL_SCREEN_WIDTH)... etc.
#ifdef _MSC_VER
	int size = sprintf(str, "%ix%i:%i@%i", w.width, w.height, w.colorDepth, w.refreshRate);
#else
    int size = std::snprintf(str, 20, "%ix%i:%i@%i", w.width, w.height, w.colorDepth, w.refreshRate);
#endif
    if (size >= 20) {
      *out << VERBOSE_LOUD << "Error: window string too long.\n";
      return false;
    }else{
      glutGameModeString(str);
      wid = glutEnterGameMode();
    }
  }

  glutReshapeFunc(reshape);

  glutMainLoop(); // TODO - this is where we go wrong with glut...
  *out << VERBOSE_LOUD << "initialising window\n";
  return true;
}

void WindowGlut::refresh()
{
  glutSwapBuffers();
}

void WindowGlut::destroy()
{
  glutDestroyWindow(wid);
}

void WindowGlut::reshape(int w, int h)
{
  // should be dealt with by graphicsopengl

  /*std::cout << "reshape" << std::endl;
  frustum.top = frustum.near; // make same as near plane for 90 degree vertical FOV
  frustum.right = frustum.top * (float) w / h;

  // this function is called once when window is initialised
  glViewport (0, 0, w, h);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glFrustum(-frustum.right, frustum.right, -frustum.top, frustum.top, frustum.near, frustum.far);

  glMatrixMode (GL_MODELVIEW);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
}

