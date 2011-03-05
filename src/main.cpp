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
#ifndef _MSC_VER
  #include <unistd.h>
  #include <getopt.h>
#endif
#include <iostream>
#include "servercontrol.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Controller *controller;

void catchInterrupt(int sig)
{
  controller->gameover();
  if (sig == SIGABRT) cout << "Program aborted." << endl;
  exit(0);
}

void processCLAs(int argc, char** argv, Args &args)
{
  struct option options[] = {
    {"dontGrab", no_argument, 0, 'd'},
    {"fullscreen", no_argument, 0, 'f'},
    {"help", no_argument, 0, 'h'},
    {"ipaddress", required_argument, 0, 'i'},
    {"quiet", no_argument, 0, 'q'},
    {"server", no_argument, 0, 's'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
  };

  int optionIndex = 0;
  int c = 0;

  // returns EOF (-1) when reaches end of CLAs
  while ((c = getopt_long(argc, argv, "dfhi:qsv", options, &optionIndex)) != EOF) {
    switch (c) {
      case 'd': // don't grab keyboard
        args.dontGrab = true;
        break;
      case 'f':
        args.fullscreen = true;
        break;
      case 'h': // help
        cout << "Usage: ./net [options]" << endl;
        cout << "Options:" << endl;
        cout << "  -d  --dontGrab" << endl;
        cout << "    don't grab keyboard" << endl;
        cout << "  -f  --fullscreen" << endl;
        cout << "    open in fullscreen mode" << endl;
        cout << "  -h  --help" << endl;
        cout << "    display this help" << endl;
        exit(0);
        break;
      case 'i':
        args.ip = optarg;
        break;
      case 'q':
        if (args.verbosity == VERBOSE_LOUD) args.verbosity = VERBOSE_NORMAL; // got v as well
        else args.verbosity = VERBOSE_QUIET;
        break;
      case 's':
        args.serv = true; // can't use SERVER cos that's defined in server.h
        break;
      case 'v':
        if (args.verbosity == VERBOSE_QUIET) args.verbosity = VERBOSE_NORMAL; // got q as well
        else args.verbosity = VERBOSE_LOUD;
        break;
      case '?': // error
        cerr << "For usage instructions use './geo -h' or './geo --help'" << endl;
        break;
    }
  }

}

int main(int argc, char** argv)
{
  signal(SIGINT, &catchInterrupt);
  signal(SIGABRT, &catchInterrupt);

  Args args;

  processCLAs(argc, argv, args);

  if (!args.serv) controller = new Clientcontrol;
  else controller = new Servercontrol;
  
  controller->init(args);
  controller->go();
  controller->gameover();

  delete controller;

  return 0;
}
