/*
 * $Id: LightBeamRenderer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
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

#include "LightBeamRenderer.h"
#include "Materials/DiffuseMaterial.h"
#include "Materials/GlossyMaterial.h"
#include "Materials/GlossyTrnsMaterial.h"
#include "../KDTree.h"
#include "Timer.h"

#include <algorithm>
#include <vector>
#include <map>

#ifdef USE_CXX11
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif

#include "OSGUtils/StitchOSG.h"


//====================================================================================================//
stitch::LightBeamRenderer::LightBeamRenderer(Scene * const scene) :
ForwardRenderer(scene, 3, 1, true),
NumRayIntersectionsToSkip_(1),
MaxLightPathLength_(3)
{
    beamTree_=new stitch::BeamTree;
}

//====================================================================================================//
void stitch::LightBeamRenderer::createBRDFPeakLightPath(stitch::Ray const & initialRay, Object const * const initialGenerator, std::vector<stitch::LightPathSegment> & rayIntersectionPath, const size_t maxPathLength)
{
    //=== Add first path segment; may or may not end in an intersection ===///
    float convolvedGeneratorSD=0.0f;
    stitch::Ray ray=initialRay;
    stitch::Object const * generator=initialGenerator;
    uint32_t pathSegmentGroupID=(generator->itemID_) << 16; //! @todo The pathSegmentGroupID could be propagated from the mesh division operation and should really include spatial as well as object ID info!
    
    stitch::Intersection intersection(ray.id0_, ray.id1_, ((float)FLT_MAX));
    scene_->calcIntersection(ray, intersection);
    
#ifdef USE_CXX11
    rayIntersectionPath.emplace_back(ray, intersection, generator, convolvedGeneratorSD, pathSegmentGroupID);
#else
    rayIntersectionPath.push_back(stitch::LightPathSegment(ray, intersection, generator, convolvedGeneratorSD, pathSegmentGroupID));
#endif
    //=== ===//
    
    while ((intersection.itemID_)&&(rayIntersectionPath.size()<maxPathLength))
    {//while there is a valid intersection (valid starting point for next segment AND our path is not at max length...
        stitch::Material *material=(static_cast<const stitch::Object *>(intersection.itemPtr_))->pMaterial_;
        
        stitch::Vec3 BRDFPeakVec;
        
        //=== ===
        if (
            //(material->getType()!=Material::SPECULAR_MATERIAL) &&
            (material->getType()!=Material::GLOSSY_MATERIAL) &&
            (material->getType()!=Material::GLOSSY_TRANS_MATERIAL) //&&
            //(material->getType()!=Material::DIFFUSE_MATERIAL)
            //            (material->getType()!=Material::PHONG_MATERIAL)
            //            (material->getType()!=Material::EMISSIVE_MATERIAL) &&
            //            (material->getType()!=Material::BLINN_PHONG_MATERIAL)
            )
        {
            BRDFPeakVec=stitch::Vec3();
        } else
        {
            BRDFPeakVec=material->getBSDFPeak(ray.direction_ * -1.0f, intersection.normal_);
        }
        //=== ===
        
        if (BRDFPeakVec.lengthSq()>0.0f)
        {
            //=== Lookup simple scatter distribution without considering neighbours. This will be updated in future to better propagate the scatter distribution.
            float materialSD=0.0f;
            
            if (material->getType()==Material::GLOSSY_MATERIAL)
            {
                materialSD=(static_cast<GlossyMaterial *>(material))->sd_;
            } else
                if (material->getType()==Material::GLOSSY_TRANS_MATERIAL)
                {
                    materialSD=(static_cast<GlossyTransMaterial *>(material))->sd_;
                } else
                    if (material->getType()==Material::DIFFUSE_MATERIAL)
                    {
                        materialSD=/*(static_cast<DiffuseMaterial *>(material))->sd_*/((float)M_PI)*0.2f;//Preset SD for diffuse material.
                    }
            
            convolvedGeneratorSD=stitch::MathUtil::min(sqrtf(convolvedGeneratorSD*convolvedGeneratorSD + materialSD*materialSD), ((float)M_PI)*0.2f);
            //=== ===
            
            ray=stitch::Ray(ray.id0_, ray.id1_, BRDFPeakVec, ray.direction_*intersection.distance_ + ray.origin_ + BRDFPeakVec*(intersection.itemPtr_->radiusBV_ * 0.00001f));
            
            generator=static_cast<const stitch::Object *>(intersection.itemPtr_);
            pathSegmentGroupID=(pathSegmentGroupID>>16) + ((intersection.itemID_)<<16);//pathSegmentGroupID should be > 0
            
            intersection=stitch::Intersection(ray.id0_, ray.id1_, ((float)FLT_MAX));
            scene_->calcIntersection(ray, intersection);
            
#ifdef USE_CXX11
            rayIntersectionPath.emplace_back(ray, intersection, generator, convolvedGeneratorSD, pathSegmentGroupID);
#else
            rayIntersectionPath.push_back(stitch::LightPathSegment(ray, intersection, generator, convolvedGeneratorSD, pathSegmentGroupID));
#endif
            //=== ===
        } else
        {
            break;//from while-loop.
        }
    }
}



//=======================================================================//

