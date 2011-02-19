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

#include "directory.h"
#include <cstring>
#ifndef _MSC_VER
  #include "dirent.h"
#endif
#include <iostream>
#include "motlab/network.h"

using std::vector;
using std::string;
using std::cerr;
using std::endl;

vector <string> Directory::getPics(string path)
{
  // read directory path

  vector <string> out;
#ifdef _MSC_VER
  return out;
#else
  DIR *dp;
  struct dirent *ep;

  dp = opendir(path.c_str());

  if (dp != NULL) {
    cerr << "Reading dir: " << path << endl;

    while ((ep = readdir(dp))) {
      //cerr << "Read file: " << ep->d_name << endl;

      if (std::strcmp(ep->d_name, ".") && std::strcmp(ep->d_name, "..")) {
        // if not current or parent directory

        /*string temp = path + ep->d_name; // add on path

        if (temp.size() < MAX_FILENAME_SIZE) {
          cerr << "Got file: " << temp << endl;
          out.push_back(temp);
        }else cerr << "Filename too big: " << temp << endl;*/

        if (strlen(ep->d_name) + 1 < MAX_FILENAME_SIZE) {
          out.push_back(ep->d_name);
          cerr << "Got file: " << out.back() << endl;
        }
      }
    }
    closedir(dp);
  }else{
    cerr << "Error with getPics, path not specified" << endl;
  }

  //out.push_back("../res/mountain.jpg");
  //out.push_back("../res/test.png");

  for (int i = 0; i < (int) out.size(); i++) {
    cerr << out[i] << endl;
  }

  return out;
#endif
}

