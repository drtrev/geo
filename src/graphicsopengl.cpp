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

#include "graphicsopengl.h"
#include "bullet.h"
#ifdef _MSC_VER
  #include <windows.h>
  #include <GL/glu.h>
//  #include "map.h" // for polygon
#endif
#include <GL/gl.h>
//#include "windowgenglut.h"
#include "outverbose.h"
#include "tess.h"
#include "windowgensdl.h"

using std::cout;
using std::cerr;
using std::endl;


// testing - didn't seem to help
//extern "C" {
//void vertexCallback(GLvoid *vertex);
//void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],
//    GLfloat weight[4], GLdouble **dataOut);
//}

GraphicsOpenGL::GraphicsOpenGL()
{
  frustum.nearb = 0.1;
  frustum.farb = 100;
  frustum.topb = 10; // calculate properly when window is opened in init
  frustum.rightb = 10;

  border.top = 100, border.right = 100, border.bottom = 100, border.left = 100;

  for (int i = 0; i < DLIST_END; i++) displayList[i] = 0; // not built any lists yet

}

bool GraphicsOpenGL::initTTF()
{
  // init SDL_ttf
  if (TTF_Init() < 0) {
    cerr << "Could not initialise TTF: " << TTF_GetError() << endl;
    return false;
  }
  // Load a font
  ttf.font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 24);
  if (ttf.font == NULL)
  {
    cerr << "TTF_OpenFont() Failed: " << TTF_GetError() << endl;
    TTF_Quit();
    SDL_Quit();
    exit(1);
  }

  // Write text to surface

  ttf.text = "";
  ttf.color.r = 255;
  ttf.color.g = 255;
  ttf.color.b = 255;

  ttf.textsurf = NULL;
  /*ttf.textsurf = TTF_RenderText_Solid(ttf.font,
      ttf.text.c_str(),
      ttf.color);

  if (ttf.textsurf == NULL)
  {
    cerr << "TTF_RenderText_Solid() Failed: " << TTF_GetError() << endl;
    TTF_Quit();
    SDL_Quit();
    exit(1);
  }else{
    cout << "Made text surface" << endl;
  }*/

  ttf.fmtsurf = 0;

  ttf.texture = 0;
  ttf.generated = false;

  return true;
}

bool GraphicsOpenGL::init(Outverbose &o, WindowInfo w, const char* font, int fontsize)
{
  initShared(o);

#ifndef _MSC_VER
#ifndef NOOGLFT
  // initialise font
  face = new OGLFT::TranslucentTexture( font, fontsize );
  face->setHorizontalJustification( OGLFT::Face::CENTER );
  face->setVerticalJustification( OGLFT::Face::MIDDLE );
#endif
#endif
  //face->setForegroundColor( 0.75, 1., .75, 1. ); -- set in drawText
  //face->setCharacterRotationX( 0 );
  //face->setCharacterRotationY( 0 );
  //face->setCharacterRotationZ( 0 );
  //face->setStringRotation( 0 );

  // Now using SDL_ttf library
  initTTF();

  //window = new WindowGlut;
  window = new WindowSDL;

  if (window->init(*out, w)) {
    frustum.topb = frustum.nearb; // make same as near plane for 90 degree vertical FOV
    frustum.rightb = frustum.topb * (float) window->getWidth() / window->getHeight();

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glFrustum(-frustum.rightb, frustum.rightb, -frustum.topb, frustum.topb, frustum.nearb, frustum.farb);

    glMatrixMode (GL_MODELVIEW);

    // initialise openGL
    glViewport(0, 0, window->getWidth(), window->getHeight());
    //*out << VERBOSE_LOUD << "width: " << window->getWidth() << ", height: " << window->getHeight() << '\n';

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glCullFace(GL_FRONT); // if we mirror it
    glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    // typical usage: modify incoming colour by it's alpha, and existing colour by 1 - source alpha
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // make lighting use glColor:
    // see man glColorMaterial
    glEnable(GL_COLOR_MATERIAL);

    // lights
    setupLights();
    return true; // success

  }else{
    return false;
  }
}

