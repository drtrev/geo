#include "bot.h"
#include "player.h"
#include "bullet.h"
#include <cstdlib>

Bot::Bot()
{
  graphics = NULL;
  myId = -1;
  graphicsActive = false;
}

Bot::~Bot()
{
}

void Bot::init(Args &args)
{
  initShared(args.verbosity, args.fullscreen);
  client.init(out, args.port, net.getFlagsize(), net.getUnitsize());
  level.init(out);

  out << VERBOSE_QUIET << "Using ip address: " << args.ip << '\n';

  if (!client.openConnection(args.ip.c_str())) {
    gameover();
    std::cerr << "Could not connect to server at address: " << args.ip << std::endl;
    std::exit(1);
  }

  out << VERBOSE_QUIET << "Finished init.\n";
}

void Bot::go()
{
}

void Bot::gameover()
{
  if (client.getConnected()) client.closeConnection();
  gameoverShared();
}

void Bot::networkloop()
{
  client.doSelect();
  unit = client.recvDataUnit(net);

  while (unit.flag > -1) {
    process(unit);
    client.doSelect();
    unit = client.recvDataUnit(net);
  }

  client.sendData();
}

void Bot::process(Unit unit)
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
      //out << VERBOSE_LOUD << "Received audio chunk for ID: " << unit.audio.id << '\n';
      //talk.receive(unit.audio.data);
      // see if buffer is filling up...
      //if (talk.getChunksRecvd() > 2) out << VERBOSE_LOUD << "Buffered " << talk.getChunksRecvd() << " chunks\n"; // TODO remove this
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
      /*if (unit.newclient.id > -1 && unit.newclient.id < MAX_CLIENTS) {
        users++;
        //player[unit.newclient.id].reset(badcontrol); // not entirely necessary for client, cos just get info from server
      }else{
        out << VERBOSE_LOUD << "Invalid ID for new client\n";
      }*/
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
        std::cout << "Created block" << std::endl;

      }else if (unit.position.id == IDHACK_LEVELROT) {
        geo::Vector pos(unit.position.x, unit.position.y, unit.position.z);
        level.setRot(pos);

      }else if (unit.position.id == IDHACK_MAKESLOPE) {
        geo::Vector temp(unit.position.x, unit.position.y, unit.position.z);
        LevelNames::Block* block = level.getBlock(temp, 2);
        if (block != NULL) {
          std::cout << "MAKESLOPE: block state: ";
          if (block->state == LevelNames::BLOCK_EMPTY) std::cout << "BLOCK_EMPTY";
          else if (block->state == LevelNames::BLOCK_SOLID) std::cout << "BLOCK_SOLID";
          else if (block->state == LevelNames::BLOCK_SLOPE) std::cout << "BLOCK_SLOPE";
          else std::cout << block->state;
          std::cout << std::endl;

          block->state = LevelNames::BLOCK_SLOPE;
        }else out << VERBOSE_LOUD << "Makeslope received but block not found\n";
      }//else out << VERBOSE_LOUD << "Error with position ID - out of range: " << unit.position.id << "\n";

      // temporary output to see what's going on
      //std::cout << "Received position: " << unit.position.x << ", " << unit.position.y << ", " << unit.position.z << std::endl;
      break;
    case UNIT_TRANSFER:
      // store chunk
      /*if (!transfercontrol.receive(unit)) {
        // send finished transfer, load picture
        Unit unit2;
        unit2.flag = UNIT_TRANSFERFIN;
        unit2.transferfin.to = unit.transfer.from;
        unit2.transferfin.from = unit.transfer.to;
        unit2.transferfin.id = unit.transfer.id;
        net.addUnit(unit2, client);

        //picturecontrol.loadRemote(unit.transfer.from, unit.transfer.id, unit.transfer.filename);
      }*/
      break;
    case UNIT_TRANSFERFIN:
      /*if (unit.transferfin.to > -1 && unit.transferfin.to < MAX_CLIENTS) {
        //if (unit.transferfin.id > -1 && unit.transferfin.id < picturecontrol.getPiclen(unit.transferfin.to)) {
        //picturecontrol.incrementLoaded(unit.transferfin.to, unit.transferfin.id);
        //}else{
        //out << VERBOSE_LOUD << "TransferFIN: error with id: " << unit.transferfin.id << '\n';
        //}
      }else{
        out << VERBOSE_LOUD << "TransferFIN: error with to: " << unit.transferfin.to << '\n';
      }*/
      break;
    case UNIT_TRANSFERREQ:
      //transfercontrol.send(unit.transferreq.to, unit.transferreq.from, unit.transferreq.id, unit.transferreq.filename, net, client);
      break;
    default:
      out << VERBOSE_LOUD << "Error, flag not found: " << unit.flag << '\n';
  }
}
