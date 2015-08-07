/*
 * $Id: Object.cpp 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Beam.cpp
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

#include "Object.h"
//#include "Beam.h"

#include "OSGUtils/StitchOSG.h"


//=======================================================================//
stitch::Object::Object(Material * const pMaterial) :
BoundingVolume(), pMaterial_(pMaterial)
{}

//=======================================================================//
stitch::Object::Object(const Object &lValue) :
BoundingVolume(lValue), pMaterial_(lValue.pMaterial_->clone())
{}

//=======================================================================//
#ifdef USE_CXX11
stitch::Object::Object(Object &&rValue) noexcept:
BoundingVolume(rValue), pMaterial_(rValue.pMaterial_)
{
    rValue.pMaterial_=nullptr;
}
#endif// USE_CXX11

//=======================================================================//
stitch::Object::~Object()
{
    delete pMaterial_;
}

//=======================================================================//
stitch::Object & stitch::Object::operator = (const Object &lValue)
{
    if (&lValue!=this)
    {
        BoundingVolume::operator=(lValue);
        
        delete pMaterial_;
        pMaterial_=lValue.pMaterial_->clone();
    }
    
    return *this;
}

#ifdef USE_CXX11
//=======================================================================//
stitch::Object & stitch::Object::operator = (Object &&rValue) noexcept
{
    BoundingVolume::operator=(rValue);
    
    delete pMaterial_;
    pMaterial_=rValue.pMaterial_;
    rValue.pMaterial_=nullptr;
    
    return *this;
}
#endif// USE_CXX11

//=== Start of sphere object's implementation!!! ===//

//=======================================================================//
stitch::Sphere::Sphere(Material * const pMaterial, const Vec3 centre, const float radius) :
Object(pMaterial)
{
    centre_=centre;
    radiusBV_=radius;
}

//=======================================================================//
stitch::Sphere::Sphere(const Sphere &lValue) :
Object(lValue)
{}

#ifdef USE_CXX11
//=======================================================================//
stitch::Sphere::Sphere(Sphere &&rValue) noexcept:
Object(rValue)
{}
#endif// USE_CXX11

//=======================================================================//
stitch::Sphere::~Sphere()
{
}

//=======================================================================//
stitch::Sphere & stitch::Sphere::operator = (const Sphere &lValue)
{
    Object::operator=(lValue);
    
    return *this;
}

#ifdef USE_CXX11
//=======================================================================//
stitch::Sphere & stitch::Sphere::operator = (Sphere &&rValue) noexcept
{
    Object::operator=(rValue);
    
    return *this;
}
#endif// USE_CXX11

//=======================================================================//
#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::Sphere::constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key) const
{
	return constructOSGNode_Sphere(VecN(centre_), radiusBV_, wireframe, key==0 ? ((uintptr_t)this) : key);
}
#endif// USE_OSG

//=======================================================================//
void stitch::Sphere::calcIntersection(const Ray &ray, Intersection &intersect) const
{
    const Vec3 origD=ray.origin_-centre_;
    const float a=ray.direction_*ray.direction_;
    const float b=(ray.direction_*origD)*(2.0f);
    const float c=origD*origD - radiusBV_*radiusBV_;
    const float d=b*b-4.0f*a*c;
    
    if (d>=0.0f)
    {//The line intersects the sphere.
        const float sqrtD=sqrtf(d);
        
        const float entry=(-b-sqrtD)/(2.0f*a);
        const float exit=(-b+sqrtD)/(2.0f*a);
        
        if (entry>0.0f)
        {
            if (entry<intersect.distance_)
            {
                intersect.distance_=entry;
                intersect.normal_=stitch::Vec3(origD, ray.direction_, entry);
                intersect.normal_.normalise();
                intersect.itemID_=this->itemID_ | ((intersect.normal_*ray.direction_>0.0f)?0:1);//back surface gets even ID, front surface gets odd ID.
                intersect.itemPtr_=this;
            }
        } else
            if (exit>0.0f)
            {
                if (exit<intersect.distance_)
                {
                    intersect.distance_=exit;
                    intersect.normal_=stitch::Vec3(origD, ray.direction_, exit);
                    intersect.normal_.normalise();
                    intersect.itemID_=this->itemID_ | ((intersect.normal_*ray.direction_>0.0f)?0:1);//back surface gets even ID, front surface gets odd ID.
                    intersect.itemPtr_=this;
                }
            }
    }
}