void GraphicsOpenGL::setupLights()
{
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  float ambient[4] = { .8, .8, .8, 1 };
  float diffuse[4] = { .8, .8, .8, 1 };
  float specular[4] = { 0, 0, 0, 1 };
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  setLightPositions();
}

void GraphicsOpenGL::setLightPositions()
{
  float pos[4] = { -0.4, 1, 0.7, 0 }; // directional, w = 0
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
}

void GraphicsOpenGL::kill()
{
  cout << "Killing graphics..." << endl;
  TTF_Quit();

  if (window->getWid() > -1) window->destroy();
  delete window;
#ifndef _MSC_VER
#ifndef NOOGLFT
  delete face;
#endif
#endif
  for (int i = 0; i < DLIST_END; i++) {
    if (displayList[i] > 0) glDeleteLists(displayList[i], 1);
  }
}

void GraphicsOpenGL::drawStart()
{
  //glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, window->getWidth(), window->getHeight());
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  //glScalef(1, -1, 1);
  //glTranslatef(-out->getWidth(), -out->getWidth(), -out->getWidth()); // move camera out

  /*float zoom = 128;
  glTranslatef(0, 0, -zoom);

  if (window->getWidth() > window->getHeight()) {
    border.top = zoom;
    border.bottom = -border.top;
    border.right = border.top * window->getWidth() / (float) window->getHeight();
    border.left = -border.right;
  }

  glNormal3f(0, 0, 1);

  glColor4f(0, 0, 0, 1);
  glBegin(GL_QUADS);
  glVertex3f(border.left, border.bottom, 0);
  glVertex3f(border.right, border.bottom, 0);
  glVertex3f(border.right, border.top, 0);
  glVertex3f(border.left, border.top, 0);
  glEnd();*/

  // draw axis (temporarily to help)
  /*glColor4f(1, 1, 1, 1);
  glBegin(GL_LINES);
  glNormal3f(0, 0, 1);
  glVertex3f(0, -50, 0);
  glVertex3f(0, 50, 0);

  glVertex3f(-50, 0, 0);
  glVertex3f(50, 0, 0);
  glEnd();
  glBegin(GL_QUADS); // testing
  glVertex3f(-100, -100, -10);
  glVertex3f(100, -100, -10);
  glVertex3f(100, 100, -10);
  glVertex3f(-100, 100, -10);
  glEnd();*/
}

void GraphicsOpenGL::drawThumbStart()
{
  glDisable(GL_DEPTH_TEST);
  glViewport((int) (window->getWidth() / 4.0 * 3), (int) (window->getHeight() / 4.0 * 3), (int) (window->getWidth() / 4.0), (int) (window->getHeight() / 4.0));
  //*out << VERBOSE_LOUD << "width: " << window->getWidth() << ", height: " << window->getHeight() << '\n';

  //glClearColor(0, 0, 0, 1);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();

  float zoom = 128;
  glTranslatef(0, 0, -zoom);

  if (window->getWidth() > window->getHeight()) {
    border.top = zoom;
    border.bottom = -border.top;
    border.right = border.top * window->getWidth() / (float) window->getHeight();
    border.left = -border.right;
  }

  glNormal3f(0, 0, 1);

  glColor4f(0, 0, 0, 1);
  glBegin(GL_QUADS);
  glVertex3f(border.left, border.bottom, 0);
  glVertex3f(border.right, border.bottom, 0);
  glVertex3f(border.right, border.top, 0);
  glVertex3f(border.left, border.top, 0);
  glEnd();

}

void GraphicsOpenGL::flip()
// call pushMatrix and pushAttrib(GL_POLYGON_BIT) (for GL_CULL_FACE_MODE) first
{
  glCullFace(GL_FRONT);
  glScalef(2, -4, 1);
  glTranslatef(border.left / 2.0 + 2, -border.top / 4.0 + 3, 1);
}

