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

#ifndef CLIENTCONTROL
#define CLIENTCONTROL

#include "motlab/client.h"
#include "controller.h"
//#include "inputX11.h"
#include "inputSDL.h"
#include "map.h"
#include "motlab/dev.h"
#include "motlab/talk.h"

/**
 * Client control class.
 *
 * Control keyboard input, audio capture, graphics output, file transfer.
 */
class Clientcontrol : public Controller {
  private:
    Client client; /**< Underlying client instance for low level operations. */
    //InputX11 input; // doesn't seem to work in fullscreen
    InputSDL input; /**< Input object. */
    SoundDev soundDev; /**< Sound device instance. */
    Talk talk; /**< Talk instance, used for capturing audio from the microphone. */

    Map map; /**< Instance of a 'map'. Could be any polygon based data or raster data (textured). */

    /** \var keys
     * Store which keys are being held down as a bitfield. Each bit represents a
     * key (on/off).
     * \see Input */
    /** \var keysOld
     * Store which keys were being held down in the previous input loop as a
     * bitfield. */
    int keys, keysOld; // which keys are being held down, each bit represents a key, see input.h

    /** Store the coords of other users, for drawing their viewboxes. */
    struct User {
      float x; /**< The x coordinate. */
      float y; /**< The y coordinate. */
      float zoom; /**< The scale factor. */
    } user[MAX_CLIENTS];

    int myId;  /**< My client ID. */

    bool graphicsHide; /**< Don't draw graphics. */

  public:
    Clientcontrol();  /**< Constructor. */
    ~Clientcontrol(); /**< Destructor. */

    /**
     * Initialise the client controller.
     *
     * \param port the port number to use for connecting to the server.
     * \param readPath the path to read files from (when using Motlab's transfer
     * functionality).
     * \param writePath the path to write files to (when using Motlab's transfer
     * functionality).
     * \param mapfile the datafile to load (not used in this example).
     * \param verbosity the verbosity level of the output.
     * \param ip the IP address of the server
     * \param dontGrab specify whether or not to grab the keyboard (relevant if using Xlib).
     * \param fullscreen specify fullscreen on or off.
     * \param polygonMax specify maximum number of polygons to read from
     * datafile (not used in this example).
     */
    void init(Args &args);
    void gameover(); /**< Close connection to server, stop capturing audio, release keyboard (if grabbed), end output, kill graphics, and say "Bye bye!". */

    void go(); /**< Run through the various control loops at set frequencies. */

    /**
     * Execute the specified loop.
     *
     * \param delay the delay in microseconds before this loop should be called.
     * \param lasttime the last time that this loop was called (so it can
     * calculate if it's time to call it again yet).
     * \param loopPtr the pointer to a control loop (e.g. input, graphics,
     * sound).
     */
    void doloop(int delay, timeval &lasttime, void (Clientcontrol::*loopPtr)());

    void inputloop();    /**< The input loop. */
    void process(Unit);  /**< Process a received data unit. Part of the network loop. */
    void networkloop();  /**< Do the network sending and receiving/processing. */
    void physicsloop();  /**< Perform any local calculations, e.g. visual effects, moving objects that have no input or randomness. */
    void graphicsloop(); /**< Display graphics. */
    void soundloop();    /**< Capture audio (if required) and update playback. */
    void transferloop(); /**< Deal with file transfer. */

};

#endif
