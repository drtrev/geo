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

#ifndef DIRECTORY
#define DIRECTORY

#include <string>
#include <vector>

/**
 * A class to read a directory and return vectors of filenames.
 *
 * This can be used for file transfer, and is currently only implemented on
 * linux as an example.
 */
class Directory {
  private:

  public:
    std::vector <std::string> getPics(std::string); /**< Get a list of the image files in a directory. */
};

#endif
