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

#include "clientcontrol.h"
#include "graphicsopengl.h"
#include "motlab/server.h"
#include "player.h"

#ifdef _MSC_VER
  #include <windows.h> // for winbase
  #include <winbase.h> // for Sleep
#endif

Clientcontrol::Clientcontrol()
{
  graphics = NULL;

  keys = 0, keysOld = 0;
  SERV = false;
  //input = new InputX11; if use this then delete in gameover

  for (int i = 0; i < MAX_CLIENTS; i++) {
    user[i].x = 0, user[i].y = 0, user[i].zoom = 0;
  }

  myId = -1;
  graphicsHide = false; // TODO make CLA

  mousesensitivity = 0.2;

  // graphicsHide and graphicsActive:
  // graphicsActive = there's a window, can take input etc.
  // then can set graphicsHide to true to not draw (e.g. only view in server)
  // graphicsActive = false means no window or graphics at all
}

Clientcontrol::~Clientcontrol()
{
  if (graphics != NULL) {
    graphics->kill();
    delete graphics;
    std::cout << "Deleted graphics." << std::endl;
  }
}

void Clientcontrol::init(Args &args)
// TODO removeint port, std::string readPath, std::string writePath, std::string mapfile, verboseEnum verbosity, char* ip, bool dontGrab, bool fullscreen, int polygonMax)
{
  initShared(args.verbosity, args.fullscreen); // init everything shared between client and server

  if (args.graphicsActive) {
    graphicsActive = true;
    graphics = new GraphicsOpenGL;
    out << VERBOSE_NORMAL << "Initialising graphics...\n";
    graphics->init(out, graphics->makeWindowInfo(0, 0, 100, 100, true, true, 60, 24, args.fullscreen, "Title"),
          "/usr/share/fonts/bitstream-vera/Vera.ttf", 42);
  }else graphicsActive = false;

  client.init(out, args.port, net.getFlagsize(), net.getUnitsize());
  level.init(out, *graphics);
  //map.init(out, *graphics, polygonMax);

/*#ifdef _MSC_VER
  if (mapfile == "") map.load("C:\\temp\\trev\\15152304-MM-topo_00.gml");
  else map.load(mapfile);
#else
  map.load("/usr/not-backed-up/work/map/15152304-MM-topo_01.gml");
  //map.load("C:\\temp\\trev\\18492304-MM-topo_00.gml"); // ripon
#endif*/

  out << VERBOSE_QUIET << "Using ip address: " << args.ip << '\n';

  if (!client.openConnection(args.ip.c_str())) {
    gameover();
    std::cerr << "Could not connect to server at address: " << args.ip << std::endl;
    exit(1);
  }

  if (!args.dontGrab) input.grab();

  // for now just keep these at /tmp
  //readPath = "/tmp";
  //writePath = "/tmp";

  //transfercontrol.setReadPath(readPath);
  //transfercontrol.setWritePath(writePath);

  soundDev.initOutput(out);
  talk.initOutput(out);
  talk.setSoundDev(soundDev);
  if (soundDev.grab() && talk.openCaptureDevice()) {
    talk.setPlayDevice(soundDev.getPlayDevice());
    talk.setCaptureDevice(soundDev.getCaptureDevice());
    talk.initStream();
    out << VERBOSE_LOUD << "Initialised stream.\n";
    //talk.captureStart(); // use this if not using key to talk
  }
  
  out << VERBOSE_QUIET << "Finished init.\n";
}

void Clientcontrol::gameover()
{
  if (talk.getCapturing()) talk.captureStop();
  if (client.getConnected()) client.closeConnection();
  if (input.getGrabbed()) input.release(); // release keyboard

  //delete input;

  gameoverShared();
}

