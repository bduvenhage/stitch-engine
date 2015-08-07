/*
 * $Id: WhittedRenderer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Renderer.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/05/09.
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

#include "WhittedRenderer.h"

#include <vector>


//=======================================================================//
stitch::WhittedRenderer::WhittedRenderer(Scene * const scene) :
ForwardRenderer(scene, 3, 1, false)
{
}


//=======================================================================//
void stitch::WhittedRenderer::gather(Ray &ray) const
{
    //ray.returnRadiance_.setZeros();
    
    if (ray.gatherDepth_>0)
    {
        stitch::Intersection intersect(ray.id0_, ray.id1_, ((float)FLT_MAX));
        scene_->calcIntersection(ray, intersect);
        
        const stitch::BoundingVolume *item=intersect.itemPtr_;
        
        if (item)
        {//There is an object in the ray's path.
            stitch::Material *pClosestMaterial=(static_cast<const stitch::Object *>(item))->pMaterial_;
            
            //=== Find radiance from closest entry ===//
            stitch::Vec3 worldPosition=ray.direction_;
            worldPosition*=intersect.distance_;
            worldPosition+=ray.origin_;
            
            stitch::Vec3 worldNormal=intersect.normal_;
            
            //Calculate emitted radiance from closest hit.
            ray.returnRadiance_+=pClosestMaterial->getEmittedRadiance(worldNormal, ray.direction_*(-1.0f), worldPosition);
            
            if (ray.gatherDepth_>1)
            {//Will still gather 'gatherDepth' levels.
                stitch::Colour_t diffuseRefl=pClosestMaterial->getDiffuseRefl(worldPosition);
                
                if (diffuseRefl.isNotZero())
                {
                    //Gather radiance from direction of point light.
                    stitch::Vec3 shadowRay=(scene_->light_->centre_ - worldPosition);
                    float lightDistSq=shadowRay.lengthSq();
                    shadowRay/=sqrtf(lightDistSq);
                    float sr=(((float)M_PI)*scene_->light_->radiusBV_ * scene_->light_->radiusBV_)/lightDistSq;
                    
                    const float cosTheta=worldNormal*shadowRay;
                    
                    if (cosTheta>0.0f)//Back faces of objects will be in shadow!
                    {
                        stitch::Ray sray(ray.id0_, ray.id1_, shadowRay,
                                         stitch::Vec3(worldPosition, shadowRay, 0.001f),
                                         1);
                        
                        gather(sray);
                        
                        ray.returnRadiance_+=diffuseRefl.cmult(sray.returnRadiance_ * (sr * cosTheta * ((float)M_1_PI)));
                    }
                }
                
                //Gather from specular trans direction.
                stitch::Colour_t specTrans=pClosestMaterial->getSpecularTrans();
                if (specTrans.isNotZero())
                {//Gather radiance from specular trans direction.
                    stitch::Vec3 transDir=pClosestMaterial->whittedSpecRefractRay(ray.direction_, worldNormal);
                    
                    stitch::Ray tray(ray.id0_, ray.id1_, transDir,
                                     stitch::Vec3(worldPosition, transDir, 0.05f), //0.05 to jump over the back face of the thin transparent brush.
                                     ray.gatherDepth_-1);
                    
                    gather(tray);
                    
                    ray.returnRadiance_+=specTrans.cmult(tray.returnRadiance_);
                }
                
                //Gather from specular refl direction.
                stitch::Colour_t specRefl=pClosestMaterial->getSpecularRefl();
                if (specRefl.isNotZero())
                {//Gather radiance from specular refl direction.
                    stitch::Vec3 reflDir=pClosestMaterial->whittedSpecReflectRay(ray.direction_, worldNormal);
                    
                    stitch::Ray rray(ray.id0_, ray.id1_, reflDir,
                                     stitch::Vec3(worldPosition, reflDir, 0.001f),
                                     ray.gatherDepth_-1);
                    gather(rray);
                    
                    ray.returnRadiance_+=specRefl.cmult(rray.returnRadiance_);
                }
                
            }
            //=========================================//
        }
    }
}


//=======================================================================//
void stitch::WhittedRenderer::preForwardRender(RadianceMap &radianceMap,
                                               const stitch::Camera * const camera,
                                               const float frameDeltaTime)
{
}



