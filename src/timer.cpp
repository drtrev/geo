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

#include "timer.h"
#include <cstdlib> // for NULL

#ifdef _MSC_VER
  #include <time.h>
//#include <winsock.h>

    #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
    #else
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
    #endif
     
    struct timezone 
    {
      int  tz_minuteswest; /* minutes W of Greenwich */
      int  tz_dsttime;     /* type of dst correction */
    };
     
    int gettimeofday(struct timeval *tv, struct timezone *tz)
    {
      FILETIME ft;
      unsigned __int64 tmpres = 0;
      static int tzflag; // probably shouldn't try to have a shared variable like this!
     
      if (NULL != tv)
      {
        GetSystemTimeAsFileTime(&ft);
     
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
     
        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS; 
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
      }
     
      if (NULL != tz)
      {
        if (!tzflag)
        {
          _tzset();
          tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
      }
     
      return 0;
    }

#endif


Timer::Timer()
{
  gettimeofday(&last, NULL); // initialise last
}

timeval Timer::difference(timeval t1, timeval t2)
// performs t1 - t2
{
  timeval since;

  since.tv_sec = t1.tv_sec - t2.tv_sec;
  since.tv_usec = t1.tv_usec - t2.tv_usec;

  // get rid of negative microseconds
  if (since.tv_usec < 0) {
    since.tv_usec += 1000000;
    since.tv_sec --;
  }

  return since;
}

timeval Timer::elapsed(timeval t)
// calculate time elapsed since t
{
  return difference(current, t);
}

timeval Timer::getCurrent()
{
  return current;
}

double Timer::update()
// call every frame
// returns time since last frame
{
  gettimeofday(&current, NULL);

  // since last frame
  timeval since = difference(current, last);

  // calculate speed multiplier, i.e. if half time has passed compared to
  // expected value, then halve speed of movement
  //double mult = (since.tv_sec * 1000000 + since.tv_usec) / MICROSECS_PER_FRAME;

  // update last
  last = current;

  //return mult;

  // time since last frame in seconds
  double sinceDbl = since.tv_sec + since.tv_usec / 1000000.0;

  return sinceDbl;
}
