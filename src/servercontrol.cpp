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

#include "servercontrol.h"
#include "graphicsopengl.h"
#include "motlab/client.h"
#include "player.h"
//#include "graphicscurses.h"
#ifdef _MSC_VER
  #include <windows.h> // for winbase
  #include <winbase.h> // for Sleep
#endif

#include <sstream>

Servercontrol::Servercontrol()
{
  graphics = NULL;

  SERV = true;
  keys = 0;

  statusText = "";
}

Servercontrol::~Servercontrol()
{
  if (graphics != NULL) {
    graphics->kill();
    delete graphics;
    std::cout << "Deleted graphics." << std::endl;
  }
}

void Servercontrol::init(Args &args)
// TODO remove int port, std::string readPath, std::string writePath, std::string mapfile, verboseEnum verbosity, char* ip, bool dontGrab, bool fullscreen, int polygonMax)
  // initialisation. ip and dontGrab are for client, here because init is virtual
{
  initShared(args.verbosity, args.fullscreen);

  if (args.graphicsActive) {
    graphicsActive = true;
    graphics = new GraphicsOpenGL;
    out << VERBOSE_NORMAL << "Initialising graphics...\n";
    graphics->init(out, graphics->makeWindowInfo(0, 0, 100, 100, true, true, 60, 24, args.fullscreen, "Title"),
          "/usr/share/fonts/bitstream-vera/Vera.ttf", 42);
  }else graphicsActive = false;

  level.init(out, *graphics);

  // note number of pending connection requests allowed
  server.init(out, args.port, 10, net.getFlagsize(), net.getUnitsize());
  server.startListening();
}

void Servercontrol::gameover()
{
  server.closeAll(); // closes any that are still active

  gameoverShared();
}

void Servercontrol::go()
{
  for (int i = 0; i < MAX_CLIENTS; i++) keyset[i] = 0;

  lastSent.tv_sec = 0;
  lastSent.tv_usec = 0;
  lastBullet.tv_sec = 0;
  lastBullet.tv_usec = 0;

  // loop frequencies in Hz (calls per second)
  int inputfreq = 20;
  int networkfreq = 30; // was 30 -- if change this then change where the data is sent in networkloop
  int physicsfreq = 100;
  int graphicsfreq = 60; // 60

  int inputdelay = (int) round(1000000.0/inputfreq);
  int networkdelay = (int) round(1000000.0/networkfreq);
  int physicsdelay = (int) round(1000000.0/physicsfreq);
  int graphicsdelay = (int) round(1000000.0/graphicsfreq);

  timeval inputtime, networktime, physicstime, graphicstime;
  timer.update();

  inputtime = timer.getCurrent();
  networktime = timer.getCurrent();
  physicstime = timer.getCurrent();
  graphicstime = timer.getCurrent();

  out << VERBOSE_NORMAL << "Server ready.\n";

  while (!(keys & KEYS_QUIT)) {
    timer.update();

    doloop(inputdelay, inputtime, &Servercontrol::inputloop);
    doloop(networkdelay, networktime, &Servercontrol::networkloop);
    doloop(physicsdelay, physicstime, &Servercontrol::physicsloop);
    if (graphicsActive) doloop(graphicsdelay, graphicstime, &Servercontrol::graphicsloop);

#ifdef _MSC_VER
	Sleep(1); // bit longer
#else
    usleep(1000); // this really isn't long!
#endif
  }

  server.closeAll();
}

void Servercontrol::doloop(int delay, timeval &lasttime, void (Servercontrol::*loopPtr) ())
{
  timeval elapsed;

  elapsed = timer.elapsed(lasttime);

  if (elapsed.tv_sec > 0 || elapsed.tv_usec > delay) {
    // note lasttime is a param, so should be unique for each loop (physics,
    // n/w etc.)
    lasttime = timer.getCurrent();
    sync = elapsed.tv_sec + elapsed.tv_usec / 1000000.0;
    if (rand()%200 == 0) std::cout << "FPS: " << (1/sync) << "\n";
    (*this.*loopPtr)();
  }
}

void Servercontrol::inputloop()
{
  // allows you to exit cleanly when server is running with graphics
  int mousexrel, mouseyrel;
  keys = input.check(keys, mousexrel, mouseyrel, 0, 0); // TODO does this do anything? Surely client deals with input and sends it into keyset?
}

void Servercontrol::sendStatus(int cid)
{
  // send out map
  for (int i = 0; i < players; i++) {
    /*unit.position.flag = UNIT_POSITION;
    unit.position.id = i; // life made easy -- ID is same as array element
    unit.position.x = map[i].getX();
    unit.position.y = map[i].getY();
    unit.position.z = map[i].getZoom();

    net.addUnitAll(unit, server, -1);
*/
    unit.position.flag = UNIT_POSITION;
    unit.position.id = i; // life made easy -- ID is same as array element
    unit.position.x = player[i].getX();
    unit.position.y = player[i].getY();
    unit.position.z = player[i].getZ();

    net.addUnitAll(unit, server, -1);
  }
}

