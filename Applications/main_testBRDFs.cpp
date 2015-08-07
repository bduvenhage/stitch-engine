/*
 * $Id: main_testBRDFs.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  main_imgDiff.cpp
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


//============ OSG Includes Begin =================
#include "OSGUtils/StitchOSG.h"
//============ OSG Includes End =================

#ifdef _OPENMP
#include <omp.h>
#endif

#include <chrono>
#include <thread>


const unsigned long g_ulWindowWidth=640;
const unsigned long g_ulWindowHeight=480;
osg::ref_ptr<osg::Group> g_rootGroup_;
stitch::View3D *g_view3D=nullptr;
float g_eye_distance=5.0f;

osgViewer::CompositeViewer *g_viewer=nullptr;
bool g_renderInBackground=true;

OpenThreads::Mutex g_sceneGraphMutex;

osg::Timer timer;

std::vector<size_t> binIndices;
std::vector<stitch::Vec3> vectors;
stitch::KDTree kdTree;

enum PDFType {
    COSINE_LOBE_PDF,
    GAUSSIAN_LOBE_PDF,
};


//==========================================================
//==========================================================
//==========================================================
void testBRDFBinDensity()
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "Testing BRDF bin uniformity...\n";
    std::cout.flush();
    
    const unsigned long numVectors=vectors.size();
    const unsigned long k=numVectors/60;
    float radiusMean=0.0f;
    float radiusSTD=0.0f;
    
    for (unsigned long i=0; i<numVectors; i++)
    {
        stitch::KNearestItems kNearestItems(vectors[i], 2.0f, k);
        
        kdTree.getNearestK(&kNearestItems);
        
        radiusMean+=sqrt(kNearestItems.searchRadiusSq_);
    }
    radiusMean/=numVectors;
    
    for (unsigned long i=0; i<numVectors; i++)
    {
        stitch::KNearestItems kNearestItems(vectors[i], 2.0f, k);
        
        kdTree.getNearestK(&kNearestItems);
        
        radiusSTD+=powf(sqrt(kNearestItems.searchRadiusSq_)-radiusMean, 2.0f);
    }
    radiusSTD=sqrtf(radiusSTD/numVectors);
    
    
    std::cout << "  Radius of " << k << " (mean,std)=(" << radiusMean << "," << radiusSTD << ")\n";
    
    endTick=timer.tick();
    
    std::cout << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}

//==========================================================
//==========================================================
//==========================================================
void generateBRDFBinVectors()
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "Generating BRDF bins...\n";
    std::cout.flush();
    
    //=================
    std::cout << "   Generating vectors and relaxing...";
    std::cout.flush();
    
    //Choose more BRDF bins for BRDFs with sharp peaks.
    stitch::Vec3::equidistantVectors_IcosahedronBased(12, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_IcosahedronBased(42, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_IcosahedronBased(162, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_IcosahedronBased(642, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_IcosahedronBased(2562, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_IcosahedronBased(10242, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_IcosahedronBased(40962, vectors, binIndices);
    
    //stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(12, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(42, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(162, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(642, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(2562, vectors, binIndices);
    //stitch::Vec3::equidistantVectors_FibonacciSpiralSphere(10242, vectors, binIndices);
    
    const unsigned long numVectors=vectors.size();
    const unsigned long numBinIndices=binIndices.size();
    
    for (int i=0; i<=10; i++)
    {
        if (i!=0) stitch::Vec3::relaxEquidistantVectorsII(vectors, 1);
        
        osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
        osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
        osg::ref_ptr<osg::Vec3Array> osgNormals=new osg::Vec3Array();
        
        /*
         for (unsigned long i=0; i<numVectors; i++)
         {
         osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
         osgVertices->push_back(osg::Vec3(vectors[i].x(), vectors[i].y(), vectors[i].z()));
         
         osgNormals->push_back(osg::Vec3(vectors[i].x(), vectors[i].y(), vectors[i].z()));
         osgNormals->push_back(osg::Vec3(vectors[i].x(), vectors[i].y(), vectors[i].z()));
         }
         */
        for (unsigned long i=0; i<numBinIndices; i+=3)
        {
            osgVertices->push_back(osg::Vec3(vectors[binIndices[i+0]].x(), vectors[binIndices[i+0]].y(), vectors[binIndices[i+0]].z()));
            osgVertices->push_back(osg::Vec3(vectors[binIndices[i+1]].x(), vectors[binIndices[i+1]].y(), vectors[binIndices[i+1]].z()));
            
            osgVertices->push_back(osg::Vec3(vectors[binIndices[i+1]].x(), vectors[binIndices[i+1]].y(), vectors[binIndices[i+1]].z()));
            osgVertices->push_back(osg::Vec3(vectors[binIndices[i+2]].x(), vectors[binIndices[i+2]].y(), vectors[binIndices[i+2]].z()));
            
            osgVertices->push_back(osg::Vec3(vectors[binIndices[i+2]].x(), vectors[binIndices[i+2]].y(), vectors[binIndices[i+2]].z()));
            osgVertices->push_back(osg::Vec3(vectors[binIndices[i+0]].x(), vectors[binIndices[i+0]].y(), vectors[binIndices[i+0]].z()));
            
            osgNormals->push_back(osg::Vec3(vectors[binIndices[i+0]].x(), vectors[binIndices[i+0]].y(), vectors[binIndices[i+0]].z()));
            osgNormals->push_back(osg::Vec3(vectors[binIndices[i+1]].x(), vectors[binIndices[i+1]].y(), vectors[binIndices[i+1]].z()));
            osgNormals->push_back(osg::Vec3(vectors[binIndices[i+1]].x(), vectors[binIndices[i+1]].y(), vectors[binIndices[i+1]].z()));
            osgNormals->push_back(osg::Vec3(vectors[binIndices[i+2]].x(), vectors[binIndices[i+2]].y(), vectors[binIndices[i+2]].z()));
            osgNormals->push_back(osg::Vec3(vectors[binIndices[i+2]].x(), vectors[binIndices[i+2]].y(), vectors[binIndices[i+2]].z()));
            osgNormals->push_back(osg::Vec3(vectors[binIndices[i+0]].x(), vectors[binIndices[i+0]].y(), vectors[binIndices[i+0]].z()));
        }
        
        osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
        osgGeometry->setVertexArray(osgVertices.get());
        osgGeometry->setNormalArray(osgNormals.get());
        osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        
        //osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,osgVertices->size()));
        osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgVertices->size()));
        
        osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
        osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        
        osg::Depth* depth = new osg::Depth();
        osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
        
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
    
    std::cout << numVectors << " vectors and " << numBinIndices/3 << " faces...done.\n";
    std::cout.flush();
    
    //=================
    std::cout << "   Building kd-tree...";
    std::cout.flush();
    
    kdTree.itemVector_.reserve(numVectors);
    for (unsigned long i=0; i<numVectors; i++)
    {
        kdTree.itemVector_.push_back(new stitch::BoundingVolume(vectors[i], 0.0f, i));
    }
    
    std::vector<stitch::Vec3> splitAxisVec;
    splitAxisVec.push_back(stitch::Vec3(1.0f, 0.0f, 0.0f));
    splitAxisVec.push_back(stitch::Vec3(0.0f, 1.0f, 0.0f));
    splitAxisVec.push_back(stitch::Vec3(0.0f, 0.0f, 1.0f));

    kdTree.build(KDTREE_DEFAULT_CHUNK_SIZE, 0, 1000000, splitAxisVec);
    std::cout << vectors.size() << " done.\n";
    std::cout.flush();
    
    endTick=timer.tick();
    
    std::cout << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}



