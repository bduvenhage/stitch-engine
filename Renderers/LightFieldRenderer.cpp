/*
 * $Id: LightFieldRenderer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  LightFieldRenderer.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2014/12/0.
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

#include "LightFieldRenderer.h"

#include <vector>
#include "OSGUtils/StitchOSG.h"



//====================================================================================================//
stitch::LightFieldRenderer::LightFieldRenderer(Scene * const scene) :
ForwardRenderer(scene, 3, 1, true)
{
    inFlightPhotonVector_.reserve(1000000);
    photonMap_=new stitch::PhotonMap;
}

//=======================================================================//
size_t stitch::LightFieldRenderer::tracePhotons(const float frameDeltaTime, const size_t photonTreeChunkSize)
{
    photonMap_->clear();
    
    stitch::Timer timer;
    stitch::Timer_t startTick=timer.tick();
    
    size_t totalRadiated=0;
    size_t totalTraced=0;
    size_t totalScattered=0;
    
    //! Num iterations of radiate, trace, record loops. More iterations would save on in-flight memory if many radiated photons are not recorded in the map.
    const size_t numIterations=50;
    const float iterFrac=1.0f / numIterations;
    
    for (size_t i=0; i<numIterations; ++i)
    {
        //=== Delete any previous in-flight photons and clear the vector...
        std::vector<stitch::Photon *>::const_iterator photonIter=inFlightPhotonVector_.begin();
        const std::vector<stitch::Photon *>::const_iterator photonIterEnd=inFlightPhotonVector_.end();
        for (; photonIter!=photonIterEnd; ++photonIter)
        {
            delete (*photonIter);
        }
        
        inFlightPhotonVector_.clear();//radiated and scattered photons.
        //===
        
        //@todo How does one radiate and trace radiance??? photons carry energy not radiance!
        
        std::cout << "  Radiating " << i*iterFrac*100.0f << "-" << (i+1)*iterFrac*100.0f << "% of photons...";
        std::cout.flush();
        scene_->light_->radiate(frameDeltaTime*iterFrac, inFlightPhotonVector_);//, (scene_->light_->centre_ - stitch::Vec3(-7.0f,-1.9f+2.0f,8.5f) ).normalised(), 150*172.5f*(((float)M_PI)/180.0f) );
        std::cout << "done.\n";
        std::cout.flush();
        
        std::cout << "  Tracing " << inFlightPhotonVector_.size() << " photons...";
        std::cout.flush();
        
        const size_t photonsRadiated=inFlightPhotonVector_.size();
        totalRadiated+=photonsRadiated;
        
        size_t photonsTraced=0;
        size_t photonsScattered=0;
        
        float nextProgressIndication=0.0f;
        
        for (size_t photonNum=0; photonNum<(photonsRadiated+photonsScattered); ++photonNum)
        {
            stitch::Photon *photon=inFlightPhotonVector_[photonNum];
            
            stitch::Intersection intersect(photonNum, 0, ((float)FLT_MAX));
            scene_->calcIntersection(stitch::Ray(photonNum, 0, photon->normDir_, photon->centre_), intersect);
            
            const stitch::BoundingVolume *item=intersect.itemPtr_;
            
            if (item)
            {//There is an object in the photon's path.
                stitch::Vec3 worldPosition=photon->centre_+photon->normDir_*intersect.distance_;
                
                {
                    photonMap_->addItem(new stitch::Photon(worldPosition, photon->normDir_, photon->energy_, photon->scatterCount_));
                }
                
                stitch::Material *pClosestMaterial=(static_cast<const stitch::Object *>(item))->pMaterial_;
                
                if (photon->scatterCount_<2)
                {//Scatter the photon.
                    stitch::Vec3 worldNormal=intersect.normal_;
                    
                    {//Scatter an individual photon...
                        stitch::Photon scatPhoton=pClosestMaterial->scatterPhoton_direct(worldNormal, worldPosition, *photon);//Russian roulette type scatter!
                        
                        if (scatPhoton.normDir_.lengthSq()>0.0f)//Successful scatter event.
                        {
                            inFlightPhotonVector_.push_back(new stitch::Photon(worldPosition+scatPhoton.normDir_*0.001f, scatPhoton.normDir_, scatPhoton.energy_, scatPhoton.scatterCount_));
                            
                            ++photonsScattered;
                            ++totalScattered;
                        }
                    }
                    
                    
                    {//! @todo Store scattered probability OR radiance lobes (similar to radiance photons, but combined with a scatter probablity distribution)
                        
                        //Want to use the lobes for calculating surface radiance AND resulting nearby irradiance (using PhD theory).
                        
                        //The key might be in the relation between scatter probability and radiance!!!
                        
                        //How does one scatter and trace radiance?
                    }

                }
                
            }
            
            ++photonsTraced;
            ++totalTraced;
            
            float progressIndication=((100.0f*photonNum)/((float)(photonsRadiated+photonsScattered)));
            if (progressIndication>nextProgressIndication)
            {
                std::cout << nextProgressIndication << "%...";
                std::cout.flush();
                nextProgressIndication+=20.0f;
            }
        }
        std::cout << "100%...done.\n";
        std::cout.flush();
    }
    
    stitch::Timer_t endTick=timer.tick();
    std::cout << "  Traced " << totalTraced << " photons in " << timer.delta_m(startTick, endTick) << " ms (" << totalTraced/timer.delta_s(startTick, endTick) << " photons/s).\n";
    std::cout.flush();
    
    
    std::cout << "  Visualising last inflight scattered photons...";
    std::cout.flush();
    
#ifdef USE_OSG
    {//Add scattered photons to osg geode.
        osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
        
        osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
        
        size_t numPhotons=inFlightPhotonVector_.size();
        size_t photonStride=stitch::MathUtil::max((size_t)1, numPhotons / ((size_t)100000));//Visualise at most 100000 photons.
        
        for (size_t photonNum=0; photonNum<numPhotons; photonNum+=photonStride)
        {
            Photon *photon=dynamic_cast<stitch::Photon *>(inFlightPhotonVector_[photonNum]);
            
            if (photon)
            {
                osgVertices->push_back(osg::Vec3(photon->centre_.x(), photon->centre_.y(), photon->centre_.z()));
                osgVertices->push_back(osg::Vec3(photon->centre_.x()+photon->normDir_.x(), photon->centre_.y()+photon->normDir_.y(), photon->centre_.z()+photon->normDir_.z()));
            }
        }
        
        osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
        osgGeometry->setVertexArray(osgVertices.get());
        osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgVertices->size()));
        
        osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
        osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        
        osg::Depth* depth = new osg::Depth();
        osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
        
        osg::Material *material = new osg::Material();
        material->setColorMode(osg::Material::DIFFUSE);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        osgStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
        
        osgGeode->addDrawable(osgGeometry.get());
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(scene_->sceneGraphMutex_);
            scene_->rendererGroup_->addChild(osgGeode);
        }
    }
#endif// USE_OSG
    std::cout << "done.\n";
    std::cout.flush();
    
    
    
    
    std::cout << "  Building photon map...";
    std::cout.flush();
    
    std::vector<stitch::Vec3> splitAxisVec;
    splitAxisVec.push_back(Vec3(1.0f, 0.0f, 0.0f));
    splitAxisVec.push_back(Vec3(0.0f, 1.0f, 0.0f));
    splitAxisVec.push_back(Vec3(0.0f, 0.0f, 1.0f));
    
    photonMap_->build(photonTreeChunkSize, 0, 1000, splitAxisVec);
    
    //=== Delete last in-flight photons and clear the vector...
    std::vector<stitch::Photon *>::const_iterator photonIter=inFlightPhotonVector_.begin();
    const std::vector<stitch::Photon *>::const_iterator photonIterEnd=inFlightPhotonVector_.end();
    for (; photonIter!=photonIterEnd; ++photonIter)
    {
        delete (*photonIter);
    }
    
    inFlightPhotonVector_.clear();//radiated and scattered photons.
    //===
    
    std::cout << "done.\n";
    std::cout.flush();
    
    return photonMap_->getNumItems();
}



//=======================================================================//
void stitch::LightFieldRenderer::gather(Ray &ray) const
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
            
            stitch::Vec3 worldPosition=ray.origin_ + ray.direction_*intersect.distance_;
            stitch::Vec3 worldNormal=intersect.normal_;
            
            //=== Photon map radiance estimate : BEGIN ===//
            if (ray.gatherDepth_>1)
            {
                stitch::Colour_t diffuseRefl=pClosestMaterial->getDiffuseRefl(worldPosition);
                
                if (diffuseRefl.lengthSq() > 0.0f)
                {
                    KNearestItems kNearestItems(worldPosition, powf(5.0f, 2.0f), 255);
                    photonMap_->getNearestK(&kNearestItems);
                    
                    if ((kNearestItems.searchRadiusSq_>0.0f)&&(kNearestItems.numItems_>0))
                    {
                        //=== Calculate photon irradiance ===//
                        stitch::Colour_t photonFlux;//Set to zero.
                        size_t numPhotons=0;
                        float filterWeightTotal=0.0f;
                        
                        const size_t numNearestItems=kNearestItems.numItems_;
                        
                        const float fluxRadiusSq=kNearestItems.heapArray_[0].first;//kNearestItems.searchRadiusSq_;//(kNearestItems.nearestItemVector_[0]->centre_-worldPosition).lengthSq();
                        
                        for (size_t i=0; i<numNearestItems; ++i)
                        {
                            stitch::Photon const *photon=dynamic_cast<Photon const *>(kNearestItems.heapArray_[i].second);
                            
                            //if (photon)
                            {
                                const float distSq=kNearestItems.heapArray_[i].first;
                                
                                const float cosTheta=((photon->normDir_ * worldNormal) * -1.0f);
                                
                                if (cosTheta>0.0f)
                                {
                                    const float filterWeight=1.0f - distSq/fluxRadiusSq;
                                    photonFlux.addScaled(photon->energy_, cosTheta*filterWeight);
                                    
                                    filterWeightTotal+=filterWeight;
                                    ++numPhotons;
                                }
                            }
                        }
                        
                        {
                            if (numPhotons>8)
                            {
                                filterWeightTotal/=numPhotons;
                                
                                //! @todo Calculate the convex hull of the photons to find the fluxArea.
                                const float fluxArea=(((float)M_PI)*fluxRadiusSq);
                                
                                Colour_t Fr=diffuseRefl*((float)M_1_PI);//pClosestMaterial->BSDF(worldPosition, photon->normDir_*(-1.0f), ray.direction_*(-1.0f), worldNormal);
                                ray.returnRadiance_+=(fluxArea==0.0f) ? stitch::Colour_t(0.0f, 0.0f, 0.0f) : (photonFlux / (filterWeightTotal*fluxArea)).cmult(Fr);
                            }
                        }
                        //=== ===//
                        
                    }
                }
                
            }
            //=== Photon map radiance estimate : END ===//
            
            
            {
                //Calculate emitted radiance from closest hit.
                ray.returnRadiance_+=pClosestMaterial->getEmittedRadiance(worldNormal, ray.direction_*(-1.0f), worldPosition);
                
                if (ray.gatherDepth_>1)
                {//Will still gather 'gatherDepth' levels.
                    
                    //Gather from specular trans direction.
                    stitch::Colour_t specTrans=pClosestMaterial->getSpecularTrans();
                    if (specTrans.isNotZero())
                    {//Gather radiance from specular trans direction.
                        stitch::Vec3 transRay=pClosestMaterial->whittedSpecRefractRay(ray.direction_, worldNormal);
                        
                        stitch::Ray tray(ray.id0_, ray.id1_,
                                         transRay,
                                         stitch::Vec3(worldPosition, transRay, 0.05f), //0.05 to jump over the back face of the thin transparent brush.
                                         ray.gatherDepth_-1);
                        
                        gather(tray);
                        
                        ray.returnRadiance_+=specTrans.cmult(tray.returnRadiance_);
                    }
                    
                    //Gather from specular refl direction.
                    stitch::Colour_t specRefl=pClosestMaterial->getSpecularRefl();
                    if (specRefl.isNotZero())
                    {//Gather radiance from specular refl direction.
                        stitch::Vec3 reflRay=pClosestMaterial->whittedSpecReflectRay(ray.direction_, worldNormal);
                        
                        stitch::Ray rray(ray.id0_, ray.id1_, reflRay,
                                         stitch::Vec3(worldPosition, reflRay, 0.001f),
                                         ray.gatherDepth_-1);
                        
                        gather(rray);
                        
                        ray.returnRadiance_+=specRefl.cmult(rray.returnRadiance_);
                    }
                }
            }
            //=========================================//
        }
    }
}

//=======================================================================//
void stitch::LightFieldRenderer::preForwardRender(RadianceMap &radianceMap,
                                                  const stitch::Camera * const camera,
                                                  const float frameDeltaTime)
{
#ifdef USE_OSG
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(scene_->sceneGraphMutex_);
        scene_->rendererGroup_->removeChildren(0, scene_->rendererGroup_->getNumChildren());
    }
#endif// USE_OSG
    
    //=== Generate the photonMap ===
    {
        size_t numRecordedPhotons=tracePhotons(frameDeltaTime, 64);
        std::cout << "  Recorded " << numRecordedPhotons << " photons.\n";
        std::cout.flush();
    }
    //==============================
}
