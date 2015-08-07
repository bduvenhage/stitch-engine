/*
 * $Id: Plane.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Plane.h
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

#ifndef STITCH_PLANE_H
#define STITCH_PLANE_H

namespace stitch {
	struct Plane;
}

#include "Vec3.h"

namespace stitch {
	
	
    //! Plane defined as "p * normal_ = d_"
	struct Plane
	{
	public:
        //!Constructor of plane that intersects the three vertices.
		explicit Plane(const Vec3 &vert0, const Vec3 &vert1, const Vec3 &vert2);
        
        //!Contructor from normal and distance from origin.
		explicit Plane(const Vec3 &normal, const float d);
		
        //!Contructor from normal and point on plane.
		explicit Plane(const Vec3 &normal, const Vec3 &point);
		
        //!Copy contructor.
        Plane(const Plane &lValue);

#ifdef USE_CXX11
        //!Move contructor.
        Plane(Plane &&rValue) noexcept;
#endif// USE_CXX11
		
        ~Plane();
		
        //!Copy assignment operator.
		Plane &operator=(const Plane &lValue);
		
#ifdef USE_CXX11
        //!Move assignment operator.
		Plane &operator=(Plane &&rValue) noexcept;
#endif// USE_CXX11

        inline bool isOnPositiveSide(const Vec3 &point) const
        {
            return ((normal_ * point) > d_);
        }
        
        //!Calculate the intersection distance between the plane and the supplied orig vector along normDir.
		inline float calcIntersectDist(const Vec3 &orig, const Vec3 &normDir) const
        {
            return (d_-normal_.v_[0]*orig.v_[0]-normal_.v_[1]*orig.v_[1]-normal_.v_[2]*orig.v_[2])/(normal_.v_[0]*normDir.v_[0]+normal_.v_[1]*normDir.v_[1]+normal_.v_[2]*normDir.v_[2]);
        }

        //!Calculate the distance of the supplied point from the plane.
        float calcDistToPoint(const Vec3 &point) const;
		
	public:
        Vec3 normal_;
		float d_;
	};
	
}

#endif// STITCH_PLANE_H
