/*
 * $Id: Scene.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Scene.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/03/03.
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

#include "Scene.h"

#include "Objects/BrushModel.h"
#include "Objects/PolygonModel.h"
#include "Lights/PointLight.h"
#include "Materials/PhongMaterial.h"
#include "Materials/BlinnPhongMaterial.h"
#include "Materials/DiffuseMaterial.h"
#include "Materials/GlossyMaterial.h"
#include "Beam.h"

//=======================================================================//
stitch::Scene::Scene()
{
    light_=nullptr;
    
    //stitch::BeamSegment::generateVolumeTexture();
    
    ballTree_=new stitch::BallTree;
#ifdef USE_OSG
    rootGroup_=new osg::Group;
    
    rendererGroup_=new osg::Group;
    rootGroup_->addChild(rendererGroup_);
#endif// USE_OSG
}

//=======================================================================//
size_t stitch::Scene::create(const std::string scene_name, const Vec3 &light_orig, const Colour_t &lightSPD,
                             const size_t objectTreeChunkSize, const size_t internalObjectTreeChunkSize,
                             bool createOSGLineGeometry,
                             bool createOSGNormalGeometry,
                             float glossySD)
{
    light_=new PointLight(light_orig, lightSPD);
    
    if (scene_name=="CausticGear")
    {
        createCausticGear(internalObjectTreeChunkSize, glossySD);
    } else
        if (scene_name=="CausticDragon")
        {
            createCausticDragon(internalObjectTreeChunkSize, glossySD);
        } else
            if (scene_name=="CausticBunny")
            {
                createCausticBunny(internalObjectTreeChunkSize, glossySD);
            } else
                if (scene_name=="CausticRing")
                {
                    createCausticRing(internalObjectTreeChunkSize, glossySD);
                } else
                    if (scene_name=="SphereBox2013")
                    {
                        createSphereBox2013(internalObjectTreeChunkSize, glossySD);
                    } else
                        if (scene_name=="MULTIBOUNCE1")
                        {
                            createMultiBounce1(internalObjectTreeChunkSize, glossySD);
                        } else
                            if (scene_name=="CGF2010")
                            {
                                createCGF2010(internalObjectTreeChunkSize, glossySD);
                            } else
                                if (scene_name=="Report1")
                                {
                                    createReport1(internalObjectTreeChunkSize, glossySD);
                                } else
                                {
                                    createSponza(internalObjectTreeChunkSize, glossySD);
                                }
    
    std::cout <<  "Number of objects: " << ballTree_->getNumItems() << "\n";
    std::cout.flush();
    
#ifdef USE_OSG
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> sceneGraphLock(sceneGraphMutex_);
        rootGroup_->addChild(ballTree_->constructOSGNode(createOSGLineGeometry, createOSGNormalGeometry, false));
    }
#endif// USE_OSG
    
    {//Create object tree acceleration structure.
        if (ballTree_->itemVector_.size()>objectTreeChunkSize)
        {
            ballTree_->build(objectTreeChunkSize, 0);
        }
        ballTree_->updateBV();
    }
    return ballTree_->getNumItems();
}


void stitch::Scene::createSphereBox2013(const size_t internalObjectTreeChunkSize, float glossySD)
{
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    /*
    {//Load PLY file.
        stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        
        polygonModel->loadPLYVertices("Data/bunny.ply", stitch::Vec3(0.0f, -2.75f, -2.0f), 20.0, false);
        polygonModel->calculateVertexNormals();
        
        polygonModel->generatePolygonObjectsFromVertices();
        polygonModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(polygonModel);
    }
    
    {
    stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::DiffuseMaterial(stitch::Colour_t(0.6, 0.6, 0.6)));
    polygonModel->loadOBJVertices("Data/teapot.obj", stitch::Vec3(0.0f, -1.0f, 0.0f), 0.1, false);
    polygonModel->calculateVertexNormals();
    polygonModel->generatePolygonObjectsFromVertices();
    polygonModel->buildBallTree(internalObjectTreeChunkSize);
    ballTree_->addItem(polygonModel);
    }
    */
    
    {//floor
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        //stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), 0.1));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//ceiling
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), -12.5f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//middle
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 0.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 0.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//right
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.09804f, 0.09804f, 0.94902f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 10.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//back
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.94902f, 0.94902f, 0.09804)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//middle, back
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 0.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 10.1f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 0.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), -20.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.1f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    
    {
        stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), glossySD));
        
        polygonModel->loadIcosahedronBasedSphere(300, stitch::Vec3(-6.0f, 4.5f, 0.1f), 2.5f, false);
        
        polygonModel->generatePolygonObjectsFromVertices();
        polygonModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(polygonModel);
    }
    
    {
        stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), glossySD));
        
        polygonModel->loadIcosahedronBasedSphere(2000, stitch::Vec3(4.0f, 1.0f, -4.0f), 3.0f, true);
        
        polygonModel->generatePolygonObjectsFromVertices();
        polygonModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(polygonModel);
    }

    {
        std::vector<stitch::Vec3> vectors;
        std::vector<size_t> indices;
        
        const size_t numThetas=360;
        const float dTheta=2.0f*((float)M_PI) / numThetas;
        const float oRadius=2.0f;
        const float iRadius=1.0f;
        
        const size_t numPhis=180;
        const float dPhi=2.0f*((float)M_PI) / numPhis;
        
        for (size_t iTheta=0; iTheta<numThetas; ++iTheta)
        {
            const float u=cosf(iTheta * dTheta);
            const float v=sinf(iTheta * dTheta);
            
            const stitch::Vec3 uv(u, v, 0.0f);
            const stitch::Vec3 uvOrth(v, -u, 0.0f);
            const stitch::Vec3 uvUp=uv^uvOrth;
            
            const float dr=powf(sinf(iTheta * dTheta * 10.0f)*0.5f+0.5f, 2.0)*0.25f;
            
            const stitch::Vec3 c=uv * ((oRadius+dr+iRadius)*0.5f);
            
            for (size_t iPhi=0; iPhi<numPhis; ++iPhi)
            {
                const float s=cosf(iPhi*dPhi) > 0.0f ? powf(fabsf(cosf(iPhi*dPhi)), 0.25f) : -powf(fabsf(cosf(iPhi*dPhi)), 0.25f);
                const float t=sinf(iPhi*dPhi);
                
                const stitch::Vec3 p = c + (uv*s)*((oRadius+dr-iRadius)*0.5f ) + (uvUp*t)*((oRadius-iRadius)*0.5f );
                
                vectors.push_back(p);
            }
        }
        
        for (size_t iTheta=0; iTheta<numThetas; ++iTheta)
        {
            for (size_t iPhi=0; iPhi<numPhis; ++iPhi)
            {
                indices.push_back(((iTheta+1)%numThetas)*numPhis + iPhi);
                indices.push_back(iTheta*numPhis + ((iPhi + 1)%numPhis));
                indices.push_back(iTheta*numPhis + iPhi);
                
                indices.push_back(((iTheta+1)%numThetas)*numPhis + iPhi);
                indices.push_back(((iTheta+1)%numThetas)*numPhis + ((iPhi + 1)%numPhis));
                indices.push_back(iTheta*numPhis + ((iPhi + 1)%numPhis));
            }
        }
        
        stitch::PolygonModel *gear1=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        gear1->loadVectorsAndIndices(vectors, indices,
                                     stitch::Vec3(0.0f-3.0f, -1.4f, -7.0f),
                                     1.0,
                                     Vec3(0.0f, 1.0f, 0.0f).normalised(),
                                     true);
        gear1->calculateVertexNormals();
        gear1->generatePolygonObjectsFromVertices();
        gear1->buildBallTree(internalObjectTreeChunkSize);
        ballTree_->addItem(gear1);
        
        stitch::PolygonModel *gear2=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        gear2->loadVectorsAndIndices(vectors, indices,
                                     stitch::Vec3(2.0f-3.0f, -0.7f, -7.0f),
                                     1.0,
                                     Vec3(0.5f, 1.0f, 0.0f).normalised(),
                                     true);
        gear2->calculateVertexNormals();
        gear2->generatePolygonObjectsFromVertices();
        gear2->buildBallTree(internalObjectTreeChunkSize);
        ballTree_->addItem(gear2);
    }
}


