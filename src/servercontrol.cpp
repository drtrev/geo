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

Servercontrol::Servercontrol()
{
  graphics = NULL;

  SERV = true;
  keys = 0;
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
      // TODO send out in rhythm, not straight away, i.e. send every netloop
      // this should be only rotation here anyway
      net.addUnitAll(unit, server, unit.position.id - 100); // don't send back to source
      if (unit.position.id > 99 && unit.position.id < players + 100) {
        geo::Vector rot(unit.position.x, unit.position.y, unit.position.z);
        player[unit.position.id - 100].setRot(rot);
        std::cout << "Set player rot to: " << player[0].getRot() << std::endl;
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

  //if (elapsed.tv_sec > 0 || elapsed.tv_usec > 33333) { // was 50000
    //lastSent = timer.getCurrent();

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

void Servercontrol::physicsloop()
{
  for (int i = 0; i < users; i++) {
    player[i].input(keyset[i], sync);
    player[i].move(level, sync);

    //picturecontrol.input(keyset[i], net, server); // pass input for each player

    // remove next and back (single effect) keys from keyset because they've been dealt with now
    keyset[i] &= ~(KEYS_NEXT | KEYS_BACK);
  }

  //picturecontrol.go(sync); // should do transmission in own loop, like sound loop in client
}

void Servercontrol::graphicsloop()
{
  graphics->drawStart();
  level.draw(player[0].getPos(), player[0].getRot());
  graphics->drawStop();

  out.refreshScreen();
  graphics->refresh();
}

