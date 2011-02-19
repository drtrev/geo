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

#ifndef CURSES
#define CURSES

#include <cmath>
#include <iostream>
#include <signal.h>
#include "timer.h"
#include "outverbose.h"
#include "level.h"
#include "motlab/net.h"
#include "motlab/network.h"
#include "motlab/transfercontrol.h"

#ifdef _MSC_VER
  inline double round(double x) { return floor(x + 0.5); }
#endif

class Client; class Graphics; class Player; class Server;

/**
 * Controller class.
 *
 * Control either the client or server. Objects that are common to both are stored here.
 */
class Controller {
  protected:
    Graphics* graphics; /**< Pointer to instance of graphics object. */
    Net net;            /**< Net instance. */
    Outverbose out;     /**< Output instance. */
    Timer timer;        /**< Timer, for calculating elapsed time. */
    Transfercontrol transfercontrol; /**< Control for transfering files. */

    double sync; /**< Time since last frame, for keeping any movement operations synchronised. */

    bool SERV; /**< Distinguish client from server. This is only needed if the server is using graphics also. */
    Unit unit; /**< Store a data unit. */
    int users; /**< The number of users. */

    Player *player;
    int players;

    Level level;

    bool graphicsActive;

  public:
    Controller();          /**< Constructor. */
    virtual ~Controller(); /**< Destructor. */

    virtual void go() = 0; /**< Perform the control loops. Needs to be implemented by Clientcontrol or Servercontrol. \see Clientcontrol \see Servercontrol */

    void gameoverShared(); /**< Kill everything that's shared by client and server. */
    virtual void gameover() = 0; /**< Kill everything that's relevant to only Client or Server. \see Clientcontrol \see Servercontrol */

    void initShared(verboseEnum, bool, int); /**< Initialise everything that's relevant to client and server. */

    /**
     * Initialise everything.
     *
     * \param port the port number to use for listening or connecting to the server.
     * \param readPath the path to read files from (when using Motlab's transfer
     * functionality).
     * \param writePath the path to write files to (when using Motlab's transfer
     * functionality).
     * \param mapfile the datafile for the client to load (not used in this example).
     * \param verbosity the verbosity level of the output.
     * \param ip the IP address of the server (not used by server itself).
     * \param dontGrab specify whether or not to grab the keyboard (relevant if using Xlib).
     * \param fullscreen specify fullscreen on or off.
     * \param polygonMax specify maximum number of polygons to read from
     * datafile (not used in this example).
     */
    virtual void init(int port, std::string readPath, std::string writePath, std::string mapfile, verboseEnum verbosity, char* ip, bool dontGrab, bool fullscreen, int polygonMax) = 0;

};

#endif