//=======================================================================//
void stitch::Scene::createCausticRing(const size_t internalObjectTreeChunkSize, float glossySD)
{
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    {//Ring Object
        const float ringMinZ=-1.90f;
        const float ringMaxZ=0.50f;
        const size_t dimZ=30;
        const float dZ=(ringMaxZ-ringMinZ) / (dimZ-1);
        const float radius=3.0f;
        
        const size_t dimTheta=(2.0f*((float)M_PI))/(dZ/radius)+1.5f;
        const float dTheta=(2.0f*((float)M_PI)) / (dimTheta-1);
        
        stitch::PolygonModel *ringModel=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        std::vector<stitch::Vec3> vectors;
        std::vector<size_t> indices;
        stitch::Vec3 centre(0.0f, 0.0f, 0.0f);
        const float scale=1.0f;
        
        for (size_t indexZ=0; indexZ<dimZ; ++indexZ)
        {
            for (size_t indexTheta=0; indexTheta<(dimTheta-1); ++indexTheta)
            {
                float theta=indexTheta * dTheta + 0.00f;
                float x=cos(theta);
                float y=sin(theta);
                float z=indexZ*dZ + ringMinZ;
                
#ifdef USE_CXX11
                vectors.emplace_back(radius*x, z, radius*y);
#else
                vectors.push_back(stitch::Vec3(radius*x, z, radius*y));
#endif
            }
        }
        
        for (size_t indexZ=0; indexZ<(dimZ-1); ++indexZ)
        {
            for (size_t indexTheta=0; indexTheta<dimTheta; ++indexTheta)
            {
                size_t index0=(indexTheta+0)%(dimTheta-1) + (indexZ+0)*(dimTheta-1);
                size_t index1=(indexTheta+1)%(dimTheta-1) + (indexZ+0)*(dimTheta-1);
                size_t index2=(indexTheta+1)%(dimTheta-1) + (indexZ+1)*(dimTheta-1);
                size_t index3=(indexTheta+0)%(dimTheta-1) + (indexZ+1)*(dimTheta-1);
                
                indices.push_back(index0);
                indices.push_back(index1);
                indices.push_back(index2);
                
                indices.push_back(index0);
                indices.push_back(index2);
                indices.push_back(index3);
            }
        }
        
        ringModel->loadVectorsAndIndices(vectors,
                                         indices,
                                         centre,
                                         scale,
                                         Vec3(0.0f, 0.0f, 1.0f),
                                         true);
        
        ringModel->calculateVertexNormals();
        
        ringModel->generatePolygonObjectsFromVertices();
        ringModel->buildBallTree(20);
        ballTree_->addItem(ringModel);
    }
    
    {//floor
        stitch::Brush *brush=new stitch::Brush(new stitch::PhongMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f), stitch::Colour_t(0.0f, 0.0f, 0.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), 10.0f, "Data/wood2.jpg"));
        //stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        //stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), 0.1));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 5.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
}


