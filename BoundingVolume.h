/*
 * $Id: BoundingVolume.h 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  BoundingVolume.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2012/11/16.
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

#ifndef STITCH_BOUNDING_VOLUME_H
#define STITCH_BOUNDING_VOLUME_H

namespace stitch {
	class BoundingVolume;
}

#include "Math/MathUtil.h"
#include "Math/Vec3.h"
#include "Math/Ray.h"
#include "Intersection.h"

#include "OSGUtils/StitchOSG.h"

#ifdef USE_CXX11
#include <cstdint>
#else
#include <stdint.h>
#endif

namespace stitch {
    
    //! The BV base class for all objects.
    class BoundingVolume
    {
    public:
        BoundingVolume():
        centre_(),
        radiusBV_(((float)FLT_MAX)),
        userIndex_(0),
        userGroupID_(0),
        itemID_(staticItemID_)
        {
            staticItemID_+=2;
        }
        
        BoundingVolume(const Vec3 &centre, const float radiusBV=((float)FLT_MAX), const uint32_t userIndex=0, const uint32_t userGroupID=0):
        centre_(centre),
        radiusBV_(radiusBV),
        userIndex_(userIndex),
        userGroupID_(userGroupID),
        itemID_(staticItemID_)
        {
            staticItemID_+=2;
        }
        
#ifdef USE_CXX11
        //!A move semantic constructor ...
        BoundingVolume(Vec3 &&centre, const float radiusBV=((float)FLT_MAX), const uint32_t userIndex=0, const uint32_t userGroupID=0):
        centre_(centre),
        radiusBV_(radiusBV),
        userIndex_(userIndex),
        userGroupID_(userGroupID),
        itemID_(staticItemID_)
        {
            staticItemID_+=2;
        }
#endif// USE_CXX11
        
        BoundingVolume(const BoundingVolume &lValue) :
        centre_(lValue.centre_),
        radiusBV_(lValue.radiusBV_),
        userIndex_(lValue.userIndex_),
        userGroupID_(lValue.userGroupID_),
        itemID_(lValue.itemID_)
        {
        }
        
#ifdef USE_CXX11
        BoundingVolume(BoundingVolume &&rValue) noexcept:
        centre_(std::move(rValue.centre_)),
        radiusBV_(rValue.radiusBV_),
        userIndex_(rValue.userIndex_),
        userGroupID_(rValue.userGroupID_),
        itemID_(rValue.itemID_)
        {
        }
#endif// USE_CXX11
        
        virtual ~BoundingVolume()
        {}
        
		virtual BoundingVolume & operator = (const BoundingVolume &lValue)
        {
            centre_=lValue.centre_;
            radiusBV_=lValue.radiusBV_;
            userIndex_=lValue.userIndex_;
            userGroupID_=lValue.userGroupID_;
            
            itemID_=lValue.itemID_;
            
            return (*this);
        }
        
#ifdef USE_CXX11
		virtual BoundingVolume & operator = (BoundingVolume &&rValue) noexcept
        {
            centre_=std::move(rValue.centre_);
            radiusBV_=rValue.radiusBV_;
            userIndex_=rValue.userIndex_;
            userGroupID_=rValue.userGroupID_;

            itemID_=rValue.itemID_;
            
            return (*this);
        }
#endif //USE_CXX11
        
        //=======================================================================//
        /*! Check whether a ray intersects the spherical bounding volume. */
        virtual bool BVIntersected(const Ray &ray) const final
        {
            const stitch::Vec3 A(ray.origin_, centre_);
            const float ADSq=A.lengthSq() - radiusBV_*radiusBV_;
            
            if (ADSq<=0.0f)
            {
                return true;
            } else
            {
                const float B = stitch::Vec3::dot(A, ray.direction_);
                
                if (B<0.0f)
                {
                    return false;
                } else
                {
                    return ((ADSq-B*B)<=0.0f);
                }
            }
        }
        
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual BoundingVolume * clone() const
        {
            return new BoundingVolume(*this);
        }        
        
#ifdef USE_OSG
        /*! Creates an OSG node that may be used to create a preview of the object.
         @param createOSGLineGeometry Boolean flag to indicate whether or not line geometry in addition to the polygon geometry should be created.
         @param createOSGNormalGeometry Boolean flag to indicate whether or not geometry to visualise the surface normal distributions should be created.
         @param wireframe Boolean flag to indicate whether or not the polygon geometry should be set to solid or wireframe.
         @return An osg node that may be used to preview the object.*/
		virtual osg::ref_ptr<osg::Node> constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key=0) const;
#endif// USE_OSG
        
        virtual void calcIntersection(const Ray &ray, Intersection &intersect) const;
        
        virtual bool pointInBV(const Vec3 &point) const
        {
            return centre_.calcDistToPointSq(point) <= (radiusBV_*radiusBV_);
        }
        
    public:
        Vec3 centre_;
        float radiusBV_;
        
        uint32_t userIndex_;
        uint32_t userGroupID_;
        
        //!Automatically allocated ID. The first two IDs (0 and 1) are reserved. Then objects are allocated even IDs with the lsb used to indicate front-face/back-face during intersection.
        uint32_t itemID_;
        
    protected:
        //!The static object ID used to assign IDs to new objects. The first two IDs (0 and 1) are reserved. Then objects are allocated even IDs with the lsb used to indicate front-face/back-face during intersection.
        static uint32_t staticItemID_;
    };
}


#endif// STITCH_BOUNDING_VOLUME_H