void Servercontrol::process(Unit unit)
  // process data unit
{
  switch (unit.flag) {
    case UNIT_AUDIO:
      net.addUnitAll(unit, server, unit.audio.id);
      break;
    case UNIT_KEYS:
      if (unit.keys.id > -1 && unit.keys.id < users) {
        keyset[unit.keys.id] = unit.keys.bits;
      }
      break;
    case UNIT_POSITION:
      // this should be only rotation here
      if (unit.position.id > IDHACK_PLAYERROT_MIN  - 1 && unit.position.id < IDHACK_PLAYERROT_MAX) {
        int id = unit.position.id - IDHACK_PLAYERROT_MIN;
        geo::Vector rot(unit.position.x, unit.position.y, unit.position.z);
        player[id].setRot(rot);
        net.addUnitAll(unit, server, id); // don't send back to source
        //std::cout << "Set player rot to: " << player[0].getRot() << std::endl;
      }
      break;
    case UNIT_TRANSFER:
      net.addUnit(unit.transfer.to, unit, server);
      break;
    case UNIT_TRANSFERFIN:
      net.addUnitAll(unit, server, unit.transferfin.from); // send out to all so they all addLoaded
      break;
    case UNIT_TRANSFERREQ:
      net.addUnit(unit.transferreq.to, unit, server);
      break;
    default:
      out << VERBOSE_LOUD << "Error, flag not found: " << unit.flag << '\n';
  }
}

void Servercontrol::networkloop()
{
  int logoff = server.checkClosedConnections();

  if (logoff > -1) {
    // get next logoff
    users--;
    unit.logoff.flag = UNIT_LOGOFF;
    unit.logoff.id = logoff;
    net.addUnitAll(unit, server, -1);
    // note logons will only occur during checkNewConn's
  }

  server.doSelect();

  if (server.checkNewConnections(net)) {
    if (users < MAX_CLIENTS) {
      users++;
      // TODO don't send out player positions to bots that don't need it
      sendStatus(users - 1); // id of new client
      out << VERBOSE_NORMAL << "New user added. Total number of users: " << users << "\n";
    }else out << VERBOSE_LOUD << "Error, logon received but too many clients connected!\n";
  }

  server.recvAll();

  unit = server.getDataUnit(net);

  while (unit.flag > -1) {
    process(unit);
    unit = server.getDataUnit(net);
  }


  // sending

  // could delay sending here - uncomment code below to change send frequency

  //timeval elapsed;

  //elapsed = timer.elapsed(lastSent);

  // Watch out: if this is working at 20Hz but receiving end may be running at 30Hz
  // which would mean every so often an update is skipped (overwritten) and
  // would result in jumpy/inconsistent movement!

  //if (elapsed.tv_sec > 0 || elapsed.tv_usec > 33333) { // was 50000 -- eh, usec not big enough here??
    //lastSent = timer.getCurrent();

    // TODO not to bots
    // send map position
    for (int i = 0; i < players; i++) { // need to hack for powerwall -- ID is same as array element fortunately
      if (fabs(player[i].getX() - oldposition[i].x) > 0.001 || fabs(player[i].getY() - oldposition[i].y) > 0.001
          || fabs(player[i].getZ() - oldposition[i].z) > 0.001) {
        // TODO this should be player.getPosition and the Vector should be a
        // global type
        oldposition[i].x = player[i].getX();
        oldposition[i].y = player[i].getY();
        oldposition[i].z = player[i].getZ();

        unit.position.flag = UNIT_POSITION;
        unit.position.id = i; // life made easy -- ID is same as array element
        unit.position.x = player[i].getX();
        unit.position.y = player[i].getY();
        unit.position.z = player[i].getZ();

        net.addUnitAll(unit, server, -1);
      }
    }

  //}

  server.sendAll();
}

int Servercontrol::createBullet(geo::Vector pos, geo::Vector rot)
// both server and client create their own copy of the bullet
// client moves it locally, server will say when it's collided
// returns bullet id for sending out by server, or -1
{
  int nextbull = -1;
  for (int bull = 0; bull < BULLETS_MAX; bull++)
    if (!bullet[bull].getActive()) { nextbull = bull; break; }

  if (nextbull > -1) {
    bullet[nextbull].setPos(pos);
    bullet[nextbull].setRot(rot);
    bullet[nextbull].setMove(bulletvec, level.getRot());
    bullet[nextbull].setActive(true);
    out << VERBOSE_LOUD << "fired: " << nextbull << "\n";
  }

  return nextbull;
}

geo::Vector Servercontrol::roundPos(geo::Vector pos)
{
  // round down to nearest mm
  geo::Vector rounded( ((int) (pos.x*1000)) / 1000.0, ((int) (pos.y*1000)) / 1000.0, ((int) (pos.z*1000)) / 1000.0);
  return rounded;
}

