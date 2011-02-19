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

#ifndef TIME
#define TIME

#ifndef _MSC_VER
#include <sys/time.h>
#else
  #include < time.h >
  #include <winsock.h> // for timeval
#endif

#define MICROSECS_PER_FRAME 16666.6666667 // 60 FPS assumed, * 1000000
#define SECS_PER_FRAME 0.0166666666667

class Timer {
  private:
    // time of last frame
    // current time
    timeval last, current;

  public:
    Timer();

    timeval difference(timeval, timeval); // t1 - t2
    timeval elapsed(timeval); // calculate time elapsed since timeval
    timeval getCurrent();

    double update(); // call every frame, returns speed multiplier
};

#endif
