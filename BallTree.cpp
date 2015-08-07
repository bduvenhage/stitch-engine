/*
 * $Id: BallTree.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  BallTree.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/02/22.
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

#include "BallTree.h"
#include "Math/Plane.h"

#include <iostream>

stitch::BallTree::BallTree() :
BoundingVolume()
{
}

stitch::BallTree::BallTree(const BallTree &lValue) :
BoundingVolume(lValue)
{
    std::vector<stitch::BoundingVolume *>::const_iterator itemIter=lValue.itemVector_.begin();
    for (; itemIter!=lValue.itemVector_.end(); ++itemIter)
    {
        itemVector_.push_back( (*itemIter)->clone() );
    }
    
    std::vector<stitch::BallTree *>::const_iterator ballTreeIter=lValue.ballTreeVector_.begin();
    for (; ballTreeIter!=lValue.ballTreeVector_.end(); ++ballTreeIter)
    {
        ballTreeVector_.push_back(new BallTree(*(*ballTreeIter)));
    }
}

stitch::BallTree::~BallTree()
{
    clear();
}

size_t stitch::BallTree::getNumItems() const
{
    size_t numItems=itemVector_.size();
    
    std::vector<stitch::BallTree *>::const_iterator ballTreeIter=ballTreeVector_.begin();
    for (; ballTreeIter!=ballTreeVector_.end(); ++ballTreeIter)
    {
        numItems+=(*ballTreeIter)->getNumItems();
    }
    
    return numItems;
}

void stitch::BallTree::clear()
{
    centre_=Vec3(0.0f, 0.0f, 0.0f);
    radiusBV_=((float)FLT_MAX);
    
    std::vector<stitch::BoundingVolume *>::const_iterator itemIter=itemVector_.begin();
    for (; itemIter!=itemVector_.end(); ++itemIter)
    {
        delete (*itemIter);
    }
    
    itemVector_.clear();
    
    
    std::vector<stitch::BallTree *>::const_iterator ballTreeIter=ballTreeVector_.begin();
    for (; ballTreeIter!=ballTreeVector_.end(); ++ballTreeIter)
    {
        (*ballTreeIter)->clear();
        delete (*ballTreeIter);
    }
    
    ballTreeVector_.clear();
}

void stitch::BallTree::linearise()
{//Collect all items into a linear list and delete the tree structure...
    std::cout << "BallTree::linearise not implemented yet!\n";
    std::cout.flush();
    exit(-1);
}

void stitch::BallTree::build(const size_t chunkSize, const uint8_t splitAxis)
{
    //New potential child trees.
    BallTree *tree0=new BallTree;
    BallTree *tree1=new BallTree;
    
    
    //=== Find centre of items ===
    Vec3 c;//Initially set to zero by vector implementation.
    float dx=0.0f, dy=0.0f, dz=0.0f;
    
    if (itemVector_.size()>0)
    {
        {
            std::vector<stitch::BoundingVolume *>::const_iterator constItemIter=itemVector_.begin();
            for (; constItemIter!=itemVector_.end(); ++constItemIter)
            {//Do a linear search through the items.
                c+=(*constItemIter)->centre_;
            }
        }
        c*=1.0f/itemVector_.size();
        
        float minx, maxx;
        float miny, maxy;
        float minz, maxz;
        
        minx=maxx=c.x();
        miny=maxy=c.y();
        minz=maxz=c.z();
        
        {
            std::vector<stitch::BoundingVolume *>::const_iterator constItemIter=itemVector_.begin();
            for (; constItemIter!=itemVector_.end(); ++constItemIter)
            {//Do a linear search through the items.
                if ((*constItemIter)->centre_.x()<minx)
                {
                    minx=(*constItemIter)->centre_.x();
                } else
                    if ((*constItemIter)->centre_.x()>maxx)
                    {
                        maxx=(*constItemIter)->centre_.x();
                    }
                if ((*constItemIter)->centre_.y()<miny)
                {
                    miny=(*constItemIter)->centre_.y();
                } else
                    if ((*constItemIter)->centre_.y()>maxy)
                    {
                        maxy=(*constItemIter)->centre_.y();
                    }
                if ((*constItemIter)->centre_.z()<minz)
                {
                    minz=(*constItemIter)->centre_.z();
                } else
                    if ((*constItemIter)->centre_.z()>maxz)
                    {
                        maxz=(*constItemIter)->centre_.z();
                    }
            }
        }
        
        dx=maxx-minx;
        dy=maxy-miny;
        dz=maxz-minz;
    }
    //==============================
    
    Plane binarySpacePartition(Vec3(0.0f, 0.0f, 0.0f), 0.0f);
    
    
    if ((dx>=dy)&&(dx>=dz))
    {
        binarySpacePartition=Plane(Vec3(1.0f, 0.0f, 0.0f), c.x());//right-hand space.
    } else
        if (dy>=dz)
        {
            binarySpacePartition=Plane(Vec3(0.0f, 1.0f, 0.0f), c.y());//right-hand space.
        } else
        {
            binarySpacePartition=Plane(Vec3(0.0f, 0.0f, 1.0f), c.z());//right-hand space.
        }
    
    //OR
    /*
     if (splitAxis==0)
     {
     binarySpacePartition=Plane(Vec3(1.0f, 0.0f, 0.0f), c.x());//right-hand space.
     } else
     if (splitAxis==1)
     {
     binarySpacePartition=Plane(Vec3(0.0f, 1.0f, 0.0f), c.y());//right-hand space.
     } else
     {
     binarySpacePartition=Plane(Vec3(0.0f, 0.0f, 1.0f), c.z());//right-hand space.
     }
    */
    
    //=== Split items into two brances according to the binary space partition plane.
    {
        std::vector<BoundingVolume *>::const_iterator itemIter=itemVector_.begin();
        size_t count=0;
        for (; itemIter!=itemVector_.end(); ++itemIter)
        {
            BoundingVolume *item=(*itemIter);
            
            if (item->centre_*binarySpacePartition.normal_ < binarySpacePartition.d_)
            {//Item is in space of tree0.
                tree0->addItem(item);
            }
            else if (item->centre_*binarySpacePartition.normal_ > binarySpacePartition.d_)
            {//item is in space of tree1.
                tree1->addItem(item);
            } else
            {//Place item in one of the two child trees.
                if (count&1)
                {
                    tree0->addItem(item);
                } else
                {
                    tree1->addItem(item);
                }
            }
            
            ++count;
        }
        
        //itemVector_.clear();//Does not delete the items pointed to by the vector entries.
        std::vector<stitch::BoundingVolume *>().swap(itemVector_);//swap with new empty vector.
    }
    //===
    
    
    //=== Add child trees to this parent and build the child hierarchy ===
    {
        if (tree0->itemVector_.size()>0)
        {
            ballTreeVector_.push_back(tree0);//Add child tree. There can be more than two child trees if the build mehod is called multiple times.
        }
        if (tree0->itemVector_.size()>chunkSize)
        {
            tree0->build(chunkSize, (splitAxis+1)%3);//Recursively build the tree.
        }
        
        if (tree1->itemVector_.size()>0)
        {
            ballTreeVector_.push_back(tree1);//Add child tree. There can be more than two child trees if the build mehod is called multiple times.
        }
        if (tree1->itemVector_.size()>chunkSize)
        {
            tree1->build(chunkSize, (splitAxis+1)%3);//Recursively build the tree.
        }
    }
    //====================================================================
}