void Servercontrol::physicsloop()
{
  std::ostringstream ss;
  for (int i = 1; i < users; i++) { // TODO this is a hack to avoid bots, i.e. start from i = 1
    player[i].input(keyset[i], level.getRot(), sync);
    player[i].move(level, sync);
    statusText = "Py: ";
    ss << player[i].getAccelY();
    statusText += ss.str();

    if (keyset[i] & KEYS_CONSTRUCT) {
      if (level.createBlock(player[i].getPos())) {
        Unit unit;
        unit.flag = UNIT_POSITION;
        unit.position.id = IDHACK_CREATE;
        unit.position.x = player[i].getPos().x;
        unit.position.y = player[i].getPos().y;
        unit.position.z = player[i].getPos().z;
        net.addUnitAll(unit, server, -1);
      }
    }
    if (keyset[i] & KEYS_FIRE) {
      // check we've delayed enough
      timeval elapsed;
      elapsed = timer.elapsed(lastBullet);

      // TODO note here not to be faster than receiving end (client) is processing the network or
      // if that's rubbish then remove this comment
      //
      if (elapsed.tv_sec > 0 || elapsed.tv_usec > 500000) {
        lastBullet = timer.getCurrent();

        int bullid = createBullet(player[i].getPos(), player[i].getRot());
        if (bullid > -1) {
          // send bullet pos and rot
          // in case we later send out levelrot in a different loop,
          // make sure client has it up to date
          geo::Vector temp = level.getRot();
          Unit unit;
          unit.flag = UNIT_POSITION;
          unit.position.id = IDHACK_LEVELROT;
          unit.position.x = temp.x;
          unit.position.y = temp.y;
          unit.position.z = temp.z;
          net.addUnitAll(unit, server, -1);
          unit.flag = UNIT_POSITION;
          unit.position.id = IDHACK_BULLETPOS_MIN + bullid;
          unit.position.x = player[i].getPos().x;
          unit.position.y = player[i].getPos().y;
          unit.position.z = player[i].getPos().z;
          net.addUnitAll(unit, server, -1);
          unit.flag = UNIT_POSITION;
          unit.position.id = IDHACK_BULLETROT_MIN + bullid;
          unit.position.x = player[i].getRot().x;
          unit.position.y = player[i].getRot().y;
          unit.position.z = player[i].getRot().z;
          net.addUnitAll(unit, server, -1);
          out << VERBOSE_QUIET << "Sent bullet position (created): " << bullid << "\n";
        }
      }
    } // end KEYS_FIRE

    //picturecontrol.input(keyset[i], net, server); // pass input for each player

    // remove next back and fire (single effect) keys from keyset because they've been dealt with now
    // this doesn't work properly, because they're not removed from client and they're sent again when something else is pressed
    //keyset[i] &= ~(KEYS_NEXT | KEYS_BACK | KEYS_FIRE);
  }

  for (int i = 0; i < BULLETS_MAX; i++) {
    if (bullet[i].getActive()) {
      bullet[i].moveSimple(level, sync);

      // if this doesn't send exactly the same position to client then
      // it won't work. Note it's rounded to 4 decimal places in motlab
      // see net.cpp writeFloat2 (and unitToBytes).
      // So here we round to 3 before checking and sending rounded position
      geo::Vector roundedpos = roundPos(bullet[i].getPos());

      //int hit = level.checkCollisionSimple(bullet[i].getPos());
      int hit = level.checkCollisionSimple(roundedpos);
      if (hit > 0) {
        bullet[i].setActive(false);
        Unit unit;
        unit.flag = UNIT_POSITION;
        unit.position.id = IDHACK_BULLETPOS_MIN + i;
        //unit.position.x = bullet[i].getPos().x;
        //unit.position.y = bullet[i].getPos().y;
        //unit.position.z = bullet[i].getPos().z;
        unit.position.x = roundedpos.x;
        unit.position.y = roundedpos.y;
        unit.position.z = roundedpos.z;
        net.addUnitAll(unit, server, -1);
        unit.flag = UNIT_GENERIC;
        unit.generic.from = 0;
        unit.generic.to = 0;
        unit.generic.objectid = i;
        unit.generic.info_i = 0; // deactive
        unit.generic.info_f = 0;
        net.addUnitAll(unit, server, -1);
        //std::cout << "bullet " << i << " hit at pos: ";
        //std::cout << bullet[i].getPos() << std::endl;
        //std::cout << "roundpos: " << roundpos << std::endl;
        explode(roundedpos);
      }
    }
  }

  // for fun
  /*geo::Vector rot(0, 0, 0.1);
  level.turn(rot);
  geo::Vector temp = level.getRot();
  Unit unit;
  unit.flag = UNIT_POSITION;
  unit.position.id = IDHACK_LEVELROT;
  unit.position.x = temp.x;
  unit.position.y = temp.y;
  unit.position.z = temp.z;
  net.addUnitAll(unit, server, -1);*/

  //picturecontrol.go(sync); // should do transmission in own loop, like sound loop in client
}

void Servercontrol::graphicsloop()
{
  graphics->drawStart();
  level.draw(player[0].getPos(), player[0].getRot(), bullet);
  GraphicsInfo g = graphics->defaultInfo();
  g.text = statusText;
  graphics->drawText(g);
  graphics->drawStop();

  out.refreshScreen();
  graphics->refresh();
}