void GraphicsOpenGL::drawMap(std::vector < std::vector < std::vector <double> > > &linestrip, std::vector < MapPolygon > &polygon, double scale, double offsetX, double offsetY)
{
  // test tessellation
  /*glPushMatrix();
    glScalef(scale, scale, 0);
    Tess poly;

	//glTranslatef(0, 0, 50);
  // concave triangle displays wrong (fills across top)
  glColor4f(0, 0, 1, 1);
  //glBegin(GL_QUADS);
  //glVertex3f(-1, 3, 0);
  //glVertex3f(0, 0, 0);
  //glVertex3f(1, 3, 0);
  //glVertex3f(0, 2, 0); // was y=2
  //glEnd();

  // this works! draws a concave triangle to test tessellation
  poly.Init();
  poly.Set_Winding_Rule(GLU_TESS_WINDING_ODD); 
  //begin the new polygon
  poly.Begin_Polygon(); 
  poly.Begin_Contour();
  // create polygon array (passed to Render_Contour and referenced in End_polygon, see man gluTessVertex: notes section)
  GLdouble quad1[4][6] = { {-1,3,0,  1,0,0}, {0,0,0,  1,0,0}, {1,3,0,  1,0,0}, {0,2,0,  1,0,0} };
  poly.Render_Contour(quad1, 4); 
  poly.End_Contour();
  // inner block (hole)
  //poly.Begin_Contour();
  //poly.Render_Contour(tri, 3); 
  //poly.End_Contour();
  poly.End_Polygon();
  //delete the tess object
  poly.End();

  glPopMatrix();*/

  //static double oldscale = 0;
    //if (oldscale != scale) {
  //std::cerr << "Scale: " << scale << std::endl;
  //oldscale = scale;
  //std::cerr << "x: " << offsetX << "y: " << offsetY << std::endl;
  //}

  glPushMatrix();
  glColor4f(0, 0, 0, 1);

  glScalef(scale, scale, 0);
  //glTranslatef(1000, -500, 0);
  glTranslatef(offsetX, offsetY, 0);

  if (displayList[DLIST_MAP] == 0) {
    displayList[DLIST_MAP] = glGenLists(1);
    cout << "Generated display list: " << displayList[DLIST_MAP] << endl;
    glNewList(displayList[DLIST_MAP], GL_COMPILE);

    for (unsigned int line = 0; line < linestrip.size(); line++) {
      glBegin(GL_LINE_STRIP);
      for (unsigned int i = 0; i < linestrip[line].size(); i++) {
        glVertex3f(linestrip[line][i][0], linestrip[line][i][1], 0);
      }
      glEnd();
    }
    cout << "Done lines" << endl;

    glColor4f(1, 0, 0, 1);

    // draw polygons

    Tess poly;

    GLdouble (*data1)[6];
    GLdouble (*data2)[6] = 0;
    bool allocatedData2 = false;

    for (unsigned int p = 0; p < polygon.size(); p++) {
		//cout << "Polygon: " << p << endl;
      switch (polygon[p].colour) {
        case 0:
          glColor4f(1, 0, 0, 1); // theme not found
          break;
        case 1:
          glColor4f(1, 1, 1, 1); // admin boundaries
          break;
        case 2:
          glColor4f(0.2, 0.2, 0.2, 1); // buildings
          break;
        case 3:
          glColor4f(0.6, 0.24, 0.12, 1); // heritage and antiquities
          break;
        case 4:
          glColor4f(0, 0.5, 0, 1); // land
          break;
        case 5:
          glColor4f(0.5, 0.2, 0.5, 1); // rail
          break;
        case 6:
          glColor4f(0.5, 0.5, 0.5, 1); // roads
          break;
        case 7:
          glColor4f(0.0, 0.8, 0.8, 1); // structures
          break;
        case 8:
          glColor4f(0, 1, 0.3, 1); // terrain
          break;
        case 9:
          glColor4f(0.2, 0.2, 1, 1); // water
          break;
        default:
          glColor4f(1, 0, 0, 1);
          cerr << "Colour out of boudns" << endl;
      }

//      Tess poly;

      poly.Init();
      poly.Set_Winding_Rule(GLU_TESS_WINDING_ODD); 
      //begin the new polygon
      poly.Begin_Polygon(); 

      // outer
      poly.Begin_Contour();
      // create polygon array (passed to Render_Contour and referenced in End_polygon, see man gluTessVertex: notes section)
      data1 = new GLdouble[polygon[p].outer.size()][6];
      for (unsigned int i = 0; i < polygon[p].outer.size(); i++) {
        data1[i][0] = polygon[p].outer[i][0];
        data1[i][1] = polygon[p].outer[i][1];
        data1[i][2] = 0; // z
        data1[i][3] = -1; // means don't change colour
        data1[i][4] = -1;
        data1[i][5] = -1;
      }
      poly.Render_Contour(data1, polygon[p].outer.size()); 
      poly.End_Contour();

	  //cout << "outer done" << endl;
      // inner
      if (polygon[p].inner.size() > 0) {
		  //cout << "doing inner" << endl;
        poly.Begin_Contour();
        //GLdouble (*data2)[6];
        data2 = new GLdouble[polygon[p].inner.size()][6];
        allocatedData2 = true;
        for (unsigned int i = 0; i < polygon[p].inner.size(); i++) {
          data2[i][0] = polygon[p].inner[i][0];
          data2[i][1] = polygon[p].inner[i][1];
          data2[i][2] = 0; // z
          data2[i][3] = -1; // means don't change colour
          data2[i][4] = -1;
          data2[i][5] = -1;
        }
        poly.Render_Contour(data2, polygon[p].inner.size()); 
        poly.End_Contour();
      }else allocatedData2 = false;

      // inner block (hole)
      //poly.Begin_Contour();
      //poly.Render_Contour(tri, 3); 
      //poly.End_Contour();
	  //cout << "About to end polygon" << endl;
	  //cout << "data1: " << data1 << ", data2: " << data2 << endl;
      poly.End_Polygon(); // error was here
	  // when end polygon is called, that's when it actually uses the callback functions in tess,
	  // and my dodgy casts to get it to compile didn't actually set things up properly for execution!
	  // thought maybe cos callbacks were out of scope, so declared them as extern "C" in here (but didn't help)
	  // workaround: use glVertex3dv (see tess.cpp)
	  //cout << "about to end tess object" << endl;
      //delete the tess object
      poly.End();
	  //cout << "Cleaning up..." << endl;
      delete [] data1; // tested in ~/test/testC++/twodim.cpp with valgrind
      if (allocatedData2) delete [] data2;
	  //cout << "Cleaned up" << endl;
    }

    glEndList();
  }else{
    glCallList(displayList[DLIST_MAP]);
  }

  glPopMatrix();
}