//==========================================================
//==========================================================
//==========================================================
void generateBRDFBinVectorsGeoDual()
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "Generating BRDF bins geo dual...\n";
    std::cout.flush();
    
    const unsigned long numVectors=vectors.size();
    
    stitch::Brush geoDualBrush(new stitch::DiffuseMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f)));
    
    for (unsigned long vectorNum=0; vectorNum<numVectors; vectorNum++)
    {
        //stitch::Vec3 v=vectors[vectorNum];
        //float absCosTheta=fabsf(v * (stitch::Vec3(1.0f, 1.0f, 1.0f).normalised()));
        
        geoDualBrush.addFace(stitch::BrushFace(stitch::Plane(vectors[vectorNum].normalised(), /*(absCosTheta*0.75f + 0.25f)*/1.0f), false));
    }
    
    //=== Calculate the face vertices from the brush face planes ===
    geoDualBrush.updateLinesVerticesAndBoundingVolume(false);
    //=== ===
    
    osg::ref_ptr<osg::Node> osgGeode=geoDualBrush.constructOSGNode(true, false, false);
    
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(g_sceneGraphMutex);
        g_rootGroup_->removeChildren(0, g_rootGroup_->getNumChildren());
        g_rootGroup_->addChild(osgGeode);
    }
    
    //=================
    
    endTick=timer.tick();
    
    std::cout << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}


//==========================================================
//==========================================================
//==========================================================
void verifyPDF(const float param, PDFType ePDFType)
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgNormals=new osg::Vec3Array();
    
    const stitch::Vec3 normal(0.0, 0.0, 1.0);
    const stitch::Vec3 xaxis(1.0, 0.0, 0.0);
    const stitch::Vec3 yaxis(0.0, 1.0, 0.0);
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(xaxis.x(), xaxis.y(), xaxis.z()));
    osgNormals->push_back(osg::Vec3(xaxis.x(), xaxis.y(), xaxis.z()));
    osgNormals->push_back(osg::Vec3(xaxis.x(), xaxis.y(), xaxis.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(yaxis.x(), yaxis.y(), yaxis.z()));
    osgNormals->push_back(osg::Vec3(yaxis.x(), yaxis.y(), yaxis.z()));
    osgNormals->push_back(osg::Vec3(yaxis.x(), yaxis.y(), yaxis.z()));
    
    const float footprintSR=4.0f * float(M_PI) / vectors.size();
    const unsigned long numVectors=vectors.size();
    const unsigned long numPDFSamples=100*numVectors;
    
    float maxProb=M_1_PI;
    switch (ePDFType)
    {
        case COSINE_LOBE_PDF:
            maxProb=stitch::Vec3::randCosineLobe_pdf(param, normal) * footprintSR;
            std::cout << "Testing Cosine lobe PDF volume (sExp=" << param << ")...\n";
            break;
        case GAUSSIAN_LOBE_PDF:
            maxProb=stitch::Vec3::randGaussianLobe_pdf(param, normal) * footprintSR;
            std::cout << "Testing Gaussian lobe PDF volume (sd=" << param << ")...\n";
            break;
    }
    
    std::cout.flush();
    
    std::vector<stitch::Vec3> scatteredPDF=vectors;
    
    std::cout << "    Generating scattered PDF ...";
    std::cout.flush();
    
    for (unsigned long s=0; s<numPDFSamples; s++)
    {
        stitch::Vec3 randomVec;
        switch (ePDFType)
        {
            case COSINE_LOBE_PDF:
                randomVec=stitch::Vec3::randCosineLobe(param);
                break;
            case GAUSSIAN_LOBE_PDF:
                randomVec=stitch::Vec3::randGaussianLobe(param);
                break;
        }
        
        //Add random vector to scatteredPDF.
        {
            unsigned long closestVectorIndex=0;
            stitch::Vec3 closestVector=vectors[0];
            float closestCosTheta=randomVec * closestVector;
            
            //=== Find closest bin to scattered direction; i.e. Voronoi bins. ===
            for (unsigned long j=1; j<numVectors; j++)
            {
                const float cosTheta=randomVec * vectors[j];
                
                if (cosTheta>closestCosTheta)
                {//Angle is smaller i.e. closer bin found.
                    closestVector=vectors[j];
                    closestVectorIndex=j;
                    closestCosTheta=cosTheta;
                }
            }
            //=== ===
            
            scatteredPDF[closestVectorIndex]+=vectors[closestVectorIndex];
        }
    }
    std::cout << "...done.\n";
    std::cout.flush();
    
    float totalProb=0.0f;
    float totalMeasuredProb=0.0f;
    float totalSAD=0.0f;
    
    std::cout << "    Computing PDF quality...";
    std::cout.flush();
    for (unsigned long i=0; i<numVectors; i++)
    {
        stitch::Vec3 dirA=vectors[i];
        dirA.normalise();
        
        //if ((dirA*normal)>0.0f)
        {
            float measuredProb=(scatteredPDF[i].length()-1.0f)/numPDFSamples;
            
            float prob=0.0f;
            switch (ePDFType)
            {
                case COSINE_LOBE_PDF:
                    prob=stitch::Vec3::randCosineLobe_pdf(param, dirA) * footprintSR;
                    break;
                case GAUSSIAN_LOBE_PDF:
                    prob=stitch::Vec3::randGaussianLobe_pdf(param, dirA) * footprintSR;
                    break;
            }
            
            totalProb+=prob;
            totalMeasuredProb+=measuredProb;
            
            float SAD=fabsf(prob-measuredProb);
            totalSAD+=SAD;
            
            osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
            osgVertices->push_back(osg::Vec3(dirA.x()*SAD/maxProb, dirA.y()*SAD/maxProb, dirA.z()*SAD/maxProb));
            
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
        }
    }
    
    std::cout << "...done.\n";
    std::cout.flush();
    
    
    osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
    osgGeometry->setVertexArray(osgVertices.get());
    osgGeometry->setNormalArray(osgNormals.get());
    osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgVertices->size()));
    
    osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
    osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
    osg::Depth* depth = new osg::Depth();
    osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    
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
    
    std::cout << "  TP (should be 1.0)  = " << totalProb << "\n";
    std::cout << "  TMP (should be 1.0) = " << totalMeasuredProb << "\n";
    std::cout << "  SAD (should be 0.0) = " << totalSAD << "\n";
    
    endTick=timer.tick();
    
    std::cout << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}



