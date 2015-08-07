/*
 * $Id: BeamTree.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  BeamTree.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
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

#include "BeamTree.h"
#include "Materials/DiffuseMaterial.h"


//=======================================================================//
stitch::BeamTree::BeamTree() :
BVBrush_(new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f))),
BVUpdated_(false)
{}

//=======================================================================//
stitch::BeamTree::BeamTree(const BeamTree &lValue) :
BVBrush_(lValue.BVBrush_),
BVUpdated_(lValue.BVUpdated_)
{
    std::vector<stitch::BeamSegment *>::const_iterator beamSegmentIter=lValue.beamSegmentVector_.begin();
    for (; beamSegmentIter!=lValue.beamSegmentVector_.end(); ++beamSegmentIter)
    {
        beamSegmentVector_.push_back(new BeamSegment(*(*beamSegmentIter)));
    }
    
    std::vector<stitch::BeamTree *>::const_iterator beamTreeIter=lValue.beamTreeVector_.begin();
    for (; beamTreeIter!=lValue.beamTreeVector_.end(); ++beamTreeIter)
    {
        beamTreeVector_.push_back(new BeamTree(*(*beamTreeIter)));
    }
}

//=======================================================================//
stitch::BeamTree & stitch::BeamTree::operator=( const BeamTree &lValue)
{
    if (&lValue!=this)
    {
        BVBrush_=lValue.BVBrush_;
        BVUpdated_=lValue.BVUpdated_;
        
        std::vector<stitch::BeamSegment *>::const_iterator beamSegmentIter=lValue.beamSegmentVector_.begin();
        for (; beamSegmentIter!=lValue.beamSegmentVector_.end(); ++beamSegmentIter)
        {
            beamSegmentVector_.push_back(new BeamSegment(*(*beamSegmentIter)));
        }
        
        std::vector<stitch::BeamTree *>::const_iterator beamTreeIter=lValue.beamTreeVector_.begin();
        for (; beamTreeIter!=lValue.beamTreeVector_.end(); ++beamTreeIter)
        {
            beamTreeVector_.push_back(new BeamTree(*(*beamTreeIter)));
        }
    }
    
    return (*this);
}

//=======================================================================//
stitch::BeamTree::~BeamTree()
{
    clear();
}

//=======================================================================//
void stitch::BeamTree::addBeamSegment(BeamSegment * const beamSegment)
{
    beamSegmentVector_.push_back(beamSegment);
    BVUpdated_=false;
}

//=======================================================================//
size_t stitch::BeamTree::getNumBeamSegments() const
{
    size_t numBeamSegments=beamSegmentVector_.size();
    
    std::vector<stitch::BeamTree *>::const_iterator beamTreeIter=beamTreeVector_.begin();
    for (; beamTreeIter!=beamTreeVector_.end(); ++beamTreeIter)
    {
        numBeamSegments+=(*beamTreeIter)->getNumBeamSegments();
    }
    
    return numBeamSegments;
}


//=======================================================================//
void stitch::BeamTree::clear()
{
    BVBrush_=stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f)));
    BVUpdated_=false;
    
    std::vector<stitch::BeamSegment *>::const_iterator beamSegmentIter=beamSegmentVector_.begin();
    for (; beamSegmentIter!=beamSegmentVector_.end(); ++beamSegmentIter)
    {
        delete (*beamSegmentIter);
    }
    
    beamSegmentVector_.clear();
    
    std::vector<stitch::BeamTree *>::const_iterator beamTreeIter=beamTreeVector_.begin();
    for (; beamTreeIter!=beamTreeVector_.end(); ++beamTreeIter)
    {
        (*beamTreeIter)->clear();
        delete (*beamTreeIter);
    }
    
    beamTreeVector_.clear();
}


//=======================================================================//
void stitch::BeamTree::build(const size_t chunkSize, const uint8_t splitAxis)
{//Only makes use of cone origins to build tree. It is assumed that cones that originate next to each other points in similar directions!
    
    //New potential child trees.
    BeamTree *tree0=new BeamTree;
    BeamTree *tree1=new BeamTree;
    
    //Build the beamTree based on the end-cap centres instead of the generator centres.
    
    //=== Find centre of beam end-cap OR BV ===
    Vec3 centre;//Set to zero.
    
    if (beamSegmentVector_.size()>0)
    {
        std::vector<stitch::BeamSegment *>::const_iterator constBeamSegmentIter=beamSegmentVector_.begin();
        for (; constBeamSegmentIter!=beamSegmentVector_.end(); ++constBeamSegmentIter)
        {//Do a linear traversal through the beam segments.
            (*constBeamSegmentIter)->updateBV();
            
            if ((*constBeamSegmentIter)->hasEndCap_)
            {
                centre+=(*constBeamSegmentIter)->endCapVertices_[0];
            } else
            {
                centre+=(*constBeamSegmentIter)->getBVBrush().centre_;
            }
            
        }
        centre/=beamSegmentVector_.size();
    }
    //================================
    
    Plane binarySpacePartition(Vec3(0.0f, 0.0f, 0.0f), 0.0f);
    
    if (splitAxis==0)
    {
        binarySpacePartition=Plane(Vec3(1.0f, 0.0f, 0.0f), centre.x());//right-hand space.
    } else
        if (splitAxis==1)
        {
            binarySpacePartition=Plane(Vec3(0.0f, 1.0f, 0.0f), centre.y());//right-hand space.
        } else
            if (splitAxis==2)
            {
                binarySpacePartition=Plane(Vec3(0.0f, 0.0f, 1.0f), centre.z());//right-hand space.
            }
    
    //=== Split beam segments into two brances according to their beam orig and the binary space partition plane.
    {
        std::vector<BeamSegment *>::const_iterator beamSegmentIter=beamSegmentVector_.begin();
        for (; beamSegmentIter!=beamSegmentVector_.end(); ++beamSegmentIter)
        {
            BeamSegment *beamSegment=(*beamSegmentIter);
            
            stitch::Vec3 refVert;
            
            if (beamSegment->hasEndCap_)
            {
                refVert=beamSegment->endCapVertices_[0];
            } else
            {
                refVert=beamSegment->getBVBrush().centre_;
            }
            
            
            if (refVert*binarySpacePartition.normal_ < binarySpacePartition.d_)
            {//Object is in space of tree0.
                tree0->addBeamSegment(beamSegment);
            }
            else
            {//Object is in space of tree1.
                tree1->addBeamSegment(beamSegment);
            }
        }
        
        //beamSegmentVector_.clear();//Does not delete the beam segments pointed to by the vector entries.
        std::vector<stitch::BeamSegment *>().swap(beamSegmentVector_);//swap with new empty vector.
    }
    //===
    
    
    //=== Add child trees to this parent and build the child hierarchy ===
    {
        if (tree0->beamSegmentVector_.size()>0)
        {
            beamTreeVector_.push_back(tree0);//Add child tree. There can be more than two child trees if the build mehod is called multiple times.
        }
        if (tree0->beamSegmentVector_.size()>chunkSize)
        {
            tree0->build(chunkSize, (splitAxis+1)%3);//Recursively build the tree.
        }
        
        if (tree1->beamSegmentVector_.size()>0)
        {
            beamTreeVector_.push_back(tree1);//Add child tree. There can be more than two child trees if the build mehod is called multiple times.
        }
        if (tree1->beamSegmentVector_.size()>chunkSize)
        {
            tree1->build(chunkSize, (splitAxis+1)%3);//Recursively build the tree.
        }
    }
    //====================================================================
}


//=======================================================================//
void stitch::BeamTree::updateBV()
{
    if (!BVUpdated_)
    {
        BVBrush_=stitch::Brush(new stitch::DiffuseMaterial(stitch::Colour_t(1.0f, 1.0f, 1.0f)));
        
        std::vector<stitch::BeamTree *>::const_iterator beamTreeIter=beamTreeVector_.begin();
        std::vector<stitch::BeamSegment *>::const_iterator beamSegmentIter=beamSegmentVector_.begin();
        
        beamTreeIter=beamTreeVector_.begin();
        for (beamTreeIter=beamTreeVector_.begin(); beamTreeIter!=beamTreeVector_.end(); ++beamTreeIter)
        {
            stitch::BeamTree *beamTree=*beamTreeIter;
            beamTree->updateBV();
            
            BVBrush_=stitch::Brush::mergeBrush(BVBrush_, beamTree->BVBrush_, true);
        }
        
        std::vector<stitch::Vec3> leavesBoundingVolumeVertexCloud;
        
        beamSegmentIter=beamSegmentVector_.begin();
        for (beamSegmentIter=beamSegmentVector_.begin(); beamSegmentIter!=beamSegmentVector_.end(); ++beamSegmentIter)
        {
            stitch::BeamSegment *beamSegment=*beamSegmentIter;
            beamSegment->updateBV();
            
            const std::vector<stitch::Vec3> beamSegmentBoundingVolumeVertexCloud = beamSegment->getBVBrush().getVertexCloud();
            
            leavesBoundingVolumeVertexCloud.insert(leavesBoundingVolumeVertexCloud.end(), beamSegmentBoundingVolumeVertexCloud.begin(), beamSegmentBoundingVolumeVertexCloud.end());
        }
        
        stitch::Brush leavesBoundingVolumeBrush(BVBrush_.pMaterial_->clone(), leavesBoundingVolumeVertexCloud, 20);
        BVBrush_=stitch::Brush::mergeBrush(BVBrush_, leavesBoundingVolumeBrush, true);
        
        BVUpdated_=true;
        
        BVBrush_.updateLinesVerticesAndBoundingVolume(true);
        BVBrush_.optimiseFaceOrder();
    }
}


#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::BeamTree::constructOSGNode(const uintptr_t key) const
{
    osg::ref_ptr<osg::Group> osgGroup=new osg::Group();
    
    osgGroup->addChild(BVBrush_.constructOSGNode(false, false, true, key));
    
    std::vector<stitch::BeamTree *>::const_iterator beamTreeIter=beamTreeVector_.begin();
    std::vector<stitch::BeamSegment *>::const_iterator beamSegmentIter=beamSegmentVector_.begin();
    
    
    beamTreeIter=beamTreeVector_.begin();
    for (beamTreeIter=beamTreeVector_.begin(); beamTreeIter!=beamTreeVector_.end(); beamTreeIter++)
    {
        stitch::BeamTree *beamTree=*beamTreeIter;
        osgGroup->addChild(beamTree->constructOSGNode(key));
    }
    
    
    
    beamSegmentIter=beamSegmentVector_.begin();
    for (beamSegmentIter=beamSegmentVector_.begin(); beamSegmentIter!=beamSegmentVector_.end(); ++beamSegmentIter)
    {
        stitch::BeamSegment *beamSegment=*beamSegmentIter;
        osgGroup->addChild(beamSegment->getBVBrush().constructOSGNode(false, false, true, key));
    }
    
    
    return osgGroup;
}
#endif// USE_OSG