void GraphicsOpenGL::drawBlock(int x, int y, int z, float scale, bool sloped)
{
  glPushMatrix();
  if (scale > 9) glColor4f(0.7, .7, .7, 1);
  else if (scale > 0.5) glColor4f(0, .7, 0, 1);
  else glColor4f(0, 0, 0.7, 1);

  if (sloped) glColor4f(0.5, 0.8, 0.2, 1);

  //if (scale < 0.2) { // temp hack to hide small blocks to see collision walls

    glTranslatef(x + 0.5, y + 0.5, z + 0.5);

    // should use facet tables here, but it's only one object so not too fussed
    float normal = 1*scale; // need normals to match the scale, so they're always unit length
    glBegin(GL_QUADS);
    glNormal3f(0, 0, normal); // front
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);

    glNormal3f(0, 0, -normal); // back
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(0.5, 0.5, -0.5);

    glNormal3f(0, normal, 0); // top
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(-0.5, 0.5, -0.5);

    glNormal3f(0, -normal, 0); // bottom
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(0.5, -0.5, -0.5);

    glNormal3f(-normal, 0, 0); // left
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, -0.5);

    glNormal3f(normal, 0, 0); // right
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glEnd();
  //}

  glPopMatrix();
}

void GraphicsOpenGL::drawBlocks(LevelNames::BlockArray *blocks, float scale)
{
  glPushMatrix();

  glScalef(scale, scale, scale);

  for (int i = 0; i < BLOCKARRAY_WIDTH; ++i) {
    for (int j = 0; j < BLOCKARRAY_HEIGHT; ++j) {
      for (int k = 0; k < BLOCKARRAY_DEPTH; ++k) {
        if (blocks->b[i][j][k].state == LevelNames::BLOCK_SOLID || blocks->b[i][j][k].state == LevelNames::BLOCK_SLOPE) {
          drawBlock(i, j, k, scale, blocks->b[i][j][k].state == LevelNames::BLOCK_SLOPE);
        }
        if (blocks->b[i][j][k].state == LevelNames::BLOCK_CHILDREN) {
          glPopMatrix(); // rescale from original
          glPushMatrix(); // for translate
          glTranslatef(i*scale, j*scale, k*scale);
          drawBlocks(blocks->b[i][j][k].children, scale/10.0);
          glPopMatrix();
          glPushMatrix(); // scale again from original
          glScalef(scale, scale, scale);
        }
      }
    }
  }

  glPopMatrix();
}

