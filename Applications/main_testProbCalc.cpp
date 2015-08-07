/*
 * $Id: main_testProbCalc.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  main_testProbCalc.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2010/01/01.
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

#include "Math/Vec3.h"
#include "Materials/BlinnPhongMaterial.h"
#include "Materials/PhongMaterial.h"
#include "Materials/DiffuseMaterial.h"
#include "Materials/GlossyMaterial.h"
#include "KDTree.h"
#include "Objects/BrushModel.h"
#include "Beam.h"


//============ OSG Includes Begin =================
#include "OSGUtils/StitchOSG.h"
//============ OSG Includes End =================

#include <thread>
#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#endif


const unsigned long g_ulWindowWidth=640;
const unsigned long g_ulWindowHeight=480;
osg::ref_ptr<osg::Group> g_rootGroup_;
stitch::View3D *g_view3D=nullptr;
float g_eye_distance=5.0f;

osgViewer::CompositeViewer *g_viewer=nullptr;
bool g_renderInBackground=true;
OpenThreads::Mutex g_sceneGraphMutex;

osg::Timer timer;


//==========================================================
//!Compare the accuracy of the probability functions.
void testAccuracy()
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    
    std::ofstream fout("probAccuracy.csv");
    
    std::cout << "  Testing accuracy of prob func...\n    ";
    std::cout.flush();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec4Array> osgColours=new osg::Vec4Array();
    
    const size_t numSamples=10000;
    
    startTick=timer.tick();
    
    fout << "probRef" << ", " << "probVolumeLookup" << ", " << "probGaus" << ", " << "Q.length()" << ", " << "area" << "\n";
    
    for (size_t sampleNum=0; sampleNum<numSamples; ++sampleNum)
    {
        
        const stitch::Vec3 Q=stitch::Vec3::randDisc()*6.0f;
        const float radius=0.25f;
        const float theta=stitch::GlobalRand::uniformSampler()*(2.0*M_PI);
        const stitch::Vec3 A=Q + stitch::Vec3(radius*cos(theta - (0.0f*M_PI/180.0f)), radius*sin(theta - (0.0f*M_PI/180.0f)), 0.0);
        const stitch::Vec3 B=Q + stitch::Vec3(radius*cos(theta - (120.0f*M_PI/180.0f)), radius*sin(theta - (120.0f*M_PI/180.0f)), 0.0);
        const stitch::Vec3 C=Q + stitch::Vec3(radius*cos(theta - (240.0f*M_PI/180.0f)), radius*sin(theta - (240.0f*M_PI/180.0f)), 0.0);
        

        /*
        const stitch::Vec3 A=stitch::Vec3::randDisc()*5.0f;
        const stitch::Vec3 B=stitch::Vec3::randDisc()*5.0f;
        const stitch::Vec3 C=stitch::Vec3::randDisc()*5.0f;
        const stitch::Vec3 Q(A, B, C, 0.5f, 0.5f, 0.5f);
        */
        
        const float probRef=stitch::BeamSegment::gaussVolumeBarycentricRandomABC(A, B, C, 1.0f, 25000).length();
        
        const float probGaus=stitch::BeamSegment::gaussVolumeGaussSubd(A, B, C);
        const float probVolumeLookup=stitch::BeamSegment::gaussVolumeLookUpOptimisedABC(A, B, C, 1.0f).length();
        //const float prob=stitch::Beam::gaussVolumeRecurABCNormInit(A, B, C);
        //const float prob=stitch::Beam::gaussVolumeBarycentricRandomABC(A, B, C, 1.0f, 250000).length();
        
        const float area=stitch::Vec3::crossLength(A, B, C)*0.5f;
        const float err=fabsf(probGaus - probRef)*1000.0f;
        
        {
            fout << probRef << ", " << probVolumeLookup << ", " << probGaus << ", " << Q.length() << ", " << area << "\n";
            fout.flush();
            
            const float colour=probRef;
            
            osgVertices->push_back(osg::Vec3(A.x(), A.y(), A.z()+err));
            osgColours->push_back(osg::Vec4(colour,
                                            colour,
                                            colour,
                                            1.0));
            
            osgVertices->push_back(osg::Vec3(B.x(), B.y(), B.z()+err));
            osgColours->push_back(osg::Vec4(colour,
                                            colour,
                                            colour,
                                            1.0));
            
            osgVertices->push_back(osg::Vec3(C.x(), C.y(), C.z()+err));
            osgColours->push_back(osg::Vec4(colour,
                                            colour,
                                            colour,
                                            1.0));
        }
        
    }
    
    endTick=timer.tick();
    
    if (osgVertices->size()>0)
    {
        osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
        osgGeometry->setVertexArray(osgVertices.get());
        osgGeometry->setColorArray(osgColours.get());
        osgGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        
        osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,osgVertices->size()));
        
        osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
        osgStateset->setNestRenderBins(false);
        osgStateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        osgStateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        
        osg::Depth* depth = new osg::Depth();
        osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
        
        osg::BlendFunc *fn = new osg::BlendFunc();
        fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
        osgStateset->setAttributeAndModes(fn, osg::StateAttribute::ON);
        
        osg::Material *material = new osg::Material();
        material->setColorMode(osg::Material::DIFFUSE);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        osgStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
        
        osgGeode->addDrawable(osgGeometry.get());
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(g_sceneGraphMutex);
            g_rootGroup_->removeChildren(0, g_rootGroup_->getNumChildren());
            g_rootGroup_->addChild(osgGeode);
        }
    }
    
    std::cout << "  " << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
    
    fout.close();
}

