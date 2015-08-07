/*
 * $Id: LightBeamRenderer.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  LightBeamRenderer.h
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

#ifndef STITCH_LIGHT_BEAM_RENDERER_H
#define STITCH_LIGHT_BEAM_RENDERER_H

namespace stitch {
    class Scene;
}

#include "BeamTree.h"
#include "KDTree.h"
#include "Renderer.h"
#include "Math/Ray.h"
#include "EntryExit.h"

//============ OSG Includes Begin =================
#include "OSGUtils/StitchOSG.h"
//============ OSG Includes End =================


namespace stitch {
    
    class LightPathSegment : public Ray, public Intersection
    {
    public:
        
        LightPathSegment(const Ray &ray, const Intersection &intersection, Object const *generator, float convolvedGeneratorSD, uint32_t userGroupID) :
        Ray(ray), Intersection(intersection),
        generator_(generator),
        convolvedGeneratorSD_(convolvedGeneratorSD),
        userGroupID_(userGroupID)
        {}
        
        Object const *generator_;
        float convolvedGeneratorSD_;
        
        uint32_t userGroupID_;
    };
    
    
    //! Test for ray path equavalence i.e. whether path segments are coherent AND paths of same length.
    inline bool operator == (const std::vector<LightPathSegment> &lhs, const std::vector<LightPathSegment> &rhs)
    {
        if (lhs.size()==rhs.size())
        {
            const size_t numRayIntersections=lhs.size();
            
            for (size_t rayIntersectionNum=0; rayIntersectionNum<numRayIntersections; ++rayIntersectionNum)
            {
                if ( (lhs[rayIntersectionNum].itemID_!=rhs[rayIntersectionNum].itemID_) ||
                     ((lhs[rayIntersectionNum].itemID_!=0)&&(rhs[rayIntersectionNum].itemID_!=0)&&(fabsf(lhs[rayIntersectionNum].normal_ * rhs[rayIntersectionNum].normal_)<cosf(5.0f*((float)M_PI)/180.0f))) )
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    
    //! Test for ray path inequavalence  i.e. whether path segments are incoherent OR paths of different length.
    inline bool operator != (const std::vector<LightPathSegment> &lhs, const std::vector<LightPathSegment> &rhs)
    {
        return !(lhs==rhs);
    }
    
    
    /*! Light Beam Tracer (LBT) renderer. */
    class LightBeamRenderer : public ForwardRenderer
    {
    public:
        LightBeamRenderer(Scene * const scene);
        
        virtual ~LightBeamRenderer()
        {
            delete beamTree_;
        }
        
    private:
        //! Light beam segment acceleration structure.
        stitch::BeamTree *beamTree_;
        
        //! Method to from an initial path create an intersection path along the peak BRDF scatter directions.
        void createBRDFPeakLightPath(stitch::Ray const & initialRay, Object const * const generator, std::vector<stitch::LightPathSegment> & rayIntersectionPath, const size_t maxPathLength);
        
        
        //! Traces the light beams and builds the beamTree_. Called from preForwardRender.
        size_t traceBeams(const float frameDeltaTime);
        
        //! Initially stores the light image mesh in the first segment of each path and then later the entire path for each light image mesh path segment.
        std::vector< std::vector<stitch::LightPathSegment> > lightPathVec_;

        //! Stores the vertex/lightpath indices of each light image mesh triangle.
        std::vector<size_t> meshIndexVec_;
        
        //! Experimental for LBT V2.0: Used to map a lightpath index back to the light image mesh triangles that make use of it.
        std::vector< std::vector<size_t> > pathIndexToMeshIndicesMap_;
        
        //! Num light path segments to skip before adding beam sgements to beamTree_. Set to 1 to not render direct lighting for example.
        const size_t NumRayIntersectionsToSkip_;
        
        //! The maximum length of any light path. Set to 3 to render only up to second bounce (three segments).
        const size_t MaxLightPathLength_;
        
    protected:
        virtual void preForwardRender(RadianceMap &radianceMap,
                                      const stitch::Camera * const camera,
                                      const float frameDeltaTime);
        
        virtual void gather(Ray &ray) const;
    };
    
}

#endif// STITCH_LIGHT_BEAM_RENDERER_H