void GraphicsOpenGL::drawWall(LevelNames::Wall& wall)
{
  glPushMatrix();
  float scale = 1.00001; // draw over existing walls - this actually offsets things slightly since coords are world
  glScalef(scale, scale, scale);
  glColor4f(1, 0, 0, 1);
  //glDisable(GL_DEPTH_TEST);
  //glDisable(GL_CULL_FACE);
  glBegin(GL_QUADS);
  glNormal3f(wall.normal.x * scale, wall.normal.y * scale, wall.normal.z * scale);
  glVertex3f(wall.corner1.x, wall.corner1.y, wall.corner1.z);
  glVertex3f(wall.corner2.x, wall.corner2.y, wall.corner2.z);
  glVertex3f(wall.corner3.x, wall.corner3.y, wall.corner3.z);
  glVertex3f(wall.corner4.x, wall.corner4.y, wall.corner4.z);
  glEnd();
  //glEnable(GL_CULL_FACE);
  //glEnable(GL_DEPTH_TEST);
  glPopMatrix();
}

void GraphicsOpenGL::drawBullet(geo::Vector bulletpos)
{
  glPushMatrix();
  glTranslatef(bulletpos.x, bulletpos.y, bulletpos.z);

  glColor4f(1, 0.5, 0.5, 1);
  float scale = 0.1; // used also for calculating normal below
  glScalef(scale, scale, scale);

  // should use facet tables here, but it's only one object so not too fussed
  float normal = 1*scale; // need normals to match the scale, so they're always unit length
  glBegin(GL_QUADS);
  glNormal3f(0, 0, normal); // front
  glVertex3f(-0.5, -0.5, 0.5);
  glVertex3f(0.5, -0.5, 0.5);
  glVertex3f(0.5, 0.5, 0.5);
  glVertex3f(-0.5, 0.5, 0.5);

  glNormal3f(0, 0, -normal); // back
  glVertex3f(0.5, -0.5, -0.5);
  glVertex3f(-0.5, -0.5, -0.5);
  glVertex3f(-0.5, 0.5, -0.5);
  glVertex3f(0.5, 0.5, -0.5);

  glNormal3f(0, normal, 0); // top
  glVertex3f(-0.5, 0.5, 0.5);
  glVertex3f(0.5, 0.5, 0.5);
  glVertex3f(0.5, 0.5, -0.5);
  glVertex3f(-0.5, 0.5, -0.5);

  glNormal3f(0, -normal, 0); // bottom
  glVertex3f(0.5, -0.5, 0.5);
  glVertex3f(-0.5, -0.5, 0.5);
  glVertex3f(-0.5, -0.5, -0.5);
  glVertex3f(0.5, -0.5, -0.5);

  glNormal3f(-normal, 0, 0); // left
  glVertex3f(-0.5, -0.5, -0.5);
  glVertex3f(-0.5, -0.5, 0.5);
  glVertex3f(-0.5, 0.5, 0.5);
  glVertex3f(-0.5, 0.5, -0.5);

  glNormal3f(normal, 0, 0); // right
  glVertex3f(0.5, -0.5, 0.5);
  glVertex3f(0.5, -0.5, -0.5);
  glVertex3f(0.5, 0.5, -0.5);
  glVertex3f(0.5, 0.5, 0.5);
  glEnd();

  glPopMatrix();
}