//=======================================================================//
void stitch::Scene::createCausticBunny(const size_t internalObjectTreeChunkSize, float glossySD)
{
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    {//Load PLY file.
        stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        
        polygonModel->loadPLYVertices("Data/bunny.ply", stitch::Vec3(0.0f, -2.75f, -2.0f), 20.0, false);
        polygonModel->calculateVertexNormals();
        
        polygonModel->generatePolygonObjectsFromVertices();
        polygonModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(polygonModel);
    }
    
    {//floor
        stitch::Brush *brush=new stitch::Brush(new stitch::PhongMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f), stitch::Colour_t(0.0f, 0.0f, 0.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), 10.0f, "Data/wood2.jpg"));
        //stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        //stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), 0.1));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 5.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
}

//=======================================================================//
void stitch::Scene::createCausticDragon(const size_t internalObjectTreeChunkSize, float glossySD)
{
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    {//Load PLY file.
        stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        
        polygonModel->loadPLYVertices("Data/dragon.ply", stitch::Vec3(0.0f, -2.75f, -2.0f), 20.0, false);
        polygonModel->calculateVertexNormals();
        
        polygonModel->generatePolygonObjectsFromVertices();
        polygonModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(polygonModel);
    }
    
    {//floor
        stitch::Brush *brush=new stitch::Brush(new stitch::PhongMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f), stitch::Colour_t(0.0f, 0.0f, 0.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), 10.0f, "Data/wood2.jpg"));
        //stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        //stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), 0.1));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 5.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 5.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
}