//==========================================================
//!Visualise the rejection sampling BRDF.
void visualiseRejectSampBRDF(stitch::Material const * const m)
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "  Visualising BRDF based on rejection sampling scatter PDF...\n    ";
    std::cout.flush();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgNormals=new osg::Vec3Array();
    
    stitch::Vec3 normal(0.0, 0.0, 1.0);
    stitch::Vec3 orthA=(normal.orthVec()).normalised();
    stitch::Vec3 orthB=(normal ^ orthA).normalised();
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    
    float footprintSR=4.0f * float(M_PI) / vectors.size();
    const unsigned long numVectors=vectors.size();
    
    stitch::Vec3 dirA(1.0, 0.0, 1.0);
    dirA.normalise();
    
    {
        std::vector<stitch::Vec3> scatteredPDF=vectors;
        
        const unsigned long numPDFSamples=100*numVectors;
        const stitch::Vec3 inVec=dirA*-1.0f;
        
        stitch::Colour_t initialPhotonColour=m->getAlbedo();//Need to get the correct vector dimension if Colour_t defined as VecN.
        initialPhotonColour.setOnes();
        initialPhotonColour=initialPhotonColour * stitch::Photon::quantumEnergy_;
        
        //=== Scatter dirA numPDFSamples times to build up a scatter PDF ===
        for (unsigned long s=0; s<numPDFSamples; s++)
        {
            const stitch::Photon scatteredPhoton=m->scatterPhoton_reject_samp(normal,
                                                                              stitch::Vec3(),
                                                                              stitch::Photon(stitch::Vec3(), inVec, initialPhotonColour, 0)
                                                                              );
            
            //=== Add scattered photon direction to PDF bin ===
            if (scatteredPhoton.normDir_.lengthSq()>0.0f)
            {
                float radiusSq=FLT_MAX;
                
                stitch::BoundingVolume *bv=kdTree.getNearest(scatteredPhoton.normDir_, radiusSq);
                
                if (bv)
                {
                    scatteredPDF[bv->userIndex_]+=vectors[bv->userIndex_];
                }
            }
            //=== ===
        }
        
        
        //=== Check sampled scatter PDF against BRDF for each outgoing direction dirB ===
        for (unsigned long o=0; o<numVectors; o++)
        {
            stitch::Vec3 dirB=vectors[o];
            dirB.normalise();
            
            const float cosTheta=dirB*normal;
            
            if ( ( acosf(stitch::MathUtil::clamp(cosTheta, -1.0f, 1.0f)) >= (0.0f*(float(M_PI)/180.0f)) ) &&
                ( acosf(stitch::MathUtil::clamp(cosTheta, -1.0f, 1.0f)) < (85.0f*(float(M_PI)/180.0f)) ) )
            {//Do not test angles close to 90 deg. The reciprocal of (dir * normal) becomes quite large and difficult to handle numerically.
                //Convert PDF generated with m->scatterPhoton to BRDF.
                float BRDF_sampled=((scatteredPDF[o].length()-1.0f) / numPDFSamples) / (footprintSR*cosTheta);
                
                //float BRDF=m->BSDF(stitch::Vec3(), dirA, dirB, normal).cavrg();
                
                osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
                osgVertices->push_back(osg::Vec3(dirB.x()*BRDF_sampled,
                                                 dirB.y()*BRDF_sampled,
                                                 dirB.z()*BRDF_sampled));
                osgNormals->push_back(osg::Vec3(dirB.x(), dirB.y(), dirB.z()));
                osgNormals->push_back(osg::Vec3(dirB.x(), dirB.y(), dirB.z()));
            }
        }
        //=== ===
        
    }
    
    osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
    osgGeometry->setVertexArray(osgVertices.get());
    osgGeometry->setNormalArray(osgNormals.get());
    osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgVertices->size()));
    
    osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
    osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
    osg::Depth* depth = new osg::Depth();
    osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    
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
    
    endTick=timer.tick();
    std::cout << "  " << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}


