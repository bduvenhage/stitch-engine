/*
 * $Id: main.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  main.cpp
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

#include "Math/MathUtil.h"
#include "Math/Vec3.h"
#include "Math/GlobalRand.h"

#include "Object.h"
#include "BallTree.h"
#include "Objects/BrushModel.h"
#include "Objects/PolygonModel.h"

#include "Light.h"
#include "Scene.h"
#include "RadianceMap.h"
#include "Camera.h"
#include "IOUtils/exr.h"

#include "Renderers/WhittedRenderer.h"
#include "Renderers/PathTraceRenderer.h"
#include "Renderers/PhotonMapRenderer.h"
#include "Renderers/LightBeamRenderer.h"
#include "Renderers/PhotonTraceRenderer.h"
#include "Renderers/LightFieldRenderer.h"

#include "Timer.h"

//============ OSG Includes Begin =================
#include "OSGUtils/StitchOSG.h"
//============ OSG Includes End =================


#ifdef _OPENMP
#include <omp.h>
#endif

#include <chrono>
#include <thread>

#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <atomic>

#include <cstdint>


bool g_snapRender=false;
std::atomic_bool busyRendering;

uint8_t g_rendererID=2;
stitch::Renderer *g_renderer=nullptr;

const float g_glossySD=0.025f;//scatter distribution standard deviation in radians. It should be less than Pi/5=0.628.

const size_t g_ulWindowWidth=800;
const size_t g_ulWindowHeight=600;
const stitch::Colour_t g_initSPD(0.0f, 0.0f, 0.0f);

stitch::RadianceMap g_radianceMap(g_ulWindowWidth, g_ulWindowHeight, g_initSPD);

float g_exposure=72.0555;
float g_tone=1.0f/2.2f;

stitch::Scene *scene=nullptr;

#ifdef USE_OSG
stitch::View2D *view2D=nullptr;
stitch::View3D *view3D=nullptr;
osgViewer::CompositeViewer *viewer=nullptr;

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    
    KeyboardEventHandler()
    {}
    
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                int key=ea.getKey();
                
                if (key=='x')
                {
                    if (g_renderer!=nullptr) g_renderer->stop();
                    std::cout << "Stop render!\n";
                    std::cout.flush();
                } else
                    if (key=='1')
                    {
                        g_rendererID=1;
                        std::cout << "renderer=" << "Whitted" << "\n";
                        std::cout.flush();
                    } else
                        if (key=='2')
                        {
                            g_rendererID=2;
                            std::cout << "renderer=" << "BackwardBeam" << "\n";
                            std::cout.flush();
                        } else
                            if (key=='3')
                            {
                                g_rendererID=3;
                                std::cout << "renderer=" << "PhotonMap" << "\n";
                                std::cout.flush();
                            } else
                                if (key=='4')
                                {
                                    g_rendererID=4;
                                    std::cout << "renderer=" << "PathTrace" << "\n";
                                    std::cout.flush();
                                } else
                                    if (key=='5')
                                    {
                                        g_rendererID=5;
                                        std::cout << "renderer=" << "PhotonTrace" << "\n";
                                        std::cout.flush();
                                    } else
                                    /*
                                     if (key==']')
                                     {
                                     g_glossySD+=0.01f;
                                     std::cout << "g_glossySD=" << g_glossySD << "\n";
                                     std::cout.flush();
                                     } else
                                     if (key=='[')
                                     {
                                     g_glossySD-=0.01f;
                                     if (g_glossySD<0.0f) g_glossySD=0.0f;
                                     std::cout << "g_glossySD=" << g_glossySD << "\n";
                                     std::cout.flush();
                                     } else
                                     */
                                        if (key=='+')
                                        {
                                            g_exposure*=1.05f;
                                            g_radianceMap.setExposure(g_exposure);
                                            std::cout << "g_exposure=" << g_exposure << "\n";
                                            std::cout.flush();
                                        } else
                                            if (key=='-')
                                            {
                                                g_exposure*=0.95f;
                                                g_radianceMap.setExposure(g_exposure);
                                                std::cout << "g_exposure=" << g_exposure << "\n";
                                                std::cout.flush();
                                            } else
                                                
                                                if (key=='T')
                                                {
                                                    g_tone*=1.05f;
                                                    g_radianceMap.setTone(g_tone);
                                                    std::cout << "g_tone=" << g_tone << "\n";
                                                    std::cout.flush();
                                                } else
                                                    
                                                    if (key=='t')
                                                    {
                                                        g_tone*=0.95f;
                                                        g_radianceMap.setTone(g_tone);
                                                        std::cout << "g_tone=" << g_tone << "\n";
                                                        std::cout.flush();
                                                    } else
                                                        
                                                        if (key=='r')
                                                        {
                                                            g_snapRender=true;
                                                            std::cout << "g_snapRender=" << g_snapRender << "\n";
                                                            std::cout.flush();
                                                        }
                return true;
            }
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                return true;
            }
                
            default:
                return false;
        }
    }
};
#endif//USE_OSG