//=======================================================================//
void stitch::Scene::createCausticGear(const size_t internalObjectTreeChunkSize, float glossySD)
{
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    
    {
        std::vector<stitch::Vec3> vectors;
        std::vector<size_t> indices;
        
        const size_t numThetas=360;
        const float dTheta=2.0f*((float)M_PI) / numThetas;
        const float oRadius=2.0f;
        const float iRadius=1.0f;
        
        const size_t numPhis=180;
        const float dPhi=2.0f*((float)M_PI) / numPhis;
        
        for (size_t iTheta=0; iTheta<numThetas; ++iTheta)
        {
            const float u=cosf(iTheta * dTheta);
            const float v=sinf(iTheta * dTheta);
            
            const stitch::Vec3 uv(u, v, 0.0f);
            const stitch::Vec3 uvOrth(v, -u, 0.0f);
            const stitch::Vec3 uvUp=uv^uvOrth;
            
            const float dr=powf(sinf(iTheta * dTheta * 10.0f)*0.5f+0.5f, 2.0)*0.25f;
            
            const stitch::Vec3 c=uv * ((oRadius+dr+iRadius)*0.5f);
            
            for (size_t iPhi=0; iPhi<numPhis; ++iPhi)
            {
                const float s=cosf(iPhi*dPhi) > 0.0f ? powf(fabsf(cosf(iPhi*dPhi)), 0.25f) : -powf(fabsf(cosf(iPhi*dPhi)), 0.25f);
                const float t=sinf(iPhi*dPhi);
                
                const stitch::Vec3 p = c + (uv*s)*((oRadius+dr-iRadius)*0.5f ) + (uvUp*t)*((oRadius-iRadius)*0.5f );
                
                vectors.push_back(p);
            }
        }
        
        for (size_t iTheta=0; iTheta<numThetas; ++iTheta)
        {
            for (size_t iPhi=0; iPhi<numPhis; ++iPhi)
            {
                indices.push_back(((iTheta+1)%numThetas)*numPhis + iPhi);
                indices.push_back(iTheta*numPhis + ((iPhi + 1)%numPhis));
                indices.push_back(iTheta*numPhis + iPhi);
                
                indices.push_back(((iTheta+1)%numThetas)*numPhis + iPhi);
                indices.push_back(((iTheta+1)%numThetas)*numPhis + ((iPhi + 1)%numPhis));
                indices.push_back(iTheta*numPhis + ((iPhi + 1)%numPhis));
            }
        }
        
        stitch::PolygonModel *gear1=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        gear1->loadVectorsAndIndices(vectors, indices,
                                            stitch::Vec3(0.0f, -1.4f, 0.0f),
                                            1.0,
                                            Vec3(0.0f, 1.0f, 0.0f).normalised(),
                                            true);
        gear1->calculateVertexNormals();
        gear1->generatePolygonObjectsFromVertices();
        gear1->buildBallTree(internalObjectTreeChunkSize);
        ballTree_->addItem(gear1);

        stitch::PolygonModel *gear2=new stitch::PolygonModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
        gear2->loadVectorsAndIndices(vectors, indices,
                                     stitch::Vec3(2.0f, -0.7f, 0.0f),
                                     1.0,
                                     Vec3(0.5f, 1.0f, 0.0f).normalised(),
                                     true);
        gear2->calculateVertexNormals();
        gear2->generatePolygonObjectsFromVertices();
        gear2->buildBallTree(internalObjectTreeChunkSize);
        ballTree_->addItem(gear2);
    }
    
    {//floor
        stitch::Brush *brush=new stitch::Brush(new stitch::PhongMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f), stitch::Colour_t(0.0f, 0.0f, 0.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), 10.0f, "Data/wood2.jpg"));
        //stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        //stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), 0.1));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//ceiling
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), -12.5f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//middle
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 0.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 0.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//right
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.09804f, 0.09804f, 0.94902f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 10.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//back
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.94902f, 0.94902f, 0.09804)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {//middle, back
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 0.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), -10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 10.1f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    {
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.7f, 0.7f, 0.7f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), 12.6f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 0.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), -20.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 20.1f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    
}

