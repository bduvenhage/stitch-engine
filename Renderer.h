/*
 * $Id: Renderer.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Renderer.h
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

#ifndef STITCH_RENDERER_H
#define STITCH_RENDERER_H

namespace stitch {
    class Scene;
}

#include "stitchengine_version.h"
#include "Scene.h"
#include "Camera.h"
#include "Math/Vec3.h"
#include "Math/Ray.h"
#include "Math/Colour.h"
#include "RadianceMap.h"

#ifdef USE_CXX11
#include <cstdint>
#else
#include <stdint.h>
#endif

namespace stitch {
    
    //=======================================================================//
    //=======================================================================//
    //=======================================================================//
    //! Abstract base class for renderers.
    class Renderer
    {
    public:
        Renderer(Scene * const scene);
        
        virtual ~Renderer()
        {}
        
        virtual void render(RadianceMap &radianceMap,
                            const stitch::Camera * const camera,
                            const float frameDeltaTime) = 0;
        
        
        virtual void stop()
        {
            stopRender_=true;
        }
        
        static void get_version(uint8_t &major, uint8_t &minor, uint8_t &as_lib)
        {
            major = STITCHENGINE_VERSION_MAJOR;
            minor = STITCHENGINE_VERSION_MINOR;
            
#ifdef STITCHENGINE_SHARED_LIBRARY
            as_lib=1;
#else
            as_lib=0;
#endif
        }
        
        static void get_copyright(std::string &copyrightStr);
        
    protected:
        Scene * const scene_;//The scene is not owned by the renderer!
        
        bool stopRender_;
    };
    
    //=======================================================================//
    //=======================================================================//
    //=======================================================================//
    /*! \brief Abstract base class for renderer with at least a forward phase.
     
     Abstract base class for forward renderers that have a common render mehod
     that loop over the camera pixels to gather radiance from the scene. An example
     of a forward renderer is a Whitted raytracer. An example of a renderer which
     is not a forward renderer is a Monte-Carlo photon simulation. The preRender
     protected method is called by the render method and may be used for the light
     pass of photon mapping or light beam tracing.*/
    class ForwardRenderer : public Renderer
    {
    public:
        ForwardRenderer(Scene * const scene, uint8_t gatherDepth, const size_t samplesPerPixel, const bool printStats);
        
        virtual ~ForwardRenderer()
        {}
        
        //Calls sub-class' preRender before doing the forward pass from the camera.
        virtual void render(RadianceMap &radianceMap,
                            const stitch::Camera * const camera,
                            const float frameDeltaTime);
        
        
    protected:
        virtual void preForwardRender(RadianceMap &radianceMap,
                                      const stitch::Camera * const camera,
                                      const float frameDeltaTime) = 0;
        
        /*! \brief A pure virtual method to gather radiance.
         
         Method that gather's radiance from the direction of the ray. Must be thread safe!!!
         */
        virtual void gather(Ray &ray) const = 0;
        
        const uint8_t gatherDepth_;
        
        //! Samples per pixel.
        const size_t samplesPerPixel_;
        
        const bool printStats_;
        
    private:
        
        //!Worker method that renders a partial frame.
        virtual void renderTask(RadianceMap * const radianceMap,
                                const stitch::Camera * const camera,
                                const size_t taskID,
                                const size_t iyStart, const size_t iyStride);
        
        
    };
    
}

#endif// STITCH_RENDERER_H
