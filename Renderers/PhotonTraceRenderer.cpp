/*
 * $Id: PhotonTraceRenderer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
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

#include "PhotonTraceRenderer.h"

#include <vector>


//=======================================================================//
stitch::PhotonTraceRenderer::PhotonTraceRenderer(Scene * const scene) : Renderer(scene)
{
    photonVector_.reserve(1000000);
    inFlightPhotonVector_.reserve(1000000);
}


//=======================================================================//
void stitch::PhotonTraceRenderer::render(RadianceMap &radianceMap,
                                         const stitch::Camera * const camera,
                                         const float frameDeltaTime)
{
    radianceMap.clear(Colour_t());
    
    const float halfWindowHeight = radianceMap.getHeight() * 0.5f;
    const float halfWindowWidth = radianceMap.getWidth() * 0.5f;
    
    const size_t imgWidth=radianceMap.getWidth();
    const size_t imgHeight=radianceMap.getHeight();
    
    const size_t numIterations=125;
    const float iterationTime=frameDeltaTime/numIterations;
    
    for (size_t i=0; i<numIterations; ++i)
    {
        tracePhotons(camera, iterationTime);
        
        std::vector<Photon *>::const_iterator iter=photonVector_.begin();
        const std::vector<Photon *>::const_iterator iterEnd=photonVector_.end();
        
        if (!stopRender_)
        {
            for (; iter!=iterEnd; ++iter)
            {
                Photon const * const photon=*iter;
                
                const float fx=(photon->centre_.x()*imgWidth+halfWindowWidth);
                const float fy=(photon->centre_.y()*imgWidth+halfWindowHeight);//Note: imgWidth is used correctly here to scale the photon location to screen space!
                
                if ((fy<imgHeight)&&(fy>=0.0f))
                {
                    if ((fx<imgWidth)&&(fx>=0.0f))
                    {
                        //Note: The energy calculation leads to fractional radiance c.(dL/dt) contributed to the radiance map.
                        radianceMap.addToMapValue(fx, fy, photon->energy_ * ((float(imgWidth) * float(imgHeight) * 0.1f)/frameDeltaTime), 0);
                    }
                }
            }
        } else
        {
            break;//from for loop
        }
    }
    
}



//=======================================================================//
size_t stitch::PhotonTraceRenderer::tracePhotons(const stitch::Camera * const camera, const float frameDeltaTime)
{
    {
        std::vector<stitch::Photon *>::const_iterator photonIter=photonVector_.begin();
        const std::vector<stitch::Photon *>::const_iterator photonIterEnd=photonVector_.end();
        for (; photonIter!=photonIterEnd; ++photonIter)
        {
            delete (*photonIter);
        }
        photonVector_.clear();
    }
    
    std::cout << "Radiating photons...";
    std::cout.flush();
    scene_->light_->radiate(frameDeltaTime, inFlightPhotonVector_);
    std::cout << "done.\n";
    std::cout.flush();
    
    std::cout << "Tracing " << inFlightPhotonVector_.size() << " photons...";
    std::cout.flush();
    
	size_t photonsRadiated=inFlightPhotonVector_.size();
	size_t photonsTraced=0;
	size_t photonsScattered=0;
    
	for (size_t photonNum=0; photonNum<(photonsRadiated+photonsScattered); ++photonNum)
	{
		stitch::Photon *inFlightPhoton=inFlightPhotonVector_[photonNum];
		
        stitch::Intersection intersect(photonNum, 0, ((float)FLT_MAX));
        scene_->calcIntersection(stitch::Ray(photonNum, 0, inFlightPhoton->normDir_, inFlightPhoton->centre_), intersect);
        
        const stitch::BoundingVolume *item=intersect.itemPtr_;
		
		if (item)
		{//There is an object in the photon's path scatter it.
            stitch::Material *pClosestMaterial=(static_cast<const stitch::Object *>(item))->pMaterial_;
            
            stitch::Vec3 worldPosition=inFlightPhoton->centre_+inFlightPhoton->normDir_*intersect.distance_;
            stitch::Vec3 worldNormal=intersect.normal_;
            
            //===
            if ((inFlightPhoton->scatterCount_<2)&&(pClosestMaterial->getType()==stitch::Material::GLOSSY_MATERIAL))
            {//Scatter the photon.
                stitch::Photon scatPhoton=pClosestMaterial->scatterPhoton_direct(worldNormal, worldPosition, *inFlightPhoton);
                
                if (scatPhoton.normDir_.lengthSq()>0.0f)
                {
                    inFlightPhotonVector_.push_back(new stitch::Photon(worldPosition+scatPhoton.normDir_*0.001, scatPhoton.normDir_, scatPhoton.energy_, scatPhoton.scatterCount_));
                    
                    //! @todo Also push shadow photon with zero energy. What does the density of shadow photons mean?
                    
                    ++photonsScattered;
                }
            }
            
            //===
            if (pClosestMaterial->getType()!=stitch::Material::GLOSSY_MATERIAL)
            {//Test if the camera can see a potentially scattered photon from the intersection position. Assume a pinhole camera.
                stitch::Vec3 cameraDir=(camera->m_position_-worldPosition);
                const float cameraDist=cameraDir.normalise_rt();
                
                stitch::Intersection intersect(photonNum, 0, ((float)FLT_MAX));
                scene_->calcIntersection(stitch::Ray(photonNum, 0, cameraDir, worldPosition+cameraDir*0.01f), intersect);
                
                if ((intersect.itemID_==0) ||
                    (intersect.distance_>cameraDist))
                {//There are no objects between the scattered photon's origin and the camera.
                    //So record a photon on the focal plane.
                    const stitch::Vec3 fpPos=camera->getFocalPlaneIntersect(worldPosition);
                    
                    const float recipCFragmentArea=fabsf(worldNormal*cameraDir)/(cameraDist*cameraDist);// 1 / (C.fragmentArea)
                    
                    const stitch::Colour_t cnvrtEnergyQuantumToDifferentialRadiance=pClosestMaterial->BSDF(worldPosition, inFlightPhoton->normDir_*(-1.0f), cameraDir, worldNormal) * (fabsf(worldNormal*inFlightPhoton->normDir_) * recipCFragmentArea);
                    
                    //Note: The energy calculation leads to fractional radiance c.(dL/dt) contributed to the radiance map.
                    photonVector_.push_back(new stitch::Photon(fpPos, stitch::Vec3(), inFlightPhoton->energy_.cmult(cnvrtEnergyQuantumToDifferentialRadiance), inFlightPhoton->scatterCount_ + 1 ));
                }
            }
            
        }
		
		++photonsTraced;
        
        /*
         if ((photonNum%10000)==0)
         {
         std::cout << ((100*photonNum)/(photonsRadiated+photonsScattered)) << "%...";
         }
         */
	}
    
    {
        //=== Delete in-flight photons and clear the vector...
        std::vector<stitch::Photon *>::const_iterator photonIter=inFlightPhotonVector_.begin();
        const std::vector<stitch::Photon *>::const_iterator photonIterEnd=inFlightPhotonVector_.end();
        for (; photonIter!=photonIterEnd; ++photonIter)
        {
            delete (*photonIter);
        }
        inFlightPhotonVector_.clear();//radiated and scattered photons.
        //===
    }
    std::cout << "(" << photonVector_.size() << " stored)...done.\n";
    std::cout.flush();
    
    return photonVector_.size();
}


