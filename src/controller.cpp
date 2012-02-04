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

#include "controller.h"
#include "bullet.h"
#include "motlab/talk.h"
#include "player.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Controller::Controller()
{
  //graphics = new GraphicsOpenGL();

  graphicsActive = true; // will be set from args

  SERV = false;

  //flagsize = 2; // number of bytes for flag
  // note if change this then need to change uint16_t

  users = 0;
  
  sync = 0; // all operations should multiply, and allow for this being zero

  //player = NULL;
  players = 5; // to start with
  player = new Player[players];

  bullet = new Bullet[BULLETS_MAX];
  bulletvec.x = 0; bulletvec.y = 0; bulletvec.z = -0.5;
}

Controller::~Controller()
{
  if (player != NULL) {
    delete [] player;
    cout << "Deleted players." << endl;
  }

  delete [] bullet;
}

void Controller::gameoverShared()
{
  out.endWin();

  cout << "Bye bye!" << endl;
}

void Controller::initShared(verboseEnum verbosity, bool fullscreen)
  // initialisation that's the same for client and server
{
  out.setVerbosity(verbosity);
  out.init();

  out.setCursor(0);

  transfercontrol.init(out); // TODO remove?

  // network TODO tidy
  net.init(out);// flagsize, unitsize, MAX_CLIENTS);
  Talk talk; // just for getting chunk bytes
  net.setAudioDataSize(talk.getChunkBytes()); // TODO when use UDP remove this
}

void Controller::explode(geo::Vector centre)
{
  float radius = 0.6;
  // TODO precalculate which blocks are effected

  // go through from centre
  for (float z = centre.z - radius; z < centre.z + radius; z+=0.1) {
    for (float y = centre.y - radius; y < centre.y + radius; y+=0.1) {
      for (float x = centre.x - radius; x < centre.x + radius; x+=0.1) {
        // is it within radius, i.e. directly
        geo::Vector current(x, y, z);
        geo::Vector distvec = current - centre;
        float dist = sqrt(distvec.x * distvec.x + distvec.y * distvec.y + distvec.z * distvec.z);
        if (dist < radius) {
          level.checkCollisionSimple(current);
        }
      }
    }
  }

}