void Clientcontrol::go()
  // main loop
{
  // client stats
  timer.update(); // to set current time for stats
  timeval statProgramStart = timer.getCurrent();

  // loop frequencies in Hz (calls per second)
  // these are not shared (i.e. not in controller.cpp) - might want them different on client and server
  // (not sure why, but might!) - also most aren't relevant to server
  int inputfreq = 20; // if too low then press and release will be processed in same loop and key will be ignored
  int networkfreq = 30; // if too low then graphics appear jerky without dead reckoning
  int physicsfreq = 100;
  int graphicsfreq = 60; // was 60
  int soundfreq = 40;
  int transferfreq = 10;

  int inputdelay = (int) round(1000000.0/inputfreq); // delay in microseconds
  int networkdelay = (int) round(1000000.0/networkfreq);
  int physicsdelay = (int) round(1000000.0/physicsfreq);
  int graphicsdelay = (int) round(1000000.0/graphicsfreq);
  int sounddelay = (int) round(1000000.0/soundfreq);
  int transferdelay = (int) round(1000000.0/transferfreq);

  timeval inputtime, networktime, physicstime, graphicstime, soundtime, transfertime;

  // timer updated above
  inputtime = timer.getCurrent();
  networktime = timer.getCurrent();
  physicstime = timer.getCurrent();
  graphicstime = timer.getCurrent();
  soundtime = timer.getCurrent();
  transfertime = timer.getCurrent();

  while (!(keys & KEYS_QUIT) && client.getConnected()) {
    timer.update();

    doloop(inputdelay, inputtime, &Clientcontrol::inputloop);
    doloop(networkdelay, networktime, &Clientcontrol::networkloop);
    doloop(physicsdelay, physicstime, &Clientcontrol::physicsloop);
    if (graphicsActive) doloop(graphicsdelay, graphicstime, &Clientcontrol::graphicsloop);
    doloop(sounddelay, soundtime, &Clientcontrol::soundloop);
    doloop(transferdelay, transfertime, &Clientcontrol::transferloop);

#ifdef _MSC_VER
	Sleep(1); // bit longer
#else
    usleep(1000);
#endif
  }

  timeval statProgramDuration = timer.elapsed(statProgramStart);
  double statProgramDurationDbl = statProgramDuration.tv_sec + statProgramDuration.tv_usec / 1000000.0;
  int bytesRecv = client.getStatRecvd();
  int bytesSend = client.getStatSent();
  int total = bytesRecv + bytesSend;
  double rate = total / statProgramDurationDbl;
  std::cerr << "Stats.\nRecv: " << bytesRecv << "\nSent: " << bytesSend << "\nTotal: " << total
    << "\nDuration: " << statProgramDurationDbl << "\nRate: " << rate << "\n";
}

void Clientcontrol::doloop(int delay, timeval &lasttime, void (Clientcontrol::*loopPtr) ())
{
  timeval elapsed;

  elapsed = timer.elapsed(lasttime);

  if (elapsed.tv_sec > 0 || elapsed.tv_usec > delay) {
    lasttime = timer.getCurrent();
    sync = elapsed.tv_sec + elapsed.tv_usec / 1000000.0;
    (*this.*loopPtr)();
  }
}

void Clientcontrol::inputloop()
{
  int mousexrel = 0, mouseyrel = 0;
  static int oldmousexrel = 0, oldmouseyrel = 0; // TODO remove static, make member vars
  if (input.getGrabbed()) keys = input.check(keys, mousexrel, mouseyrel, graphics->getWindowWidth(), graphics->getWindowHeight());

  if (keys != keysOld) {
    // if keys have changed, send them to server

    if (keys & KEYS_TALK) {
      if (!talk.getCapturing()) talk.captureStart();
    }else{
      if (talk.getCapturing()) talk.captureStop();
    }

    keysOld = keys;

    // make unit
    unit.keys.flag = UNIT_KEYS;
    unit.keys.id = myId;
    unit.keys.bits = keys;

    net.addUnit(unit, client);

  }

  if (mousexrel != oldmousexrel || mouseyrel != oldmouseyrel) {
    oldmousexrel = mousexrel;
    oldmouseyrel = mouseyrel;
    geo::Vector rot;
    rot.x = mouseyrel * mousesensitivity;
    rot.y = mousexrel * mousesensitivity;
    rot.z = 0;
    if (myId > -1 && myId < players) player[myId].turn(rot);
  }

  // auto slope test
  //static int rotDir = 1;
  //if (rand() % 10 == 0) rotDir = -rotDir;
  //geo::Vector rot(0, 0, rotDir);
  //if (myId > -1 && myId < players) player[myId].turn(rot);

}

