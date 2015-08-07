/*
 * $Id: Renderer.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
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

#include "Renderer.h"
#include "Timer.h"

#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef USE_CXX11
#include <thread>
#else
#include <boost/thread.hpp>
#endif


void stitch::Renderer::get_copyright(std::string &copyrightStr)
{
    copyrightStr=std::string("Copyright $Date: 2015-03-25 15:00:40 +0200 (Wed, 25 Mar 2015) $ Bernardt Duvenhage. All rights reserved.\n This file is part of StitchEngine. StitchEngine is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. StitchEngine is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. You should have received a copy of the GNU Lesser General Public License along with StitchEngine.  If not, see <http://www.gnu.org/licenses/>.");
}



//=======================================================================//
stitch::Renderer::Renderer(Scene * const scene) : scene_(scene)
{
}


//=======================================================================//
stitch::ForwardRenderer::ForwardRenderer(Scene * const scene, uint8_t gatherDepth, const size_t samplesPerPixel, const bool printStats) :
Renderer(scene),
gatherDepth_(gatherDepth),
samplesPerPixel_(samplesPerPixel),
printStats_(printStats)
{
}


//=======================================================================//
void stitch::ForwardRenderer::renderTask(RadianceMap * const radianceMap,
                                         const stitch::Camera * const camera,
                                         const size_t taskID,
                                         const size_t iyStart, const size_t iyStride)
{
    const size_t imgWidth=radianceMap->getWidth();
    const size_t imgHeight=radianceMap->getHeight();
    
    const float halfWindowHeight = radianceMap->getHeight() * 0.5f;
    const float halfWindowWidth = radianceMap->getWidth() * 0.5f;
    const float recipWindowWidth = 1.0f / radianceMap->getWidth();
    
    stitch::Timer timer;
    stitch::Timer_t startTick, endTick;
    
    //Note!!!: The (ix,iy) is remapped/shuffled by the below call to radianceMap->getRandomisedXY(...)!
    for (size_t iy=iyStart; iy<imgHeight; iy+=iyStride)
    {
        if (!stopRender_)
        {
            for (size_t ix=0; ix<imgWidth; ++ix)
            {
                /*
                 startTick=timer.tick();
                 */
                
                size_t x=ix;
                size_t y=iy;
                
                radianceMap->getShuffledXY(ix, iy, x, y);

                Colour_t mapRadiance;
                
                for (size_t s=0; s<samplesPerPixel_; ++s)
                {
                    Ray ray=camera->getPrimaryRay(x, y,//RAY IDs
                                                  (x+0.5f-halfWindowWidth)*recipWindowWidth,
                                                  (y+0.5f-halfWindowHeight)*recipWindowWidth);
                    
                    ray.gatherDepth_=gatherDepth_;
                    
                    this->gather(ray);
                    
                    mapRadiance+=ray.returnRadiance_;
                }
                
                mapRadiance*=1.0f/samplesPerPixel_;
                
                /*
                 endTick=timer.tick();
                 
                 const double pixelTime=timer.delta_n(startTick, endTick)*0.00001;
                 mapRadiance(pixelTime);
                 */
                
                //Note: currently the angle between the radiancemap pixel normal and the incoming radiance direction is ignored!
                radianceMap->setMapValue(x, y, mapRadiance, iyStart);                
            }
            
            if ((iy%(imgHeight/100))==0)
            {
                std::cout << ".";
                std::cout.flush();
                
                if ((taskID==0)&&(printStats_))
                {
                    std::cout << (iy * 100 / imgHeight) << "%..";
                }
            }
        }
    }
}


//=======================================================================//
void stitch::ForwardRenderer::render(RadianceMap &radianceMap,
                                     const stitch::Camera * const camera,
                                     const float frameDeltaTime)
{
    stopRender_=false;
    
    radianceMap.clear(Colour_t());
    
    //=== Pre-render e.g. light pass ===//
    {
        stitch::Timer timer;
        stitch::Timer_t startTick, endTick;
        
        std::cout << " Doing pre-render...\n";
        std::cout.flush();
        
        startTick=timer.tick();
        preForwardRender(radianceMap, camera, frameDeltaTime);//Call sub-class' preRender before doing the forward pass from the camera.
        endTick=timer.tick();
        
        std::cout << " pre-render in " << timer.delta_m(startTick, endTick) << " ms...done.\n";
        std::cout.flush();
    }
    //=== ===//
    
    
    //===  Forward render i.e. camera pass ===//
    {
        stitch::Timer timer;
        stitch::Timer_t startTick, endTick;
        
        std::cout << " Doing forward render...";
        std::cout.flush();
        
        
#ifdef USE_CXX11
        size_t numRenderThreads=std::thread::hardware_concurrency();
#else
        size_t numRenderThreads=boost::thread::hardware_concurrency();
#endif
        
        if (numRenderThreads==0) numRenderThreads=2;//Setup numRenderThreads in case system reports 0.
        
#ifdef USE_CXX11
        std::vector<std::thread> threadVect_;
#else
        std::vector<boost::shared_ptr<boost::thread> > threadVect_;
#endif
        threadVect_.reserve(numRenderThreads);
        
        
        std::cout <<"["<< numRenderThreads << " render thread(s)]...";
        std::cout.flush();
        
        startTick=timer.tick();
        
        for (size_t threadNum=0; threadNum<numRenderThreads; ++threadNum)
        {
#ifdef USE_CXX11
            threadVect_.emplace_back(&stitch::ForwardRenderer::renderTask, this,
                                     &radianceMap, camera,
                                     threadNum,
                                     threadNum, numRenderThreads
                                     );
#else
            threadVect_.push_back(boost::shared_ptr<boost::thread>(new boost::thread(&stitch::ForwardRenderer::renderTask, this,
                                                                                     &radianceMap, camera,
                                                                                     threadNum,
                                                                                     threadNum, numRenderThreads)
                                                                   )
                                  );
#endif
        }
        
        
#ifdef USE_CXX11
        std::vector<std::thread>::iterator threadIter=threadVect_.begin();
        std::vector<std::thread>::iterator threadIterEnd=threadVect_.end();
#else
        std::vector<boost::shared_ptr<boost::thread> >::iterator threadIter=threadVect_.begin();
        std::vector<boost::shared_ptr<boost::thread> >::iterator threadIterEnd=threadVect_.end();
#endif
        
        //Wait for each render thread to finish.
        for (; threadIter!=threadIterEnd; threadIter++)
        {
#ifdef USE_CXX11
            (*threadIter).join();
#else
            (*threadIter)->join();
#endif
        }
        
        
        if (!stopRender_)
        {
            std::cout << "100%.\n";
            std::cout.flush();
        }
        
        endTick=timer.tick();
        
        std::cout << " forward render in " << timer.delta_m(startTick, endTick) << " ms...done.\n";
        std::cout.flush();
    }
    //=== ===//
}
