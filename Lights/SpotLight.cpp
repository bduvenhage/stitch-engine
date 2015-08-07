/*
 * $Id: SpotLight.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  SpotLight.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/03/02.
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

#include "SpotLight.h"
#include "OSGUtils/StitchOSG.h"


//=======================================================================//
#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::SpotLight::constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key) const
{
    return constructOSGNode_Sphere(VecN(centre_), radiusBV_, wireframe, key==0 ? ((uintptr_t)this) : key);
}
#endif// USE_OSG

//=======================================================================//
void stitch::SpotLight::calcIntersection(const Ray &ray, Intersection &intersect) const
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


//=======================================================================//
void stitch::SpotLight::radiate(const float time, std::vector<Photon *> &photonVector, const Vec3 blockerDirection, float blockerSubtendHA) const
{
    const float cosBlockerSubtendHA=cosf(blockerSubtendHA);
    
    const float toRadiate_energy=SPD_.csum() * time;
    const size_t toRadiate_photons=(toRadiate_energy/Photon::quantumEnergy_)+0.5f;
    
    size_t radiated_photons=0;
    
    const Vec3 orthVecA=(direction_.orthVec()).normalised();
    const Vec3 orthVecB=stitch::Vec3::crossNormalised(direction_, orthVecA);
    
    {
        while ( radiated_photons < toRadiate_photons )
        {
            Vec3 radiateDirectionLocal(Vec3::randCosineLobe(cosExp_));
            Vec3 radiateDirection=orthVecA*radiateDirectionLocal.x() + orthVecB*radiateDirectionLocal.y() + direction_*radiateDirectionLocal.z();
            
            if ((radiateDirection*blockerDirection)<=cosBlockerSubtendHA)
            {
                Photon *photonRed=new Photon(centre_+radiateDirection*(radiusBV_*1.01f), radiateDirection, normalisedSPD_*Photon::quantumEnergy_, 0);
                photonVector.push_back(photonRed);
            }
            
            ++radiated_photons;
        }
    }
}

void stitch::SpotLight::radiate(const float time, std::vector<Photon *> &photonVector) const
{
    radiate(time, photonVector, Vec3(1.0f, 0.0f, 0.0f), 0);
}
