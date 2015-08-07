/*
 * $Id: Timer.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Timer.h
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


#ifndef STITCH_TIMER_H
#define STITCH_TIMER_H


#ifdef USE_CXX11
//======== C++ 11 Timer Code =====================//
#include <cstdint>
#include <chrono>

namespace stitch
{
    //! Typedef for stitch::Timer wrapper.
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> Timer_t;

    //! Wrapper for C++11 chrono timer.
    class Timer
    {
    public:
        Timer();
        ~Timer() {}
        
        Timer_t tick() const;
        
        inline double getSecondsPerTick() const { return SecondsPerTick_; }
        
        inline double delta_s( Timer_t t1, Timer_t t2 ) const { return std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count(); }
        
        inline double delta_m( Timer_t t1, Timer_t t2 ) const { return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count(); }
        
        inline double delta_u( Timer_t t1, Timer_t t2 ) const { return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count(); }
        
        inline double delta_n( Timer_t t1, Timer_t t2 ) const { return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count(); }
        
        protected :
        const double  SecondsPerTick_;
    };
}
//=============================================//

#else//USE_CXX11

//========= Pre-C++11 Timer Code ================//
#if defined(_MSC_VER)
namespace stitch
{
    //! Typedef for stitch::Timer wrapper.
    typedef __int64 Timer_t;
}
#else
#include <stdint.h>
namespace stitch
{
    //! Typedef for stitch::Timer wrapper.
    typedef uint64_t Timer_t;
}
#endif

namespace stitch
{
    //! Wrapper for C++98 Win32/Posix timer.
    class Timer
    {
    public:
        Timer();
        ~Timer() {}
        
        Timer_t tick() const;
        
        inline double getSecondsPerTick() const { return SecondsPerTick_; }
        
        inline double delta_s( Timer_t t1, Timer_t t2 ) const { return (double)(t2 - t1)*SecondsPerTick_; }
        
        inline double delta_m( Timer_t t1, Timer_t t2 ) const { return delta_s(t1,t2)*1e3; }
        
        inline double delta_u( Timer_t t1, Timer_t t2 ) const { return delta_s(t1,t2)*1e6; }
        
        inline double delta_n( Timer_t t1, Timer_t t2 ) const { return delta_s(t1,t2)*1e9; }
        
        protected :
        double  SecondsPerTick_;
    };
}
//=============================================//

#endif//else USE_CXX11


#endif// STITCH_TIMER_H
