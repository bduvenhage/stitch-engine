/*
 * $Id: Intersection.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Intersection.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
 *  Copyright $Date: 2015-08-07 16:57:10 +0200 (Fri, 07 Aug 2015) $ Bernardt Duvenhage. All rights reserved.
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


#ifndef STITCH_INTERSECION_H
#define STITCH_INTERSECION_H

namespace stitch {
	class Intersection;
}

#include "Math/Vec3.h"
#include "BoundingVolume.h"

#ifdef USE_CXX11
#include <cstdint>
#else
#include <stdint.h>
#endif

namespace stitch {
	
    //! The intersection between a line and an object such as polygon.
	class Intersection
	{
	public:
        /*! Constructor when the intersection distance is known.
         *  \param rayID0 The ID of the ray that lead to this intersection.
         *  \param rayID1 Secondary ID of the ray that lead to this intersection.
         *  \param distance The distance of the intersection. */
		Intersection(const uint32_t rayID0,
                     const uint16_t rayID1,
                     const float distance) :
        rayID0_(rayID0),
        rayID1_(rayID1),
		distance_(distance), normal_(),
        itemID_(0),
        itemPtr_(nullptr)
		{}
        
        /*! Destructor */
		~Intersection() {}
		
        /*! Copy constructor */
		Intersection(const Intersection &lValue) :
        rayID0_(lValue.rayID0_),
        rayID1_(lValue.rayID1_),
		distance_(lValue.distance_), normal_(lValue.normal_),
        itemID_(lValue.itemID_),
        itemPtr_(lValue.itemPtr_)
		{}
        
#ifdef USE_CXX11
        /*! Move constructor */
		Intersection(Intersection &&rValue) noexcept:
        rayID0_(rValue.rayID0_),
        rayID1_(rValue.rayID1_),
		distance_(rValue.distance_), normal_(std::move(rValue.normal_)),
        itemID_(rValue.itemID_),
        itemPtr_(rValue.itemPtr_)
		{}
#endif// USE_CXX11
        
        /*! Assignment operator */
		inline Intersection & operator=(const Intersection &lValue)
		{
            rayID0_=lValue.rayID0_;
            rayID1_=lValue.rayID1_;
			distance_=lValue.distance_;
			normal_=lValue.normal_;
            itemID_=lValue.itemID_;
            itemPtr_=lValue.itemPtr_;
			return (*this);
		}
		
#ifdef USE_CXX11
        /*! Assignment move operator */
		inline Intersection & operator=(Intersection &&rValue) noexcept
		{
            rayID0_=rValue.rayID0_;
            rayID1_=rValue.rayID1_;
			distance_=rValue.distance_;
			normal_=std::move(rValue.normal_);
            itemID_=rValue.itemID_;
            itemPtr_=rValue.itemPtr_;
			return (*this);
		}
#endif// USE_CXX11
		
        /*! The ID0 of the ray that lead to this intersection */
        uint32_t rayID0_;
        /*! The ID1 of the ray that lead to this intersection */
        uint16_t rayID1_;
        
        /*! The distance to the intersection */
		float distance_;
        
        /*! The surface normal at the intersection */
		Vec3 normal_;
		
        /*! The id of the item intersected */
        uint32_t itemID_;
        
        /*! The pointer to the item intersected */
        BoundingVolume const * itemPtr_;
	};
}


#endif// STITCH_INTERSECION_H