void Clientcontrol::process(Unit unit)
//void Curses::process(Unit unit, Client &client, Server &server, Talk &talk, int keyset[])
  // process data unit
{
  switch (unit.flag) {
    case UNIT_ASSIGN:
      myId = unit.assign.id;
      out << VERBOSE_NORMAL << "Received my client ID: " << myId << '\n';
      users++;
      if (myId < 0) {
        out << VERBOSE_LOUD << "Error with assigned ID: " << myId << '\n';
      }
      break;
    case UNIT_AUDIO:
      out << VERBOSE_LOUD << "Received audio chunk for ID: " << unit.audio.id << '\n';
      talk.receive(unit.audio.data);
      // see if buffer is filling up...
      if (talk.getChunksRecvd() > 2) out << VERBOSE_LOUD << "Buffered " << talk.getChunksRecvd() << " chunks\n"; // TODO remove this
      break;
    case UNIT_GENERIC:
      // used for deactivating bullets for now
      if (unit.generic.objectid < 0 || unit.generic.objectid > BULLETS_MAX - 1) {
        out << VERBOSE_LOUD << "Error: generic unit, bad id: " << unit.generic.objectid << "\n";
      }else{
        // pos will have been received first
        int hit = level.checkCollisionSimple(bullet[unit.generic.objectid].getPos());
        if (hit == 0) {
          out << VERBOSE_LOUD << "--------------Error: we're out of sync. No hit here! bulletid: " << unit.generic.objectid << "\n";
          std::cout << bullet[unit.generic.objectid].getPos() << std::endl;
        }
        explode(bullet[unit.generic.objectid].getPos());
        bullet[unit.generic.objectid].setActive(false);
        out << VERBOSE_QUIET << "bullet deactivated. bulletid: " << unit.generic.objectid << "\n";
      }
      break;
    case UNIT_LOGOFF:
      if (users > 0) {
        if (myId == unit.logoff.id) {
          out << VERBOSE_LOUD << "Error, server says I logged off!\n";
        }else{
          if (myId > unit.logoff.id) myId--;
          //for (int i = unit.logoff.id; i < users - 1; i++) {
            //player[i] = player[i+1]; // when new users logon, they are reset in curses.cpp, see checkNewConnections
          //}
        }
        users--;
      }
      else out << VERBOSE_LOUD << "Error, logoff received but no one left!\n";
      break;
    case UNIT_NEWCLIENT:
      if (unit.newclient.id > -1 && unit.newclient.id < MAX_CLIENTS) {
        users++;
        //player[unit.newclient.id].reset(badcontrol); // not entirely necessary for client, cos just get info from server
      }else{
        out << VERBOSE_LOUD << "Invalid ID for new client\n";
      }
      break;
    case UNIT_POSITION:
      if (unit.position.id > IDHACK_PLAYERPOS_MIN - 1 && unit.position.id < IDHACK_PLAYERPOS_MAX) { // was > -1 and < players
        player[unit.position.id].setX(unit.position.x);
        player[unit.position.id].setY(unit.position.y);
        player[unit.position.id].setZ(unit.position.z);
        //user[unit.position.id].x = unit.position.x;
        //user[unit.position.id].y = unit.position.y;
        //user[unit.position.id].zoom = unit.position.z;
      //map.setUser(unit.position.id, unit.position.x, unit.position.y, unit.position.z);

      }else if (unit.position.id > IDHACK_PLAYERROT_MIN - 1 && unit.position.id < IDHACK_PLAYERROT_MAX) { // was 99 to players+100
        geo::Vector rot(unit.position.x, unit.position.y, unit.position.z);
        player[unit.position.id - IDHACK_PLAYERROT_MIN].setRot(rot);

      }else if (unit.position.id > IDHACK_BULLETPOS_MIN - 1 && unit.position.id < IDHACK_BULLETPOS_MAX) {
        int id = unit.position.id - IDHACK_BULLETPOS_MIN;
        bullet[id].setX(unit.position.x);
        bullet[id].setY(unit.position.y);
        bullet[id].setZ(unit.position.z);
        out << VERBOSE_QUIET << "received bullet pos for bulletid: " << id << "\n";

      }else if (unit.position.id > IDHACK_BULLETROT_MIN - 1 && unit.position.id < IDHACK_BULLETROT_MAX) {
        geo::Vector rot(unit.position.x, unit.position.y, unit.position.z);
        int id = unit.position.id - IDHACK_BULLETROT_MIN;
        bullet[id].setRot(rot);
        bullet[id].setMove(bulletvec, level.getRot());
        bullet[id].setActive(true);
        out << VERBOSE_QUIET << "created bullet: " << id << "\n";

      }else if (unit.position.id == IDHACK_CREATE) {
        // create block
        geo::Vector pos(unit.position.x, unit.position.y, unit.position.z);
        level.createBlock(pos);

      }else if (unit.position.id == IDHACK_LEVELROT) {
        geo::Vector pos(unit.position.x, unit.position.y, unit.position.z);
        level.setRot(pos);

      }else out << VERBOSE_LOUD << "Error with position ID - out of range: " << unit.position.id << "\n";

      // temporary output to see what's going on
      //std::cout << "Received position: " << unit.position.x << ", " << unit.position.y << ", " << unit.position.z << std::endl;
      break;
    case UNIT_TRANSFER:
      // store chunk
      if (!transfercontrol.receive(unit)) {
        // send finished transfer, load picture
        Unit unit2;
        unit2.flag = UNIT_TRANSFERFIN;
        unit2.transferfin.to = unit.transfer.from;
        unit2.transferfin.from = unit.transfer.to;
        unit2.transferfin.id = unit.transfer.id;
        net.addUnit(unit2, client);

        //picturecontrol.loadRemote(unit.transfer.from, unit.transfer.id, unit.transfer.filename);
      }
      break;
    case UNIT_TRANSFERFIN:
      if (unit.transferfin.to > -1 && unit.transferfin.to < MAX_CLIENTS) {
        //if (unit.transferfin.id > -1 && unit.transferfin.id < picturecontrol.getPiclen(unit.transferfin.to)) {
        //picturecontrol.incrementLoaded(unit.transferfin.to, unit.transferfin.id);
        //}else{
        //out << VERBOSE_LOUD << "TransferFIN: error with id: " << unit.transferfin.id << '\n';
        //}
      }else{
        out << VERBOSE_LOUD << "TransferFIN: error with to: " << unit.transferfin.to << '\n';
      }
      break;
    case UNIT_TRANSFERREQ:
      transfercontrol.send(unit.transferreq.to, unit.transferreq.from, unit.transferreq.id, unit.transferreq.filename, net, client);
      break;
    default:
      out << VERBOSE_LOUD << "Error, flag not found: " << unit.flag << '\n';
  }
}

