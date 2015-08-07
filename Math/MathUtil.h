/*
 * $Id: MathUtil.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  StitchMinMax.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/04/26.
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

#ifndef STITCH_MIN_MAX_H
#define STITCH_MIN_MAX_H

#include <cstdio>
#include <float.h>
#include <math.h>

#ifdef USE_CXX11
  #include <cstdint>
#else
  #include <stdint.h>
#endif

//#include <mmintrin.h>

namespace stitch {
    //! Provides max, min and clamp for most integer and real types.
    class MathUtil
    {
    public:
        
        template <class tType>
        static tType max(tType a, tType b) {
            return (a>b) ? a : b;
        }
        
        template <class tType>
        static tType min(tType a, tType b) {
            return (a<b) ? a : b;
        }
        
        template <typename tType>
        static tType clamp(const tType& a, const tType& min, const tType& max) {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static int64_t max(const int64_t &a, const int64_t &b)
        {
            return (a>b) ? a : b;
        }
        inline static int64_t min(const int64_t &a, const int64_t &b)
        {
            return (a<b) ? a : b;
        }
        inline static int64_t clamp(const int64_t &a, const int64_t &min, const int64_t &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static uint64_t max(const uint64_t &a, const uint64_t &b)
        {
            return (a>b) ? a : b;
        }
        inline static uint64_t min(const uint64_t &a, const uint64_t &b)
        {
            return (a<b) ? a : b;
        }
        inline static uint64_t clamp(const uint64_t &a, const uint64_t &min, const uint64_t &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static int32_t max(const int32_t &a, const int32_t &b)
        {
            return (a>b) ? a : b;
        }
        inline static int32_t min(const int32_t &a, const int32_t &b)
        {
            return (a<b) ? a : b;
        }
        inline static int32_t clamp(const int32_t &a, const int32_t &min, const int32_t &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static uint32_t max(const uint32_t &a, const uint32_t &b)
        {
            return (a>b) ? a : b;
        }
        inline static uint32_t min(const uint32_t &a, const uint32_t &b)
        {
            return (a<b) ? a : b;
        }
        inline static uint32_t clamp(const uint32_t &a, const uint32_t &min, const uint32_t &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static int16_t max(const int16_t &a, const int16_t &b)
        {
            return (a>b) ? a : b;
        }
        inline static int16_t min(const int16_t &a, const int16_t &b)
        {
            return (a<b) ? a : b;
        }
        inline static int16_t clamp(const int16_t &a, const int16_t &min, const int16_t &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static uint16_t max(const uint16_t &a, const uint16_t &b)
        {
            return (a>b) ? a : b;
        }
        inline static uint16_t min(const uint16_t &a, const uint16_t &b)
        {
            return (a<b) ? a : b;
        }
        inline static uint16_t clamp(const uint16_t &a, const uint16_t &min, const uint16_t &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static float max(const float &a, const float &b)
        {
            return (a>b) ? a : b;
        }
        inline static float min(const float &a, const float &b)
        {
            return (a<b) ? a : b;
        }
        inline static float clamp(const float &a, const float &min, const float &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
        
        inline static double max(const double &a, const double &b)
        {
            return (a>b) ? a : b;
        }
        inline static double min(const double &a, const double &b)
        {
            return (a<b) ? a : b;
        }
        inline static double clamp(const double &a, const double &min, const double &max)
        {
            return (a>max) ? max : ((a<min) ? min : a);
        }
    };
}

#endif// STITCH_MIN_MAX_H