//=======================================================================//
void stitch::Scene::createCGF2010(const size_t internalObjectTreeChunkSize, float glossySD)
{
    /*
     {//Ring Object
     float ringMinY=-1.90f;
     float ringMaxY=-0.50f;
     float ringDY=(ringMaxY-ringMinY)/10;
     float radiusInside=3.0f;
     float radiusOutside=3.05f;
     size_t numFaces=100;//16 * 32;
     
     stitch::BrushModel *brushModel=new stitch::BrushModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
     
     for (float ringY=ringMinY; ringY<ringMaxY; ringY+=ringDY)
     {
     
     float dTheta=(2.0f*((float)M_PI)) / numFaces;
     
     for (size_t faceNum=0; faceNum<numFaces; ++faceNum)
     {
     stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
     
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), (ringY+ringDY)), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), -ringY), true));
     
     float theta=faceNum * dTheta + 0.00f;
     
     float xLeft=cos(theta);
     float yLeft=sin(theta);
     
     float x=cos(theta+dTheta*0.5f);
     float y=sin(theta+dTheta*0.5f);
     
     float xRight=cos(theta+dTheta);
     float yRight=sin(theta+dTheta);
     
     stitch::Vec3 leftNormal(yLeft, 0.0f, -xLeft);
     stitch::Vec3 normal(x, 0.0f, y);
     stitch::Vec3 rightNormal(-yRight, 0.0f, xRight);
     
     brush->addFace(stitch::BrushFace(stitch::Plane(normal, radiusOutside), false));
     brush->addFace(stitch::BrushFace(stitch::Plane(normal*(-1.0f), -radiusInside), false));
     
     brush->addFace(stitch::BrushFace(stitch::Plane(leftNormal, 0.0f), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(rightNormal, 0.0f), true));
     
     brush->updateLinesVerticesAndBoundingVolume();
     brush->optimiseFaceOrder();
     
     
     brushModel->addBrush(brush);
     }
     
     }
     
     brushModel->updateVertexNormalsToSmooth();
     
     brushModel->buildObjectTree(internalObjectTreeChunkSize);
     
     objectTree_->addObject(brushModel);
     }
     */
#ifdef USE_OSG
    {//stained glass...
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(std::string("Data/stained_1.png"));
        
        if (image.get()==0)
        {//Image not found.
            std::cout << " Image for stained glass not found!\n";
            std::cout.flush();
            exit(1);
        }
        
        int imageWidth=image->s();
        int imageHeight=image->t();
        osg::Vec4 colour(1.0, 0.3, 1.0, 0.5);
        
        float scale=0.5f*1.0f;
        
        stitch::BrushModel *brushModel=new stitch::BrushModel(new stitch::GlossyMaterial(stitch::Colour_t(colour._v[0], colour._v[1], colour._v[2]), glossySD));
        
        for (int t=0; t<imageHeight; ++t)
        {
            for (int s=0; s<imageWidth; ++s)
            {
                colour=image->getColor(s,t);
                
                stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(stitch::Colour_t(colour._v[0], colour._v[1], colour._v[2]), glossySD));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), -7.0f), false));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 7.1f), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f),   ((t+1)*scale - 1.9f)), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), -((t)*scale - 1.9f)), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f),   ((s+1)*scale + 8.5f)), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), -((s)*scale + 8.5f)), true));
                
                brush->updateLinesVerticesAndBoundingVolume(false);
                brush->optimiseFaceOrder();
                
                brushModel->addBrush(brush);
            }
        }
        
        //brushModel->updateVertexNormalsToSmooth();
        
        brushModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(brushModel);
    }