//==========================================================
//==========================================================
//==========================================================
//!Compare the BRDF against the direct scatter PDF and the reject samp. scatter PDF.
void verifyBRDFImpl(stitch::Material const * const m)
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "  Error between the BRDF and the scatter PDFs...\n    ";
    std::cout.flush();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgNormals=new osg::Vec3Array();
    
    stitch::Vec3 normal(0.0, 0.0, 1.0);
    stitch::Vec3 orthA=(normal.orthVec()).normalised();
    stitch::Vec3 orthB=(normal ^ orthA).normalised();
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    
    float footprintSR=4.0f * float(M_PI) / vectors.size();
    const unsigned long numVectors=vectors.size();
    
    float sumError_direct=0.0f, maxError_direct=0.0f, minError_direct=0.0f;
    float sumError_reject_samp=0.0f, maxError_reject_samp=0.0f, minError_reject_samp=0.0f;
    
    unsigned long sampleCount=0;
    bool statsInitialised=false;
    
    float nextProgressUpdate=0.0f;
    unsigned long testStride=numVectors/50;
    
    
    //Note: The BRDF method is compared to the BRDF generated from the scatterPhoton method.
    //       For each incoming direction dirA many scatter events are simulated to generate a PDF.
    //       The PDF is compared to the BRDF method for each outgoing direction dirB.
    for (unsigned long i=0; i<numVectors; i+=testStride)
    {
        stitch::Vec3 dirA=vectors[i];
        dirA.normalise();
        
        if ( ( acosf(stitch::MathUtil::clamp(dirA*normal, -1.0f, 1.0f)) >= (0.0f*(float(M_PI)/180.0f)) ) &&
            ( acosf(stitch::MathUtil::clamp(dirA*normal, -1.0f, 1.0f)) < (70.0f*(float(M_PI)/180.0f)) ) )
        {
            std::vector<stitch::Vec3> scatteredPDF_direct=vectors;
            std::vector<stitch::Vec3> scatteredPDF_reject_samp=vectors;
            
            const unsigned long numPDFSamples=500*numVectors;
            const stitch::Vec3 inVec=dirA*-1.0f;
            
            stitch::Colour_t initialPhotonColour=m->getAlbedo();//Need to get the correct vector dimension if Colour_t defined as VecN.
            initialPhotonColour.setOnes();
            initialPhotonColour=initialPhotonColour * stitch::Photon::quantumEnergy_;
            
            //=== Scatter dirA numPDFSamples times to build up a scatter PDF ===
            for (unsigned long s=0; s<numPDFSamples; s++)
            {
                {
                    const stitch::Photon scatteredPhoton_direct=m->scatterPhoton_direct(normal,
                                                                                        stitch::Vec3(),
                                                                                        stitch::Photon(stitch::Vec3(), inVec, initialPhotonColour, 0)
                                                                                        );
                    
                    //=== Add direct scattered photon direction to PDF bin ===
                    if (scatteredPhoton_direct.normDir_.lengthSq()>0.0f)
                    {
                        float radiusSq=FLT_MAX;
                        
                        stitch::BoundingVolume *bv=kdTree.getNearest(scatteredPhoton_direct.normDir_, radiusSq);
                        
                        if (bv)
                        {
                            scatteredPDF_direct[bv->userIndex_]+=vectors[bv->userIndex_];
                        }
                    }
                    //=== ===
                }
                
                
                {
                    const stitch::Photon scatteredPhoton_reject_samp=m->scatterPhoton_reject_samp(normal,
                                                                                                  stitch::Vec3(),
                                                                                                  stitch::Photon(stitch::Vec3(), inVec, initialPhotonColour, 0)
                                                                                                  );
                    
                    //=== Add reject samp direct scattered photon direction to PDF bin ===
                    if (scatteredPhoton_reject_samp.normDir_.lengthSq()>0.0f)
                    {
                        float radiusSq=FLT_MAX;
                        
                        stitch::BoundingVolume *bv=kdTree.getNearest(scatteredPhoton_reject_samp.normDir_, radiusSq);
                        
                        if (bv)
                        {
                            scatteredPDF_reject_samp[bv->userIndex_]+=vectors[bv->userIndex_];
                        }
                    }
                    //=== ===
                }
                
            }
            
            
            //=== Check sampled scatter PDF against BRDF for each outgoing direction dirB ===
            float error_direct=0.0f;
            float error_reject_samp=0.0f;
            for (unsigned long o=0; o<numVectors; o++)
            {
                stitch::Vec3 dirB=vectors[o];
                dirB.normalise();
                
                const float cosTheta=dirB*normal;
                
                if ( ( acosf(stitch::MathUtil::clamp(cosTheta, -1.0f, 1.0f)) >= (0.0f*(float(M_PI)/180.0f)) ) &&
                    ( acosf(stitch::MathUtil::clamp(cosTheta, -1.0f, 1.0f)) < (70.0f*(float(M_PI)/180.0f)) ) )
                {//Do not test angles close to 90 deg. The reciprocal of (dir * normal) becomes quite large and difficult to handle numerically.
                    
                    //Convert PDF generated with m->scatterPhoton to BRDF for comparison.
                    float BRDF_direct=((scatteredPDF_direct[o].length()-1.0f) / numPDFSamples);// / (footprintSR * cosTheta);
                    float BRDF_reject_samp=((scatteredPDF_reject_samp[o].length()-1.0f) / numPDFSamples);// / (footprintSR * cosTheta);
                    
                    float BRDF=m->BSDF(stitch::Vec3(), dirA, dirB, normal).cavrg();
                    
                    error_direct += fabsf(BRDF*(footprintSR*cosTheta)-BRDF_direct/**(footprintSR*cosTheta)*/);
                    error_reject_samp += fabsf(BRDF*(footprintSR*cosTheta)-BRDF_reject_samp/**(footprintSR*cosTheta)*/);
                }
            }
            //=== ===
            
            
            //=== Keep stats on the error ===
            if (statsInitialised)
            {
                sumError_direct+=error_direct;
                if (error_direct<minError_direct) minError_direct=error_direct;
                if (error_direct>maxError_direct) maxError_direct=error_direct;
                
                sumError_reject_samp+=error_reject_samp;
                if (error_reject_samp<minError_reject_samp) minError_reject_samp=error_reject_samp;
                if (error_reject_samp>maxError_reject_samp) maxError_reject_samp=error_reject_samp;
                
                sampleCount++;
            } else
            {
                minError_direct=maxError_direct=error_direct;
                sumError_direct=error_direct;
                
                minError_reject_samp=maxError_reject_samp=error_reject_samp;
                sumError_reject_samp=error_reject_samp;
                
                sampleCount=1;
                statsInitialised=true;
            }
            //=== ===
            
            osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
            osgVertices->push_back(osg::Vec3(dirA.x()*(error_direct + error_reject_samp),
                                             dirA.y()*(error_direct + error_reject_samp),
                                             dirA.z()*(error_direct + error_reject_samp)));
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
        }
        
        //=== Report on progress ===
        const float progress=((float)i)/numVectors;
        if ( progress > nextProgressUpdate )
        {
            std::cout << progress*100.0f << "% ";
            nextProgressUpdate=progress+0.01f;
        }
        //=== ===
    }
    
    osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
    osgGeometry->setVertexArray(osgVertices.get());
    osgGeometry->setNormalArray(osgNormals.get());
    osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgVertices->size()));
    
    osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
    osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
    osg::Depth* depth = new osg::Depth();
    osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    
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
    
    std::cout << "100%\n";
    std::cout << "    _reject_samp [c|min%|avrg%|max%] = [" << sampleCount<< " | " << minError_reject_samp*100.0f << "% | " << (sumError_reject_samp/sampleCount)*100.0f << "% | " << maxError_reject_samp*100.0f << "%]\n";
    std::cout << "    _direct [c|min%|avrg%|max%] = [" << sampleCount<< " | " << minError_direct*100.0f  << "% | " << (sumError_direct/sampleCount)*100.0f << "% | " << maxError_direct*100.0f << "%]\n";
    std::cout.flush();
    
    endTick=timer.tick();
    std::cout << "  " << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}



//==========================================================
void verifyBRDFEnergyConservation(stitch::Material const * const m)
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "  Energy conservation (should be <= 1.0)...\n    ";
    std::cout.flush();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgNormals=new osg::Vec3Array();
    
    stitch::Vec3 normal(0.0, 0.0, 1.0);
    stitch::Vec3 orthA=(normal.orthVec()).normalised();
    stitch::Vec3 orthB=(normal ^ orthA).normalised();
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    
    float footprintSR=4.0f * float(M_PI) / vectors.size();
    unsigned long numVectors=vectors.size();
    
    stitch::Colour_t minResult;
    stitch::Colour_t sumResult;
    stitch::Colour_t maxResult;
    unsigned long sampleCount=0;
    bool statsInitialised=false;
    
    float nextProgressUpdate=0.0f;
    
    
    //Note: Calculate the energy conservation integral for each incoming vector dirA.
    //      Stats are kept on the min, avrg and max results.
    for (unsigned long i=0; i<numVectors; i+=1)
    {
        stitch::Vec3 dirA=vectors[i];
        dirA.normalise();
        
        if ( ( acosf(stitch::MathUtil::clamp(dirA*normal, -1.0f, 1.0f)) >= (0.0f*(float(M_PI)/180.0f)) ) &&
            ( acosf(stitch::MathUtil::clamp(dirA*normal, -1.0f, 1.0f)) < (70.0f*(float(M_PI)/180.0f)) ) )
        {
            stitch::Colour_t Result;
            
            //=== Calculate the energy conservation integral over the outgoing directions dirB ===
            for (unsigned long o=0; o<numVectors; o++)
            {
                stitch::Vec3 dirB=vectors[o];
                dirB.normalise();
                
                float cosTheta=dirB*normal;
                
                if (cosTheta>0.0f)
                {
                    Result+=m->BSDF(stitch::Vec3(), dirA, dirB, normal) * (cosTheta * footprintSR);
                }
            }
            //=== ===
            
            //=== Keep stats on the Result ===
            if (statsInitialised)
            {
                sumResult+=Result;
                if (Result.csum()>maxResult.csum()) maxResult=Result;
                if (Result.csum()<minResult.csum()) minResult=Result;
                sampleCount++;
            } else
            {
                sumResult=minResult=maxResult=Result;
                sampleCount=1;
                statsInitialised=true;
            }
            //=== ===
            
            osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
            osgVertices->push_back(osg::Vec3(dirA.x()*Result.csum()/Result.getNumComponents(),
                                             dirA.y()*Result.csum()/Result.getNumComponents(),
                                             dirA.z()*Result.csum()/Result.getNumComponents()));
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
            
        }
        
        //=== Report on progress ===
        const float progress=((float)i)/numVectors;
        if ( progress > nextProgressUpdate )
        {
            std::cout << progress*100.0f << "% ";
            nextProgressUpdate=progress+0.01f;
        }
        //=== ===
    }
    
    osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
    osgGeometry->setVertexArray(osgVertices.get());
    osgGeometry->setNormalArray(osgNormals.get());
    osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,osgVertices->size()));
    
    osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
    osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
    osg::Depth* depth = new osg::Depth();
    osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    
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
    
    std::cout << "100%\n";
    std::cout << "    [c|min|avrg|max] = [" << sampleCount<< " | " << minResult.cavrg() << " | " << sumResult.cavrg()/sampleCount << " | " << maxResult.cavrg() << "]\n";
    std::cout.flush();
    
    endTick=timer.tick();
    
    std::cout << "  " << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}



