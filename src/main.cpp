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

void processCLAs(int argc, char** argv, string &mapfile, char* ip, verboseEnum &verbosity, bool &dontGrab, bool &fullscreen, bool &SERV, int &polygonMax)
{
#ifndef _MSC_VER
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
  while ((c = getopt_long(argc, argv, "dfhi:n:qsv", options, &optionIndex)) != EOF) {
    switch (c) {
      case 'd': // don't grab keyboard
        dontGrab = true;
        break;
      case 'f':
        fullscreen = true;
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
        if (strlen(optarg) + 1 > 50) {
          cerr << "IP address too long" << endl;
        }else{
          memcpy(ip, optarg, strlen(optarg) + 1);
        }
        break;
      case 'n':
        polygonMax = atoi(optarg);
        break;
      case 'q':
        if (verbosity == VERBOSE_LOUD) verbosity = VERBOSE_NORMAL; // got v as well
        else verbosity = VERBOSE_QUIET;
        break;
      case 's':
        SERV = true; // can't use SERVER cos that's defined in server.h
        break;
      case 'v':
        if (verbosity == VERBOSE_QUIET) verbosity = VERBOSE_NORMAL; // got q as well
        else verbosity = VERBOSE_LOUD;
        break;
      case '?': // error
        cerr << "For usage instructions use './net -h' or './net --help'" << endl;
        break;
    }
  }
#else

	if (argc > 1 && !strcmp(argv[1], "-s")) {
		SERV = true;
	}

	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-h")) {
			cout << "Usage: ./map [options]" << endl;
			cout << "Options:" << endl;
			cout << "  -f" << endl;
			cout << "    open in fullscreen mode" << endl;
			cout << "  -g <filename>" << endl;
			cout << "    filename for GML file to load" << endl;
			cout << "  -h" << endl;
			cout << "    display this help" << endl;
			cout << "  -i <ipaddress>" << endl;
			cout << "    ip address of server" << endl;
			cout << "  -n <integer>" << endl;
			cout << "    maximum number of polygons to load from GML file" << endl;
			cout << "  -s" << endl;
			cout << "    run as server" << endl;
			exit(0);
		}
		if (!strcmp(argv[i], "-f")) {
			fullscreen = true;
		}
		if (!strcmp(argv[i], "-n") && i < argc - 1) {
			polygonMax = atoi(argv[i+1]);
		}
	    if (argc > i + 1 && !strcmp(argv[i], "-i")) {
	      if (strlen(argv[i+1]) + 1 > 50) {
            cerr << "IP address too long" << endl;
          }else{
            memcpy(ip, argv[i+1], strlen(argv[i+1]) + 1);
          }
    	}
		if (argc > i + 1 && !strcmp(argv[i], "-g")) {
			mapfile = argv[i+1];
			cout << "Set GML file as: " << mapfile << endl;
		}
	}
#endif

}

int main(int argc, char** argv)
{
  signal(SIGINT, &catchInterrupt);
  signal(SIGABRT, &catchInterrupt);

  string mapfile = "";
  char localhost[50], ip[50];
  string localstr = "127.0.0.1";
  strncpy(localhost, localstr.c_str(), localstr.length() + 1);
  ip[0] = '\0';
  verboseEnum verbosity = VERBOSE_NORMAL;
  bool dontGrab = false; // grab keyboard
  bool fullscreen = false;
  bool SERV = false;
  int polygonMax = 100000;

  processCLAs(argc, argv, mapfile, ip, verbosity, dontGrab, fullscreen, SERV, polygonMax);
  if (strlen(ip) == 0) memcpy(ip, localhost, strlen(localhost) + 1);

  if (!SERV) controller = new Clientcontrol;
  else controller = new Servercontrol;
  
  int port = 3496;

  string readPath = "", writePath = ""; // not used

  controller->init(port, readPath, writePath, mapfile, verbosity, ip, dontGrab, fullscreen, polygonMax);
  controller->go();
  controller->gameover();

  delete controller;

  return 0;
}