#endif// USE_OSG
    
    
    /*
     {//water surface.
     const ssize_t imageWidth=32;
     const ssize_t imageHeight=32;
     
     float scale=5.0f/32;
     float n=1.5f;
     
     stitch::BrushModel *brushModel=new stitch::BrushModel(new stitch::GlossyMaterial(stitch::Colour_t(0.6f, 0.8f, 0.9f), glossySD));
     
     for (ssize_t t=-(imageHeight>>1); t<(imageHeight>>1); ++t)
     {
     for (ssize_t s=-(imageWidth>>1); s<(imageWidth>>1); ++s)
     {
     stitch::WaterSurfaceNormalDescriptor normalDescriptor(5.0, 0.03);
     
     //Water vertices.
     stitch::Vec3 v0=normalDescriptor.getSurface(stitch::Vec3((s+imageWidth*1.11)*scale,   0.0f, (t+imageHeight*2.25)*scale));
     stitch::Vec3 v1=normalDescriptor.getSurface(stitch::Vec3((s+1+imageWidth*1.11)*scale, 0.0f, (t+imageHeight*2.25)*scale));
     stitch::Vec3 v2=normalDescriptor.getSurface(stitch::Vec3((s+1+imageWidth*1.11)*scale, 0.0f, (t+1+imageHeight*2.25)*scale));
     stitch::Vec3 v3=normalDescriptor.getSurface(stitch::Vec3((s+imageWidth*1.11)*scale,   0.0f, (t+1+imageHeight*2.25)*scale));
     
     stitch::Vec3 v0_b=v0+stitch::Vec3(0.0f,0.01f,0.0f);
     stitch::Vec3 v1_b=v1+stitch::Vec3(0.0f,0.01f,0.0f);
     stitch::Vec3 v2_b=v2+stitch::Vec3(0.0f,0.01f,0.0f);
     stitch::Vec3 v3_b=v3+stitch::Vec3(0.0f,0.01f,0.0f);
     
     {
     stitch::Brush *brush=new stitch::Brush(new stitch::GlossyTransFacetMaterial(stitch::Colour_t(1.0, 1.0, 1.0), 0.1, n));
     
     brush->addFace(stitch::BrushFace(stitch::Plane(v0, v1, v2), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(v2_b, v1_b, v0_b), false));
     brush->addFace(stitch::BrushFace(stitch::Plane(v0_b, v1_b, v1), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(v1_b, v2_b, v2), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(v2_b, v0_b, v0), true));
     
     brush->updateLinesVerticesAndBoundingVolume();
     brush->optimiseFaceOrder();
     
     brushModel->addBrush(brush);
     }
     
     {
     stitch::Brush *brush=new stitch::Brush(new stitch::GlossyTransFacetMaterial(stitch::Colour_t(1.0, 1.0, 1.0), 0.1, n));
     
     brush->addFace(stitch::BrushFace(stitch::Plane(v2, v3, v0), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(v0_b, v3_b, v2_b), false));
     brush->addFace(stitch::BrushFace(stitch::Plane(v2_b, v3_b, v3), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(v3_b, v0_b, v0), true));
     brush->addFace(stitch::BrushFace(stitch::Plane(v0_b, v2_b, v2), true));
     
     brush->updateLinesVerticesAndBoundingVolume();
     brush->optimiseFaceOrder();
     
     brushModel->addBrush(brush);
     }
     
     }
     }
     
     brushModel->updateVertexNormalsToSmooth();
     
     brushModel->buildObjectTree(internalObjectTreeChunkSize);
     
     objectTree_->addObject(brushModel);
     }
     */
    
    //=================================
    //=================================
    //=================================
    
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    {//Add floor to root of brush tree.
        //stitch::Brush *brush=new stitch::Brush(new stitch::PhongMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f), stitch::Colour_t(0.0f, 0.0f, 0.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), 10.0f, "Data/wood2.jpg"));
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.8f, 0.8f, 0.8f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 15.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 5.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
    
    /*
     {
     stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::DiffuseMaterial(stitch::Colour_t(0.6, 0.6, 0.6)));
     
     polygonModel->loadIcosahedron(stitch::Vec3(-5.0f, 1.0f, 0.0f), 1.0f);
     polygonModel->refineVertexMesh(3);
     
     polygonModel->generatePolygonObjectsFromVertices();
     polygonModel->buildObjectTree(internalObjectTreeChunkSize);
     
     objectTree_->addObject(polygonModel);
     
     }
     */
    /*
     {//Load PLY file.
     
     //polygonModel->loadPLY("Data/bunny.ply", stitch::Vec3(4.0f, -1.3f, 7.5f), 20.0, false);
     polygonModel->loadPLYVertices("Data/bunny.ply", stitch::Vec3(4.0f, 0.0f, 7.5f), 20.0, false);
     polygonModel->updateSharedVertexNormalsToSmooth();
     
     polygonModel->generatePolygonObjectsFromVertices();
     polygonModel->refinePolygonObjects(1);
     polygonModel->buildObjectTree(internalObjectTreeChunkSize);
     
     objectTree_->addObject(polygonModel);
     }
     
     {//Load PLY file.
     stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::DiffuseMaterial(stitch::Colour_t(0.6, 0.6, 0.6)));
     
     polygonModel->loadPLYVertices("Data/bunny.ply", stitch::Vec3(-3.0f, 0.0f, 7.5f), 20.0, false);
     polygonModel->updateSharedVertexNormalsToSmooth();
     
     polygonModel->generatePolygonObjectsFromVertices();
     polygonModel->buildObjectTree(internalObjectTreeChunkSize);
     
     objectTree_->addObject(polygonModel);
     }
     */
    
    /*
     {
     stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.9, 0.8, 0.5)));
     
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 1.0, 0.0), -0.5f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, -1.0, 0.0), -(-2.0f)),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0, 0.0, 0.0), 8.5f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0, 0.0, 0.0), -1.5f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 0.0, 1.0), 8.7f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 0.0, -1.0), -6.5f),false));
     
     brush->updateLinesVerticesAndBoundingVolume();
     brush->optimiseFaceOrder();
     
     objectTree_->addObject(brush);
     }
     {
     stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.9, 0.8, 0.5)));
     
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 1.0, 0.0), -0.5f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, -1.0, 0.0), -(-2.0f)),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0, 0.0, 0.0), 3.1f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0, 0.0, 0.0), -1.5f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 0.0, 1.0), 14.0f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 0.0, -1.0), -6.5f),false));
     
     brush->updateLinesVerticesAndBoundingVolume();
     brush->optimiseFaceOrder();
     
     objectTree_->addObject(brush);
     }
     {
     stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.9, 0.8, 0.5)));
     
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 1.0, 0.0), -0.5f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, -1.0, 0.0), -(-2.0f)),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0, 0.0, 0.0), 9.6f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0, 0.0, 0.0), -8.0f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 0.0, 1.0), 14.0f),false));
     brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0, 0.0, -1.0), -6.5f),false));
     
     brush->updateLinesVerticesAndBoundingVolume();
     brush->optimiseFaceOrder();
     
     objectTree_->addObject(brush);
     }
     */
}