void stitch::BallTree::updateBV()
{//! @todo Should be done in one traversal of the tree!
    std::vector<stitch::BoundingVolume *>::const_iterator constItemIter=itemVector_.begin();
    centre_.setZeros();
    radiusBV_=0.0f;
    
    size_t numBoundingSpheres=0;
    
    //=== Traversal 1: Find boundingSphereCentre_ ===//
    std::vector<stitch::BallTree *>::const_iterator constBallTreeIter=ballTreeVector_.begin();
    for (; constBallTreeIter!=ballTreeVector_.end(); ++constBallTreeIter)
    {//Do a linear search through the ballTrees.
        stitch::BallTree *ballTree=*constBallTreeIter;
        ballTree->updateBV();
        centre_+=ballTree->centre_;
        ++numBoundingSpheres;
    }
    
    for (; constItemIter!=itemVector_.end(); ++constItemIter)
    {//Do a linear search through the items.
        stitch::BoundingVolume *item=*constItemIter;
        centre_+=item->centre_;
        ++numBoundingSpheres;
    }
    
    centre_*=1.0f/numBoundingSpheres;
    //=================================//
    
    
    //=== Traversal 2: Find boundingSphereRadius_ ===//
    constBallTreeIter=ballTreeVector_.begin();
    for (; constBallTreeIter!=ballTreeVector_.end(); ++constBallTreeIter)
    {//Do a linear search through the ballTrees.
        stitch::BallTree *ballTree=*constBallTreeIter;
        float r=Vec3::calcDistToPoint(ballTree->centre_, centre_)+ballTree->radiusBV_;
        
        if (r>radiusBV_)
        {
            radiusBV_=r;
        }
    }
    
    constItemIter=itemVector_.begin();
    for (; constItemIter!=itemVector_.end(); ++constItemIter)
    {//Do a linear search through the items.
        stitch::BoundingVolume *item=*constItemIter;
        float r=Vec3::calcDistToPoint(item->centre_, centre_)+item->radiusBV_;
        
        if (r>radiusBV_)
        {
            radiusBV_=r;
        }
    }
    //==================================//
    
}

