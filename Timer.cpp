/*
 * $Id: Timer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Timer.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2013/09/18.
 *  Copyright $Date: 2015-03-25 15:00:40 +0200 (Wed, 25 Mar 2015) $ Bernardt Duvenhage. All rights reserved.
 *
 *
 *  This file is part of StitchEngine.
 
 *  StitchEngine is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 
 *  StitchEngine is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with StitchEngine.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdio>
#include <string>
#include <cstring>

#include "Timer.h"


#ifdef USE_CXX11

//======== C++ 11 Timer Code =====================//
#include <chrono>

stitch::Timer::Timer( void ) :
SecondsPerTick_(std::chrono::high_resolution_clock::period::num / ((double)std::chrono::high_resolution_clock::period::den))
{}

stitch::Timer_t stitch::Timer::tick() const
{
    return std::chrono::high_resolution_clock::now();
}
//=============================================//

#else//USE_CXX11

//========= Pre-C++11 Timer Code ================//
    #ifdef WIN32
        #include <sys/types.h>
        #include <fcntl.h>
        #include <windows.h>
        #include <winbase.h>

        stitch::Timer::Timer()
        {
            LARGE_INTEGER frequency;
            if(QueryPerformanceFrequency(&frequency))
            {
                SecondsPerTick_ = 1.0/(double)frequency.QuadPart;
            }
            else
            {
                SecondsPerTick_ = 1.0;
            }
        }

        stitch::Timer_t stitch::Timer::tick() const
        {
            LARGE_INTEGER qpc;
            if (QueryPerformanceCounter(&qpc))
            {
                return qpc.QuadPart;
            }
            else
            {
                return 0;
            }
        }
    #else
        #include <unistd.h>

        stitch::Timer::Timer( void )
        {
            SecondsPerTick_ = (1.0 / (double) 1000000);
        }

        #if defined(_POSIX_TIMERS) && ( _POSIX_TIMERS > 0 ) && defined(_POSIX_MONOTONIC_CLOCK)
            #include <time.h>

            stitch::Timer_t stitch::Timer::tick() const
            {
                struct timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                return ((stitch::Timer_t)ts.tv_sec)*1000000+(stitch::Timer_t)ts.tv_nsec/1000;
            }
        #else
            #include <sys/time.h>

            stitch::Timer_t stitch::Timer::tick() const
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                return ((stitch::Timer_t)tv.tv_sec)*1000000+(stitch::Timer_t)tv.tv_usec;
            }
        #endif
    #endif
//=============================================//

#endif//else USE_CXX11