//=======================================================================//
void stitch::Scene::createMultiBounce1(const size_t internalObjectTreeChunkSize, float glossySD)
{
    
    {//stained glass...
#ifdef USE_OSG
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(std::string("Data/stained_1.png"));
        
        if (image.get()==0)
        {//Image not found.
            std::cout << " Image for stained glass not found!\n";
            std::cout.flush();
            exit(1);
        }
        
        int imageWidth=3;//image->s();
        int imageHeight=3;//image->t();
#else
        int imageWidth=3;
        int imageHeight=3;
#endif// USE_OSG
        
        stitch::Colour_t colour(1.0, 1.0, 1.0);
        
        float scale=1.333f*1.0f;
        
        stitch::BrushModel *brushModel=new stitch::BrushModel(new stitch::GlossyMaterial(colour, glossySD));
        
        for (int t=0; t<imageHeight; ++t)
        {
            for (int s=0; s<imageWidth; ++s)
            {
#ifdef USE_OSG
                osg::Vec4 osgColour=image->getColor(s,t);
                colour=stitch::Colour_t(osgColour._v[0], osgColour._v[1], osgColour._v[2])*0.66f + stitch::Colour_t(1.0f, 1.0f, 1.0f)*0.33f;
                colour*=0.2f;
#else
#endif// USE_OSG
                
                stitch::Brush *brush=new stitch::Brush(new stitch::GlossyMaterial(colour, glossySD));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), -4.0f), false));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 4.1f), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f),   ((t+1)*scale - 1.9f)), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), -((t)*scale - 1.9f)), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f),   ((s+1)*scale + 6.5f)), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), -((s)*scale + 6.5f)), true));
                
                brush->updateLinesVerticesAndBoundingVolume(false);
                brush->optimiseFaceOrder();
                
                brushModel->addBrush(brush);
            }
        }
        
        //brushModel->updateVertexNormalsToSmooth();
        
        brushModel->buildBallTree(internalObjectTreeChunkSize);
        
        //Test copy of brush model and internal object tree.
        stitch::BrushModel *brushModelCopy=new stitch::BrushModel(*brushModel);
        delete brushModel;
        brushModel=0;
        
        ballTree_->addItem(brushModelCopy);
    }
    
    
    {//stained glass 2...
#ifdef USE_OSG
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(std::string("Data/stained_1.png"));
        
        if (image.get()==0)
        {//Image not found.
            std::cout << " Image for stained glass not found!\n";
            std::cout.flush();
            exit(1);
        }
        
        int imageWidth=3;//image->s();
        int imageHeight=3;//image->t();
#else
        int imageWidth=3;
        int imageHeight=3;
#endif// USE_OSG
        
        stitch::Colour_t colour(1.0, 1.0, 1.0);
        
        float scale=1.3333f*1.0f;
        
        stitch::BrushModel *brushModel=new stitch::BrushModel(new stitch::DiffuseMaterial(colour));
        
        for (int t=0; t<imageHeight; ++t)
        {
            for (int s=0; s<imageWidth; ++s)
            {
#ifdef USE_OSG
                osg::Vec4 osgColour=image->getColor(s,t);
                colour=stitch::Colour_t(osgColour._v[0], osgColour._v[1], osgColour._v[2])*0.66f + stitch::Colour_t(1.0f, 1.0f, 1.0f)*0.33f;
#else
#endif// USE_OSG
                
                stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(colour));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), -4.0f), false));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 4.1f), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f),   ((t+1)*scale - 1.9f)), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), -((t)*scale - 1.9f)), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f),   ((s+1)*scale - 0.5f)), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), -((s)*scale - 0.5f)), true));
                
                brush->updateLinesVerticesAndBoundingVolume(false);
                brush->optimiseFaceOrder();
                
                brushModel->addBrush(brush);
            }
        }
        
        //brushModel->updateVertexNormalsToSmooth();
        
        brushModel->buildBallTree(internalObjectTreeChunkSize);
        
        //Test copy of brush model and internal object tree.
        stitch::BrushModel *brushModelCopy=new stitch::BrushModel(*brushModel);
        delete brushModel;
        brushModel=0;
        
        ballTree_->addItem(brushModelCopy);
    }
    
    
    /*
     stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::DiffuseMaterial(stitch::Colour_t(0.6, 0.6, 0.6)));
     polygonModel->loadOBJVertices("Data/teapot.obj", stitch::Vec3(0.0f, -1.0f, 9.0f), 0.1, false);
     polygonModel->calculateVertexNormals();
     polygonModel->generatePolygonObjectsFromVertices();
     polygonModel->buildBallTree(internalObjectTreeChunkSize);
     ballTree_->addItem(polygonModel);
     */
    //=================================
    //=================================
    //=================================
    
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    {//Add floor to root of brush tree.
        stitch::Brush *brush=new stitch::Brush(new stitch::PhongMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), 60.0f, "Data/wood2.jpg"));
        //stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(0.3f, 0.3f, 0.3f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 15.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 5.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
}