size_t stitch::LightBeamRenderer::traceBeams(const float frameDeltaTime)
{
    beamTree_->clear();
    
    {
        //Steps to follow:
        //1a) Trace light ray intersection paths.
        //1b) Refine light ray intersection paths if required.
        //1c) Fix cracks.
        
        //2a) Propagate generator scatter distribution.
        //2b) Setup beams (i.e. beam tree) from light path segments.
        //2c) Merge neighbouring beams?
        
        meshIndexVec_.clear();
        lightPathVec_.clear();
        
        stitch::Timer timer;
        stitch::Timer_t startTick, endTick;
        
        {//=== 1a) Initial light ray intersection paths. ===
            std::cout << "  Equidistant vectors...";
            std::cout.flush();
            startTick=timer.tick();
            
            std::vector<size_t> binIndices;
            std::vector<stitch::Vec3> vectors;
            stitch::Vec3::equidistantVectors_IcosahedronBased(642, vectors, binIndices);
            //stitch::Vec3::equidistantVectors_IcosahedronBased(2562, vectors, binIndices);
            //stitch::Vec3::equidistantVectors_IcosahedronBased(10242, vectors, binIndices);
            //stitch::Vec3::equidistantVectors_IcosahedronBased(40962, vectors, binIndices);
            //stitch::Vec3::equidistantVectors_IcosahedronBased(163842, vectors, binIndices);
            //stitch::Vec3::equidistantVectors_IcosahedronBased(655362, vectors, binIndices);
            
            
            const size_t numVectors=vectors.size();
            endTick=timer.tick();
            std::cout << numVectors << " vectors in "<< timer.delta_m(startTick, endTick) << " ms...done.\n";
            std::cout.flush();
            
            /*
             std::cout << "  Relaxing equidistant vectors...";
             std::cout.flush();
             startTick=timer.tick();
             */
            // stitch::Vec3::relaxEquidistantVectorsII(vectors, 7);
            /*
             endTick=timer.tick();
             std::cout << numVectors << " vectors in "<< timer.delta_m(startTick, endTick) << " ms...done.\n";
             std::cout.flush();
             */
            
            std::cout << "  Tracing light paths...";
            std::cout.flush();
            startTick=timer.tick();
            
            for (size_t vecNum=0; vecNum<numVectors; ++vecNum)
            {
                std::vector<stitch::LightPathSegment> lightPath;
                
                stitch::Ray initialRay(vecNum, 0,
                                       vectors[vecNum],
                                       stitch::Vec3(scene_->light_->centre_ , vectors[vecNum], scene_->light_->radiusBV_*1.01f)
                                       );
                
                createBRDFPeakLightPath(initialRay, scene_->light_,
                                        lightPath,
                                        MaxLightPathLength_);
                
                lightPathVec_.push_back(lightPath);
            }
            
            size_t numBinIndices=binIndices.size();
            for (size_t indexNum=0; indexNum<numBinIndices; ++indexNum)
            {
                meshIndexVec_.push_back(binIndices[indexNum]);
            }
            
            endTick=timer.tick();
            std::cout << lightPathVec_.size() << " paths in "<< timer.delta_m(startTick, endTick) << " ms...done.\n";
            std::cout.flush();
        }//=== ===
        
        
        stitch::KDTree kdTree;//Keep a tame kd-tree on the side for efficient searching of duplicate vertices to be merged while sub-dividing.
        
        {//=== 1b) Refine ray intersection paths ===
            // Note: This code is similar to the refinement code in stitch::Vec3::equidistantVectors_IcosahedronBased
            // This refinement is however done conditionally based on path equivalance.
            std::cout << "  Refining light paths...";
            std::cout.flush();
            startTick=timer.tick();
            
            const size_t numIterations=6;
            for (size_t i=0; i<numIterations; ++i)
            {
                size_t numBeamIndices=meshIndexVec_.size();
                
                for (size_t indexNum=0; indexNum<numBeamIndices; indexNum+=3)
                {
                    const size_t i0=meshIndexVec_[indexNum+0];
                    const size_t i1=meshIndexVec_[indexNum+1];
                    const size_t i2=meshIndexVec_[indexNum+2];
                    
                    const std::vector<LightPathSegment> lightPath0=lightPathVec_[i0];
                    const std::vector<LightPathSegment> lightPath1=lightPathVec_[i1];
                    const std::vector<LightPathSegment> lightPath2=lightPathVec_[i2];
                    
                    if ((lightPath1!=lightPath0) ||
                        (lightPath2!=lightPath0)
                        )
                    {//These paths differ, therefore subdivide.
                        const stitch::Vec3 v0=lightPath0[0].origin_;
                        const stitch::Vec3 v1=lightPath1[0].origin_;
                        const stitch::Vec3 v2=lightPath2[0].origin_;
                        
                        const stitch::Vec3 n0=lightPath0[0].direction_;
                        const stitch::Vec3 n1=lightPath1[0].direction_;
                        const stitch::Vec3 n2=lightPath2[0].direction_;
                        
                        stitch::Vec3 n01=(n0+n1);
                        n01.normalise();
                        stitch::Vec3 n12=(n1+n2);
                        n12.normalise();
                        stitch::Vec3 n20=(n2+n0);
                        n20.normalise();
                        
                        const stitch::Vec3 v01=scene_->light_->centre_ + n01*scene_->light_->radiusBV_*1.01f;
                        const stitch::Vec3 v12=scene_->light_->centre_ + n12*scene_->light_->radiusBV_*1.01f;
                        const stitch::Vec3 v20=scene_->light_->centre_ + n20*scene_->light_->radiusBV_*1.01f;
                        
                        int32_t i01=-1;
                        int32_t i12=-1;
                        int32_t i20=-1;
                        
                        //=== Check if any of the new vertices already exist ===
                        {
                            float radius01=1.0f;
                            stitch::BoundingVolume *bv01=kdTree.getNearest(v01, radius01);
                            
                            float radius12=1.0f;
                            stitch::BoundingVolume *bv12=kdTree.getNearest(v12, radius12);
                            
                            float radius20=1.0f;
                            stitch::BoundingVolume *bv20=kdTree.getNearest(v20, radius20);
                            
                            if (bv01)
                            {
                                const uint32_t vertexNum01=bv01->userIndex_;
                                if ( Vec3::calcDistToPointSq(lightPathVec_[vertexNum01][0].origin_, v01) < stitch::Vec3::calcDistToPoint(v1, v0)*0.0001f )
                                {
                                    i01=vertexNum01;
                                }
                            }
                            
                            if (bv12)
                            {
                                const uint32_t vertexNum12=bv12->userIndex_;
                                if ( Vec3::calcDistToPointSq(lightPathVec_[vertexNum12][0].origin_, v12) < stitch::Vec3::calcDistToPoint(v2, v1)*0.0001f )
                                {
                                    i12=vertexNum12;
                                }
                            }
                            
                            if (bv20)
                            {
                                const uint32_t vertexNum20=bv20->userIndex_;
                                if ( Vec3::calcDistToPointSq(lightPathVec_[vertexNum20][0].origin_, v20) < stitch::Vec3::calcDistToPoint(v0, v2)*0.0001f )
                                {
                                    i20=vertexNum20;
                                }
                            }
                        }
                        //=== ===//
                        
                        if (i01<0)
                        {//This is a new vertex, so add it.
                            const Ray initialRay01(lightPath0[0].id0_, lightPath0[0].id1_, n01, v01);
                            
                            std::vector<stitch::LightPathSegment> lightPath01;
                            createBRDFPeakLightPath(initialRay01, scene_->light_,
                                                    lightPath01,
                                                    MaxLightPathLength_);
                            
                            lightPathVec_.push_back(lightPath01);
                            
                            i01=lightPathVec_.size()-1;
                            
                            kdTree.addItem(new stitch::BoundingVolume(lightPathVec_[lightPathVec_.size()-1][0].origin_,
                                                                      0.0f,
                                                                      lightPathVec_.size()-1));
                        }
                        
                        if (i12<0)
                        {//This is a new vertex, so add it.
                            const Ray initialRay12(lightPath1[0].id0_, lightPath1[0].id1_, n12, v12);
                            
                            std::vector<stitch::LightPathSegment> lightPath12;
                            createBRDFPeakLightPath(initialRay12, scene_->light_,
                                                    lightPath12,
                                                    MaxLightPathLength_);
                            
                            lightPathVec_.push_back(lightPath12);
                            
                            i12=lightPathVec_.size()-1;
                            
                            kdTree.addItem(new stitch::BoundingVolume(lightPathVec_[lightPathVec_.size()-1][0].origin_,
                                                                      0.0f,
                                                                      lightPathVec_.size()-1));
                        }
                        
                        if (i20<0)
                        {//This is a new vertex, so add it.
                            const Ray initialRay20(lightPath2[0].id0_, lightPath2[0].id1_, n20, v20);
                            
                            std::vector<stitch::LightPathSegment> rayIntersectionPath20;
                            createBRDFPeakLightPath(initialRay20, scene_->light_,
                                                    rayIntersectionPath20,
                                                    MaxLightPathLength_);
                            
                            lightPathVec_.push_back(rayIntersectionPath20);
                            
                            i20=lightPathVec_.size()-1;
                            
                            kdTree.addItem(new stitch::BoundingVolume(lightPathVec_[lightPathVec_.size()-1][0].origin_,
                                                                      0.0f,
                                                                      lightPathVec_.size()-1));
                        }
                        
                        std::vector<stitch::Vec3> splitAxisVec;
                        splitAxisVec.push_back(Vec3(1.0f, 0.0f, 0.0f));
                        splitAxisVec.push_back(Vec3(0.0f, 1.0f, 0.0f));
                        splitAxisVec.push_back(Vec3(0.0f, 0.0f, 1.0f));
                        
                        kdTree.build(KDTREE_DEFAULT_CHUNK_SIZE, 0, 1000, splitAxisVec);
                        
                        //=== Add refined mesh elements from new vertices or previously refined neighbours ===
                        {
                            meshIndexVec_[indexNum+0]=i0;
                            meshIndexVec_[indexNum+1]=i01;
                            meshIndexVec_[indexNum+2]=i20;
                        }
                        
                        {
                            meshIndexVec_.push_back(i01);
                            meshIndexVec_.push_back(i12);
                            meshIndexVec_.push_back(i20);
                        }
                        
                        {
                            meshIndexVec_.push_back(i20);
                            meshIndexVec_.push_back(i12);
                            meshIndexVec_.push_back(i2);
                        }
                        
                        {
                            meshIndexVec_.push_back(i01);
                            meshIndexVec_.push_back(i1);
                            meshIndexVec_.push_back(i12);
                        }
                        //=== ===
                    }
                }
            }
            
            endTick=timer.tick();
            std::cout << lightPathVec_.size() << " paths in "<< timer.delta_m(startTick, endTick) << " ms...done.\n";
            std::cout.flush();
        }//=== ===
        
        
        /*
         {//=== 1c) Remove potential cracks from mesh ===
         std::cout << "  Removing potential light image mesh cracks...";
         std::cout.flush();
         startTick=timer.tick();
         
         bool cracksRemainMaybe;
         do
         {
         cracksRemainMaybe=false;
         
         size_t numBeamIndices=meshIndexVec_.size();
         
         for (size_t indexNum=0; indexNum<numBeamIndices; indexNum+=3)
         {
         const size_t i0=meshIndexVec_[indexNum+0];
         const size_t i1=meshIndexVec_[indexNum+1];
         const size_t i2=meshIndexVec_[indexNum+2];
         
         const std::vector<LightPathSegment> lightPath0=lightPathVec_[i0];
         const std::vector<LightPathSegment> lightPath1=lightPathVec_[i1];
         const std::vector<LightPathSegment> lightPath2=lightPathVec_[i2];
         
         const stitch::Vec3 v0=lightPath0[0].origin_;
         const stitch::Vec3 v1=lightPath1[0].origin_;
         const stitch::Vec3 v2=lightPath2[0].origin_;
         
         const stitch::Vec3 n0=lightPath0[0].direction_;
         const stitch::Vec3 n1=lightPath1[0].direction_;
         const stitch::Vec3 n2=lightPath2[0].direction_;
         
         stitch::Vec3 n01=(n0+n1);
         n01.normalise();
         stitch::Vec3 n12=(n1+n2);
         n12.normalise();
         stitch::Vec3 n20=(n2+n0);
         n20.normalise();
         
         //Find positions of potential subdivided neighbours' vertices.
         const stitch::Vec3 v01=scene_->light_->centre_ + n01*scene_->light_->radiusBV_*1.01f;
         const stitch::Vec3 v12=scene_->light_->centre_ + n12*scene_->light_->radiusBV_*1.01f;
         const stitch::Vec3 v20=scene_->light_->centre_ + n20*scene_->light_->radiusBV_*1.01f;
         
         
         int32_t i01=-1;
         int32_t i12=-1;
         int32_t i20=-1;
         
         {//Find potential neighbour subdivision.
         float radius01=1.0f;
         stitch::BoundingVolume *bv01=kdTree.getNearest(v01, radius01);
         
         float radius12=1.0f;
         stitch::BoundingVolume *bv12=kdTree.getNearest(v12, radius12);
         
         float radius20=1.0f;
         stitch::BoundingVolume *bv20=kdTree.getNearest(v20, radius20);
         
         if (bv01)
         {
         const uint32_t vertexNum01=bv01->userIndex_;
         if ( Vec3::calcDistToPointSq(lightPathVec_[vertexNum01][0].origin_, v01) < stitch::Vec3::calcDistToPoint(v1, v0)*0.0001f )
         {
         i01=vertexNum01;
         }
         }
         
         if (bv12)
         {
         const uint32_t vertexNum12=bv12->userIndex_;
         if ( Vec3::calcDistToPointSq(lightPathVec_[vertexNum12][0].origin_, v12) < stitch::Vec3::calcDistToPoint(v2, v1)*0.0001f )
         {
         i12=vertexNum12;
         }
         }
         
         if (bv20)
         {
         const uint32_t vertexNum20=bv20->userIndex_;
         if ( Vec3::calcDistToPointSq(lightPathVec_[vertexNum20][0].origin_, v20) < stitch::Vec3::calcDistToPoint(v0, v2)*0.0001f )
         {
         i20=vertexNum20;
         }
         }
         }
         //=== ===//
         
         size_t numCracks=0;
         
         if (i01>=0)
         {//There is a neighbouring subdivision.
         cracksRemainMaybe=true;
         ++numCracks;
         }
         
         if (i12>=0)
         {//There is a neighbouring subdivision.
         cracksRemainMaybe=true;
         ++numCracks;
         }
         
         if (i20>=0)
         {//There is a neighbouring subdivision.
         cracksRemainMaybe=true;
         ++numCracks;
         }
         
         if (numCracks==1)
         {
         if (i01>=0)
         {
         meshIndexVec_[indexNum+0]=i0;
         meshIndexVec_[indexNum+1]=i01;
         meshIndexVec_[indexNum+2]=i2;
         
         meshIndexVec_.push_back(i01);
         meshIndexVec_.push_back(i1);
         meshIndexVec_.push_back(i2);
         } else
         if (i12>=0)
         {
         meshIndexVec_[indexNum+0]=i0;
         meshIndexVec_[indexNum+1]=i1;
         meshIndexVec_[indexNum+2]=i12;
         
         meshIndexVec_.push_back(i0);
         meshIndexVec_.push_back(i12);
         meshIndexVec_.push_back(i2);
         } else
         if (i20>=0)
         {
         meshIndexVec_[indexNum+0]=i0;
         meshIndexVec_[indexNum+1]=i1;
         meshIndexVec_[indexNum+2]=i20;
         
         meshIndexVec_.push_back(i20);
         meshIndexVec_.push_back(i1);
         meshIndexVec_.push_back(i2);
         }
         } else
         if (numCracks==2)
         {
         if ((i01>=0)&&(i12>=0))
         {
         meshIndexVec_[indexNum+0]=i0;
         meshIndexVec_[indexNum+1]=i01;
         meshIndexVec_[indexNum+2]=i2;
         
         meshIndexVec_.push_back(i01);
         meshIndexVec_.push_back(i1);
         meshIndexVec_.push_back(i12);
         
         meshIndexVec_.push_back(i01);
         meshIndexVec_.push_back(i12);
         meshIndexVec_.push_back(i2);
         } else
         if ((i12>=0)&&(i20>=0))
         {
         meshIndexVec_[indexNum+0]=i0;
         meshIndexVec_[indexNum+1]=i1;
         meshIndexVec_[indexNum+2]=i12;
         
         meshIndexVec_.push_back(i12);
         meshIndexVec_.push_back(i2);
         meshIndexVec_.push_back(i20);
         
         meshIndexVec_.push_back(i0);
         meshIndexVec_.push_back(i12);
         meshIndexVec_.push_back(i20);
         } else
         if ((i20>=0)&&(i01>=0))
         {
         meshIndexVec_[indexNum+0]=i0;
         meshIndexVec_[indexNum+1]=i01;
         meshIndexVec_[indexNum+2]=i20;
         
         meshIndexVec_.push_back(i01);
         meshIndexVec_.push_back(i1);
         meshIndexVec_.push_back(i20);
         
         meshIndexVec_.push_back(i1);
         meshIndexVec_.push_back(i2);
         meshIndexVec_.push_back(i20);
         }
         } else
         if (numCracks==3)
         {//numCracks is three
         meshIndexVec_[indexNum+0]=i0;
         meshIndexVec_[indexNum+1]=i01;
         meshIndexVec_[indexNum+2]=i20;
         
         meshIndexVec_.push_back(i01);
         meshIndexVec_.push_back(i12);
         meshIndexVec_.push_back(i20);
         
         meshIndexVec_.push_back(i20);
         meshIndexVec_.push_back(i12);
         meshIndexVec_.push_back(i2);
         
         meshIndexVec_.push_back(i01);
         meshIndexVec_.push_back(i1);
         meshIndexVec_.push_back(i12);
         }
         //=== ===
         }
         
         std::cout << ".";
         std::cout.flush();
         } while (cracksRemainMaybe);
         
         endTick=timer.tick();
         std::cout << timer.delta_m(startTick, endTick) << " ms...done.\n";
         std::cout.flush();
         }//=== ===
         */
        
        
        
        {//2b) LBT Version 1.0: Setup beams (i.e. beam tree) from light paths
            std::cout << "  Beams from light image mesh...";
            std::cout.flush();
            
            startTick=timer.tick();
            
            const size_t numLightMeshIndices=meshIndexVec_.size();
            
            for (size_t indexNum=0; indexNum<numLightMeshIndices; indexNum+=3)
            {
                const std::vector<LightPathSegment> lightPath0=lightPathVec_[meshIndexVec_[indexNum+0]];
                const std::vector<LightPathSegment> lightPath1=lightPathVec_[meshIndexVec_[indexNum+1]];
                const std::vector<LightPathSegment> lightPath2=lightPathVec_[meshIndexVec_[indexNum+2]];
                
                
                const size_t numLightPathSegments=stitch::MathUtil::min(stitch::MathUtil::min(lightPath0.size(),
                                                                                              lightPath1.size()),
                                                                        lightPath2.size());
                
                if (numLightPathSegments>NumRayIntersectionsToSkip_)
                {
                    stitch::Colour_t fluxSPD;
                    
                    for (size_t lightPathSegmentNum=0; lightPathSegmentNum<numLightPathSegments; ++lightPathSegmentNum)
                    {
                        const LightPathSegment lightPathSegment0(lightPath0[lightPathSegmentNum]);
                        const LightPathSegment lightPathSegment1(lightPath1[lightPathSegmentNum]);
                        const LightPathSegment lightPathSegment2(lightPath2[lightPathSegmentNum]);
                        
                        if (
                            (/*(lightPathSegment0.itemID_)&&(lightPathSegment1.itemID_)&&(lightPathSegment2.itemID_)*/true) &&
                            (lightPathSegment0.userGroupID_==lightPathSegment1.userGroupID_)&&(lightPathSegment0.userGroupID_==lightPathSegment2.userGroupID_) && //generator group path id.
                            (lightPathSegment0.itemID_==lightPathSegment1.itemID_)&&(lightPathSegment0.itemID_==lightPathSegment2.itemID_) //this intersection
                            )
                        {
                            //! @todo Investigate the below averaging in future.
                            const float glossySD=(lightPathSegment0.convolvedGeneratorSD_+
                                                  lightPathSegment1.convolvedGeneratorSD_+
                                                  lightPathSegment2.convolvedGeneratorSD_)/3.0f;
                            
                            const stitch::Vec3 v0(lightPathSegment0.origin_);
                            const stitch::Vec3 v1(lightPathSegment1.origin_);
                            const stitch::Vec3 v2(lightPathSegment2.origin_);
                            
                            const stitch::Vec3 vertVec0=lightPathSegment0.direction_;
                            const stitch::Vec3 vertVec1=lightPathSegment1.direction_;
                            const stitch::Vec3 vertVec2=lightPathSegment2.direction_;
                            
                            const stitch::Vec3 vC((v0+v1+v2)/3.0f);
                            stitch::Vec3 vertVecC=(vertVec0+vertVec1+vertVec2);
                            vertVecC.normalise();
                            
                            if (lightPathSegmentNum==0)
                            {
                                const float lightDistanceSq=Vec3::calcDistToPointSq(vC, scene_->light_->centre_);
                                const float solidAngle=stitch::Vec3::crossLength(v1, v2, v0) * 0.5f / lightDistanceSq;
                                fluxSPD=scene_->light_->getSPD() * solidAngle/(4.0f*((float)M_PI));
                            } else
                            {
                                fluxSPD*=lightPathSegment0.generator_->pMaterial_->getAlbedo();
                            }
                            
                            if (lightPathSegmentNum>=NumRayIntersectionsToSkip_)
                            {
                                bool beamSegmentHasRealEndCap = false;
                                
                                const stitch::Plane generatorPlane(v0, v1, v2);
                                
                                BeamSegment * const beamSegment=new BeamSegment(generatorPlane,
                                                                                std::vector<Vec3>({vC, v0, v1, v2}),
                                                                                std::vector<Vec3>({vertVecC, vertVec0, vertVec1, vertVec2}),
                                                                                glossySD, fluxSPD);
                                
                                {//=== Calculate the end cap - only use info from light ===
                                    /*
                                     Vec3 endCapNormal;
                                     
                                     if (lightPathSegment0.objectID_) endCapNormal+=(generatorPlane.normal_*lightPathSegment0.normal_)<0.0f ? lightPathSegment0.normal_ : lightPathSegment0.normal_*(-1.0f);
                                     if (lightPathSegment1.objectID_) endCapNormal+=(generatorPlane.normal_*lightPathSegment1.normal_)<0.0f ? lightPathSegment1.normal_ : lightPathSegment1.normal_*(-1.0f);
                                     if (lightPathSegment2.objectID_) endCapNormal+=(generatorPlane.normal_*lightPathSegment2.normal_)<0.0f ? lightPathSegment2.normal_ : lightPathSegment2.normal_*(-1.0f);
                                     
                                     endCapNormal.normalise();
                                     
                                     stitch::Vec3 endCapIntersect0;
                                     stitch::Vec3 endCapIntersect1;
                                     stitch::Vec3 endCapIntersect2;
                                     
                                     size_t endCapDCount=0;
                                     float endCapD=0.0f;
                                     if (lightPathSegment0.objectID_)
                                     {
                                     endCapIntersect0=v0+vertVec0*(lightPathSegment0.distance_+0.001f);
                                     endCapD+=endCapIntersect0 * endCapNormal;
                                     ++endCapDCount;
                                     }
                                     if (lightPathSegment1.objectID_)
                                     {
                                     endCapIntersect1=v1+vertVec1*(lightPathSegment1.distance_+0.001f);
                                     endCapD+=endCapIntersect1 * endCapNormal;
                                     ++endCapDCount;
                                     }
                                     if (lightPathSegment2.objectID_)
                                     {
                                     endCapIntersect2=v2+vertVec2*(lightPathSegment2.distance_+0.001f);
                                     endCapD+=endCapIntersect2 * endCapNormal;
                                     ++endCapDCount;
                                     }
                                     endCapD/=endCapDCount;
                                     
                                     Plane endCap(endCapNormal, endCapD);
                                     */
                                    //OR
                                    
                                    std::vector<Vec3> endCapVertices;
                                    
                                    endCapVertices.emplace_back();//Reserve space for centre vertex.
                                    
                                    if ((lightPathSegment0.itemID_)&&(lightPathSegment1.itemID_)&&(lightPathSegment2.itemID_))
                                    {
                                        endCapVertices.emplace_back(v0+vertVec0*(lightPathSegment0.distance_+0.001f));//+0.001f to include the beam segment endcap within the beam.
                                        endCapVertices.emplace_back(v1+vertVec1*(lightPathSegment1.distance_+0.001f));
                                        endCapVertices.emplace_back(v2+vertVec2*(lightPathSegment2.distance_+0.001f));
                                        beamSegmentHasRealEndCap=true;
                                    } else
                                    {
                                        endCapVertices.emplace_back(v0+vertVec0*(100.0f+0.001f));//+0.001f to include the beam endcap within the beam.
                                        endCapVertices.emplace_back(v1+vertVec1*(100.0f+0.001f));
                                        endCapVertices.emplace_back(v2+vertVec2*(100.0f+0.001f));
                                        beamSegmentHasRealEndCap=false;
                                    }
                                    
                                    Plane endCap(endCapVertices[1], endCapVertices[2], endCapVertices[3]);
                                    
                                    endCapVertices[0]=(endCapVertices[1]+endCapVertices[2]+endCapVertices[3]);
                                    endCapVertices[0]/=3.0f;
                                    
                                    beamSegment->setEndCap(endCap, endCapVertices);
                                }//=== ===
                                
                                if (beamSegmentHasRealEndCap)
                                {//Only store coherent terminating beam Segments in BVH so that BVH efficiency is not thrashed.
                                    beamSegment->sanitise();//ensure the generator and end-caps orientations are correct.
                                    
                                    beamSegment->updateBV();
                                    
                                    beamTree_->beamSegmentVector_.push_back(beamSegment);
                                }
                            }
                        }
                    }
                }
            }
            
            std::cout << "building tree...";
            std::cout.flush();
            
            if (beamTree_->getNumBeamSegments())
            {
                beamTree_->build(16, 0);
                beamTree_->updateBV();
            }
            
            endTick=timer.tick();
            
            std::cout << beamTree_->getNumBeamSegments() << " beam Segments in "<< timer.delta_m(startTick, endTick) << " ms...done.\n";
            std::cout.flush();
        }
    }
    //=== ===
    
#ifdef USE_OSG
    {//Add ray paths to osg geode.
        osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
        
        osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
        osg::ref_ptr<osg::Vec3Array> osgVertColours=new osg::Vec3Array();
        
        size_t numBeamIndices=meshIndexVec_.size();
        for (size_t indexNum=0; indexNum<numBeamIndices; indexNum+=3)
        {
            const size_t pi0=meshIndexVec_[indexNum+0];
            const size_t pi1=meshIndexVec_[indexNum+1];
            const size_t pi2=meshIndexVec_[indexNum+2];
            
            size_t numRayIntersections=stitch::MathUtil::min(lightPathVec_[pi0].size(), stitch::MathUtil::min(lightPathVec_[pi1].size(), lightPathVec_[pi2].size()));
            
            for (size_t rayintersectionNum=NumRayIntersectionsToSkip_; rayintersectionNum<numRayIntersections; ++rayintersectionNum)
            {
                LightPathSegment ri0=lightPathVec_[pi0][rayintersectionNum];
                LightPathSegment ri1=lightPathVec_[pi1][rayintersectionNum];
                LightPathSegment ri2=lightPathVec_[pi2][rayintersectionNum];
                
                if ((ri0.generator_==ri1.generator_) &&
                    (ri0.generator_==ri2.generator_))
                {
                    stitch::Vec3 intersection0;
                    if (ri0.itemID_)
                    {
                        intersection0=ri0.origin_ + ri0.direction_*(ri0.distance_*0.99f);
                    }
                    
                    stitch::Vec3 intersection1;
                    if (ri1.itemID_)
                    {
                        intersection1=ri1.origin_ + ri1.direction_*(ri1.distance_*0.99f);
                    }
                    
                    stitch::Vec3 intersection2;
                    if (ri2.itemID_)
                    {
                        intersection2=ri2.origin_ + ri2.direction_*(ri2.distance_*0.99f);
                    }
                    
                    if ((ri0.itemID_)&&(ri1.itemID_)&&(ri2.itemID_) &&
                        (ri0.itemID_==ri1.itemID_)&&(ri0.itemID_==ri2.itemID_) &&
                        (ri0.userGroupID_==ri1.userGroupID_)&&(ri0.userGroupID_==ri2.userGroupID_))
                    {
                        stitch::Vec3 generatorColour0=Vec3::uniqueValue((uintptr_t)ri0.userGroupID_);
                        generatorColour0.positivise();
                        
                        stitch::Vec3 generatorColour1=Vec3::uniqueValue((uintptr_t)ri1.userGroupID_);
                        generatorColour1.positivise();
                        
                        stitch::Vec3 generatorColour2=Vec3::uniqueValue((uintptr_t)ri2.userGroupID_);
                        generatorColour2.positivise();
                        
                        //===
                        osgVertices->push_back(osg::Vec3(intersection0.x(), intersection0.y(), intersection0.z()));
                        osgVertices->push_back(osg::Vec3(intersection1.x(), intersection1.y(), intersection1.z()));
                        osgVertColours->push_back(osg::Vec3(generatorColour0.x(), generatorColour0.y(), generatorColour0.z()));
                        osgVertColours->push_back(osg::Vec3(generatorColour1.x(), generatorColour1.y(), generatorColour1.z()));
                        
                        //osgVertices->push_back(osg::Vec3(ri0.origin_.x(), ri0.origin_.y(), ri0.origin_.z()));
                        //osgVertices->push_back(osg::Vec3(intersection0.x(), intersection0.y(), intersection0.z()));
                        //osgVertColours->push_back(osg::Vec3(generatorColour0.x(), generatorColour0.y(), generatorColour0.z()));
                        //osgVertColours->push_back(osg::Vec3(generatorColour0.x(), generatorColour0.y(), generatorColour0.z()));
                        
                        //===
                        osgVertices->push_back(osg::Vec3(intersection1.x(), intersection1.y(), intersection1.z()));
                        osgVertices->push_back(osg::Vec3(intersection2.x(), intersection2.y(), intersection2.z()));
                        osgVertColours->push_back(osg::Vec3(generatorColour1.x(), generatorColour1.y(), generatorColour1.z()));
                        osgVertColours->push_back(osg::Vec3(generatorColour2.x(), generatorColour2.y(), generatorColour2.z()));
                        
                        //===
                        osgVertices->push_back(osg::Vec3(intersection2.x(), intersection2.y(), intersection2.z()));
                        osgVertices->push_back(osg::Vec3(intersection0.x(), intersection0.y(), intersection0.z()));
                        osgVertColours->push_back(osg::Vec3(generatorColour2.x(), generatorColour2.y(), generatorColour2.z()));
                        osgVertColours->push_back(osg::Vec3(generatorColour0.x(), generatorColour0.y(), generatorColour0.z()));
                    }
                }
            }
        }
        
        
        osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
        osgGeometry->setVertexArray(osgVertices.get());
        osgGeometry->setColorArray(osgVertColours.get());
        osgGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
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
            
            //=== Show beam Segment tree geometry ===
            //scene_->rendererGroup_->addChild(beamTree_->constructOSGNode());
        }
    }
