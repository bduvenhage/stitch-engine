/*
 * $Id: Scene.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Scene.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/02/01.
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

#ifndef STITCH_SCENE_H
#define STITCH_SCENE_H

namespace stitch {
    class Scene;
}

#include "BallTree.h"
#include "Math/Vec3.h"
#include "Math/Colour.h"

#include "Light.h"

namespace stitch {
	
    //! Contains the object tree and light source that together make up the scene to be rendered. 
	class Scene
    {
    public:
        
        Scene();
        
        ~Scene()
        {
            delete ballTree_;
        }
        
        size_t create(const std::string scene_name, const Vec3 &light_orig, const Colour_t &lightSPD,
                             const size_t objectTreeChunkSize,
                             const size_t internalObjectTreeChunkSize,
                             bool createOSGLineGeometry,
                             bool createOSGNormalGeometry,
                             float glossySD);
        
        void createCausticRing(const size_t internalObjectTreeChunkSize, float glossySD);
        void createCausticBunny(const size_t internalObjectTreeChunkSize, float glossySD);
        void createCausticDragon(const size_t internalObjectTreeChunkSize, float glossySD);
        void createCausticGear(const size_t internalObjectTreeChunkSize, float glossySD);

        void createSphereBox2013(const size_t internalObjectTreeChunkSize, float glossySD);
        void createCGF2010(const size_t internalObjectTreeChunkSize, float glossySD);
        void createSponza(const size_t internalObjectTreeChunkSize, float glossySD);
        void createReport1(const size_t internalObjectTreeChunkSize, float glossySD);
        void createMultiBounce1(const size_t internalObjectTreeChunkSize, float glossySD);
        
        Light *light_;
        
        inline void calcIntersection(const Ray &ray, Intersection &intersect) const
        {
            ballTree_->calcIntersection(ray, intersect);
        }

        
    private:
        stitch::BallTree *ballTree_;
        
        
    public:
#ifdef USE_OSG
        osg::ref_ptr<osg::Group> rootGroup_;
        osg::ref_ptr<osg::Group> rendererGroup_;
        
        OpenThreads::Mutex sceneGraphMutex_;
#endif// USE_OSG
	};
}

#endif// STITCH_SCENE_H
