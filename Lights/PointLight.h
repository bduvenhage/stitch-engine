/*
 * $Id: PointLight.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  PointLight.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/03/01.
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

#ifndef STITCH_POINT_LIGHT_H
#define STITCH_POINT_LIGHT_H

namespace stitch {
    class PointLight;
}

#include "EntryExit.h"
#include "Light.h"
#include "Materials/EmissiveMaterial.h"

namespace stitch {
#define POINT_LIGHT_RADIUS 1.0f
    
	class PointLight : public Light
	{
	public:
		PointLight(const Vec3 &orig, const Colour_t &SPD) :
        Light(SPD, new EmissiveMaterial( SPD * ((float)(1.0f/(4.0f*((float)M_PI) * POINT_LIGHT_RADIUS*POINT_LIGHT_RADIUS * ((float)M_PI)))) ) )
		{
            radiusBV_=POINT_LIGHT_RADIUS;
            centre_=orig;
		}
		
		PointLight(const PointLight &lValue) :
        Light(lValue)
		{}
		
        virtual PointLight * clone() const
        {
            return new PointLight(*this);
        }

		virtual PointLight & operator = (const PointLight &lValue)
		{
            Light::operator=(lValue);
            
			return *this;
		}
		
		virtual ~PointLight()
		{}
		
#ifdef USE_OSG
		virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
#endif// USE_OSG
        
		virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
		
		//adds photons to photonVector.
		virtual void radiate(const float time, std::vector<Photon *> &photonVector) const;
		virtual void radiate(const float time, std::vector<Photon *> &photonVector, const Vec3 blockerDirection, float blockerSubtendHA) const;
        
	};
}

#endif// STITCH_POINT_LIGHT_H
