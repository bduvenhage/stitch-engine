/*
 * $Id: PathTraceRenderer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
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

#include "PathTraceRenderer.h"
//#include "Objects/Brush.h"
//#include "Objects/PolygonModel.h"

#include <vector>

//====================================================================================================//
stitch::PathTraceRenderer::PathTraceRenderer(Scene * const scene) :
ForwardRenderer(scene, 4, 5000, true)
{
}


//=======================================================================//
void stitch::PathTraceRenderer::gather(Ray &ray) const
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
            
            //Importance sampling.
            if (ray.gatherDepth_>1)
            {
                stitch::Vec3 toVertexDir=worldPosition - ray.origin_;
                toVertexDir.normalise();
                
                const Colour_t sRefl=pClosestMaterial->getSpecularRefl();
                const float avrgSpecRefl=sRefl.cavrg();
                
                const Colour_t dRefl=pClosestMaterial->getDiffuseRefl(worldPosition);
                const float avrgDiffRefl=dRefl.cavrg();
                
                const float avrgAlbedo=avrgSpecRefl + avrgDiffRefl;
                
                if (avrgAlbedo>0.0f)
                {
                    const float r=stitch::GlobalRand::uniformSampler() * avrgAlbedo;
                    
                    if (r<avrgSpecRefl)
                    {//specular component sampled.
                        stitch::Vec3 importanceDir=pClosestMaterial->stochasticSpecReflectRay(toVertexDir, worldNormal);
                        
                        if (importanceDir.isNotZero())
                        {
                            stitch::Ray importanceRay(ray.id0_, ray.id1_, importanceDir,
                                                      stitch::Vec3(worldPosition, importanceDir, 0.001f),
                                                      ray.gatherDepth_ - 1);
                            gather(importanceRay);
                            
                            const stitch::Colour_t refl=sRefl;
                            ray.returnRadiance_+=refl.cmult(importanceRay.returnRadiance_);
                        }
                    } else
                    {//diffuse component sampled.
                        stitch::Vec3 importanceDir=pClosestMaterial->stochasticDiffuseReflectRay(toVertexDir, worldNormal);
                        
                        if (importanceDir.isNotZero())
                        {
                            stitch::Ray importanceRay(ray.id0_, ray.id1_, importanceDir,
                                                      stitch::Vec3(worldPosition, importanceDir, 0.001f),
                                                      ray.gatherDepth_ - 1);
                            gather(importanceRay);
                            
                            const stitch::Colour_t refl=dRefl;
                            ray.returnRadiance_+=refl.cmult(importanceRay.returnRadiance_);
                        }
                    }
                }
                
            }
            //=========================================//
        }
    }
}

//=======================================================================//
void stitch::PathTraceRenderer::preForwardRender(RadianceMap &radianceMap,
                                                 const stitch::Camera * const camera,
                                                 const float frameDeltaTime)
{
}
