/*
 * $Id: BrushModel.cpp 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Brush.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
 *  Copyright $Date: 2015-08-07 16:57:10 +0200 (Fri, 07 Aug 2015) $ Bernardt Duvenhage. All rights reserved.
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

#include "BrushModel.h"
//#include "Beam.h"
#include "Math/MathUtil.h"
#include "Math/VecN.h"

#include "Materials/GlossyMaterial.h"
#include "Materials/GlossyTrnsMaterial.h"

#include "OSGUtils/StitchOSG.h"


//=======================================================================//
stitch::Brush::Brush(Material * const pMaterial) :
Object(pMaterial)
{}

//=======================================================================//
stitch::Brush::Brush(Material * const pMaterial, std::vector<stitch::Vec3> const &vertexCloud, const size_t minNumBrushFaces) :
Object(pMaterial)
{
    const size_t numVertices=vertexCloud.size();
    
    stitch::Vec3 centre;//zeroed in Vec3 constructor.
    for (size_t vertexNum=0; vertexNum<numVertices; ++vertexNum)
    {
        centre+=vertexCloud[vertexNum];
    }
    centre/=numVertices;
    
    size_t actualNumBrushFaces=0;
    size_t actualNumBrushEdges=0;
    size_t actualNumBrushVertices=0;
    
    stitch::Vec3::equidistantVectors_IcosahedronBased_GetDetails_minNumBins(minNumBrushFaces, actualNumBrushVertices, actualNumBrushEdges, actualNumBrushFaces);
    
    std::vector<stitch::Vec3> eqVectors;
    eqVectors.reserve(actualNumBrushVertices);
    
    std::vector<size_t> eqBinIndices;
    eqBinIndices.reserve(actualNumBrushFaces * 3);
    
    stitch::Vec3::equidistantVectors_IcosahedronBased(actualNumBrushVertices, eqVectors, eqBinIndices);
    //OR
    //    {
    //        eqVectors.push_back(stitch::Vec3(1.0f, 0.0f, 0.0f));
    //        eqVectors.push_back(stitch::Vec3(-1.0f, 0.0f, 0.0f));
    //        eqVectors.push_back(stitch::Vec3(0.0f, 1.0f, 0.0f));
    //        eqVectors.push_back(stitch::Vec3(0.0f, -1.0f, 0.0f));
    //        eqVectors.push_back(stitch::Vec3(0.0f, 0.0f, 1.0f));
    //        eqVectors.push_back(stitch::Vec3(0.0f, 0.0f, -1.0f));
    //
    //        eqVectors.push_back(stitch::Vec3(1.0f, 1.0f, 0.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(-1.0f, -1.0f, 0.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(-1.0f, 1.0f, 0.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(1.0f, -1.0f, 0.0f).normalised());
    //
    //        eqVectors.push_back(stitch::Vec3(1.0f, 0.0f, 1.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(-1.0f, 0.0f, -1.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(-1.0f, 0.0f, 1.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(1.0f, 0.0f, -1.0f).normalised());
    //
    //        eqVectors.push_back(stitch::Vec3(0.0f, 1.0f, 1.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(0.0f, -1.0f, -1.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(0.0f, -1.0f, 1.0f).normalised());
    //        eqVectors.push_back(stitch::Vec3(0.0f, 1.0f, -1.0f).normalised());
    //    }
    
    
    
    const size_t numEqVectors=eqVectors.size();
    faceVector_.reserve(numEqVectors);
    for (size_t vectorNum=0; vectorNum<numEqVectors; ++vectorNum)
    {
        float maxD=0.0f;
        
        for (size_t vertexNum=0; vertexNum<numVertices; ++vertexNum)
        {
            float d=(vertexCloud[vertexNum]-centre) * eqVectors[vectorNum];
            
            if (d>maxD) maxD=d;
        }
        
        addFace(stitch::BrushFace(stitch::Plane(eqVectors[vectorNum], maxD+centre*eqVectors[vectorNum]), false));
    }
}

//=======================================================================//
stitch::Brush::Brush(const Brush &lValue) :
Object(lValue),
faceVector_(lValue.faceVector_)
{}

#ifdef USE_CXX11
stitch::Brush::Brush(Brush &&rValue) noexcept:
Object(rValue),
faceVector_(std::move(rValue.faceVector_))
{}
#endif// USE_CXX11

//=======================================================================//
stitch::Brush & stitch::Brush::operator=(const Brush &lValue)
{
    Object::operator=(lValue);
    faceVector_=lValue.faceVector_;
    
    return *this;
}

#ifdef USE_CXX11
stitch::Brush & stitch::Brush::operator=(Brush &&rValue) noexcept
{
    Object::operator=(rValue);
    faceVector_=std::move(rValue.faceVector_);
    
    return *this;
}
#endif// USE_CXX11

//=======================================================================//
stitch::Brush::~Brush()
{
}

//=======================================================================//
void stitch::Brush::addFace(const BrushFace &face)
{
    faceVector_.push_back(face);
}

//=======================================================================//
void stitch::Brush::optimiseFaceOrder()
{
    const size_t numFaces=faceVector_.size();
    
    std::vector<stitch::Vec3> axisVec;
    axisVec.push_back(stitch::Vec3(1.0f, 0.0f, 0.0f));
    axisVec.push_back(stitch::Vec3(-1.0f, 0.0f, 0.0f));
    axisVec.push_back(stitch::Vec3(0.0f, 1.0f, 0.0f));
    axisVec.push_back(stitch::Vec3(0.0f, -1.0f, 0.0f));
    axisVec.push_back(stitch::Vec3(0.0f, 0.0f, 1.0f));
    axisVec.push_back(stitch::Vec3(0.0f, 0.0f, -1.0f));
    
    //Arrange faces by x,y,z normal directions.
    for (size_t i=0; i<numFaces; ++i)
    {
        size_t optimalFace=i;
        float optimalFaceCosDist=axisVec[i % (axisVec.size())] * faceVector_[optimalFace].plane_.normal_;
        
        for (size_t j=i+1; j<numFaces; ++j)
        {
            float faceCosDist=axisVec[i % (axisVec.size())] * faceVector_[j].plane_.normal_;
            
            if (faceCosDist>optimalFaceCosDist)
            {
                optimalFaceCosDist=faceCosDist;
                optimalFace=j;
            }
        }
        
        std::swap(faceVector_[i],faceVector_[optimalFace]);
    }
}

//=======================================================================//
void stitch::Brush::updateLinesVerticesAndBoundingVolume(const bool trashLineGeometry)
{
    //!@todo Need to replace with algorithm that has better time complexity if possible.
    
    std::vector<BrushFace>::iterator faceIter;
    
    //=== Update lines : BEGIN ===//
    faceIter=faceVector_.begin();
    for (; faceIter!=faceVector_.end(); ++faceIter)
    {//Iterate over all planes of brush.
        faceIter->lineVector_.clear();
        std::vector<BrushFace>::const_iterator intersectFaceIter=faceVector_.begin();
        
        //Find all intersection lines between *planeIter and the other planes of the brush.
        for (; intersectFaceIter!=faceVector_.end(); ++intersectFaceIter)
        {//Iterate over all other (intersector) planes of brush.
            
            if (intersectFaceIter!=faceIter)
            {//This plane may be treated as an intersector. Find line intersection.
                Line line((*faceIter).plane_, (*intersectFaceIter).plane_);
                
                if (line.isValid())
                {
                    //Trim the line.
                    std::vector<BrushFace>::const_iterator trimFaceIter=faceVector_.begin();
                    for (; trimFaceIter!=faceVector_.end(); ++trimFaceIter)
                    {
                        if ((trimFaceIter!=faceIter)&&(trimFaceIter!=intersectFaceIter))
                        {//Trim the line.
                            line.trim((*trimFaceIter).plane_);
                            
                            if (line.getTStart()>=line.getTEnd())
                            {//line has been trimmed away!
                                break;//from for-loop.
                            }
                        }
                    }
                    
                    if (line.getTStart()<(line.getTEnd()-0.001f))//!@todo Change the size/order of this delta to be dependent on brush size.
                    {//Only add line if valid i.e. line segment length is significant.
                        faceIter->lineVector_.push_back(line);
                    }
                }
            }
        }
    }
    //=== Update lines : END ===//
    
    
    //=== Update vertices : BEGIN ===//
    faceIter=faceVector_.begin();
    while (faceIter!=faceVector_.end())
    {//Iterate over all planes of brush.
        //Line segments are already directed counter clock with respect to *planeIter normal.
        //Order lines counter clock.
        //Store brush plane's triangle vertices...
        
        faceIter->vertexCoordVector_.clear();
        //faceIter->vertexNormalVector_.clear();
        
        faceIter->area_=0.0f;
        
        size_t numLines=faceIter->lineVector_.size();
        if (numLines>=3)
        {
            //Find centre of brush face.
            Vec3 vCenter;
            for (size_t lineNum=0; lineNum<numLines; ++lineNum)
            {
                vCenter=vCenter + faceIter->lineVector_[lineNum].getVStart();
            }
            vCenter=vCenter * (1.0f/numLines);
            
            //Selection sort counter clock order of lines.
            for (size_t lineNum=0; lineNum<(numLines-2); ++lineNum)
            {
                size_t nextLineNum=lineNum+1;
                float nextDistSq=(faceIter->lineVector_[nextLineNum].getVStart()-faceIter->lineVector_[lineNum].getVEnd()).lengthSq();
                
                for (size_t tmpLineNum=lineNum+2; tmpLineNum<numLines; ++tmpLineNum)
                {
                    float tmpDistSq=(faceIter->lineVector_[tmpLineNum].getVStart()-faceIter->lineVector_[lineNum].getVEnd()).lengthSq();
                    
                    if (tmpDistSq<nextDistSq)
                    {
                        nextLineNum=tmpLineNum;
                        nextDistSq=tmpDistSq;
                    }
                }
                
                //Insertion/swap operation.
                std::swap(faceIter->lineVector_[lineNum+1], faceIter->lineVector_[nextLineNum]);
            }
            
            //Store brush face vertices in counter clock wise order. The first vertex is the center vertex!
            faceIter->vertexCoordVector_.push_back(vCenter);//The first vertex is the center vertex!
            //faceIter->vertexNormalVector_.push_back(faceIter->plane_.normal_);//The first vertex is the center vertex!
            for (size_t lineNum=0; lineNum<numLines; ++lineNum)
            {
                faceIter->vertexCoordVector_.push_back(faceIter->lineVector_[lineNum].getVStart());
                //faceIter->vertexNormalVector_.push_back(faceIter->plane_.normal_);
            }
            
            //Calculate the brush face's area.
            Vec3 areaVect;
            size_t numVertices=numLines+1;
            for (size_t vertexNum=2; vertexNum<numVertices; ++vertexNum)//The first vertex is the center vertex!
            {
                areaVect+=stitch::Vec3::cross(faceIter->vertexCoordVector_[vertexNum-1], faceIter->vertexCoordVector_[vertexNum], faceIter->vertexCoordVector_[0]);
            }
            
            faceIter->area_=areaVect.length();
        }
        
        if (faceIter->area_ == 0.0f)
        {//This face is degenerate.
            *faceIter = faceVector_.back();
            faceVector_.pop_back();
        } else
        {
            ++faceIter;
        }
    }
    //=== Update vertices : END ===//
    
    if (trashLineGeometry)
    {
        //Completely clear the line vector to save memory!
        for (auto & brushFace : faceVector_)
        {
            std::vector<stitch::Line>().swap(brushFace.lineVector_);//swap with empty vector.
        }
    }
    
    {//=== Calculate the bounding volume ===//
        centre_.setZeros();
        size_t numVertices=0;
        radiusBV_=0.0f;
        
        std::vector<BrushFace>::const_iterator faceIter=faceVector_.begin();
        for (; faceIter!=faceVector_.end(); ++faceIter)
        {//Iterate over all planes in brush.
            std::vector<Vec3>::const_iterator vertexIter=faceIter->vertexCoordVector_.begin();
            for (; vertexIter!=faceIter->vertexCoordVector_.end(); ++vertexIter)
            {//Iterate over all vertices on the brush face.
                centre_+=(*vertexIter);
                ++numVertices;
            }
        }
        centre_*=1.0f/numVertices;
        
        faceIter=faceVector_.begin();
        for (; faceIter!=faceVector_.end(); ++faceIter)
        {//Iterate over all planes in brush.
            std::vector<Vec3>::const_iterator vertexIter=faceIter->vertexCoordVector_.begin();
            for (; vertexIter!=faceIter->vertexCoordVector_.end(); ++vertexIter)
            {//Iterate over all vertices on the brush face.
                float vertexRadius=((*vertexIter)-centre_).length();
                
                if (vertexRadius>radiusBV_)
                {
                    radiusBV_=vertexRadius;
                }
            }
        }
        
        
        //=== Optimise the BV ===
        for (size_t i=0; i<10; i++)
        {
            stitch::Vec3 c=centre_+stitch::Vec3::randNorm()*(radiusBV_*0.01f);
            
            float r=0.0f;
            faceIter=faceVector_.begin();
            for (; faceIter!=faceVector_.end(); ++faceIter)
            {//Iterate over all planes in brush.
                std::vector<Vec3>::const_iterator vertexIter=faceIter->vertexCoordVector_.begin();
                for (; vertexIter!=faceIter->vertexCoordVector_.end(); ++vertexIter)
                {//Iterate over all vertices on the brush face.
                    float vertexRadius=((*vertexIter)-centre_).length();
                    
                    if (vertexRadius>r)
                    {
                        r=vertexRadius;
                    }
                }
            }
            
            if (r<radiusBV_)
            {
                centre_=c;
                radiusBV_=r;
            }
        }
        //=== ===
        
        
    }
    //====================================//
}


//=======================================================================//
stitch::Brush stitch::Brush::mergeBrush(const Brush &brushA, const Brush &brushB, const bool trashLineGeometry)
{
    Brush brushACopy=brushA;//Make local copy of input brush.
    Brush brushBCopy=brushB;//Make local copy of input brush.
    
    //Make sure the brush vertices are valid.
    brushACopy.updateLinesVerticesAndBoundingVolume(true);
    brushBCopy.updateLinesVerticesAndBoundingVolume(true);
    
    std::vector<stitch::Vec3> vertexCloud;
    
    {//brushACopy vertices to vertexCloud
        std::vector<stitch::BrushFace>::const_iterator faceIter=brushACopy.faceVector_.begin();
        const std::vector<stitch::BrushFace>::const_iterator faceIterEnd=brushACopy.faceVector_.end();
        
        for (; faceIter!=faceIterEnd; ++faceIter)
        {
            const size_t numVertices=faceIter->vertexCoordVector_.size();
            
            for (size_t vertexNum=1; vertexNum<numVertices; ++vertexNum)
            {
                vertexCloud.push_back(faceIter->vertexCoordVector_[vertexNum]);
            }
        }
    }
    
    {//brushBCopy vertices to vertexCloud
        std::vector<stitch::BrushFace>::const_iterator faceIter=brushBCopy.faceVector_.begin();
        const std::vector<stitch::BrushFace>::const_iterator faceIterEnd=brushBCopy.faceVector_.end();
        
        for (; faceIter!=faceIterEnd; ++faceIter)
        {
            const size_t numVertices=faceIter->vertexCoordVector_.size();
            
            for (size_t vertexNum=1; vertexNum<numVertices; ++vertexNum)
            {
                vertexCloud.push_back(faceIter->vertexCoordVector_[vertexNum]);
            }
        }
    }
    
    //Create bush from cloud of vertices
    stitch::Brush brush(brushACopy.pMaterial_->clone(), vertexCloud, 20);
    brush.updateLinesVerticesAndBoundingVolume(trashLineGeometry);
    
    return brush;
}


//=======================================================================//
#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::Brush::constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key) const
{
    Vec3 colour(stitch::Vec3::uniqueValue(key==0 ? ((uintptr_t)this) : key ));
    colour.positivise();
    
    osg::ref_ptr<osg::Group> osgGroup=new osg::Group();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    
    osg::ref_ptr<osg::Vec3Array> osgTriangleVertices=new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> osgTriangleNormals=new osg::Vec3Array();
    
    
    //=== Construct triangle geometry ===
    {
        std::vector<BrushFace>::const_iterator faceIter=faceVector_.begin();
        for (; faceIter!=faceVector_.end(); ++faceIter)
        {//Iterate over all planes in brush.
            std::vector<Vec3>::const_iterator vertexIter=faceIter->vertexCoordVector_.begin();
            //std::vector<Vec3>::const_iterator normalIter=faceIter->vertexNormalVector_.begin();
            
            if (vertexIter!=faceIter->vertexCoordVector_.end())
            {//The plane has vertices...
                osg::Vec3f planeNormal(faceIter->plane_.normal_.x(), faceIter->plane_.normal_.y(), faceIter->plane_.normal_.z());
                
                VecN v(*(vertexIter++));
                osg::Vec3f osgV0(v.x(), v.y(), v.z());
                
                v=*(vertexIter++);
                osg::Vec3f osgV1(v.x(), v.y(), v.z());
                osg::Vec3f osgV2Final=osgV1;
                
                VecN n(faceIter->plane_.normal_);//*(normalIter++);
                osg::Vec3f osgN0(n.x(), n.y(), n.z());
                
                n=faceIter->plane_.normal_;//*(normalIter++);
                osg::Vec3f osgN1(n.x(), n.y(), n.z());
                osg::Vec3f osgN2Final=osgN1;
                
                while (vertexIter!=faceIter->vertexCoordVector_.end())
                {//Iterate over all vertices on the plane.
                    v=*(vertexIter++);
                    osg::Vec3f osgV2(v.x(), v.y(), v.z());
                    
                    n=faceIter->plane_.normal_;//*(normalIter++);
                    osg::Vec3f osgN2(n.x(), n.y(), n.z());
                    
                    if (!wireframe)
                    {
                        osgTriangleVertices->push_back(osgV0);
                    }
                    osgTriangleVertices->push_back(osgV1);
                    osgTriangleVertices->push_back(osgV2);
                    
                    if (!wireframe)
                    {
                        osgTriangleNormals->push_back(osgN0);
                    }
                    osgTriangleNormals->push_back(osgN1);
                    osgTriangleNormals->push_back(osgN2);
                    
                    osgV1=osgV2;
                    osgN1=osgN2;
                }
                
                //Add final/loop triangle.
                if (!wireframe)
                {
                    osgTriangleVertices->push_back(osgV0);
                }
                osgTriangleVertices->push_back(osgV1);
                osgTriangleVertices->push_back(osgV2Final);
                
                if (!wireframe)
                {
                    osgTriangleNormals->push_back(osgN0);
                }
                osgTriangleNormals->push_back(osgN1);
                osgTriangleNormals->push_back(osgN2Final);
            }
        }
        
        osg::ref_ptr<osg::Geometry> osgTriangleGeometry=new osg::Geometry();
        osgTriangleGeometry->setVertexArray(osgTriangleVertices.get());
        osgTriangleGeometry->setNormalArray(osgTriangleNormals.get());
        osgTriangleGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        //osgTriangleGeometry->setUseVertexBufferObjects(true);
        
        if (!wireframe)
        {
            osgTriangleGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,osgTriangleVertices->size()));
        } else
        {
            osgTriangleGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgTriangleVertices->size()));
        }
        
        osg::ref_ptr<osg::StateSet> osgTriangleStateset=osgTriangleGeometry->getOrCreateStateSet();
        osgTriangleStateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
        //osgTriangleStateset->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);//Re-normalise scaled normal vectors.
        osgTriangleStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        //osgTriangleStateset->setMode(GL_LIGHT_MODEL_TWO_SIDE, osg::StateAttribute::ON);
        
        {//Solid or wireframe?
            osg::PolygonMode* pm = dynamic_cast<osg::PolygonMode*>(osgTriangleStateset->getAttribute(osg::StateAttribute::POLYGONMODE));
            if (!pm)
            {
                pm = new osg::PolygonMode;
            }
            
            if (wireframe)
            {
                pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
            } else
            {
                pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
            }
            
            osgTriangleStateset->setAttribute(pm);
        }
        
        osg::Depth* depth = new osg::Depth();
        osgTriangleStateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
        
        osg::Material *material = new osg::Material();
        material->setColorMode(osg::Material::DIFFUSE);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(colour.x(), colour.y(), colour.z(), 1.0));
        material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(pow(colour.x(), 0.1), pow(colour.y(), 0.1), pow(colour.z(), 0.1), 1.0));
        material->setShininess(osg::Material::FRONT_AND_BACK, 10.0);
        osgTriangleStateset->setAttributeAndModes(material, osg::StateAttribute::ON);
        
        osgGeode->addDrawable(osgTriangleGeometry.get());
    }
    //=======
    
    osgGroup->addChild(osgGeode);
    
    if (createOSGLineGeometry)
    {
        osgGroup->addChild(constructOSGLineNode());
    }
    
    if (createOSGNormalGeometry)
    {
        osgGroup->addChild(constructOSGNormalNode());
    }
    
    return osgGroup;
}

//=======================================================================//
osg::ref_ptr<osg::Node> stitch::Brush::constructOSGLineNode() const
{
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    
    osg::ref_ptr<osg::Vec3Array> osgLineVertices=new osg::Vec3Array();
    
    //=== Construct line geometry ===
    {
        std::vector<BrushFace>::const_iterator faceIter=faceVector_.begin();
        for (; faceIter!=faceVector_.end(); ++faceIter)
        {//Iterate over all planes in brush.
            std::vector<Line>::const_iterator lineIter=faceIter->lineVector_.begin();
            
            for (; lineIter!=faceIter->lineVector_.end(); ++lineIter)
            {//Add line to vertexArray.
                Line line=*lineIter;
                if ((line.getTStart()>-((float)FLT_MAX))&&
                    (line.getTEnd()<((float)FLT_MAX)) )
                {
                    VecN start(line.getVStart() + faceIter->plane_.normal_*(radiusBV_*0.01f)); //1% offset of line geometry to just outside brush.
                    VecN end(line.getVEnd() + faceIter->plane_.normal_*(radiusBV_*0.01f));
                    
                    osgLineVertices->push_back(osg::Vec3f(start.x(), start.y(), start.z()));
                    osgLineVertices->push_back(osg::Vec3f(end.x(), end.y(), end.z()));
                }
            }
        }
        
        osg::ref_ptr<osg::Geometry> osgLineGeometry=new osg::Geometry();
        osgLineGeometry->setVertexArray(osgLineVertices.get());
        osgLineGeometry->setUseVertexBufferObjects(true);
        osgLineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgLineVertices->size()));
        
        osg::ref_ptr<osg::StateSet> osgLineStateset=osgLineGeometry->getOrCreateStateSet();
        osgLineStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        osgLineStateset->setMode(GL_BLEND,osg::StateAttribute::OFF);
        
        osgGeode->addDrawable(osgLineGeometry.get());
    }
    //=======
    
    
    return osgGeode;
}

//=======================================================================//
osg::ref_ptr<osg::Node> stitch::Brush::constructOSGNormalNode() const
{
    std::cout << "stitch::Brush::constructOSGNormalNode()...";
    std::cout.flush();
    
    osg::ref_ptr<osg::Geode> osgGeode=new osg::Geode();
    
    osg::ref_ptr<osg::Vec3Array> osgLineVertices=new osg::Vec3Array();
    
    
    float glossySD=0.0f;
    if (pMaterial_->getType()==stitch::Material::GLOSSY_MATERIAL)
    {
        glossySD=((static_cast<stitch::GlossyMaterial *>(pMaterial_))->sd_);
    } else
        if (pMaterial_->getType()==stitch::Material::GLOSSY_TRANS_MATERIAL)
        {
            glossySD=((static_cast<stitch::GlossyTransMaterial *>(pMaterial_))->sd_);
        } else
            if (pMaterial_->getType()==stitch::Material::SPECULAR_MATERIAL)
            {
                glossySD=0.0f;
            }
    
    
    {
        std::vector<BrushFace>::const_iterator faceIter=faceVector_.begin();
        for (; faceIter!=faceVector_.end(); ++faceIter)
        {//Iterate over all planes in brush.
            size_t numBins=glossySD==0.0f ? 1 : 642;
            size_t numSamples=glossySD==0.0f ? 1 : (642*30);
            
            std::vector<stitch::Vec3> binVector;
            std::vector<size_t> binIndices;
            
            //=== Generate vectors that represent bins. ===
            if (numBins>1)
            {
                stitch::Vec3::equidistantVectors_IcosahedronBased(numBins, binVector, binIndices);
            } else
            {//Only one bin when glossySD is 0.0f
                binVector.push_back(faceIter->plane_.normal_);
            }
            
            //=== Generate accumulate samples from normal distribution in bins ===
            if (numBins>1)
            {
                for (size_t sampleNum=0; sampleNum<numSamples; ++sampleNum)
                {
                    stitch::Vec3 randVecLocal=stitch::Vec3::randGaussianLobe(glossySD*0.5);
                    //stitch::Vec3 randVecLocal=stitch::Vec3::randBall();
                    
                    const Vec3 orthVecA=((faceIter->plane_.normal_).orthVec()).normalised();
                    const Vec3 orthVecB=((faceIter->plane_.normal_) ^ orthVecA).normalised();
                    
                    stitch::Vec3 randVec=orthVecA*randVecLocal.x() + orthVecB*randVecLocal.y() + (faceIter->plane_.normal_)*randVecLocal.z();
                    
                    for (size_t binNum=0; binNum<numBins; ++binNum)
                    {
                        float cosTheta=(randVec * binVector[binNum])/(randVec.length() * binVector[binNum].length());
                        
                        float cosToN=powf((cosTheta+1.0f)*0.5f, 50.0f);//The cosine kernel exponent should depend on the number of bins.
                        
                        binVector[binNum]+=binVector[binNum].normalised()*cosToN;
                    }
                }
                
                
                //=== Normalise bin entries to largest bin ===
                float maxBinLength=binVector[0].length()-1.0f;
                for (size_t binNum=1; binNum<numBins; ++binNum)
                {
                    float binLength=binVector[binNum].length()-1.0f;
                    if (binLength>maxBinLength)
                    {
                        maxBinLength=binLength;
                    }
                }
                
                for (size_t binNum=0; binNum<numBins; ++binNum)
                {
                    binVector[binNum]=(binVector[binNum]-binVector[binNum].normalised())/maxBinLength;
                }
                //=== ===
                
            } else
            {
                binVector[0]+=faceIter->plane_.normal_;
            }
            
            
            //=== Construct vector geometry ===
            
            Vec3 start=faceIter->vertexCoordVector_[0];//The first vertex is the centre vertex.
            
            //=== Add vertices for every bin in  all bins ===
            std::vector<stitch::Vec3>::const_iterator binVectorIter=binVector.begin();
            for (; binVectorIter!=binVector.end(); ++binVectorIter)
            {
                Vec3 end=start + (*binVectorIter);
                
                osgLineVertices->push_back(osg::Vec3f(start.x(), start.y(), start.z()));
                osgLineVertices->push_back(osg::Vec3f(end.x(), end.y(), end.z()));
            }
            
            std::vector<size_t>::const_iterator binIndicesIter=binIndices.begin();
            for (; binIndicesIter!=binIndices.end(); binIndicesIter+=3)
            {
                Vec3 v0=start + binVector[*(binIndicesIter+0)];
                Vec3 v1=start + binVector[*(binIndicesIter+1)];
                Vec3 v2=start + binVector[*(binIndicesIter+2)];
                
                osgLineVertices->push_back(osg::Vec3f(v0.x(), v0.y(), v0.z()));
                osgLineVertices->push_back(osg::Vec3f(v1.x(), v1.y(), v1.z()));
                
                osgLineVertices->push_back(osg::Vec3f(v1.x(), v1.y(), v1.z()));
                osgLineVertices->push_back(osg::Vec3f(v2.x(), v2.y(), v2.z()));
                
                osgLineVertices->push_back(osg::Vec3f(v2.x(), v2.y(), v2.z()));
                osgLineVertices->push_back(osg::Vec3f(v0.x(), v0.y(), v0.z()));
            }
            
        }
        
        osg::ref_ptr<osg::Geometry> osgLineGeometry=new osg::Geometry();
        osgLineGeometry->setVertexArray(osgLineVertices.get());
        //osgLineGeometry->setUseVertexBufferObjects(true);
        osgLineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,osgLineVertices->size()));
        
        osg::ref_ptr<osg::StateSet> osgLineStateset=osgLineGeometry->getOrCreateStateSet();
        osgLineStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        osgLineStateset->setMode(GL_BLEND,osg::StateAttribute::OFF);
        
        osgGeode->addDrawable(osgLineGeometry.get());
    }
    //=======
    
    
    std::cout << "done.\n";
    std::cout.flush();
    
    return osgGeode;
}
#endif// USE_OSG

//=======================================================================//
void stitch::Brush::calcIntersection(const Ray &ray, Intersection &intersect) const
{
    //if (BVIntersected(orig, normDir)) This is currently executed by the calling code!
    {
        EntryExit entryExit(-((float)FLT_MAX), ((float)FLT_MAX));
        bool brushIntersected=true;
        
        bool entryHidden=false;
        bool exitHidden=false;
        ssize_t entryFaceNum=-1;
        ssize_t exitFaceNum=-1;
        
        const size_t numFaces=faceVector_.size();
        
        for (size_t faceNum=0; faceNum<numFaces; ++faceNum)
        {
            const Plane *plane=&(faceVector_[faceNum].plane_);
            const bool planeHidden=faceVector_[faceNum].hidden_;
            
            const float cosTheta=ray.direction_*plane->normal_;
            
            if (cosTheta!=0.0f)
            {
                const float dist=(plane->d_-plane->normal_*ray.origin_)/cosTheta;
                
                if (cosTheta<=0.0f)
                {//entry plane...
                    if (dist>entryExit.entryDistance_)
                    {
                        entryExit.entryDistance_=dist;
                        entryExit.entryNormal_=plane->normal_;
                        entryHidden=planeHidden;
                        entryFaceNum=faceNum;
                    }
                } else
                {//exit plane...
                    if (dist<entryExit.exitDistance_)
                    {
                        entryExit.exitDistance_=dist;
                        entryExit.exitNormal_=plane->normal_;
                        exitHidden=planeHidden;
                        exitFaceNum=faceNum;
                    }
                }
                
                if (entryExit.entryDistance_>entryExit.exitDistance_)
                {//No intersection.
                    brushIntersected=false;
                    break;//No intersection! Break from for loop.
                }
            } else
            {//Line is travelling alongside the plane. On which side of the plane?
                if ((ray.origin_*plane->normal_)>plane->d_)
                {//Line is travelling outside of brush. No intersection.
                    brushIntersected=false;
                    break;//No intersection! Break from for loop.
                }
            }
        }
        
        if (brushIntersected)
        {
            entryExit.object_=this;
            ssize_t faceNum=-1;
            const uint32_t incomingObjectID=intersect.itemID_;
            
            if ((entryExit.entryDistance_>0.0f)&&(!entryHidden))
            {
                if (entryExit.entryDistance_<intersect.distance_)
                {
                    intersect.distance_=entryExit.entryDistance_;
                    intersect.normal_=entryExit.entryNormal_;
                    intersect.itemID_=this->itemID_ | ((intersect.normal_*ray.direction_>0.0f)?0:1);//back surface gets even ID, front surface gets odd ID.
                    intersect.itemPtr_=this;
                }
                
                faceNum=entryFaceNum;
            } else
                if ((entryExit.exitDistance_>0.0f)&&(!exitHidden))
                {
                    if (entryExit.exitDistance_<intersect.distance_)
                    {
                        intersect.distance_=entryExit.exitDistance_;
                        intersect.normal_=entryExit.exitNormal_;
                        intersect.itemID_=this->itemID_ | ((intersect.normal_*ray.direction_>0.0f)?0:1);//back surface gets even ID, front surface gets odd ID.
                        intersect.itemPtr_=this;
                    }
                    
                    faceNum=exitFaceNum;
                }
            
            if (intersect.itemID_!=incomingObjectID)
            {
                Vec3 P=ray.origin_;
                P.addScaled(ray.direction_, intersect.distance_);
                //Use the recorded faceNum and worldPos of the intersection to calculate the interpolated normal at the intersection.
                //The face's centre normal has been modified to be the smooth centre normal.
                
                //1) Find the two outside face vertices that along with the centre vertex encircle the intersection worldPos.
                //2) === Find the barycentric coords of the intersection worldPos relative to the three encircling vertices. ===
                //3) === Calculate the interpolated normal based on the barycentric coordinates of the intersection. ===
                
                const BrushFace *brushFace=&(faceVector_[faceNum]);
                const size_t numVertices=brushFace->vertexCoordVector_.size();
                
                
                //===
                Vec3 const * const v0=&(brushFace->vertexCoordVector_[0]); //At centre of brush face.
                
                //Vec3 const * const n0=&(brushFace->vertexNormalVector_[0]);
                Vec3 const * const n0=&(brushFace->plane_.normal_);
                
                
                Vec3 v0P=P;
                v0P-=*v0;
                const float v0PLengthSq=v0P.lengthSq();
                //===
                
                
                //=== 01 ===
                Vec3 const *v1=&(brushFace->vertexCoordVector_[numVertices-1]);
                
                //Vec3 const *n1=&(brushFace->vertexNormalVector_[numVertices-1]);
                Vec3 const *n1=&(brushFace->plane_.normal_);
                
                Vec3 v1P=P;
                v1P-=*v1;
                
                stitch::Vec3 v01_norm_temp=*v1;
                v01_norm_temp-=*v0;
                
                const float v01_length_temp=v01_norm_temp.normalise_rt();
                float A01=0.5f * (sqrtf(v0PLengthSq - powf(v0P*v01_norm_temp,2.0f))) * v01_length_temp;
                //=== ===
                
                
                Vec3 v2P, v02_norm_temp, v12_norm_temp;
                for (size_t vertexNum=1; vertexNum<numVertices; ++vertexNum)
                {
                    //=== 02 ===
                    Vec3 const * const v2=&(brushFace->vertexCoordVector_[vertexNum]);
                    
                    //Vec3 const * const n2=&(brushFace->vertexNormalVector_[vertexNum]);
                    Vec3 const * const n2=&(brushFace->plane_.normal_);
                    
                    v2P=P;
                    v2P-=*v2;
                    
                    v02_norm_temp=*v2;
                    v02_norm_temp-=*v0;
                    
                    const float v02_length_temp=v02_norm_temp.normalise_rt();
                    const float A02=0.5f * (sqrtf(v0PLengthSq - powf(v0P*v02_norm_temp,2.0f))) * v02_length_temp;
                    //=== ===
                    
                    
                    //=== 12 ===
                    v12_norm_temp=*v2;
                    v12_norm_temp-=*v1;
                    
                    const float v12_length_temp=v12_norm_temp.normalise_rt();
                    const float A12=0.5f * (sqrtf(v1P.lengthSq() - powf(v1P*v12_norm_temp,2.0f))) * v12_length_temp;
                    //=== ===
                    
                    
                    //===
                    const float A=stitch::Vec3::crossLength(*v0, *v1, *v2)*0.5f;
                    
                    if ((A01+A12+A02)<=(A*1.00001f))
                    {
                        intersect.normal_.setToSumScaleAndNormalise(*n0, A12, *n1, A02, *n2, A01);
                        break;
                    }
                    //===
                    
                    
                    //v02 will become v01 in next iteration. Reuse calculations.
                    v1=v2;
                    n1=n2;
                    A01=A02;
                }
                // === ===
                
            }
        }
    }
}



#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::BrushModel::constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key) const
{
    return ballTree_->constructOSGNode(createOSGLineGeometry, createOSGNormalGeometry, wireframe, key);
}
#endif// USE_OSG

//=======================================================================//
void stitch::BrushModel::calcIntersection(const Ray &ray, Intersection &intersect) const
{
    ballTree_->calcIntersection(ray, intersect);
}