//==========================================================
//!Visualise the probability function.
void visualiseProbFunc()
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "  Visualising prob func...\n    ";
    std::cout.flush();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec4Array> osgColours=new osg::Vec4Array();
    
    
    for (size_t iz=0; iz<256; iz+=4)
    {
        for (size_t iy=0; iy<512; iy+=4)
        {
            for (size_t ix=0; ix<512; ix+=4)
            {
                const stitch::Vec3 B=stitch::Vec3(0.0f, 0.0f, 0.0f);
                
                const stitch::Vec3 A=stitch::Vec3(1.0f, 0.0f, 0.0f)*(((ix+0.5f)/512.0f)*4.0f) + B;
                const stitch::Vec3 orthA=stitch::Vec3(0.0f, 1.0f, 0.0f);
                
                const float theta=((iz+0.5f)/256.0f)*M_PI;
                
                const stitch::Vec3 C=(A.normalised()*cos(theta) + orthA.normalised()*sin(theta)) * (((iy+0.5f)/512.0f)*4.0f) + B;
                
                const float prob=stitch::BeamSegment::gaussVolumeBarycentricRandomABC(A, B, C, 1.0f, 10000).length();
                //const float prob=stitch::Beam::gaussVolumeGaussMC(A, B, C);
                //const float prob=stitch::Beam::gaussVolumeGaussSubd(A, B, C);
                //const float prob=stitch::Beam::gaussVolumeLookUpOptimisedABC(A, B, C, 1.0f).length();
                //const float prob=stitch::Beam::gaussVolumeRecurABCNormInit(A, B, C);
                
                if (prob>0.075f)
                {
                    osg::Vec3 coord=osg::Vec3(ix/512.0f,
                                              iy/512.0f,
                                              iz/256.0f);
                    
                    osgVertices->push_back(coord);
                    
                    osgColours->push_back(osg::Vec4(prob, prob, prob, 1.0));
                }
            }
        }
    }
    
    if (osgVertices->size()>0)
    {
        osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
        osgGeometry->setVertexArray(osgVertices.get());
        osgGeometry->setColorArray(osgColours.get());
        osgGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        
        osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,osgVertices->size()));
        osgGeometry->getOrCreateStateSet()->setAttribute( new osg::Point( 6.0f ), osg::StateAttribute::ON);
        
        osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
        osgStateset->setNestRenderBins(false);
        osgStateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        osgStateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        
        osg::Depth* depth = new osg::Depth();
        osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
        
        osg::BlendFunc *fn = new osg::BlendFunc();
        fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
        osgStateset->setAttributeAndModes(fn, osg::StateAttribute::ON);
        
        osg::Material *material = new osg::Material();
        material->setColorMode(osg::Material::DIFFUSE);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        osgStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
        
        osgGeode->addDrawable(osgGeometry.get());
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(g_sceneGraphMutex);
            g_rootGroup_->removeChildren(0, g_rootGroup_->getNumChildren());
            g_rootGroup_->addChild(osgGeode);
        }
    }
    
    endTick=timer.tick();
    
    std::cout << "  " << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}