//===
std::thread renderThread;
float frameDeltaTime=1.0f;

stitch::SimplePinholeCamera camera(stitch::Vec3(0.0f, 10.0f, 15.0f),
                                   stitch::Vec3(0.0f, 0.5f, 3.0f));//position, look-at

//=== Executed in renderThread to render a frame ===//
void RenderRun()
{
    busyRendering=true;
    
    //Make a clone of the camera that will be unaffected by potential manipulation of the preview during rendering.
    stitch::Camera *snapCamera=camera.clone();
    
    //=== Do render pass ===//
    stitch::Timer timer;
    stitch::Timer_t startTick, endTick;
    
    {
        std::cout << "Doing render...\n";
        std::cout.flush();
        startTick=timer.tick();
        
        switch (g_rendererID) {
            case 1:
                g_renderer=new stitch::WhittedRenderer(scene);
                break;
            case 2:
                g_renderer=new stitch::LightBeamRenderer(scene);
                break;
            case 3:
                g_renderer=new stitch::PhotonMapRenderer(scene);
                break;
            case 4:
                g_renderer=new stitch::PathTraceRenderer(scene);
                break;
            case 5:
                g_renderer=new stitch::PhotonTraceRenderer(scene);
                break;
            case 6:
                g_renderer=new stitch::LightFieldRenderer(scene);
                break;
            default:
                g_renderer=new stitch::WhittedRenderer(scene);
                break;
        }
        
        g_renderer->render(g_radianceMap, snapCamera, frameDeltaTime);
        
        //=== Save displayBuffer ===
#ifdef USE_OSG
        {
            osg::ref_ptr<osg::Image> osgimage = new osg::Image;
            osgimage->setImage(g_radianceMap.getWidth(), g_radianceMap.getHeight(), 1, GL_RGB, GL_RGBA, GL_UNSIGNED_BYTE, (uint8_t *)g_radianceMap.getDisplayBuffer(), osg::Image::NO_DELETE);
            
            osgDB::writeImageFile(*osgimage, "output.tiff");
        }
#endif//USE_OSG
        //=== ===
        
        delete g_renderer;
        g_renderer=nullptr;
        
        endTick=timer.tick();
        std::cout << "Rendered in " << timer.delta_m(startTick, endTick) << " milli seconds. \n";
        std::cout.flush();
        
        //=== Save radiance map ===
#ifdef USE_OPENEXR
        stitch::Exr::saveExr(&g_radianceMap, "output.exr");
#else//USE_OPENEXR
        std::cout << "Note: Output radiance map not saved because OpenEXR not used!\n";
        std::cout.flush();
#endif//else USE_OPENEXR
        //=== ===
    }
    //======================//
    
    delete snapCamera;
    busyRendering=false;
}
//=== ===//