void GraphicsOpenGL::drawLevel(LevelNames::BlockArray *blocks, geo::Vector& levelpos, geo::Vector& levelrot, geo::Vector& playerpos, geo::Vector& playerrot, std::vector<LevelNames::Wall>& collisionWalls, Bullet* bullet)
{
  glPushMatrix();

  glRotatef(playerrot.x + levelrot.x, 1, 0, 0);
  glRotatef(playerrot.y + levelrot.y, 0, 1, 0);
  glRotatef(playerrot.z + levelrot.z, 0, 0, 1);
  //if (rand() % 10) cout << "levelrot.z: " << levelrot.z << endl;
  glTranslatef(levelpos.x - playerpos.x, levelpos.y - playerpos.y, levelpos.z - playerpos.z);

  setLightPositions();

  // top level, 10m x 10m blocks
  float scale = 10;
  drawBlocks(blocks, scale);

  for (unsigned int i = 0; i < collisionWalls.size(); i++) drawWall(collisionWalls[i]);

  for (int i = 0; i < BULLETS_MAX; i++) if (bullet[i].getActive()) drawBullet(bullet[i].getPos());

  glPopMatrix();
}

void GraphicsOpenGL::drawText(GraphicsInfo g)
{
#ifndef _MSC_VER
#ifndef NOOGLFT
  // TODO remove this, not available for win
  if (g.visible) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glPushMatrix();
    glTranslatef(g.x, g.y, g.z);
    glTranslatef(0, 0, g.pivotZ); // change pivot point
    glRotatef(g.angleX, 1, 0, 0);
    glRotatef(g.angleY, 0, 1, 0);
    glRotatef(g.angleZ, 0, 0, 1);
    glTranslatef(0, 0, -g.pivotZ); // change pivot point
    glScalef(0.5, 0.5, 0.5); // get higher resolution font by choosing large fontsize then halving rendering
    glScalef(g.scaleX, g.scaleY, g.scaleZ);
    glEnable(GL_TEXTURE_2D);

    face->setForegroundColor( g.color.red, g.color.green, g.color.blue, g.color.alpha );
    //face->draw(g.x, g.y, g.z, g.text.c_str());
    face->draw(0, 0, 0, g.text.c_str());

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisable(GL_BLEND);
  }