//==========================================================
void displayUpdateRun()
{
    unsigned long framesDone=0;
    
    while (g_renderInBackground)
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(g_sceneGraphMutex);
            
            g_view3D->setCamera(stitch::Vec3(g_eye_distance*cos(framesDone*0.00f), g_eye_distance*sin(framesDone*0.00f), 0.0f),
                                stitch::Vec3(0.0f, 0.0f, 0.0f),
                                stitch::Vec3(0.0f, 0.0f, 1.0f));
            
            g_view3D->preframe();
            g_viewer->frame();
        }
        
        framesDone++;
        
        std::chrono::milliseconds dura(100);
        std::this_thread::sleep_for(dura);
    }
    
}


//==========================================================
int main(void)
{
    stitch::GlobalRand::initialiseUniformSamplerArray();
    //stitch::BeamSegment::generateVolumeTexture();
    
    {
        int argc=1;
        char **argv=new char *[10];
        argv[0]=new char [256];
        strcpy(argv[0], "Prob Calc Test");
        
        osg::ArgumentParser arguments(&argc,argv);
        g_viewer=new osgViewer::CompositeViewer(arguments);
        
        delete [] argv[0];
        delete [] argv;
    }
    
    g_viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    g_rootGroup_=new osg::Group;
    g_view3D=new stitch::View3D(g_ulWindowWidth, g_ulWindowHeight, g_rootGroup_);
    g_view3D->init();
    //g_view3D->getOSGView()->addEventHandler(new KeyboardEventHandler());
    g_viewer->addView(g_view3D->getOSGView());
    
    if (!g_viewer->isRealized())
    {
        g_viewer->realize();
    }
    
    g_view3D->setCamera(stitch::Vec3(0.0f, g_eye_distance, g_eye_distance), stitch::Vec3(0.0f, 0.0f, 0.0f), stitch::Vec3(0.0f, 0.0f, 1.0f));
    
    g_renderInBackground=true;
    std::thread displayUpdateThread(displayUpdateRun);
    
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    //========================//
    //=== Start Prob Calc tests ===//
    //========================//
    //visualiseProbFunc();
    testAccuracy();
    //========================//
    //========================//
    //========================//
    
    endTick=timer.tick();
    
    std::cout << "Completed in " << timer.delta_s(startTick, endTick) << " seconds.\n\n";
    std::cout.flush();
    
    //====================================//
    //=== Continue rendering until ESC ===//
    //====================================//
    g_renderInBackground=false;
    displayUpdateThread.join();//Wait for background render thread to exit.
    
    //Continue rendering in main thread until viewer exited.
    while (!g_viewer->done())
    {
        g_view3D->preframe();
        g_viewer->frame();
        
        std::chrono::milliseconds dura(50);
        std::this_thread::sleep_for(dura);
    }
    //====================================//
    //====================================//
    //====================================//
    
    std::chrono::milliseconds dura(1000);
    std::this_thread::sleep_for(dura);
    
    delete g_viewer;
}