//==========================================================
void verifyBRDFSymmetry(stitch::Material const * const m)
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "  BRDF symmetry (<->) difference (should be zero)...\n    ";
    std::cout.flush();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgNormals=new osg::Vec3Array();
    
    stitch::Vec3 normal(0.0, 0.0, 1.0);
    stitch::Vec3 orthA=(normal.orthVec()).normalised();
    stitch::Vec3 orthB=(normal ^ orthA).normalised();
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    osgNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    osgNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
    
    osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    osgVertices->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    osgNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
    
    float footprintSR=4.0f * float(M_PI) / vectors.size();
    
    const unsigned long numVectors=vectors.size();
    
    float sumError=0.0f, maxError=0.0f, minError=0.0f;
    unsigned long sampleCount=0;
    bool statsInitialised=false;
    
    float nextProgressUpdate=0.0f;
    
    //Note: For a sample of the incoming directions dirA the symmetry of the BRDF is tested.
    for (unsigned long i=0; i<numVectors; i+=10)
    {
        stitch::Vec3 dirA=vectors[i];
        dirA.normalise();
        
        if ( ( acosf(stitch::MathUtil::clamp(dirA*normal, -1.0f, 1.0f)) >= (0.0f*(float(M_PI)/180.0f)) ) &&
            ( acosf(stitch::MathUtil::clamp(dirA*normal, -1.0f, 1.0f)) < (85.0f*(float(M_PI)/180.0f)) ) )
        {
            
            float error=0.0f;
            for (unsigned long o=0; o<numVectors; o++)
            {
                stitch::Vec3 dirB=vectors[o];
                dirB.normalise();
                
                if ( ( acosf(stitch::MathUtil::clamp(dirB*normal, -1.0f, 1.0f)) >= (0.0f*(float(M_PI)/180.0f)) ) &&
                    ( acosf(stitch::MathUtil::clamp(dirB*normal, -1.0f, 1.0f)) < (85.0f*(float(M_PI)/180.0f)) ) )
                {
                    float BRDF_forward=m->BSDF(stitch::Vec3(), dirA, dirB, normal).cavrg();
                    float BRDF_backward=m->BSDF(stitch::Vec3(), dirB, dirA, normal).cavrg();
                    
                    error += fabs( (BRDF_forward-BRDF_backward)*footprintSR*(normal*dirB) );
                }
            }
            
            //=== Keep stats on the error ===
            if (statsInitialised)
            {
                sumError+=error;
                if (error<minError) minError=error;
                if (error>maxError) maxError=error;
                sampleCount++;
            } else
            {
                minError=maxError=error;
                sumError=error;
                
                sampleCount=1;
                statsInitialised=true;
            }
            //=== ===
            
            osgVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
            osgVertices->push_back(osg::Vec3(dirA.x()*error,
                                             dirA.y()*error,
                                             dirA.z()*error));
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
            osgNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
        }
        
        //=== Report on progress ===
        const float progress=((float)i)/numVectors;
        if ( progress > nextProgressUpdate )
        {
            std::cout << progress*100.0f << "% ";
            nextProgressUpdate=progress+0.01f;
        }
        //=== ===
    }
    
    osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
    osgGeometry->setVertexArray(osgVertices.get());
    osgGeometry->setNormalArray(osgNormals.get());
    osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgVertices->size()));
    
    osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
    osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
    osg::Depth* depth = new osg::Depth();
    osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    
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
    
    std::cout << "100%\n";
    std::cout << "    [c|min|avrg|max] = [" << sampleCount<< " | " << minError << " | " << sumError/sampleCount << " | " << maxError << "]\n";
    std::cout.flush();
    
    endTick=timer.tick();
    
    std::cout << "  " << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}

