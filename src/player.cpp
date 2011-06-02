/* This file is part of Geo - A tool for viewing 2D map data using Motlab (a
 * heterogeneous collaborative visualization library).
 *
 * Copyright (C) 2009 Trevor Dodds <trev@comp.leeds.ac.uk>
 *
 * Player is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * Player is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Player.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "player.h"
// TODO cleanup headers, many commented
//#ifdef _MSC_VER
//#include <regex.hpp>
//#else
//#include <boost/regex.hpp>
//#endif

/*#include <cmath>
#include <fstream>
#include "graphics.h"
#include "input.h"
#include <iostream>
#include "outverbose.h"
#include <string>
#include <stdlib.h>

#ifdef _MSC_VER
  #include <stdio.h>
  #define snprintf sprintf_s
#endif

using std::string;
using std::cerr;
using std::endl;
using std::vector;*/

Player::Player()
{
  props.pos.x = 5; props.pos.y = 12; props.pos.z = 5;
  props.power = 2000;
}

Player::~Player()
{
}