void stitch::BallTree::calcIntersection(const Ray &ray, Intersection &intersect) const
{
    //if (BVIntersected(orig, normDir)) This is currently executed by the calling code!
    {
        //=== 1) Find closest ray-item intersection. ===
        for (const auto itemPtr : itemVector_)
        {//Do a linear search through the items stored in this tree node.
            if (itemPtr->BVIntersected(ray))
            {
                itemPtr->calcIntersection(ray, intersect);
            }
        }
        //============================================
        
        //=== 2) Continue closest ray-item intersection to the items stored in the tree children. ===
        for (const auto balltreePtr : ballTreeVector_)
        {//Do a linear search through the child trees.
            if (balltreePtr->BVIntersected(ray))
            {
                balltreePtr->calcIntersection(ray, intersect);
            }
        }
        //============================================
    }
}


#ifdef USE_OSG
osg::ref_ptr<osg::Node> stitch::BallTree::constructOSGNode(const bool createOSGLineGeometry, const bool createOSGNormalGeometry, const bool wireframe, const uintptr_t key) const
{
    osg::ref_ptr<osg::Group> osgGroup=new osg::Group();
    
    for (auto item : itemVector_)
    {
        osgGroup->addChild(item->constructOSGNode(createOSGLineGeometry, createOSGNormalGeometry, wireframe, key));
    }
    
    for (auto ballTree : ballTreeVector_)
    {
        osgGroup->addChild(ballTree->constructOSGNode(createOSGLineGeometry, createOSGNormalGeometry, wireframe, key));
    }
    
    return osgGroup;
}


osg::ref_ptr<osg::Node> stitch::BallTree::constructOSGBVNode() const
{
    osg::ref_ptr<osg::Group> osgGroup=new osg::Group();
    
    std::vector<stitch::BoundingVolume *>::const_iterator itemIter=itemVector_.begin();
    for (; itemIter!=itemVector_.end(); ++itemIter)
    {
        osgGroup->addChild(constructOSGNode_Sphere(VecN((*itemIter)->centre_), (*itemIter)->radiusBV_,
                                                   true,
                                                   uintptr_t (*itemIter)));
    }
    
    std::vector<stitch::BallTree *>::const_iterator ballTreeIter=ballTreeVector_.begin();
    for (; ballTreeIter!=ballTreeVector_.end(); ++ballTreeIter)
    {
        osgGroup->addChild(constructOSGNode_Sphere(VecN((*ballTreeIter)->centre_), (*ballTreeIter)->radiusBV_,
                                                   true));
        
        osgGroup->addChild((*ballTreeIter)->constructOSGBVNode());
    }
    
    return osgGroup;
}
#endif// USE_OSG