void visualiseBRDF(stitch::Material const * const m, const stitch::Vec3 &dirA)
{
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "  BRDF visualisation of material " << m->getTypeString() << "...\n";
    std::cout.flush();
    
    std::vector<stitch::Vec3> BRDFSmoothVector;
    std::vector<stitch::Vec3> BRDFSmoothDirectScatterVector;
    std::vector<stitch::Vec3> BRDFSmoothRejectSampScatterVector;
    
    std::vector<stitch::Vec3> BRDFSmoothNormal;
    std::vector<stitch::Vec3> BRDFSmoothDirectScatterNormal;
    std::vector<stitch::Vec3> BRDFSmoothRejectSampScatterNormal;
    
    stitch::KDTree BRDFKdTree;
    stitch::KDTree BRDFDirectScatterKdTree;
    stitch::KDTree BRDFRejectSampScatterKdTree;
    
    std::vector<stitch::BoundingVolume *> BRDFVector;
    std::vector<stitch::BoundingVolume *> BRDFDirectScatterVector;
    std::vector<stitch::BoundingVolume *> BRDFRejectSampScatterVector;
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    osg::ref_ptr<osg::Vec3Array> osgVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgNormals=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec4Array> osgColours=new osg::Vec4Array();
    
    stitch::Vec3 normal(0.0, 0.0, 1.0);
    stitch::Vec3 orthA=(normal.orthVec()).normalised();
    stitch::Vec3 orthB=(normal ^ orthA).normalised();
    
    float footprintSR=4.0f * float(M_PI) / vectors.size();
    
    const unsigned long numVectors=vectors.size();
    const unsigned long numBinIndices=binIndices.size();
    
    {
        std::cout << "    " << "Generating scattered PDF...\n";
        std::cout << "      ";
        std::cout.flush();
        
        std::vector<stitch::Vec3> scatteredPDF_direct=vectors;
        std::vector<stitch::Vec3> scatteredPDF_reject_samp=vectors;
        
        const unsigned long numPDFSamples=500*numVectors;
        const stitch::Vec3 inVec=dirA*-1.0f;
        
        stitch::Colour_t initialPhotonColour=m->getAlbedo();//Need to get the correct vector dimension if Colour_t defined as VecN.
        initialPhotonColour.setOnes();
        initialPhotonColour=initialPhotonColour * stitch::Photon::quantumEnergy_;
        
        //=== Scatter dirA numPDFSamples times to build up a scatter PDF ===
        for (unsigned long s=0; s<numPDFSamples; s++)
        {
            const stitch::Photon scatteredPhoton_reject_samp=m->scatterPhoton_reject_samp(normal,
                                                                                          stitch::Vec3(),
                                                                                          stitch::Photon(stitch::Vec3(), inVec, initialPhotonColour, 0)
                                                                                          );
            
            const stitch::Photon scatteredPhoton_direct=m->scatterPhoton_direct(normal,
                                                                                stitch::Vec3(),
                                                                                stitch::Photon(stitch::Vec3(), inVec, initialPhotonColour, 0)
                                                                                );
            
            //=== Add direct scattered photon direction to PDF bin ===
            if (scatteredPhoton_direct.normDir_.lengthSq()>0.0f)
            {
                float radiusSq=FLT_MAX;
                
                stitch::BoundingVolume *bv=kdTree.getNearest(scatteredPhoton_direct.normDir_, radiusSq);
                
                if (bv)
                {
                    scatteredPDF_direct[bv->userIndex_]+=vectors[bv->userIndex_];
                }
            }
            //=== ===
            
            //=== Add reject samp direct scattered photon direction to PDF bin ===
            if (scatteredPhoton_reject_samp.normDir_.lengthSq()>0.0f)
            {
                float radiusSq=FLT_MAX;
                
                stitch::BoundingVolume *bv=kdTree.getNearest(scatteredPhoton_reject_samp.normDir_, radiusSq);
                
                if (bv)
                {
                    scatteredPDF_reject_samp[bv->userIndex_]+=vectors[bv->userIndex_];
                }
            }
            //=== ===
            
            if ( (s % (numPDFSamples/5)) == 0)
            {
                std::cout << (s/((float)numPDFSamples))*100.0f << "%...";
                std::cout.flush();
            }
        }
        std::cout << "100%\n";
        std::cout << "    " << "done.\n";
        std::cout.flush();
        
        
        std::cout << "    " << "Smooth and render BRDF results...";
        std::cout.flush();
        
        for (unsigned long o=0; o<numVectors; o++)
        {
            stitch::Vec3 dirB=vectors[o];
            dirB.normalise();
            
            float cosTheta=dirB*normal;
            
            float BRDF=(cosTheta>0.05f) ?  (m->BSDF(stitch::Vec3(), dirA, dirB, normal).cavrg()) : 0.0f;
            float BRDF_direct=(cosTheta>0.05f) ? ((scatteredPDF_direct[o].length()-1.0f) / numPDFSamples) / (footprintSR*cosTheta) : 0.0f;
            float BRDF_reject_samp=(cosTheta>0.05f) ? ((scatteredPDF_reject_samp[o].length()-1.0f) / numPDFSamples) / (footprintSR*cosTheta) : 0.0f;
            
            stitch::Vec3 BRDF_vect=dirB * BRDF;
            stitch::Vec3 BRDF_direct_vect=dirB * BRDF_direct;
            stitch::Vec3 BRDF_reject_samp_vect=dirB * BRDF_reject_samp;
            
            BRDFVector.push_back(new stitch::BoundingVolume(BRDF_vect, 0.0f));
            BRDFDirectScatterVector.push_back(new stitch::BoundingVolume(BRDF_direct_vect, 0.0f));
            BRDFRejectSampScatterVector.push_back(new stitch::BoundingVolume(BRDF_reject_samp_vect, 0.0f));
            
            BRDFKdTree.addItem(BRDFVector[BRDFVector.size()-1]);
            BRDFDirectScatterKdTree.addItem(BRDFDirectScatterVector[BRDFDirectScatterVector.size()-1]);
            BRDFRejectSampScatterKdTree.addItem(BRDFRejectSampScatterVector[BRDFRejectSampScatterVector.size()-1]);
        }
        
        
        std::vector<stitch::Vec3> splitAxisVec;
        splitAxisVec.push_back(stitch::Vec3(1.0f, 0.0f, 0.0f));
        splitAxisVec.push_back(stitch::Vec3(0.0f, 1.0f, 0.0f));
        splitAxisVec.push_back(stitch::Vec3(0.0f, 0.0f, 1.0f));
        
        BRDFKdTree.build(48, 0, 1000000, splitAxisVec);
        BRDFDirectScatterKdTree.build(48, 0, 1000000, splitAxisVec);
        BRDFRejectSampScatterKdTree.build(48, 0, 1000000, splitAxisVec);
        
        
        for (unsigned long o=0; o<numVectors; o++)
        {
            stitch::Vec3 dirB=vectors[o];
            dirB.normalise();
            
            {
                stitch::KNearestItems BRDFKNearestItems(BRDFVector[o]->centre_, 1.0f, 10);
                BRDFKdTree.getNearestK(&BRDFKNearestItems);
                stitch::Vec3 BRDFVecCentre;
                for (size_t i=0; i<BRDFKNearestItems.numItems_; i++)
                {
                    BRDFVecCentre+=BRDFKNearestItems.itemArray_[i]->centre_;
                }
                BRDFVecCentre/=BRDFKNearestItems.numItems_;
                float BRDF=BRDFVecCentre.length();
                BRDFSmoothVector.push_back(dirB * BRDF);
                
                stitch::Vec3 BRDFVecNormal;
                for (size_t i=0; i<BRDFKNearestItems.numItems_; i++)
                {
                    for (size_t j=i+1; j<BRDFKNearestItems.numItems_; j++)
                    {
                        stitch::Vec3 normal=(BRDFKNearestItems.itemArray_[i]->centre_-BRDFVecCentre)^(BRDFKNearestItems.itemArray_[j]->centre_-BRDFVecCentre);
                        
                        if ((normal*BRDFVecNormal)>=0.0f)
                        {
                            BRDFVecNormal+=normal;
                        } else
                        {
                            BRDFVecNormal-=normal;
                        }
                    }
                }
                BRDFSmoothNormal.push_back(BRDFVecNormal.normalised());
            }
            
            
            {
                stitch::KNearestItems BRDFDirectScatterKNearestItems(BRDFDirectScatterVector[o]->centre_, 1.0f, 20);
                BRDFDirectScatterKdTree.getNearestK(&BRDFDirectScatterKNearestItems);
                stitch::Vec3 BRDFDirectScatterVecCentre;
                for (size_t i=0; i<BRDFDirectScatterKNearestItems.numItems_; i++)
                {
                    BRDFDirectScatterVecCentre+=BRDFDirectScatterKNearestItems.itemArray_[i]->centre_;
                }
                BRDFDirectScatterVecCentre/=BRDFDirectScatterKNearestItems.numItems_;
                float BRDFDirectScatter=BRDFDirectScatterVecCentre.length();
                BRDFSmoothDirectScatterVector.push_back(dirB * BRDFDirectScatter);
                
                stitch::Vec3 BRDFDirectScatterVecNormal;
                for (size_t i=0; i<BRDFDirectScatterKNearestItems.numItems_; i++)
                {
                    for (size_t j=i+1; j<BRDFDirectScatterKNearestItems.numItems_; j++)
                    {
                        stitch::Vec3 normal=(BRDFDirectScatterKNearestItems.itemArray_[i]->centre_-BRDFDirectScatterVecCentre)^(BRDFDirectScatterKNearestItems.itemArray_[j]->centre_-BRDFDirectScatterVecCentre);
                        
                        if ((normal*dirB)>=0.0f)
                        {
                            BRDFDirectScatterVecNormal+=normal;
                        } else
                        {
                            BRDFDirectScatterVecNormal-=normal;
                        }
                    }
                }
                BRDFSmoothDirectScatterNormal.push_back(BRDFDirectScatterVecNormal.normalised());
            }
            
            
            {
                stitch::KNearestItems BRDFRejectSampScatterKNearestItems(BRDFRejectSampScatterVector[o]->centre_, 1.0f, 20);
                BRDFRejectSampScatterKdTree.getNearestK(&BRDFRejectSampScatterKNearestItems);
                stitch::Vec3 BRDFRejectSampScatterVecCentre;
                for (size_t i=0; i<BRDFRejectSampScatterKNearestItems.numItems_; i++)
                {
                    BRDFRejectSampScatterVecCentre+=BRDFRejectSampScatterKNearestItems.itemArray_[i]->centre_;
                }
                BRDFRejectSampScatterVecCentre/=BRDFRejectSampScatterKNearestItems.numItems_;
                float BRDFRejectSampScatter=BRDFRejectSampScatterVecCentre.length();
                BRDFSmoothRejectSampScatterVector.push_back(dirB * BRDFRejectSampScatter);
                
                stitch::Vec3 BRDFRejectSampScatterVecNormal;
                for (size_t i=0; i<BRDFRejectSampScatterKNearestItems.numItems_; i++)
                {
                    for (size_t j=i+1; j<BRDFRejectSampScatterKNearestItems.numItems_; j++)
                    {
                        stitch::Vec3 normal=(BRDFRejectSampScatterKNearestItems.itemArray_[i]->centre_-BRDFRejectSampScatterVecCentre)^(BRDFRejectSampScatterKNearestItems.itemArray_[j]->centre_-BRDFRejectSampScatterVecCentre);
                        
                        if ((normal*dirB)>=0.0f)
                        {
                            BRDFRejectSampScatterVecNormal+=normal;
                        } else
                        {
                            BRDFRejectSampScatterVecNormal-=normal;
                        }
                    }
                }
                BRDFSmoothRejectSampScatterNormal.push_back(BRDFRejectSampScatterVecNormal.normalised());
            }
        }
        
        
        {//Generate OSG geomtry vertices...
            
            for (unsigned long i=0; i<numBinIndices; i+=3)
            {
                stitch::Vec3 v0=BRDFSmoothVector[binIndices[i+0]];
                stitch::Vec3 v1=BRDFSmoothVector[binIndices[i+1]];
                stitch::Vec3 v2=BRDFSmoothVector[binIndices[i+2]];
                
                osgVertices->push_back(osg::Vec3(v0.x(), v0.y(), v0.z()));
                osgVertices->push_back(osg::Vec3(v1.x(), v1.y(), v1.z()));
                osgVertices->push_back(osg::Vec3(v2.x(), v2.y(), v2.z()));
                
                stitch::Vec3 normal0=BRDFSmoothNormal[binIndices[i+0]];//((v1-v0)^(v2-v0)).normalised();
                stitch::Vec3 normal1=BRDFSmoothNormal[binIndices[i+1]];//((v2-v1)^(v0-v1)).normalised();
                stitch::Vec3 normal2=BRDFSmoothNormal[binIndices[i+2]];//((v0-v2)^(v1-v2)).normalised();
                
                osgNormals->push_back(osg::Vec3(normal0.x(), normal0.y(), normal0.z()));
                osgNormals->push_back(osg::Vec3(normal1.x(), normal1.y(), normal1.z()));
                osgNormals->push_back(osg::Vec3(normal2.x(), normal2.y(), normal2.z()));
                
                osgColours->push_back(osg::Vec4(1.0, 0.5, 0.5, 0.7));
                osgColours->push_back(osg::Vec4(1.0, 0.5, 0.5, 0.7));
                osgColours->push_back(osg::Vec4(1.0, 0.5, 0.5, 0.7));
            }
            
            /*
            for (unsigned long i=0; i<numBinIndices; i+=3)
            {
                stitch::Vec3 v0=BRDFSmoothDirectScatterVector[binIndices[i+0]];
                stitch::Vec3 v1=BRDFSmoothDirectScatterVector[binIndices[i+1]];
                stitch::Vec3 v2=BRDFSmoothDirectScatterVector[binIndices[i+2]];
                
                osgVertices->push_back(osg::Vec3(v0.x(), v0.y(), v0.z()));
                osgVertices->push_back(osg::Vec3(v1.x(), v1.y(), v1.z()));
                osgVertices->push_back(osg::Vec3(v2.x(), v2.y(), v2.z()));
                
                stitch::Vec3 normal0=BRDFSmoothDirectScatterNormal[binIndices[i+0]];//((v1-v0)^(v2-v0)).normalised();
                stitch::Vec3 normal1=BRDFSmoothDirectScatterNormal[binIndices[i+1]];//((v2-v1)^(v0-v1)).normalised();
                stitch::Vec3 normal2=BRDFSmoothDirectScatterNormal[binIndices[i+2]];//((v0-v2)^(v1-v2)).normalised();
                
                osgNormals->push_back(osg::Vec3(normal0.x(), normal0.y(), normal0.z()));
                osgNormals->push_back(osg::Vec3(normal1.x(), normal1.y(), normal1.z()));
                osgNormals->push_back(osg::Vec3(normal2.x(), normal2.y(), normal2.z()));
                
                osgColours->push_back(osg::Vec4(0.5, 1.0, 0.5, 0.7));
                osgColours->push_back(osg::Vec4(0.5, 1.0, 0.5, 0.7));
                osgColours->push_back(osg::Vec4(0.5, 1.0, 0.5, 0.7));
            }
            
            
             for (unsigned long i=0; i<numBinIndices; i+=3)
             {
             stitch::Vec3 v0=BRDFSmoothRejectSampScatterVector[binIndices[i+0]];
             stitch::Vec3 v1=BRDFSmoothRejectSampScatterVector[binIndices[i+1]];
             stitch::Vec3 v2=BRDFSmoothRejectSampScatterVector[binIndices[i+2]];
             
             osgVertices->push_back(osg::Vec3(v0.x(), v0.y(), v0.z()));
             osgVertices->push_back(osg::Vec3(v1.x(), v1.y(), v1.z()));
             osgVertices->push_back(osg::Vec3(v2.x(), v2.y(), v2.z()));
             
             stitch::Vec3 normal0=BRDFSmoothRejectSampScatterNormal[binIndices[i+0]];//((v1-v0)^(v2-v0)).normalised();
             stitch::Vec3 normal1=BRDFSmoothRejectSampScatterNormal[binIndices[i+1]];//((v2-v1)^(v0-v1)).normalised();
             stitch::Vec3 normal2=BRDFSmoothRejectSampScatterNormal[binIndices[i+2]];//((v0-v2)^(v1-v2)).normalised();
             
             osgNormals->push_back(osg::Vec3(normal0.x(), normal0.y(), normal0.z()));
             osgNormals->push_back(osg::Vec3(normal1.x(), normal1.y(), normal1.z()));
             osgNormals->push_back(osg::Vec3(normal2.x(), normal2.y(), normal2.z()));
             
             osgColours->push_back(osg::Vec4(0.5, 0.5, 1.0, 0.7));
             osgColours->push_back(osg::Vec4(0.5, 0.5, 1.0, 0.7));
             osgColours->push_back(osg::Vec4(0.5, 0.5, 1.0, 0.7));
             }
             */
        }
        
        std::cout << "done.\n";
        std::cout.flush();
    }
    
    
    for (size_t i=0; i<osgVertices->size(); i+=3)
    {
        osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
        osgGeometry->setVertexArray(osgVertices.get());
        osgGeometry->setNormalArray(osgNormals.get());
        osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        osgGeometry->setColorArray(osgColours.get());
        osgGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        
        osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,i,3));
        
        osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
        osgStateset->setNestRenderBins(false);
        osgStateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        osgStateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        osgStateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
        
        osg::Depth* depth = new osg::Depth();
        osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
        
        osg::BlendFunc *fn = new osg::BlendFunc();
        fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
        osgStateset->setAttributeAndModes(fn, osg::StateAttribute::ON);
        
        osg::Material *material = new osg::Material();
        material->setColorMode(osg::Material::DIFFUSE);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.7f));
        material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.7f));
        material->setShininess(osg::Material::FRONT_AND_BACK, 50.0f);
        osgStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
        
        osgGeode->addDrawable(osgGeometry.get());
    }
    
    {
        osg::ref_ptr<osg::Vec3Array> osgAxisVertices=new osg::Vec3Array();
        osg::ref_ptr<osg::Vec3Array> osgAxisNormals=new osg::Vec3Array();
        osg::ref_ptr<osg::Vec4Array> osgAxisColours=new osg::Vec4Array();
        
        osgAxisVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        osgAxisVertices->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
        osgAxisNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
        osgAxisNormals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));
        osgAxisColours->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        osgAxisColours->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        
        osgAxisVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        osgAxisVertices->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
        osgAxisNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
        osgAxisNormals->push_back(osg::Vec3(orthA.x(), orthA.y(), orthA.z()));
        osgAxisColours->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        osgAxisColours->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        
        osgAxisVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        osgAxisVertices->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
        osgAxisNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
        osgAxisNormals->push_back(osg::Vec3(orthB.x(), orthB.y(), orthB.z()));
        osgAxisColours->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        osgAxisColours->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        
        osgAxisVertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        osgAxisVertices->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
        osgAxisNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
        osgAxisNormals->push_back(osg::Vec3(dirA.x(), dirA.y(), dirA.z()));
        osgAxisColours->push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0));
        osgAxisColours->push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0));
        
        osg::ref_ptr<osg::Geometry> osgGeometry=new osg::Geometry();
        osgGeometry->setVertexArray(osgAxisVertices.get());
        osgGeometry->setNormalArray(osgAxisNormals.get());
        osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        osgGeometry->setColorArray(osgAxisColours.get());
        osgGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        
        osgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgAxisVertices->size()));
        
        osg::ref_ptr<osg::StateSet> osgStateset=osgGeometry->getOrCreateStateSet();
        osgStateset->setNestRenderBins(false);
        osgStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        
        osg::Depth* depth = new osg::Depth();
        osgStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
        
        osg::Material *material = new osg::Material();
        material->setColorMode(osg::Material::DIFFUSE);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        osgStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
        
        osgGeode->addDrawable(osgGeometry.get());
    }
    
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(g_sceneGraphMutex);
        g_rootGroup_->removeChildren(0, g_rootGroup_->getNumChildren());
        g_rootGroup_->addChild(osgGeode);
    }
    
    endTick=timer.tick();
    
    std::cout << "  " << timer.delta_m(startTick, endTick) << " ms...done.\n\n";
    std::cout.flush();
}

