//
//  Ray.h
//  stitchEngine
//
//  Created by Bernardt Duvenhage on 2012/05/01.
//  Copyright 2012 self. All rights reserved.
//
/*
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

#ifndef stitchEngine_Ray_h
#define stitchEngine_Ray_h

namespace stitch
{
    class Ray;
}

#include "Vec3.h"
#include "Colour.h"

#include <utility> // for std::move

#ifdef USE_CXX11
#include <cstdint>
#else
#include <stdint.h>
#endif

namespace stitch {
    //! Different from stitch::Line in that a ray propagates radiance and keeps track of the gather depth.
    class Ray
    {
    public:
        Ray(const uint32_t rayID0,
            const uint16_t rayID1,
            const Vec3 &direction, const Vec3 &origin
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
            , const uint8_t gatherDepth=0
#endif
                ) :
        id0_(rayID0),
        id1_(rayID1),
        direction_(direction), origin_(origin)
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
        , gatherDepth_(gatherDepth),
        returnRadiance_()
#endif
        {}
		
#ifdef USE_CXX11
        Ray(const uint32_t rayID0,
            const uint16_t rayID1,
            Vec3 &&direction, const Vec3 &origin,
            const uint8_t gatherDepth=0) :
        id0_(rayID0),
        id1_(rayID1),
        direction_(std::move(direction)), origin_(origin)
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
        , gatherDepth_(gatherDepth),
        returnRadiance_()
#endif
        {}
        
        Ray(const uint32_t rayID0,
            const uint16_t rayID1,
            const Vec3 &direction, Vec3 &&origin,
            const uint8_t gatherDepth=0) :
        id0_(rayID0),
        id1_(rayID1),
        direction_(direction), origin_(std::move(origin))
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
        , gatherDepth_(gatherDepth),
        returnRadiance_()
#endif
        {}
        
        Ray(const uint32_t rayID0,
            const uint16_t rayID1,
            Vec3 &&direction, Vec3 &&origin,
            const uint8_t gatherDepth=0) :
        id0_(rayID0),
        id1_(rayID1),
        direction_(std::move(direction)), origin_(std::move(origin))
  #ifdef STITCH_RAY_RADIANCE_PAYLOAD
        , gatherDepth_(gatherDepth),
        returnRadiance_()
#endif
        {}
#endif// USE_CXX11
        
		Ray(const Ray &lValue) :
        id0_(lValue.id0_),
        id1_(lValue.id1_),
        direction_(lValue.direction_), origin_(lValue.origin_)
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
        , gatherDepth_(lValue.gatherDepth_),
        returnRadiance_(lValue.returnRadiance_)
#endif
        {}
        
#ifdef USE_CXX11
		Ray(Ray &&rValue) noexcept:
        id0_(rValue.id0_),
        id1_(rValue.id1_),
        direction_(std::move(rValue.direction_)), origin_(std::move(rValue.origin_))
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
        , gatherDepth_(rValue.gatherDepth_),
        returnRadiance_(std::move(rValue.returnRadiance_))
#endif
        {}
#endif// USE_CXX11
        
        Ray & operator=(const Ray &lValue)
        {
            id0_=lValue.id0_;
            id1_=lValue.id1_;
            direction_=lValue.direction_;
            origin_=lValue.origin_;
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
            gatherDepth_=lValue.gatherDepth_;
            returnRadiance_=lValue.returnRadiance_;
#endif
            return (*this);
        }
        
#ifdef USE_CXX11
        Ray & operator=(Ray &&rValue) noexcept
        {
            id0_=rValue.id0_;
            id1_=rValue.id1_;
            direction_=std::move(rValue.direction_);
            origin_=std::move(rValue.origin_);
#ifdef STITCH_RAY_RADIANCE_PAYLOAD
            gatherDepth_=rValue.gatherDepth_;
            returnRadiance_=std::move(rValue.returnRadiance_);
#endif
            return (*this);
        }
#endif// USE_CXX11
        
        ~Ray() {}
        
    public:
        uint32_t id0_;
        uint16_t id1_;
        Vec3 direction_;
        Vec3 origin_;

#ifdef STITCH_RAY_RADIANCE_PAYLOAD
        uint8_t gatherDepth_;
        Colour_t returnRadiance_;
#endif
    };
}

#endif