#endif// USE_OSG
    
    return beamTree_->getNumBeamSegments();
}


//=======================================================================//
void stitch::LightBeamRenderer::gather(stitch::Ray &ray) const
{
    ray.returnRadiance_.setZeros();
    
    if (ray.gatherDepth_>0)
    {
        stitch::Intersection intersect=stitch::Intersection(ray.id0_, ray.id1_, ((float)FLT_MAX));
        scene_->calcIntersection(ray, intersect);
        
        if (intersect.itemPtr_)
        {
            stitch::Material const * const intersectMaterial=(static_cast<const stitch::Object *>(intersect.itemPtr_))->pMaterial_;
            stitch::Vec3 intersectPosition=ray.origin_ + ray.direction_*intersect.distance_;
            stitch::Vec3 intersectNormal=intersect.normal_;
            
            
            //=====================================================//
            {//=== LBT1.0 beamtree radiance estimate : BEGIN ===//
                //===================================================//
                //Notes: A beamtree exists that was built from the single bounce code or from the multi-bounce light path code.
                //       The beamtree is searched for beam segment bounding volumes containing the worldPosition.
                //       Problem: The bounding volumes for the beam Segments can become quite wide which would make the tree search less and less efficient.
                if (ray.gatherDepth_>1)
                {//Will still gather 'gatherDepth' levels.
                    //if (beamTree_->pointInBV(worldPosition)) Seems slower to include this test!
                    {
                        std::vector<stitch::BeamSegment const *> contribBeamSegmentVector;
                        //contribBeamSegmentVector.reserve(20000);
                        
                        beamTree_->getContributingBeamSegmentVector(intersectPosition, contribBeamSegmentVector);
                        
#ifdef USE_CXX11
                        for (auto const beamSegment : contribBeamSegmentVector)
                        {
#else
                            std::vector<stitch::BeamSegment const *>::const_iterator beamSegmentIter=contribBeamSegmentVector.begin();
                            const std::vector<stitch::BeamSegment const *>::const_iterator beamSegmentIterEnd=contribBeamSegmentVector.end();
                            for (; beamSegmentIter!=beamSegmentIterEnd; ++beamSegmentIter)
                            {
                                stitch::BeamSegment const * const beamSegment=*beamSegmentIter;
#endif
                                float specFluxArea=0.0f;
                                stitch::Vec3 specScatterDir;
                                const bool inSpecBeamSegment=beamSegment->pointInSpecBeamSegment(intersectPosition, specFluxArea, specScatterDir);
                                
                                if (specFluxArea!=0.0f)
                                {
                                    //Calculate irradiance and store in beamiance.
                                    if (beamSegment->glossySDRad_<=0.0f)
                                    {
                                        if (inSpecBeamSegment)
                                        {
                                            Colour_t beamSegmentIrradiance=beamSegment->fluxSPD_;
                                            beamSegmentIrradiance*=(fabsf(specScatterDir * intersectNormal)/specFluxArea);
                                            
                                            //Shadowing sorted by subdivision.
                                            Colour_t fr=intersectMaterial->getDiffuseRefl(intersectPosition)/((float)M_PI);//intersectMaterial->BSDF(intersectPosition, specScatterDir * (-1.0f), ray.direction_ * (-1.0f), intersectNormal);
                                            ray.returnRadiance_.addScaled(beamSegmentIrradiance, fr);
                                        }
                                    } else
                                    {
                                        const float beamSegmentFluxProb=beamSegment->calcGlossyFluxProb(intersectPosition);
                                        
                                        const float beamSegmentGeneratorArea=stitch::Vec3::crossLength(beamSegment->vertices_[1], beamSegment->vertices_[2], beamSegment->vertices_[3]) * 0.5f;
                                        const float areaAugment=0.0125f*(specFluxArea+beamSegmentGeneratorArea);
                                        
                                        if (inSpecBeamSegment)
                                        {//Shadowing sorted by subdivision.
                                            Colour_t beamSegmentIrradiance=beamSegment->fluxSPD_;
                                            beamSegmentIrradiance*=(fabsf(specScatterDir * intersectNormal)/(specFluxArea + areaAugment)) * beamSegmentFluxProb;
                                            
                                            const Colour_t fr=intersectMaterial->getDiffuseRefl(intersectPosition)/((float)M_PI);//intersectMaterial->BSDF(intersectPosition, specScatterDir * (-1.0f), ray.direction_ * (-1.0f), intersectNormal);
                                            ray.returnRadiance_.addScaled(beamSegmentIrradiance, fr);
                                        }
                                        else
                                        {//Trace shadow rays if glossy beamSegment and outside spec BV.
                                            Vec3 shadingVect=intersectPosition - beamSegment->vertices_[0];
                                            shadingVect.normalise();
                                            
                                            float shadingCosTheta=fabsf(shadingVect * intersectNormal);//beamSegment flux plane to intersect normal cosine for irradiance calc.
                                            
                                            //=== Use the specular scatter direction only while its shading cosine is larger! ===//
                                            const float specShadingCosTheta=fabsf(specScatterDir * intersectNormal);//beamSegment flux plane to intersect normal cosine for irradiance calc.
                                            
                                            if (specShadingCosTheta>shadingCosTheta)
                                            {
                                                shadingCosTheta=specShadingCosTheta;
                                                shadingVect=specScatterDir;
                                            }
                                            //=== ===//
                                            
                                            Colour_t rayRadiance=beamSegment->fluxSPD_;
                                            rayRadiance*=(shadingCosTheta/(specFluxArea + areaAugment)) * beamSegmentFluxProb;
                                            rayRadiance*=intersectMaterial->getDiffuseRefl(intersectPosition)/((float)M_PI);//intersectMaterial->BSDF(intersectPosition, shadingVect * (-1.0f), ray.direction_ * (-1.0f), intersectNormal);
                                            
                                            //if (rayRadiance.lengthSq()>(ray.returnRadiance_.lengthSq()*0.001f))//! Ignore relatively small beamSegment irradiances.
                                            {
                                                const float shadowSamplesPMSq=100.0f;
                                                const size_t numShadowSamples=lroundf(shadowSamplesPMSq*(beamSegmentGeneratorArea)+0.5f);//ToDo: should be projected beamSegmentGeneratorArea.
                                                
                                                std::vector<std::pair<stitch::Vec3, float> > shadowSamples;
                                                shadowSamples.reserve(numShadowSamples);
                                                //float maxScatterDistrValue=0.001f;//Used for importance sampling.
                                                
                                                
                                                /* todo: need to update the sampling... Do a close inspection of specular result...
                                                 for (size_t shadowSampleNum=0; shadowSampleNum<numShadowSamples; ++shadowSampleNum)
                                                 {
                                                 const float r1=stitch::GlobalRand::uniformSamplerFromArray();
                                                 const float r2=stitch::GlobalRand::uniformSamplerFromArray();
                                                 const float sqrtR1=sqrtf(r1);
                                                 
                                                 //Uniform sampling of triangle using barycentric coordinates.
                                                 const float a=1.0f-sqrtR1;
                                                 const float b=(1.0f-r2)*sqrtR1;
                                                 const float c=r2*sqrtR1;
                                                 
                                                 const stitch::Vec3 P(beamSegment->vertices_[1], beamSegment->vertices_[2], beamSegment->vertices_[3], a, b, c);
                                                 
                                                 const stitch::Vec3 shadowVec=P-intersectPosition;
                                                 
                                                 const float offSpecScatterAngle=acosf(fabsf(specScatterDir * shadowVec)/shadowVec.length());
                                                 
                                                 const float scatterDistrValue=expf(-powf((offSpecScatterAngle/beamSegment->glossySDRad_), 2.0f) * 0.5f);
                                                 
                                                 shadowSamples.push_back(std::pair<stitch::Vec3, float>(shadowVec, scatterDistrValue));
                                                 
                                                 if (scatterDistrValue>maxScatterDistrValue)
                                                 {
                                                 maxScatterDistrValue=scatterDistrValue;
                                                 }
                                                 }
                                                 */
                                                //OR
                                                //shadowSamples.push_back(std::pair<stitch::Vec3, float>(specScatterDir*(-1.0f), 10.0f));
                                                
                                                size_t shadowRaysNotBlocked=1;
                                                size_t shadowRaysTraced=1;
                                                
                                                /*
                                                 for (auto & shadowSample : shadowSamples)
                                                 {
                                                 const float randValue=stitch::GlobalRand::uniformSamplerFromArray() * maxScatterDistrValue;
                                                 
                                                 if (randValue<=shadowSample.second)
                                                 {
                                                 stitch::Vec3 shadowDir=shadowSample.first;
                                                 const float generatorDist=shadowDir.normalise_rt();
                                                 
                                                 Ray shadowRay(ray.id0_, ray.id1_, shadowDir, intersectPosition+shadowDir*intersect.itemPtr_->radiusBV_*0.0001f, 1);
                                                 stitch::Intersection intersect(ray.id0_, ray.id1_, ((float)FLT_MAX));
                                                 
                                                 scene_->calcIntersection(shadowRay, intersect);
                                                 
                                                 //todo: the shadow/self-shadow test should be overalled.
                                                 if ((intersect.itemID_!=0)&&((intersect.distance_)>=generatorDist*0.9f))//! @todo *0.9 to ensure that the intersection is not shorted due to shadow ray's 0.001 offset or curved surfaces.
                                                 {
                                                 ++shadowRaysNotBlocked;
                                                 }
                                                 
                                                 ++shadowRaysTraced;
                                                 }
                                                 }
                                                 */
                                                
                                                if (shadowRaysTraced)
                                                {
                                                    ray.returnRadiance_.addScaled(rayRadiance, (shadowRaysNotBlocked/((float)(shadowRaysTraced))));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                    }
                    //=================================================//
                }//=== LBT1.0 beamtree radiance estimate : END ===//
                //===================================================//
                
                
                //Calculate emitted radiance from closest hit.
                ray.returnRadiance_+=intersectMaterial->getEmittedRadiance(intersectNormal, ray.direction_*(-1.0f), intersectPosition);
                
                if (ray.gatherDepth_>1)
                {//Will still gather 'gatherDepth' levels. Simple Whitted forward rendering.
                    /*
                     //Gather from specular trans direction.
                     stitch::Colour_t specTrans=pClosestMaterial->getSpecularTrans();
                     if (specTrans.isNotZero())
                     {//Gather radiance from specular trans direction.
                     stitch::Vec3 transRay=pClosestMaterial->whittedSpecRefractRay(ray.direction_, worldNormal);
                     stitch::Ray tray(transRay, worldPosition+transRay*0.05, //0.2 to jump over the back face of the thin transparent brush.
                     ray.gatherDepth_-1);
                     
                     gather(tray);
                     
                     ray.returnRadiance_+=specTrans.cmult(tray.returnRadiance_);
                     }
                     */
                    
                    //Gather from specular refl direction.
                    stitch::Colour_t specRefl=intersectMaterial->getSpecularRefl();
                    if (specRefl.isNotZero())
                    {//Gather radiance from specular refl direction.
                        stitch::Vec3 reflDir=intersectMaterial->whittedSpecReflectRay(ray.direction_, intersectNormal);
                        
                        stitch::Ray rray(ray.id0_, ray.id1_, reflDir,
                                         stitch::Vec3(intersectPosition, reflDir, intersect.itemPtr_->radiusBV_*0.0001f),
                                         ray.gatherDepth_-1);
                        
                        gather(rray);
                        
                        ray.returnRadiance_+=specRefl.cmult(rray.returnRadiance_);
                    }
                }
            }
        }
    }
    
    
    //=======================================================================//
    void stitch::LightBeamRenderer::preForwardRender(RadianceMap &radianceMap,
                                                     const stitch::Camera * const camera,
                                                     const float frameDeltaTime)
    {
#ifdef USE_OSG
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(scene_->sceneGraphMutex_);
            scene_->rendererGroup_->removeChildren(0, scene_->rendererGroup_->getNumChildren());
        }
#endif// USE_OSG
        
        //=== Generate the beams ===
        {
            size_t numBeams=traceBeams(frameDeltaTime);
            
            std::cout << "  Generated " << numBeams << " beams.\n";
            std::cout.flush();
        }
        //======================
    }
    
    