void Clientcontrol::networkloop()
{
  client.doSelect();
  unit = client.recvDataUnit(net);

  while (unit.flag > -1) {
    process(unit);
    client.doSelect();
    unit = client.recvDataUnit(net);
  }

  // Send player orientation
  if (myId > -1 && myId < players) {
    static geo::Vector oldrot; // TODO make member var
    geo::Vector rot = player[myId].getRot();

    if (rot != oldrot) {
      oldrot = rot;
      unit.position.flag = UNIT_POSITION;
      unit.position.id = myId + 100;
      unit.position.x = rot.x;
      unit.position.y = rot.y;
      unit.position.z = rot.z;

      net.addUnit(unit, client);
    }

  }

  client.sendData();
}

void Clientcontrol::physicsloop()
{
  // do any local calculations (e.g. visual effects; moving objects that have no input or randomness)
  //for (int i = 0; i < users; i++) user[i].local(sync);
  //picturecontrol.local(sync);
  for (int i = 0; i < BULLETS_MAX; i++)
    if (bullet[i].getActive()) bullet[i].moveSimple(level, sync);
}

void Clientcontrol::graphicsloop()
{
  // important the way this is done (if using curses):
  // graphics: draw->refresh->clear, then status changes in other loops
  // this means output is cleared in the correct location, and avoids problems of
  // clear->draw->refresh, then status change: deactivate which would mean deactivated
  // objects are not cleared

  if (!graphicsHide && myId > -1 && myId < players) {

    graphics->drawStart();
    //map.draw(false); // TODO remove
    level.draw(player[myId].getPos(), player[myId].getRot(), bullet);

    //for (int i = 0; i < users; i++) user[i].draw();

    //picturecontrol.draw(users);

    graphics->drawStop();

    //graphics->drawThumbStart();
    /*map.draw(true);

    // colours
    float r = 1, g = 1, b = 0, a = 1;
    if (myId == 1) r = 0, g = 1, b = 1, a = 1;

    // thumbnail x, y, zoom
    float tx = -500, ty = -500, tzoom = 0.2;
    graphics->drawViewbox(map.getX(), map.getY(), map.getZoom(), tx, ty, tzoom, r, g, b, a); // for me

    if (myId == 0) r = 0, g = 1, b = 1, a = 1;
    else r = 1, g = 1, b = 0, a = 1;

    for (int i = 0; i < users; i++) {
      graphics->drawViewbox(user[i].x, user[i].y, user[i].zoom, tx, ty, tzoom, r, g, b, a);
    }

    graphics->drawThumbStop();
  */

    out.refreshScreen();
    graphics->refresh();

    // for curses
    //for (int i = 0; i < users; i++) user[i].clear();
  }

}

void Clientcontrol::soundloop()
{
  if (talk.getCapturing()) talk.capture(myId, net, client);
  if (soundDev.checkPlayContext()) talk.update();
}

void Clientcontrol::transferloop()
// deal with file transfer
{
  //NOstatic bool loadedpics = false;

  /*if (myId > -1 && !loadedpics) {
    out << VERBOSE_LOUD << "Calling load...\n";
    picturecontrol.load(myId, net, client); // loads if path was set
    loadedpics = true;
  }*/

  //out << VERBOSE_LOUD << "Calling fetch\n";
  //picturecontrol.fetch(myId, net, client); // deal with fetching

  // net files transmission requests using startTransmission()
  //out << VERBOSE_LOUD << "Calling transfer.go\n";
  transfercontrol.go(net, client); // process transactions

  // server passes on data straight to client, which calls transfercontrol.receive() from net.cpp
  //out << VERBOSE_LOUD << "Finished transferloop\n";
}