//=======================================================================//
void stitch::Scene::createReport1(const size_t internalObjectTreeChunkSize, float glossySD)
{
    
#ifdef USE_OSG
    {//stained glass...
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(std::string("Data/stained_2.png"));
        
        if (image.get()==0)
        {//Image not found.
            std::cout << " Image for stained glass not found!\n";
            std::cout.flush();
            exit(1);
        }
        
        int imageWidth=1;//image->s();
        int imageHeight=1;//image->t();
        int s0=0;
        int t0=5;
        osg::Vec4 colour;
        
        float scale=1.0f;
        
        stitch::BrushModel *brushModel=new stitch::BrushModel(new stitch::GlossyMaterial(stitch::Colour_t(1.0f/*colour._v[0]*/, 1.0f/*colour._v[1]*/, 1.0f/*colour._v[2]*/), glossySD));
        
        
        for (int t=0; t<imageHeight; ++t)
        {
            for (int s=0; s<imageWidth; ++s)
            {
                colour=image->getColor(s,t);
                
                stitch::Brush *brush=new stitch::Brush(
                                                       new stitch::GlossyMaterial(
                                                                                  /*stitch::Colour_t(colour._v[0], colour._v[1], colour._v[2])*/stitch::Colour_t(1.0f, 1.0f, 1.0f), glossySD));
                //new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f)));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), -7.0f), false));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 7.1f), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), (t+1l+t0)*scale - 1.9f), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), -(t+t0)*scale + 1.9f), true));
                
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), (s+1l+s0)*scale + 8.5f), true));
                brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), -(s+s0)*scale - 8.5f), true));
                
                brush->updateLinesVerticesAndBoundingVolume(false);
                brush->optimiseFaceOrder();
                
                brushModel->addBrush(brush);
            }
        }
        
        //brushModel->updateVertexNormalsToSmooth();
        
        brushModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(brushModel);
    }
#endif// USE_OSG
    
    
    //=================================
    //=================================
    //=================================
    
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
    {//Add floor to root of brush tree.
        //stitch::Brush *brush=new stitch::Brush(new stitch::PhongMaterial(stitch::Colour_t(0.9f, 0.9f, 0.9f), stitch::Colour_t(0.0f, 0.0f, 0.0f), stitch::Colour_t(0.0f, 0.0f, 0.0f), 10.0f, "Data/wood2.jpg"));
        stitch::Brush *brush=new stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f)));
        
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 1.0f, 0.0f), -2.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, -1.0f, 0.0f), 2.1f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(-1.0f, 0.0f, 0.0f), 10.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, 1.0f), 15.0f),false));
        brush->addFace(stitch::BrushFace(stitch::Plane(stitch::Vec3(0.0f, 0.0f, -1.0f), 5.0f),false));
        
        brush->updateLinesVerticesAndBoundingVolume(false);
        brush->optimiseFaceOrder();
        
        ballTree_->addItem(brush);
    }
    
}


//=======================================================================//
void stitch::Scene::createSponza(const size_t internalObjectTreeChunkSize, float glossySD)
{
    
    {
        stitch::PolygonModel *polygonModel=new stitch::PolygonModel(new stitch::DiffuseMaterial(stitch::Colour_t(0.9, 0.9, 0.9)));
        
        polygonModel->loadOBJVertices("Data/crytek/sponza.obj", stitch::Vec3(0.0f, 0.0f, 0.0f), 20.0, false);
        polygonModel->calculateVertexNormals();
        
        polygonModel->generatePolygonObjectsFromVertices();
        polygonModel->buildBallTree(internalObjectTreeChunkSize);
        
        ballTree_->addItem(polygonModel);
    }
    
    
    {//Add light to root of object tree.
        ballTree_->addItem(light_);
    }
    
}