int main(void)
{
    std::string copyrightStr;
    stitch::Renderer::get_copyright(copyrightStr);
    
    std::cout << copyrightStr << "\n\n";
    std::cout.flush();
    
    //=== Check version of StitchEngine interface ===//
    uint8_t version_major=0, version_minor=0, as_lib=0;
    stitch::Renderer::get_version(version_major, version_minor, as_lib);
    std::cout << "StitchEngine Version " << (int)version_major << "." << (int)version_minor << " " << (as_lib==1 ? "[shared lib]" : "[cpp code]") << "\n";
    std::cout << __FILE__ << " SVN $Revision: 298 $\n";
    std::cout.flush();
    //=== ===//

    //=== Print keyboard commands ===//
    std::cout << "Keyboard Commands: \n";
    std::cout << "'1' - Select Whitted renderer.\n";
    std::cout << "'2' - Select LBT renderer.\n";
    std::cout << "'3' - Select PM renderer.\n";
    std::cout << "'4' - Select path trace renderer (not completed yet).\n";
    std::cout << "'5' - Select light trace renderer.\n";
    std::cout << "'r' - Trigger render of frame.\n";
    std::cout << "'+' - Increase display exposure level.\n";
    std::cout << "'-' - Decrease display exposure level.\n";
    std::cout << "'t/T' - Adjust tone mapping.\n";
    std::cout << "\n";
    std::cout.flush();
    //=== ===//

    stitch::GlobalRand::initialiseUniformSamplerArray();
    
    scene=new stitch::Scene();
    
    g_radianceMap.setExposure(g_exposure);
    g_radianceMap.setTone(g_tone);
    g_radianceMap.shuffleOffsetMap();
    
    
#ifdef USE_OSG
    //=== Instantiate Viewer ===//
    {
        int argc=1;
        char **argv=new char *[10];
        argv[0]=new char [256];
        strcpy(argv[0], "Stitch");
        
        osg::ArgumentParser arguments(&argc,argv);
        viewer=new osgViewer::CompositeViewer(arguments);
        
        delete [] argv[0];
        delete [] argv;
    }
    
    view2D=new stitch::View2D(g_ulWindowWidth, g_ulWindowHeight, g_radianceMap.getDisplayBuffer());
    view2D->init();
    viewer->addView(view2D->getOSGView());
    
    view3D=new stitch::View3D(g_ulWindowWidth, g_ulWindowHeight, scene->rootGroup_);
    view3D->init();
    view3D->getOSGView()->addEventHandler(new KeyboardEventHandler());
    viewer->addView(view3D->getOSGView());
#endif//USE_OSG
    
    
    stitch::Timer timer;
    stitch::Timer_t startTick, endTick;
    
    //=== Create the scene ===//
    {
        startTick=timer.tick();
        
        //=== 1) Sphere box scene ===//
                size_t numObjects=scene->create("SphereBox2013",//scene name
                                                stitch::Vec3(0.0f, 9.0f, 0.0f),//light orig
                                                stitch::Colour_t(50.0f, 50.0f, 50.0f),//light SPD
                                                1, 16, false, false, g_glossySD);//objectTreeChunkSize, internalObjectTreeChunkSize, createOSGLinesNode, createOSGNormalsNode
        //=== ===//
        //OR
        //=== 2) Caustic gears ===//
        //        size_t numObjects=scene->create("CausticGear",//scene name
        //                                        stitch::Vec3(0.0f, 6.0f, 13.5f),//light orig
        //                                        stitch::Colour_t(50.0f, 50.0f, 50.0f),//light SPD
        //                                        1, 16, false, false, g_glossySD);//objectTreeChunkSize, internalObjectTreeChunkSize, createOSGLinesNode, createOSGNormalsNode
        //=== ===//
        //OR
        //=== 3) Caustic ring scene ===//
        //size_t numObjects=scene->create("CausticRing",//scene name
        //                                stitch::Vec3(0.0f, 3.8f, 9.0f),//light orig
        //                                stitch::Colour_t(50.0f, 50.0f, 50.0f),//light SPD
        //                                1, 16, false, false, g_glossySD);//objectTreeChunkSize, internalObjectTreeChunkSize, createOSGLinesNode, createOSGNormalsNode
        //=== ===//
        
        endTick=timer.tick();
        std::cout <<  "Created " << numObjects << " objects in "<< timer.delta_m(startTick, endTick) << " milli seconds. \n";
        std::cout.flush();
    }
    //========================//
    
    size_t frameCount=0;
    
    busyRendering=false;
    
#ifndef USE_OSG
    g_snapRender=true; //force a snap!
#endif//USE_OSG
    
    
#ifdef USE_OSG
    viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    
    if (!viewer->isRealized())
    {
        viewer->realize();
    }
    
    
    //=== Create the camera for the scene ===//
    //=== 1) caustic ring camera ===//
//    view3D->setCamera(stitch::Vec3(0.0f, 20.0f, 5.0f),//eye pos
//                      stitch::Vec3(0.0f, 0.5f, 1.0f),//centre pos
//                      stitch::Vec3(0.0f, 1.0f, 0.0f));//up vec
    //=== ===//
    //OR
    //=== 2) caustic gears camera ===//
//    view3D->setCamera(stitch::Vec3(0.0f, 10.0f, 15.0f),//eye pos
//                      stitch::Vec3(0.0f, 0.0f, 2.0f),//centre pos
//                      stitch::Vec3(0.0f, 1.0f, 0.0f));//up vec
    //=== ===//
    //OR
    //=== 3) spherebox ===//
    view3D->setCamera(stitch::Vec3(-3.0f, 10.0f, 30.0f),//eye pos
                      stitch::Vec3(0.0f, 4.25f, 2.0f),//centre pos
                      stitch::Vec3(0.0f, 1.0f, 0.0f));//up vec
    //=== ===//
    //=======================================//
    
    while (!viewer->done())
#endif//USE_OSG
    {
#ifdef USE_OSG
        camera=view3D->getCamera();
#endif//USE_OSG
        
        if ((g_snapRender)&&(busyRendering==false))
        {
            //=== See if previous render thread has stopped and can be joined now ===
            if (renderThread.get_id() != std::thread::id())
            {
                renderThread.join();
            }
            //=== ===
            
            //=== Start new thread once previous render thread refers to not-a-thread ===
            if (renderThread.get_id() == std::thread::id())
            {
                renderThread=std::thread(RenderRun);
                g_snapRender=false;//flag that a snap event has registered.
            }
            //=== ===
        }
        
        
#ifdef USE_OSG
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(scene->sceneGraphMutex_);
            view3D->preframe();
        }
        
        std::chrono::milliseconds dura(1000);
        std::this_thread::sleep_for(dura);
        
        //=== Delay the preview rendering more if a render thread is started ===//
        if (busyRendering)
        {
            std::chrono::milliseconds dura(1000);//Pre-sleep for longer while rendering???
            std::this_thread::sleep_for(dura);
        }
        //=== ===//
        
        g_radianceMap.updateVoronoiDisplayBuffer();
        //g_radianceMap.updateDisplayBuffer();
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(scene->sceneGraphMutex_);
            view2D->preframe();
            viewer->frame();
        }
#endif//USE_OSG
        
        ++frameCount;
    }
    
#ifndef USE_OSG
    //Wait for frame to finish when not in interactive mode.
    renderThread.join();
#endif//USE_OSG
    
    std::cout << "Waiting for renderThread to join...";
    std::cout.flush();
    {
        if (g_renderer!=nullptr) g_renderer->stop();
        
        if (renderThread.get_id() != std::thread::id())
        {
            renderThread.join();
        }
    }
    std::cout << "done.\n";
    std::cout.flush();
    
#ifdef USE_OSG
    std::chrono::milliseconds dura(1000);
    std::this_thread::sleep_for(dura);
#endif//USE_OSG
    
    return 0;
}