//==========================================================
//==========================================================
//==========================================================
bool testBRDF(stitch::Material const * const m)
{
    //=== ===
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    std::cout << "Testing BRDF of material " << m->getTypeString() << ":\n";
    std::cout.flush();
    
    verifyBRDFImpl(m);
    verifyBRDFSymmetry(m);
    verifyBRDFEnergyConservation(m);
    
    endTick=timer.tick();
    
    std::cout << timer.delta_m(startTick, endTick) << " ms...done.\n\n\n";
    std::cout.flush();
    //=== ===
    
    return true;
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

        std::chrono::milliseconds dura(500);
        std::this_thread::sleep_for(dura);
    }
    
}


//==========================================================
int main(void)
{
    stitch::GlobalRand::initialiseUniformSamplerArray();
    
    //==========================//
    //=== Initialise Viewer ===//
    //==========================//
    {
        int argc=1;
        char **argv=new char *[10];
        argv[0]=new char [256];
        strcpy(argv[0], "BRDF Test");
        
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
    //==========================//
    //==========================//
    //==========================//
    
    
    
    //===================================//
    //=== Initilise materials to test ===//
    //===================================//
    
    const float lobeExponent=30.0f;
    const float standardDeviation=0.1f;
    stitch::Colour_t dRefl(1.0f, 1.0f, 1.0f);
    stitch::Colour_t sRefl(1.0f, 1.0f, 1.0f);
    
    stitch::DiffuseMaterial *diff = new stitch::DiffuseMaterial(dRefl);
    
    stitch::GlossyMaterial *glossy = new stitch::GlossyMaterial(sRefl, standardDeviation);
    
    stitch::PhongMaterial *phong = new stitch::PhongMaterial(sRefl.oneMinus(),
                                                             sRefl,
                                                             stitch::Colour_t(0.0, 0.0, 0.0),
                                                             lobeExponent,
                                                             "");
    
    stitch::BlinnPhongMaterial *blinnPhong = new stitch::BlinnPhongMaterial(sRefl.oneMinus(),
                                                                            sRefl,
                                                                            stitch::Colour_t(0.0, 0.0, 0.0),
                                                                            lobeExponent,
                                                                            "");
    
    //===================================//
    //===================================//
    //===================================//
    
    
    osg::Timer_t startTick=0.0, endTick=0.0;
    startTick=timer.tick();
    
    //========================//
    //=== Start BRDF tests ===//
    //========================//
    
    
    generateBRDFBinVectors();
    //generateBRDFBinVectorsGeoDual();
    //testBRDFBinDensity();
    
    verifyPDF(lobeExponent, COSINE_LOBE_PDF);
    verifyPDF(standardDeviation, GAUSSIAN_LOBE_PDF);
    
    //diff
    testBRDF(diff);
    visualiseRejectSampBRDF(diff);
    visualiseBRDF(diff, stitch::Vec3(1.0, 1.0, 2.0).normalised());
    
    //phong
    //testBRDF(phong);
    //visualiseRejectSampBRDF(phong);
    //visualiseBRDF(phong, stitch::Vec3(1.0, 1.0, 2.0).normalised());
    
    //Glossy
    //testBRDF(glossy);
    //visualiseRejectSampBRDF(glossy);
    //visualiseBRDF(glossy, stitch::Vec3(1.0, 1.0, 2.0).normalised());
    
    //Blinn-Phong
    //testBRDF(blinnPhong);
    //visualiseRejectSampBRDF(blinnPhong);
    //visualiseBRDF(blinnPhong, stitch::Vec3(1.0, 1.0, 2.0).normalised());
    
    //========================//
    //========================//
    //========================//
    
    endTick=timer.tick();
    
    std::cout << "All tests completed in " << timer.delta_s(startTick, endTick) << " seconds.\n\n";
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