#endif
#endif
  // SDL_ttf
  if (g.visible && g.text != "") {
    // These were useful, especially second one
    // http://stackoverflow.com/questions/5289447/using-sdl-ttf-with-opengl
    // http://www.gamedev.net/topic/284259-for-reference-using-sdl_ttf-with-opengl/

    if (g.text != ttf.text) {

      ttf.text = g.text;

      int width = 256;
      int height = 32;

      if (!ttf.generated)
        ttf.fmtsurf = SDL_CreateRGBSurface(0, width, height, 32, 
            0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

      if (ttf.textsurf != NULL) SDL_FreeSurface(ttf.textsurf);
      ttf.textsurf = TTF_RenderText_Solid(ttf.font, ttf.text.c_str(), ttf.color);
      if (ttf.textsurf == NULL) {
        cerr << "TTF_RenderText_Solid() Failed: " << TTF_GetError() << endl;
      }else{
        if (ttf.generated) SDL_FillRect(ttf.fmtsurf, 0, SDL_MapRGBA(ttf.fmtsurf->format, 0, 0, 0, 0)); // clear it
        SDL_BlitSurface(ttf.textsurf, 0, ttf.fmtsurf, 0);


        if (!ttf.generated) {
          glGenTextures(1, &(ttf.texture));
        }

        glBindTexture(GL_TEXTURE_2D, ttf.texture);
        //  lod, internal format,                 border
        //glTexImage2D(GL_TEXTURE_2D, 0, 4, ttf.textsurf->w, ttf.textsurf->h, 0,
        //    GL_BGRA, GL_UNSIGNED_BYTE, ttf.textsurf->pixels);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
            GL_BGRA, GL_UNSIGNED_BYTE, ttf.fmtsurf->pixels);

        if (!ttf.generated) {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
      }

      ttf.generated = true;
    }

    glPushMatrix();
    glEnable(GL_BLEND);
    //done in init: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, ttf.texture);
    glNormal3f(0, 0, 1);
    glColor3f(ttf.color.r/255.0, ttf.color.g/255.0, ttf.color.b/255.0);

    glTranslatef(g.x, g.y, g.z);
    glTranslatef(0, 0, g.pivotZ); // change pivot point
    glRotatef(g.angleX, 1, 0, 0);
    glRotatef(g.angleY, 0, 1, 0);
    glRotatef(g.angleZ, 0, 0, 1);
    glTranslatef(0, 0, -g.pivotZ); // change pivot point
    glScalef(0.5, 0.5, 0.5); // get higher resolution font by choosing large fontsize then halving rendering
    glScalef(g.scaleX, g.scaleY, g.scaleZ);

    float sizeX = 1, sizeY = 0.1, Z = -1;

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(-sizeX, -sizeY, Z);
    glTexCoord2f(1.0, 1.0); glVertex3f(sizeX, -sizeY, Z);
    glTexCoord2f(1.0, 0.0); glVertex3f(sizeX, sizeY, Z);
    glTexCoord2f(0.0, 0.0); glVertex3f(-sizeX, sizeY, Z);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glPopMatrix();

  }
}

void GraphicsOpenGL::drawViewbox(double posX, double posY, double scale, double thumbOffsetX, double thumbOffsetY, double thumbScale, float r, float g, float b, float a)
  // TODO need to take size of other's window as parameter and calculate border accordingly
{
  //glPopMatrix();
  //glPushMatrix();
  //glTranslatef(0, 0, -128);
  //glScalef(1/scale, 1/scale, 0);
  //glTranslatef(-500, -500, 0);
  /*glLineWidth(2);
  glColor4f(1, 1, 0, 1);
  glBegin(GL_LINE_STRIP);
  glVertex3f(-posX + border.left, -posY + border.bottom, 0);
  glVertex3f(-posX + border.right, -posY + border.bottom, 0);
  glVertex3f(-posX + border.right, -posY + border.top, 0);
  glVertex3f(-posX + border.left, -posY + border.top, 0);
  glVertex3f(-posX + border.left, -posY + border.bottom, 0);
  glEnd();*/

  glPushMatrix();

  glScalef(thumbScale, thumbScale, 0);
  glScalef(1/scale, 1/scale, 0);
  glTranslatef(-posX*scale+thumbOffsetX*scale, -posY*scale+thumbOffsetY*scale, 0);

  // draw viewbox
  glLineWidth(2);
  glColor4f(r, g, b, a);
  glBegin(GL_LINE_STRIP);
  glVertex3f(border.left, border.bottom, 0);
  glVertex3f(border.right, border.bottom, 0);
  glVertex3f(border.right, border.top, 0);
  glVertex3f(border.left, border.top, 0);
  glVertex3f(border.left, border.bottom, 0);
  glEnd();

  glPopMatrix();

}

void GraphicsOpenGL::drawStop()
{
  glPopMatrix();
  //glEnable(GL_DEPTH_TEST);
}

void GraphicsOpenGL::drawThumbStop()
{
  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
}

void GraphicsOpenGL::refresh()
{
  window->refresh();
}

float GraphicsOpenGL::getWidth()
{
  return border.right - border.left;
}

int GraphicsOpenGL::getWindowWidth()
{
  return window->getWidth();
}

int GraphicsOpenGL::getWindowHeight()
{
  return window->getHeight();
}

