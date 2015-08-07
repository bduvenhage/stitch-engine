/*
 * $Id: KDTree.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  KDTree.cpp
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

#include "KDTree.h"

//=======================================================================//
stitch::KDTree::KDTree() :
binarySpacePartition_(Vec3(0.0f, 0.0f, 0.0f), 0.0f),
left_(nullptr),
right_(nullptr),
totalItems_(0)
{
}


//=======================================================================//
void stitch::KDTree::clear()
{
    binarySpacePartition_=Plane(Vec3(0.0f, 0.0f, 0.0f), 0.0f);
    
    std::vector<stitch::BoundingVolume *>::const_iterator itemIter=itemVector_.begin();
    const std::vector<stitch::BoundingVolume *>::const_iterator itemIterEnd=itemVector_.end();
    for (; itemIter!=itemIterEnd; ++itemIter)
    {
        delete (*itemIter);
    }
    
    itemVector_.clear();
    
    if (left_)
    {
        delete left_;
        left_=nullptr;
    }
    
    if (right_)
    {
        delete right_;
        right_=nullptr;
    }
    
    totalItems_=0;
}

//=======================================================================//
void stitch::KDTree::build(const size_t chunkSize,
                           const uint8_t splitAxis, const size_t maxDepth,
                           std::vector<stitch::Vec3> const &splitAxisVec)
{
    const size_t itemVectorSize=itemVector_.size();
    
    if ((itemVectorSize>chunkSize)&&(maxDepth>0))
    {
        if (!left_)
        {//No tree below this node yet. Let's add.
            left_=new KDTree;
            right_=new KDTree;
            
            Vec3 centre;//Set to zero.
            for (size_t i=0; i<itemVectorSize; ++i)
            {
                centre+=itemVector_[i]->centre_;
            }
            centre*=1.0f/itemVectorSize;
            
            
            float splitPlaneD=splitAxisVec[splitAxis]*centre;
            left_->binarySpacePartition_=Plane(splitAxisVec[splitAxis]*(-1.0f), -splitPlaneD);//left-hand space.
            right_->binarySpacePartition_=Plane(splitAxisVec[splitAxis], splitPlaneD);//right-hand space.
        }
        
        //=== Split items into two brances according to the binary space partition planes.
        {
            for (size_t i=0; i<itemVectorSize; ++i)
            {
                BoundingVolume *item=itemVector_[i];
                
                if (item->centre_*left_->binarySpacePartition_.normal_ > left_->binarySpacePartition_.d_)
                {//Item is in space of tree0.
                    left_->addItem(item);
                } else
                    if (item->centre_*left_->binarySpacePartition_.normal_ < left_->binarySpacePartition_.d_)
                    {//Item is in space of tree1.
                        right_->addItem(item);
                    }
                    else
                    {//Anything goes - choose one of the two sides.
                        if ((i%2)==0)
                        {
                            left_->addItem(item);
                        } else
                        {
                            right_->addItem(item);
                        }
                    }
            }
            
            //itemVector_.clear();
            std::vector<BoundingVolume *>().swap(itemVector_);//swap with new empty vector.
        }
        //===
        
        
        //=== Build the child hierarchy ===
        {
            left_->build(chunkSize, (splitAxis+1)%splitAxisVec.size(), maxDepth-1, splitAxisVec);//Recursively build the tree.
            right_->build(chunkSize, (splitAxis+1)%splitAxisVec.size(), maxDepth-1, splitAxisVec);//Recursively build the tree.
        }
        //===
    }
}


//=======================================================================//
stitch::BoundingVolume *stitch::KDTree::getNearest(const Vec3 &centre, float &searchRadiusSq) const
{
    BoundingVolume *closestItem=nullptr;
    
    //Find nearest item (within searchRadiusSq) in local item vector.
    const size_t numItems=itemVector_.size();
    
    for (size_t itemNum=0; itemNum<numItems; ++itemNum)
    {
        BoundingVolume *item=itemVector_[itemNum];
        
        //const float itemDistSq=Vec3::calcDistToPointSq(item->centre_, centre);
        const float dx=(centre.v_[0]-item->centre_.v_[0]);
        float itemDistSq=dx*dx;
        
        if (itemDistSq < searchRadiusSq)
        {
            const float dy=(centre.v_[1]-item->centre_.v_[1]);
            itemDistSq+=dy*dy;
            
            if (itemDistSq < searchRadiusSq)
            {
                const float dz=(centre.v_[2]-item->centre_.v_[2]);
                itemDistSq+=dz*dz;
                
                if (itemDistSq < searchRadiusSq)
                {
                    closestItem=item;
                    searchRadiusSq=itemDistSq;
                }
            }
        }
    }
    
    if (left_)
    {
        const float centreDistFromPlaneLeft=centre*left_->binarySpacePartition_.normal_ - left_->binarySpacePartition_.d_;
        
        if (centreDistFromPlaneLeft>0.0f)
        {
            BoundingVolume *closestItemFromTree=left_->getNearest(centre, searchRadiusSq);
            if (closestItemFromTree) closestItem=closestItemFromTree;
            
            if (centreDistFromPlaneLeft<=sqrtf(searchRadiusSq))
            {
                BoundingVolume *closestItemFromTree=right_->getNearest(centre, searchRadiusSq);
                if (closestItemFromTree) closestItem=closestItemFromTree;
            }
        } else
        {
            BoundingVolume *closestItemFromTree=right_->getNearest(centre, searchRadiusSq);
            if (closestItemFromTree) closestItem=closestItemFromTree;
            
            if (centreDistFromPlaneLeft>=-sqrtf(searchRadiusSq))
            {
                BoundingVolume *closestItemFromTree=left_->getNearest(centre, searchRadiusSq);
                if (closestItemFromTree) closestItem=closestItemFromTree;
            }
        }
    }
    
    return closestItem;
}

//=======================================================================//
//NB: Note hat this is a single threaded recursive kNN query. Single threaded due to shared kNearestItems.
void stitch::KDTree::getNearestK(KNearestItems * const kNearestItems) const
{
    //Find items within centre+radius from itemVector.
    const size_t numItems=itemVector_.size();
    
    for (size_t itemNum=0; itemNum<numItems; ++itemNum)
    {
        BoundingVolume * const item=itemVector_[itemNum];
        
        kNearestItems->insert(item);
    }
    
    
    if ((left_)&&(left_->totalItems_ || right_->totalItems_))
    {
        const float centreDistFromPlaneLeft=kNearestItems->centre_*left_->binarySpacePartition_.normal_ - left_->binarySpacePartition_.d_;
        
        if (centreDistFromPlaneLeft>0.0f)
        {
            if (left_->totalItems_) left_->getNearestK(kNearestItems);
            
            if (right_->totalItems_)
            {
                if (centreDistFromPlaneLeft<=sqrtf(kNearestItems->searchRadiusSq_))
                {
                    right_->getNearestK(kNearestItems);
                }
            }
        } else
        {
            if (right_->totalItems_) right_->getNearestK(kNearestItems);
            
            if (left_->totalItems_)
            {
                if (centreDistFromPlaneLeft>=-sqrtf(kNearestItems->searchRadiusSq_))
                {
                    left_->getNearestK(kNearestItems);
                }
            }
        }
    }
}


//=======================================================================//
//NB: Note hat this is a single threaded recursive kNN query. Single threaded due to shared kNearestItems.
#ifdef _LIBCPP_VERSION
void stitch::KDTree::getNearestKPerGroupID(KNearestItems const * const kNearestItemsRef, std::unordered_map<uint32_t, KNearestItems * const> *kNearestItemsMap) const
#else
void stitch::KDTree::getNearestKPerGroupID(KNearestItems const * const kNearestItemsRef, std::tr1::unordered_map<uint32_t, KNearestItems * const> *kNearestItemsMap) const
#endif// USE_CXX11
{
    //Find items within centre+radius from itemVector.
    const size_t numItems=itemVector_.size();
    
    for (size_t itemNum=0; itemNum<numItems; ++itemNum)
    {
        BoundingVolume * const item=itemVector_[itemNum];
        
        const float itemDistSq=Vec3::calcDistToPointSq(item->centre_, kNearestItemsRef->centre_);
        
        if (itemDistSq < kNearestItemsRef->searchRadiusSq_)
        {
#ifdef _LIBCPP_VERSION
            std::unordered_map<uint32_t, KNearestItems * const >::iterator mapIter=kNearestItemsMap->find(item->userGroupID_);
#else
            std::tr1::unordered_map<uint32_t, KNearestItems * const >::iterator mapIter=kNearestItemsMap->find(item->userGroupID_);
#endif// _LIBCPP_VERSION
            
            if (mapIter==kNearestItemsMap->end())
            {//item->userGroupID_ not in map yet.
                KNearestItems *kNearestItems=new KNearestItems(kNearestItemsRef->centre_, kNearestItemsRef->searchRadiusSq_, kNearestItemsRef->k_);
                kNearestItemsMap->insert(std::pair<uint32_t, KNearestItems * const >(item->userGroupID_, kNearestItems));
                
                kNearestItems->insert(item);
            } else
            {
                KNearestItems * const kNearestItems=mapIter->second;
                
                kNearestItems->insert(item);
            }
        }
    }
    
    
    if ((left_)&&(left_->totalItems_ || right_->totalItems_))
    {
        const float centreDistFromPlaneLeft=stitch::Vec3::dot(kNearestItemsRef->centre_,left_->binarySpacePartition_.normal_) - left_->binarySpacePartition_.d_;
        
        if (centreDistFromPlaneLeft>0.0f)
        {
            if (left_->totalItems_) left_->getNearestKPerGroupID(kNearestItemsRef, kNearestItemsMap);
            
            if (right_->totalItems_)
            {
                if (centreDistFromPlaneLeft<=sqrtf(kNearestItemsRef->searchRadiusSq_))
                {
                    right_->getNearestKPerGroupID(kNearestItemsRef, kNearestItemsMap);
                }
            }
        } else
        {
            if (right_->totalItems_) right_->getNearestKPerGroupID(kNearestItemsRef, kNearestItemsMap);
            
            if (left_->totalItems_)
            {
                if (centreDistFromPlaneLeft>=-sqrtf(kNearestItemsRef->searchRadiusSq_))
                {
                    left_->getNearestKPerGroupID(kNearestItemsRef, kNearestItemsMap);
                }
            }
        }
    }
}
